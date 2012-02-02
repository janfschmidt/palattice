/* Calculation of the magnetic spectrum (horizontal, vertical) of a periodic accelerator  *
 * Based on MadX output data and (optional for ELSA) on measured orbit and corrector data *
 * Used as input for Simulations of polarization by solving Thomas-BMT equation           *
 * 27.01.2012 - J.Schmidt                                                                 *
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
  unsigned int fmax_x = 6;     // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 6;
  double t = 0.531;    // moment of elsa-cycle (s)
  char importFile[1024] = "../project/madx/madx.twiss";
  char spurenFolder[1024] = "../project/ELSA-Spuren/2012-01-24-15-11-27";
  bool elsa = true;       // true: orbit, correctors, k & m read from /sgt/elsa/bpm/... 

  char filename[1024];
  string tmp;
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


  // check input
  if (fmax_x >= n_samp/2 || fmax_z >= n_samp/2) {
    printf("\nThe maximum frequency is to large to be calculated with %d sampling points.\n", n_samp);
    return 1;
  }
  if (t < 0.0) {
    printf("t = %lf < 0 is no valid moment of ELSA cycle.\n", t);
    return 1;
  }


  // read input-arguments
  if (argc>1) {
    if (strcmp(argv[1], "-madx")==0) {
      elsa = false;
      if (argc==3) {
	strncpy(importFile, argv[2], 1024);
      }
      else if (argc>3) {
	cout << "ERROR: use -madx path_of_madxfile" << endl;
	return 1;
      }
    }
    else if (strcmp(argv[1], "-elsa")==0) {
      elsa = true;
      if (argc==3) {
	strncpy(spurenFolder, argv[2], 1024);
      }
      else if (argc==4) {
	strncpy(spurenFolder, argv[2], 1024);
	strncpy(importFile, argv[3], 1024);
      }
      else if (argc>4) {
	cout << "ERROR: use -elsa path_of_spuren [path_of_madxfile]" << endl;
	return 1;
      }
    }
    else {
      cout << "ERROR: first argument sets mode: -elsa or -madx" << endl;
      return 1;
    }
  }


  //metadata for spectrum files
  METADATA metadata;
  char madxLabels[100];
  snprintf(madxLabels, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
  metadata.madximport(madxLabels, importFile);
  metadata.add("personal message", "juchu, es klappt :-)");
  tmp = metadata.getbyLabel("LENGTH");
  if (tmp == "NA") {
    printf("cannot read accelerator circumference from MadX file.\n");
    return 1;
  }
  else {
    circumference = strtod(tmp.c_str(), NULL);
  }

  
  // output
  printf("\n---------------------------------\n");
  printf("calculate magnetic field spectrum\n");
  printf("---------------------------------\n");
  printf("* %d sampling points along ELSA ring\n", n_samp);
  printf("* maximum frequencies used for B-field evaluation: Bx:%d, Bz:%d\n", fmax_x, fmax_z);


  // read particle orbit and lattice (magnet positions & strengths) from MADX
  madximport(importFile, bpmorbit, dipols, quads, sexts, vcorrs);

  // elsa=true: re-read from ELSA "Spuren": orbit, corrector data, quad-&sext-strengths
  if (elsa) {
    metadata.add("Program Mode", "elsa");
    ELSAimport(ELSAbpms, ELSAvcorrs, quads, sexts, spurenFolder); 
    ELSAimport_getbpmorbit(ELSAbpms, bpmorbit, t);
    ELSAimport_getvcorrs(ELSAvcorrs, vcorrs, t);
    printf("* %d dipoles, %d quadrupoles, %d sextupoles read from %s.\n", dipols.size(), quads.size(), sexts.size(), importFile);
    printf("* %d correctors and %i BPMs read from %s.\n", vcorrs.size(), bpmorbit.size(), spurenFolder);
  }
  else {
    metadata.add("Program Mode", "madx");
    printf("* %d dipoles, %d quadrupoles, %d sextupoles, %d correctors and %d BPMs read from %s.\n", dipols.size(), quads.size(), sexts.size(), vcorrs.size(), bpmorbit.size(), importFile);
  }

  // interpolate orbit, calculate field distribution & spectrum
  getorbit(orbit, circumference, bpmorbit, n_samp);
  getfields(B, circumference, orbit, dipols, quads, sexts, vcorrs);
  getspectrum(bx, bz, B, n_samp, fmax_x, fmax_z, circumference);


  // generate output files
  printf("Write output files:\n");

  if (elsa) {
    //BPM data
    snprintf(filename, 1024, "%s/elsabpms.dat", OUTPUTFOLDER);
    bpms_out(ELSAbpms, t, filename);
    //corrector data
    snprintf(filename, 1024, "%s/elsacorrs.dat", OUTPUTFOLDER);
    corrs_out(ELSAvcorrs, t, filename);
  }
  //orbit data (interpolated BPMs)
  snprintf(filename, 1024, "%s/orbit.dat", OUTPUTFOLDER);
  orbit_out(orbit, filename);
  //field data
  snprintf(filename, 1024, "%s/fields.dat", OUTPUTFOLDER);
  fields_out(B, n_samp, filename);
  //evaluated field data
  snprintf(filename, 1024, "%s/eval.dat", OUTPUTFOLDER);
  eval_out(bx, bz, fmax_x, fmax_z, n_samp, circumference, filename);
  //export spectrum files for polarization-calculation
  exportfile(bx, fmax_x, metadata, "horizontal", true);
  exportfile(bx, fmax_x, metadata, "horizontal", false); //second file without timestamp for gnuplot-script
  exportfile(bz, fmax_z, metadata, "vertical", true);
  exportfile(bz, fmax_z, metadata, "vertical", false);   //second file without timestamp for gnuplot-script
  
  printf("Finished. (Run polsim.gp for plots)\n\n");
  

  return 0;
}


