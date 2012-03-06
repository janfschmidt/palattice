/* Calculation of the magnetic spectrum (horizontal, vertical) of a periodic accelerator  *
 * Based on MadX output data and (optional for ELSA) on measured orbit and corrector data *
 * Used as input for Simulations of polarization by solving Thomas-BMT equation           *
 * 06.03.2012 - J.Schmidt                                                                 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include "constants.hpp"
#include "types.hpp"
#include "getorbit.hpp"
#include "getfields.hpp"
#include "getspectrum.hpp"
#include "exportfile.hpp"
#include "madximport.hpp"
#include "ELSAimport.hpp"
#include "metadata.hpp"

using namespace std;

int main (int argc, char *argv[])
{
  unsigned int n_samp = 16440;   // number of sampling points along ring for magn. field strengths
  unsigned int fmax_x = 1000;     // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 1000;
  double t = 0.530;    // moment of elsa-cycle (s)
  bool elsa = false;       // true: orbit, correctors, k & m read from /sgt/elsa/bpm/...
  char spuren[20] = "2011-03-01-18-18-59";

  char filename[1024];
  char importFile[1024];
  char spurenFolder[1024];
  char outputFolder[1024];
  string tmp;
  char ctmp[10];
  double circumference=0;
  BPM ELSAbpms[NBPMS];         //ELSAbpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR ELSAvcorrs[NVCORRS];    //ELSAvcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  magnetvec dipols;            // use vector class for magnets and orbit; .name shows right labels
  magnetvec quads;
  magnetvec sexts;
  magnetvec vcorrs;
  orbitvec bpmorbit;           // orbit at discrete positions (e.g. BPMs) for a specific time in elsa-cycle
  orbitvec orbit;              // orbit interpolated from bpmorbit with n_samp sampling points
  FIELD B[n_samp];             // magnetic field along ring, [B]=1/m (missing factor gamma*m*c/e)
  SPECTRUM bx[fmax_x+1];       // magnetic spectrum (amplitudes & phases) up to fmax
  SPECTRUM bz[fmax_z+1];


  // read input-arguments
  if(argc>1) {
    snprintf(importFile, 1024, "%s/madx/madx.twiss", argv[1]);
    snprintf(outputFolder, 1024, "%s/inout", argv[1]);
    if(argc>2) {
      if (strcmp(argv[2], "-madx")==0) {
	elsa = false;
	if (argc>3) {
	  cout << "ERROR: -madx has no additional arguments." << endl;
	  return 1;
	}
      }
      else if (strcmp(argv[2], "-elsa")==0) {
	elsa = true;
	if (argc==3) {
	  cout << endl << "WARNING: default Spuren "<< spuren << " used." << endl;
	}
	else if (argc>=4 && argc<=6) {
	  strncpy(spuren, argv[3], 20);
	  if (argc==5 || argc==6) {
	    t = strtod(argv[4], NULL);
	  }
	  if (argc==6) {
	    fmax_x = fmax_z = atoi(argv[5]);
	  }
	}
	else {
	  cout << "ERROR: Too many arguments for -elsa." << endl << "Try e.g. -elsa [spur] [t] [fmax]" << endl;
	  return 1;
	}
	snprintf(spurenFolder, 1024, "%s/ELSA-Spuren/%s", argv[1], spuren);
      }
      else {
	cout << "ERROR: Second argument sets mode: -elsa or -madx" << endl;
	return 1;
      }
    }
  }
  else {
    cout << "Please enter project path as first argument." << endl;
    return 1;
  }


  // check input
  if (fmax_x >= n_samp/2 || fmax_z >= n_samp/2) {
    cout << "ERROR: The maximum frequency is to large to be calculated with "<<n_samp<<" sampling points." << endl;
    return 1;
  }
  if (t < 0.0) {
    cout << "ERROR: t = "<<t<<" < 0 is no valid moment of ELSA cycle." << endl;
    return 1;
  }


  //metadata for spectrum files
  METADATA metadata;
  char madxLabels[100];
  metadata.add("Project path", argv[1]);
  snprintf(madxLabels, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
  metadata.madximport(madxLabels, importFile);
  tmp = metadata.getbyLabel("LENGTH");
  if (tmp == "NA") {
    cout << "ERROR: cannot read accelerator circumference from MadX file." << endl;
    return 1;
  }
  else {
    circumference = strtod(tmp.c_str(), NULL);
  }

  
  // output
  cout << endl;
  cout << "--------------------------------------------" << endl;
  cout << "Bsupply: calculate magnetic field & spectrum" << endl;
  cout << "--------------------------------------------" << endl;
  cout << "* "<<n_samp<<" sampling points along ring" << endl;
  if (elsa) cout << "* "<<t<<" s after start of ELSA cycle" << endl;
  cout << "* maximum frequencies used for B-field evaluation: Bx:"<<fmax_x<<", Bz:"<<fmax_z << endl;


  // read particle orbit and lattice (magnet positions & strengths) from MADX
  madximport(importFile, bpmorbit, dipols, quads, sexts, vcorrs);

  // elsa=true: re-read from ELSA "Spuren": orbit, corrector data, quad-&sext-strengths
  if (elsa) {
    metadata.add("Program Mode", "elsa");
    metadata.add("Spuren", spuren);
    snprintf(ctmp, 10, "%.3lf s", t);
    metadata.add("Time in cycle", ctmp);
    ELSAimport(ELSAbpms, ELSAvcorrs, quads, sexts, spurenFolder); 
    ELSAimport_getbpmorbit(ELSAbpms, bpmorbit, t);
    ELSAimport_getvcorrs(ELSAvcorrs, vcorrs, t);
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "<<sexts.size()<<" sextupoles read from "<<importFile << endl;
    cout << "* "<<vcorrs.size()<<" correctors and "<<bpmorbit.size()<<" BPMs read from "<<spurenFolder << endl;
  }
  else {
    metadata.add("Program Mode", "madx");
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "<<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors and "<<bpmorbit.size()<<" BPMs read from "<<importFile << endl;
  }

  // interpolate orbit, calculate field distribution & spectrum
  getorbit(orbit, circumference, bpmorbit, n_samp);
  getfields(B, circumference, orbit, dipols, quads, sexts, vcorrs);
  getspectrum(bx, bz, B, n_samp, fmax_x, fmax_z, circumference);


  // generate output files
  cout << "Write output files:" << endl;

  if (elsa) {
    //BPM data
    snprintf(filename, 1024, "%s/elsabpms.dat", outputFolder);
    bpms_out(ELSAbpms, t, filename);
    //corrector data
    snprintf(filename, 1024, "%s/elsacorrs.dat", outputFolder);
    corrs_out(ELSAvcorrs, t, filename);
  }
  //orbit data (interpolated BPMs)
  snprintf(filename, 1024, "%s/orbit.dat", outputFolder);
  orbit_out(orbit, filename);
  //field data
  snprintf(filename, 1024, "%s/fields.dat", outputFolder);
  fields_out(B, n_samp, filename);
  //evaluated field data
  snprintf(filename, 1024, "%s/eval.dat", outputFolder);
  eval_out(bx, bz, fmax_x, fmax_z, n_samp, circumference, filename);
  //export spectrum files for polarization-calculation
  //exportfile(bx, fmax_x, metadata, outputFolder, "horizontal", true);
  exportfile(bx, fmax_x, metadata, outputFolder, "horizontal", false); // file without timestamp (false)
  //exportfile(bz, fmax_z, metadata, outputFolder, "vertical", true);
  exportfile(bz, fmax_z, metadata, outputFolder, "vertical", false);   // file without timestamp (false)
  
  cout << "Finished. (Run "<<outputFolder<<"/Bsupply.gp for plots)" << endl << endl;
  

  return 0;
}


