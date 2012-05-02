/* difference-mode for harmcorr analysis */
/* 16.04.2012 - J. Schmidt */

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

using namespace std;

// read and subtract reference orbit & corrector data
int difference(char *ReferenceFolder, unsigned int t, double corrlength, orbitvec &bpmorbit, magnetvec &vcorrs, bool elsa)
{

  unsigned int i;
  orbitvec Ref_bpmorbit;
  magnetvec Ref_vcorrs;

  //read reference
  if (elsa) {
    BPM Ref_ELSAbpms[NBPMS];
    CORR Ref_ELSAvcorrs[NVCORRS];
    ELSAimport(Ref_ELSAbpms, Ref_ELSAvcorrs, ReferenceFolder); 
    ELSAimport_getbpmorbit(Ref_ELSAbpms, Ref_bpmorbit, t);
    ELSAimport_getvcorrs(Ref_ELSAvcorrs, Ref_vcorrs, corrlength, t);
    cout << "* "<<t<<" ms: ";
  }
  else {
    magnetvec tmpDip, tmpQuad, tmpSext;
    madximport(ReferenceFolder, Ref_bpmorbit, tmpDip, tmpQuad, tmpSext, Ref_vcorrs);
    cout << "* ";
  }
  cout <<Ref_vcorrs.size()<<" correctors and "
       <<Ref_bpmorbit.size()<<" BPMs read"<<endl<<"  from "<< ReferenceFolder << endl;

  //subtract orbit
  if (bpmorbit.size() != Ref_bpmorbit.size()) {
    cout << "ERROR: difference.cpp: Unequal number of BPMs to subtract."<< endl;
    return 1;
  }
  for (i=0; i<bpmorbit.size(); i++) {
    if (bpmorbit[i].pos==Ref_bpmorbit[i].pos) {
      bpmorbit[i].x -= Ref_bpmorbit[i].x;
      bpmorbit[i].z -= Ref_bpmorbit[i].z;
    }
    else {
      cout << "ERROR: difference.cpp: Unequal positions of "<<i+1<<". BPM for subtraction."<< endl;
      return 1;
    }
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
int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, char *filename)
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
int harmcorr(SPECTRUM *hc, int fmax_x, magnetvec vcorrs, magnetvec quads, orbitvec orbit, magnetvec dipols, double circumference, int n_samp, char *filename)
{
 unsigned int i=0,j=0,k=0;
 unsigned int nd = dipols.size();
 unsigned int nc = vcorrs.size();
 unsigned int nq = quads.size();
 double *HCvcorr = new double[2*nd];
 double *HCquad = new double[2*nd];
 double *HCsum = new double[2*nd];
 double length;
 double sample = circumference/n_samp;

 memset(HCvcorr, 0, 2*nd*sizeof(double));
 memset(HCquad, 0, 2*nd*sizeof(double));
 memset(HCsum, 0, 2*nd*sizeof(double));

 for (i=0; i<nd; i++) {
     while(vcorrs[j].start < dipols[i].start && j < nc) {
       length = vcorrs[j].end - vcorrs[j].start;
       HCvcorr[2*i] += vcorrs[j].strength * length * 1000; // mrad
       j++;
     }
     while(quads[k].start < dipols[i].start && k < nq) {
       length = quads[k].end - quads[k].start;
       HCquad[2*i] += quads[k].strength * orbit[int(quads[k].start/sample+0.5)].z * length * 1000; // mrad
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
   HCquad[0] += quads[k].strength * orbit[int(quads[k].start/sample+0.5)].z * length * 1000; // mrad
   k++;
 }

 for (i=0; i<nd; i++) {
   HCsum[2*i] = HCvcorr[2*i]+HCquad[2*i];
 }

 harmcorr_out(HCvcorr, HCquad, HCsum, nd, filename);
 fft(hc, HCsum, nd, fmax_x, circumference);

 delete[] HCvcorr;
 delete[] HCquad;
 delete[] HCsum;

 return 0;
}

