/* difference-mode for harmcorr analysis */
/* 26.06.2012 - J. Schmidt */

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <exception>
#include "difference.hpp"

using namespace std;

// read and subtract reference orbit & corrector data
int difference(const char *ReferenceFile, unsigned int t, FunctionOfPos<AccPair> &bpmorbit, AccLattice &lattice, BPM *Ref_ELSAbpms, CORR *Ref_ELSAvcorrs, bool elsa)
{

  unsigned int i = 0, n;

  //read reference
  if (elsa) {
    Ref_lattice = lattice; // take names,positions,lengths (pos will be checked again in setELSACorrectors()
    Ref_bpmorbit.elsaClosedOrbit(Ref_ELSAbpms, t);
    n = Ref_lattice.setELSACorrectors(Ref_ELSAvcorrs, t);
    cout << "* "<<t<<" ms: ";
  }
  else {
    Ref_lattice.madximport(ReferenceFile);
    Ref_lattice.madximportMisalignments(ReferenceFile);
    n = Ref_lattice.size(corrector);
    cout << "* ";
  }
  cout <<n<<" correctors and "
       <<Ref_bpmorbit.samples()<<" BPMs read"<<endl<<"  from "<< ReferenceFile << endl;

  //subtract orbit
  try {
    bpmorbit -= (Ref_bpmorbit);
  }
  catch (exception &e){
    cout << "ERROR: difference(): " << e.what() << endl;
    cout << "  Unequal positions of "<<i<<". BPM for subtraction."<< endl;
    return 1;
  }

  //subtract corrector strengths
  if (lattice.size(corrector) != Ref_lattice.size(corrector)) {
    cout << "ERROR: difference.cpp: Unequal number of correctors to subtract."<< endl;
    return 1;
  }
  /*
  for (i=0; i<lattice.size(corrector); i++) {
    if (vcorrs[i].start==Ref_vcorrs[i].start) {
      vcorrs[i].strength -= Ref_vcorrs[i].strength;
    }
    else {
      cout << "ERROR: difference.cpp: Unequal positions of "<<i+1<<". VC for subtraction."<< endl;
      return 1;
    }
  }
  */

  return 0;
}


/*
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


//calculates integral vcorr, quad and vcorr+quad kicks per dipole interval for harmcorr analysis
int harmcorr(AccLattice lattice, FunctionOfPos<AccPair> &orbit, const char *filename)
{
 unsigned int i=0,j=0,k=0;
 unsigned int nd = lattice.size(dipole);
 unsigned int nc = vcorrs.size(corrector);
 unsigned int nq = quads.size();
 double *HCvcorr = new double[2*nd](); //() -> initialize to zero
 double *HCquad = new double[2*nd]();
 double *HCsum = new double[2*nd]();
 double length;

 

 for (i=0; i<nd; i++) {
     while(vcorrs[j].start < dipols[i].start && j < nc) {
       length = vcorrs[j].end - vcorrs[j].start;
       HCvcorr[2*i] += vcorrs[j].strength * length * 1000; // mrad
       j++;
     }
     while(quads[k].start < dipols[i].start && k < nq) {
       length = quads[k].end - quads[k].start;
       HCquad[2*i] += quads[k].strength * orbit.interp(quads[k].end).z * length * 1000; // mrad
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
   HCquad[0] += quads[k].strength * orbit.interp(quads[k].end).z * length * 1000; // mrad
   k++;
 }

 for (i=0; i<nd; i++) {
   HCsum[2*i] = HCvcorr[2*i]+HCquad[2*i];
 }

 harmcorr_out(HCvcorr, HCquad, HCsum, nd, filename);

 delete[] HCvcorr;
 delete[] HCquad;
 delete[] HCsum;

 return 0;
}
*/
