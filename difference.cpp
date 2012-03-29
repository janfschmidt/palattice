/* difference-mode for harmcorr analysis */
/* 29.03.2012 - J. Schmidt */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "types.hpp"
#include "constants.hpp"
#include "ELSAimport.hpp"
#include "madximport.hpp"

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



//output file with difference-corrector data (->harmcorr) as a function of spin-phaseadvance
int harmcorr_out(magnetvec vcorrs, magnetvec quads, orbitvec orbit, magnetvec dipols, double sample, char *filename)
{
 unsigned int i=0,j=0,k=0;
 int w=14;
 unsigned int nd = dipols.size();
 unsigned int nc = vcorrs.size();
 unsigned int nq = quads.size();
 double vcorr_tmp=0, quad_tmp=0, vcorr0_tmp=0, quad0_tmp=0;
 double length;
 fstream file;
 
 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: harmcorr_out: Cannot open " << filename << "." << endl;
   return 1;
 }
 
 file <<setw(w)<< "dipol interval" <<setw(w)<< "corrs [mrad]" <<setw(w)<< "quads [mrad]" <<setw(w)<< "sum [mrad]" << endl;

 // first=last dipol-interval! save kicks of first interval.
 while(vcorrs[j].start < dipols[0].start && j < nc) {
   length = vcorrs[j].end - vcorrs[j].start;
   vcorr0_tmp += vcorrs[j].strength * length * 1000; // mrad
       j++;
 }
 while(quads[k].start < dipols[0].start && k < nq) {
   length = quads[k].end - quads[k].start;
   quad0_tmp += quads[k].strength * orbit[int(quads[k].start/sample+0.5)].z * length * 1000; // mrad
   k++;
 }

 for (i=1; i<=nd; i++) {
   if (i<nd) {
     while(vcorrs[j].start < dipols[i].start && j < nc) {
       length = vcorrs[j].end - vcorrs[j].start;
       vcorr_tmp += vcorrs[j].strength * length * 1000; // mrad
       j++;
     }
     while(quads[k].start < dipols[i].start && k < nq) {
       length = quads[k].end - quads[k].start;
       quad_tmp += quads[k].strength * orbit[int(quads[k].start/sample+0.5)].z * length * 1000; // mrad
       k++;
     }
   }
   else if (i==nd) {
     vcorr_tmp = vcorr0_tmp; //add kicks of first interval
     quad_tmp = quad0_tmp;
     while (j<nc) {
       length = vcorrs[j].end - vcorrs[j].start;
       vcorr_tmp += vcorrs[j].strength * length * 1000; // mrad
       j++;
     }
     while (k<nq) {
       length = quads[k].end - quads[k].start;
       quad_tmp += quads[k].strength * orbit[int(quads[k].start/sample+0.5)].z * length * 1000; // mrad
       k++;
     }
   }
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
   file <<setw(w)<< i <<setw(w)<< vcorr_tmp <<setw(w)<< quad_tmp <<setw(w)<< (vcorr_tmp+quad_tmp) << endl;
   vcorr_tmp = 0; quad_tmp = 0;
 }

 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
