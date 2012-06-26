/* Calculation of the magnetic spectrum (horizontal, vertical) of a periodic accelerator  *
 * Based on MadX output data and (optional for ELSA) on measured orbit and corrector data *
 * Used as input for Simulations of polarization by solving Thomas-BMT equation           *
 * 26.06.2012 - J.Schmidt                                                                 *
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
#include "timetag.hpp"

using namespace std;

int main (int argc, char *argv[])
{
  unsigned int i;
  unsigned int n_samp = 16440;   // number of sampling points along ring for magn. field strengths
  unsigned int fmax_x = 6;      // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 6;
  unsigned int fmax_s = 0;
  unsigned int fmax_hc = 12;    // harmcorr spectrum fmax = #dipoles/2 (is set to dipols.size()/2 below)
  TIMETAG t(530);               // moment(s) of elsa-cycle (ms)
  bool elsa = false;            // true: orbit, correctors, k & m read from /sgt/elsa/bpm/...
  bool diff = false;            // true: "harmcorr mode", calculate difference of two "Spuren"...
  bool allout = false;          // true: additional output files (orbit, field, bpms, ...)
  char spuren[20] = "dummy";
  char Reference[50] = "dummy";
  char filename[1024];
  char importFile[1024];
  char misalignFile[1024];
  char spurenFolder[1024];
  char ReferenceFolder[1024];
  char outputFolder[1024];
  string tmp;
  double circumference=0;
  BPM ELSAbpms[NBPMS];         // ELSAbpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR ELSAvcorrs[NVCORRS];    // ELSAvcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  BPM Ref_ELSAbpms[NBPMS];
  CORR Ref_ELSAvcorrs[NVCORRS];
  magnetvec dipols;            // use vector class for magnets and orbit; .name shows right labels
  magnetvec quads;
  magnetvec sexts;
  magnetvec vcorrs;
  orbitvec bpmorbit;            // orbit at discrete positions (e.g. BPMs) for a specific time in elsa-cycle
  orbitvec orbit;               // orbit interpolated from bpmorbit with n_samp sampling points
  FIELD *B = new FIELD[n_samp]; // magnetic field along ring, [B]=1/m (missing factor gamma*m*c/e)
  char difftag[6] = "";
  int err;

  int opt, warnflg=0, conflictflg=0;          //for getopt()
  extern char *optarg;
  extern int optopt, optind;
 


  // read input arguments
  optind=2;
  if (argc<=1) {                             // handle input without project-path
    cout << "Please enter project path as first argument." << endl;
    return 1;
  }
  else if (strncmp(argv[1],"-",1)==0) {
    if (strncmp(argv[1],"-h",2)==0)         // -h is ok without project-path
      optind=1;
    else {
      cout << "Please enter project path as first argument." << endl;
      return 1;
    }
  }
  snprintf(importFile, 1024, "%s/madx/madx.twiss", argv[1]);
  snprintf(outputFolder, 1024, "%s/inout", argv[1]);
  while ((opt = getopt(argc, argv, ":e:t:f:d:m:ah")) != -1) {
    switch(opt) {
    case 'e':
      elsa = true;
      strncpy(spuren, optarg, 20);
      snprintf(spurenFolder, 1024, "%s/ELSA-Spuren/%s", argv[1], spuren);
      break;
    case 't':
      conflictflg++; warnflg++;
      t.set(atoi(optarg)); //single time
      break;
    case 'm':
      conflictflg++; warnflg++;
      t.set(optarg); //file with multiple times
      break;
    case 'f':
      fmax_x = atoi(optarg);
      fmax_z = atoi(optarg);
      break;
    case 'd':
      diff = true;
      strncpy(difftag, "_diff", 6);
      strncpy(Reference, optarg, 50);
      break;
    case 'a':
      allout = true;
      break;
    case 'h':
      cout << endl << "Bsupply HELP:" << endl;
      cout << "* First argument is project path." << endl;
      cout << "* -e [spuren] enables ELSA-mode, Spuren as argument (path: [project]/ELSA-Spuren/) " << endl;
      cout << "* -f [fmax] sets maximum frequency for B-Field spectrum output" << endl;
      cout << "* -t [time] sets time of ELSA cycle to evaluate BPMs and correctors (in ms)" << endl;
      cout << "* -m [tagfile] multiple times of ELSA cycle evaluated. Times listed in [tagfile]" << endl;
      cout << "* -a enables all output files (orbit, fields, correctors, ...)" << endl;
      cout << "* -d [reference] enables difference-mode, where a reference orbit and corrector kicks are subtracted to analyse harmcorr."  << endl;
      cout << "     in ELSA-mode [reference] are Spuren ([project]/ELSA-Spuren/), else a MadX-twiss file ([project]/madx/)" << endl;
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
  if (conflictflg==2) {
    cout << "ERROR: do not use both options -t and -m. Use -h for help." << endl;
    return 1;
  }
  if (fmax_x >= n_samp/2 || fmax_z >= n_samp/2) {
    cout << "ERROR: The maximum frequency is to large to be calculated with "<<n_samp<<" sampling points." << endl;
    return 1;
  }
  if (warnflg && !elsa) {
    cout << endl;
    cout << "================================================================================" << endl;
    cout << "WARNING: options -t and -m are only used in ELSA-mode (-e). Use -h for help." << endl;
    cout << "================================================================================" << endl;
    t.set(0); // reset to single time to disable multi-mode
  }
  


  //metadata for spectrum files
  METADATA metadata(argv[1], elsa, diff, spuren, Reference);
  char madxLabels[100];
  snprintf(madxLabels, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
  metadata.madximport(madxLabels, importFile);
  circumference = strtod(metadata.getbyLabel("LENGTH").c_str(), NULL);
  if (circumference == 0) {
    cout << "ERROR: metadata: cannot read accelerator circumference from "<< importFile << endl;
    return 1;
  }

  
  // output
  cout << endl;
  cout << "--------------------------------------------" << endl;
  cout << "Bsupply: calculate magnetic field & spectrum" << endl;
  if (elsa) cout << "         ELSA-mode" << endl;
  if (diff) cout << "         harmcorr analysis (difference-mode)" << endl;
  cout << "--------------------------------------------" << endl;
  cout << "* "<<n_samp<<" sampling points along ring" << endl;
  cout << "* maximum frequencies used for B-field evaluation: Bx:"<<fmax_x<<", Bz:"<<fmax_z << endl;


  // MAD-X: read particle orbit and lattice (magnet positions & strengths)
  madximport(importFile, bpmorbit, dipols, quads, sexts, vcorrs);
  snprintf(misalignFile, 1024, "%s/madx/dipols.ealign", argv[1]); // will be replaced by filename-class
  misalignments(misalignFile, dipols);


  // elsa=true: quad-&sext-strengths, BPM- & corrector-data from ELSA "Spuren"
  if (elsa) {
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors read"<<endl<<"  from "<<importFile << endl;
    ELSAimport_magnetstrengths(quads, sexts, spurenFolder);
    ELSAimport(ELSAbpms, ELSAvcorrs, spurenFolder);
    if (diff) {
      snprintf(ReferenceFolder, 1024, "%s/ELSA-Spuren/%s", argv[1], Reference);
      ELSAimport(Ref_ELSAbpms, Ref_ELSAvcorrs, ReferenceFolder); 
    }
  }
  else {
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors and "
	 <<bpmorbit.size()<<" BPMs read"<<endl<<"  from "<<importFile << endl;
    if (diff) snprintf(ReferenceFolder, 1024, "%s/madx/%s", argv[1], Reference);
  }
  cout << "--------------------------------------------" << endl;


 // magnetic spectrum (amplitudes & phases) up to fmax
  fmax_hc = dipols.size() / 2;
  SPECTRUM bx[fmax_x+1];
  SPECTRUM bz[fmax_z+1];
  SPECTRUM bs[fmax_s+1];
  SPECTRUM hc[fmax_hc+1]; //harmcorr
 

  for(i=0; i<t.size(); i++) {
    if (elsa) {
      if (t.get(i) < 0) {
	cout << "ERROR: t = "<<t.get(i)<<" < 0 is no valid moment of ELSA cycle." << endl;
	return 1;
      }

      metadata.setbyLabel("Time in cycle", t.label(i));
      err += ELSAimport_getbpmorbit(ELSAbpms, bpmorbit, t.get(i));
      err += ELSAimport_getvcorrs(ELSAvcorrs, vcorrs, t.get(i));
      if (err != 0) return 1;
      cout << "* "<<t.label(i)<<": "<<vcorrs.size()<<" correctors and "
	   <<bpmorbit.size()<<" BPMs read"<<endl<<"  from "<<spurenFolder << endl;
    }
    //diff=true: read and subtract reference orbit & corrector data
    if (diff) {
      if (difference(ReferenceFolder, t.get(i), bpmorbit, vcorrs, Ref_ELSAbpms, Ref_ELSAvcorrs, elsa) != 0) return 1;
    }

    // interpolate orbit, calculate field distribution & spectrum
    getorbit(orbit, circumference, bpmorbit, n_samp);
    getfields(B, circumference, orbit, dipols, quads, sexts, vcorrs);
    getspectrum(bx, bz, B, n_samp, fmax_x, fmax_z, circumference);
    
    // generate output files
    if (allout) {
      //BPM data
      snprintf(filename, 1024, "%s/bpms%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      bpms_out(bpmorbit, filename);
      //corrector data
      snprintf(filename, 1024, "%s/vcorrs%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      corrs_out(vcorrs, filename);
      //orbit data (interpolated BPMs)
      snprintf(filename, 1024, "%s/orbit%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      orbit_out(orbit, filename);
      //field data
      snprintf(filename, 1024, "%s/fields%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      fields_out(B, n_samp, filename);
      //evaluated field data
      snprintf(filename, 1024, "%s/eval%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      eval_out(bx, bz, fmax_x, fmax_z, n_samp, circumference, filename);
    }

    //export spectrum files for polarization-calculation
    snprintf(filename, 1024, "%s/horizontal%s%s.spectrum", outputFolder, t.tag(i).c_str(), difftag);
    exportfile(bx, fmax_x, metadata, "horizontal", filename);
    snprintf(filename, 1024, "%s/vertical%s%s.spectrum", outputFolder, t.tag(i).c_str(), difftag);
    exportfile(bz, fmax_z, metadata, "vertical", filename);
    snprintf(filename, 1024, "%s/longitudinal%s%s.spectrum", outputFolder, t.tag(i).c_str(), difftag);
    exportfile(bs, fmax_s-1, metadata, "longitudinal", filename); // empty spectrum (fmax_s-1)
    //harmcorr data
    if (diff) {
      snprintf(filename, 1024, "%s/harmcorr%s%s.dat", outputFolder, t.tag(i).c_str(), difftag);
      harmcorr(hc, fmax_hc, vcorrs, quads, orbit, dipols, circumference, n_samp, filename);
      snprintf(filename, 1024, "%s/harmcorr%s%s.spectrum", outputFolder, t.tag(i).c_str(), difftag);
      exportfile(hc, fmax_hc, metadata, "harmcorr", filename);
    }

    cout << "--------------------------------------------" << endl;
  }
  
  cout << "Finished. (Run "<<outputFolder<<"/Bsupply"<<difftag<<".gp for plots)" << endl << endl;
  delete[] B;

  return 0;
}


