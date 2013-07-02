/* difference-mode for harmcorr analysis */
/* 26.06.2012 - J. Schmidt */

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "types.hpp"
#include "constants.hpp"
#include "ELSAimport.hpp"
#include "madximport.hpp"
#include "getspectrum.hpp"
#include "fieldmap.hpp"
#include "spectrum.hpp"
#include "orbit.hpp"

using namespace std;

// read and subtract reference orbit & corrector data
int difference(const char *ReferenceFolder, unsigned int t, CLOSEDORBIT &bpmorbit, magnetvec &vcorrs, BPM *Ref_ELSAbpms, CORR *Ref_ELSAvcorrs, bool elsa)
{

  unsigned int i = 0;
  int err;
  CLOSEDORBIT Ref_bpmorbit;
  magnetvec Ref_vcorrs;

  //read reference
  if (elsa) {
    Ref_vcorrs  = vcorrs; // take names,positions,lengths from vcorrs (pos will be checked again in getvcorrs()
    ELSAimport_getbpmorbit(Ref_ELSAbpms, Ref_bpmorbit, t);
    ELSAimport_getvcorrs(Ref_ELSAvcorrs, Ref_vcorrs, t);
    cout << "* "<<t<<" ms: ";
  }
  else {
    magnetvec tmpDip, tmpQuad, tmpSext;
    madximport(ReferenceFolder, Ref_bpmorbit, tmpDip, tmpQuad, tmpSext, Ref_vcorrs);
    cout << "* ";
  }
  cout <<Ref_vcorrs.size()<<" correctors and "
       <<Ref_bpmorbit.bpms()<<" BPMs read"<<endl<<"  from "<< ReferenceFolder << endl;

  //subtract orbit
  err = bpmorbit.diff(Ref_bpmorbit);
  if (err < 0) return 1;
  else if (err > 0) {
    cout << "ERROR: difference.cpp: Unequal positions of "<<i<<". BPM for subtraction."<< endl;
    return 1;
    }

  //subtract corrector strengths
  if (vcorrs.size() != Ref_vcorrs.size()) {
    cout << "ERROR: difference.cpp: Unequal number of VCs to subtract."<< endl;
    return 1;
  }
  for (i=0; i<vcorrs.size(); i++) {
    if (vcorrs[i].start==Ref_vcorrs[i].start) {
      vcorrs[i].strength -= Ref_vcorrs[i].strength;
    }
    else {
      cout << "ERROR: difference.cpp: Unequal positions of "<<i+1<<". VC for subtraction."<< endl;
      return 1;
    }
  }

  return 0;
}


//output file with harmcorr data
int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, const char *filename)
{
  unsigned int i=0;
  int w=14;
  fstream file;
  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: harmcorr_out: Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "dipol interval" <<setw(w)<< "corrs [mrad]" <<setw(w)<< "quads [mrad]" <<setw(w)<< "sum [mrad]" << endl;

  file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
  for (i=0; i<nd; i++) {
    file <<setw(w)<< i <<setw(w)<< HCvcorr[2*i] <<setw(w)<< HCquad[2*i] <<setw(w)<< HCsum[2*i] << endl;
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}


//calculates difference-corrector data (->harmcorr) as a function of spin-phaseadvance
//and does fft for harmcorr-spectrum hc
int harmcorr(SPECTRUM &hc, magnetvec vcorrs, magnetvec quads, CLOSEDORBIT &orbit, magnetvec dipols, double circumference, const char *filename)
{
 unsigned int i=0,j=0,k=0;
 unsigned int nd = dipols.size();
 unsigned int nc = vcorrs.size();
 unsigned int nq = quads.size();
 double *HCvcorr = new double[2*nd](); //() -> initialize to zero
 double *HCquad = new double[2*nd]();
 double *HCsum = new double[2*nd]();
 double length;

 /*
 memset(HCvcorr, 0, 2*nd*sizeof(double));
 memset(HCquad, 0, 2*nd*sizeof(double));
 memset(HCsum, 0, 2*nd*sizeof(double));
 */

 for (i=0; i<nd; i++) {
     while(vcorrs[j].start < dipols[i].start && j < nc) {
       length = vcorrs[j].end - vcorrs[j].start;
       HCvcorr[2*i] += vcorrs[j].strength * length * 1000; // mrad
       j++;
     }
     while(quads[k].start < dipols[i].start && k < nq) {
       length = quads[k].end - quads[k].start;
       HCquad[2*i] += quads[k].strength * orbit.interp_z(quads[k].end) * length * 1000; // mrad
       k++;
     }
 }
 // first=last dipol-interval! add kicks of last interval to first.
 while (j<nc) {
   length = vcorrs[j].end - vcorrs[j].start;
   HCvcorr[0] += vcorrs[j].strength * length * 1000; // mrad
   j++;
 }
 while (k<nq) {
   length = quads[k].end - quads[k].start;
   HCquad[0] += quads[k].strength * orbit.interp_z(quads[k].end) * length * 1000; // mrad
   k++;
 }

 for (i=0; i<nd; i++) {
   HCsum[2*i] = HCvcorr[2*i]+HCquad[2*i];
 }

 harmcorr_out(HCvcorr, HCquad, HCsum, nd, filename);
 fft(hc, HCsum, nd, nd, circumference);

 delete[] HCvcorr;
 delete[] HCquad;
 delete[] HCsum;

 return 0;
}

