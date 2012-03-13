/* Calculation of the magnetic spectrum (horizontal, vertical) of a periodic accelerator  *
 * Based on MadX output data and (optional for ELSA) on measured orbit and corrector data *
 * Used as input for Simulations of polarization by solving Thomas-BMT equation           *
 * 13.03.2012 - J.Schmidt                                                                 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "constants.hpp"
#include "types.hpp"
#include "getorbit.hpp"
#include "getfields.hpp"
#include "getspectrum.hpp"
#include "exportfile.hpp"
#include "madximport.hpp"
#include "ELSAimport.hpp"
#include "metadata.hpp"
#include "difference.hpp"

using namespace std;

int main (int argc, char *argv[])
{
  unsigned int n_samp = 16440;   // number of sampling points along ring for magn. field strengths
  unsigned int fmax_x = 6;      // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 6;
  int t = 530;                  // moment of elsa-cycle (s)
  bool elsa = false;            // true: orbit, correctors, k & m read from /sgt/elsa/bpm/...
  bool diff = false;            // true: "harmcorr mode", calculate difference of two "Spuren"...
  bool multi = false;           // true: t added to spectrum filenames ("multi" for "multiple files")
  char spuren[20] = "2011-03-01-18-18-59";
  char Ref_spuren[20];

  char filename[1024];
  char importFile[1024];
  char spurenFolder[1024];
  char Ref_spurenFolder[1024];
  char outputFolder[1024];
  string tmp;
  char t_str[10];
  double circumference=0;
  BPM ELSAbpms[NBPMS];         // ELSAbpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR ELSAvcorrs[NVCORRS];    // ELSAvcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  magnetvec dipols;            // use vector class for magnets and orbit; .name shows right labels
  magnetvec quads;
  magnetvec sexts;
  magnetvec vcorrs;
  orbitvec bpmorbit;            // orbit at discrete positions (e.g. BPMs) for a specific time in elsa-cycle
  orbitvec orbit;               // orbit interpolated from bpmorbit with n_samp sampling points
  FIELD *B = new FIELD[n_samp]; // magnetic field along ring, [B]=1/m (missing factor gamma*m*c/e)
  double corrlength;
  char difftag[6] = "";
  char timetag[10] = "";

  int opt, warnflg=0;          //for getopt()
  extern char *optarg;
  extern int optopt, optind;


  // read input arguments
  if (argc<=1 || strncmp(argv[1], "-", 1)==0) {
    cout << "Please enter project path as first argument." << endl;
    return 1;
  }
  snprintf(importFile, 1024, "%s/madx/madx.twiss", argv[1]);
  snprintf(outputFolder, 1024, "%s/inout", argv[1]);
  optind = 2;
  while ((opt = getopt(argc, argv, ":e:t:f:d:mh")) != -1) {
    switch(opt) {
    case 'e':
      elsa = true;
      strncpy(spuren, optarg, 20);
      snprintf(spurenFolder, 1024, "%s/ELSA-Spuren/%s", argv[1], spuren);
      break;
    case 't':
      if (!elsa) warnflg++;
      t = atoi(optarg);
      break;
    case 'f':
      fmax_x = atoi(optarg);
      fmax_z = atoi(optarg);
      break;
    case 'd':
      if (!elsa) warnflg++;
      diff = true;
      strncpy(difftag, "_diff", 6);
      strncpy(Ref_spuren, optarg, 20);
      snprintf(Ref_spurenFolder, 1024, "%s/ELSA-Spuren/%s", argv[1], Ref_spuren);
      break;
    case 'm':
      if (!elsa) warnflg++;
      multi = true;
      break;
    case 'h':
      cout << endl << "Bsupply HELP:" << endl;
      cout << "* First argument is project path." << endl;
      cout << "* -e [spuren] enables ELSA-mode, Spuren as argument (path: [project]/ELSA-Spuren/) " << endl;
      cout << "* -f [fmax] sets maximum frequency for B-Field spectrum output" << endl;
      cout << "* -t [time] sets time of ELSA cycle to evaluate BPMs and correctors (in ms)" << endl;
      cout << "* -m multiple spectrum files: Adds [time] (-t) to spectrum filename" << endl;
      cout << "* -d [refspuren] enables difference-mode, where refspuren are subtracted from spuren (set with -e) to analyse harmcorr"  << endl;
      cout << "* -h displays this help" << endl << endl;
      return 0;
    case ':':
      cout << "ERROR: -" << (char)optopt << " without argument. Use -h for help." << endl;
      return 1;
    case '?':
      cout << "ERROR: unknown option -" << (char)optopt << ". Use -h for help." << endl;
      return 1;
    }
  }


  // check input
  if (warnflg) {
    cout << endl;
    cout << "================================================================================" << endl;
    cout << "WARNING: options -t, -m and -d are only used in ELSA-mode (-e). Use -h for help." << endl;
    cout << "================================================================================" << endl;
  }
  if (fmax_x >= n_samp/2 || fmax_z >= n_samp/2) {
    cout << "ERROR: The maximum frequency is to large to be calculated with "<<n_samp<<" sampling points." << endl;
    return 1;
  }
  if (t < 0) {
    cout << "ERROR: t = "<<t<<" < 0 is no valid moment of ELSA cycle." << endl;
    return 1;
  }


  // magnetic spectrum (amplitudes & phases) up to fmax
  SPECTRUM bx[fmax_x+1];
  SPECTRUM bz[fmax_z+1];


  //metadata for spectrum files
  METADATA metadata;
  char madxLabels[100];
  metadata.add("Project path", argv[1]);
  snprintf(madxLabels, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
  metadata.madximport(madxLabels, importFile);
  if (elsa) {
    if (diff) { metadata.add("Program Mode", "elsa + difference"); }
    else      { metadata.add("Program Mode", "elsa"); }
    metadata.add("Spuren", spuren);
    if (diff) { metadata.add("Referenz-Spuren", Ref_spuren); }
    snprintf(t_str, 10, "%d ms", t);
    metadata.add("Time in cycle", t_str);
  }
  else {
    metadata.add("Program Mode", "madx");
  }
  tmp = metadata.getbyLabel("LENGTH");
  if (tmp == "NA") {
    cout << "ERROR: metadata: cannot read accelerator circumference from "<< importFile << endl;
    return 1;
  }
  else {
    circumference = strtod(tmp.c_str(), NULL);
  }

  
  // output
  cout << endl;
  cout << "--------------------------------------------" << endl;
  cout << "Bsupply: calculate magnetic field & spectrum" << endl;
  if (elsa) cout << "         ELSA-mode" << endl;
  if (diff) cout << "         harmcorr analysis (difference-mode)" << endl;
  cout << "--------------------------------------------" << endl;
  cout << "* "<<n_samp<<" sampling points along ring" << endl;
  if (elsa) cout << "* "<<t<<" ms after start of ELSA cycle" << endl;
  cout << "* maximum frequencies used for B-field evaluation: Bx:"<<fmax_x<<", Bz:"<<fmax_z << endl;


  // read particle orbit and lattice (magnet positions & strengths) from MADX
  madximport(importFile, bpmorbit, dipols, quads, sexts, vcorrs);
  corrlength = vcorrs[1].end-vcorrs[1].start; //save corrector length. vcorrs[1] chosen, all have equal length


  // elsa=true: re-read from ELSA "Spuren": orbit, corrector data, quad-&sext-strengths
  if (elsa) {
    ELSAimport_magnetstrengths(quads, sexts, spurenFolder);
    ELSAimport(ELSAbpms, ELSAvcorrs, spurenFolder); 
    ELSAimport_getbpmorbit(ELSAbpms, bpmorbit, t);
    ELSAimport_getvcorrs(ELSAvcorrs, vcorrs, corrlength, t);
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles read from "<<importFile << endl;
    cout << "* "<<vcorrs.size()<<" correctors and "
	 <<bpmorbit.size()<<" BPMs read from "<<spurenFolder << endl;
  }
  else {
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors and "
	 <<bpmorbit.size()<<" BPMs read from "<<importFile << endl;
  }


  //diff=true: read and subtract reference orbit & corrector data
  if (diff) {
    if (difference(Ref_spurenFolder, spuren, Ref_spuren, t, corrlength, bpmorbit, vcorrs) != 0) return 1;
  }


  // interpolate orbit, calculate field distribution & spectrum
  getorbit(orbit, circumference, bpmorbit, n_samp);
  getfields(B, circumference, orbit, dipols, quads, sexts, vcorrs);
  getspectrum(bx, bz, B, n_samp, fmax_x, fmax_z, circumference);


  // generate output files
  if (elsa && multi) { snprintf(timetag, 10, "_%dms", t); }
  cout << "Write output files:" << endl;

  if (elsa) {
    //BPM data
    snprintf(filename, 1024, "%s/elsabpms%s%s.dat", outputFolder, timetag, difftag);
    bpms_out(bpmorbit, filename);
    //corrector data
    snprintf(filename, 1024, "%s/elsacorrs%s%s.dat", outputFolder, timetag, difftag);
    corrs_out(vcorrs, filename);
    if (diff) {
      //harmcorr data
      snprintf(filename, 1024, "%s/harmcorr%s%s.dat", outputFolder, timetag, difftag);
      harmcorr_out(vcorrs, dipols, filename);
    }
  }

  //orbit data (interpolated BPMs)
  snprintf(filename, 1024, "%s/orbit%s%s.dat", outputFolder, timetag, difftag);
  orbit_out(orbit, filename);
  //field data
  snprintf(filename, 1024, "%s/fields%s%s.dat", outputFolder, timetag, difftag);
  fields_out(B, n_samp, filename);
  //evaluated field data
  snprintf(filename, 1024, "%s/eval%s%s.dat", outputFolder, timetag, difftag);
  eval_out(bx, bz, fmax_x, fmax_z, n_samp, circumference, filename);

  //export spectrum files for polarization-calculation
  snprintf(filename, 1024, "%s/horizontal%s%s.spectrum", outputFolder, timetag, difftag);
  exportfile(bx, fmax_x, metadata, "horizontal", filename);
  snprintf(filename, 1024, "%s/vertical%s%s.spectrum", outputFolder, timetag, difftag);
  exportfile(bz, fmax_z, metadata, "vertical", filename);
  
  cout << "Finished. (Run "<<outputFolder<<"/Bsupply"<<difftag<<".gp for plots)" << endl << endl;
  
  delete[] B;

  return 0;
}


