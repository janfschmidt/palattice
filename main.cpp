/* Calculation of the magnetic spectrum (horizontal, vertical) of a periodic accelerator  *
 * Based on MadX output data and (optional for ELSA) on measured orbit and corrector data *
 * Used as input for Simulations of polarization by solving Thomas-BMT equation           *
 * 21.11.2012 - J.Schmidt                                                                 *
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
#include "orbit.hpp"
#include "fieldmap.hpp"
#include "spectrum.hpp"
#include "getfields.hpp"
#include "getspectrum.hpp"
#include "exportfile.hpp"
#include "madximport.hpp"
#include "ELSAimport.hpp"
#include "metadata.hpp"
#include "difference.hpp"
#include "timetag.hpp"
#include "filenames.hpp"
#include "resonances.hpp"

using namespace std;

int main (int argc, char *argv[])
{
  //-----------------------------
  unsigned int n_samp = 1644; // number of sampling points along ring for magn. field strengths
  bool sgt_access=false;       //special option for elsa-mode:
                               //if 1, spuren are read from /sgt/elsa/data/bpm/ instead of [project]/ELSA-Spuren/
  //-----------------------------
  unsigned int i;
  unsigned int fmax_x = 6;      // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 6;
  unsigned int fmax_s = 0;
  unsigned int fmax_hc = 12;    // harmcorr spectrum fmax = #dipoles/2 (is set to dipols.size()/2 below)
  unsigned int fmax_res = 0;
  unsigned int particle = 1;    // particle counter (for individual fields based on multi-particle tracking)
  TIMETAG t(530);               // moment(s) of elsa-cycle (ms)
  bool elsa = false;            // true: orbit, correctors, k & m read from /sgt/elsa/bpm/...
  bool diff = false;            // true: "harmcorr mode", calculate difference of two "Spuren"...
  bool allout = false;          // true: additional output files (orbit, field, bpms, ...)
  char spuren[20] = "dummy";
  char Reference[50] = "dummy";
  string tmp;
  double circumference=0;
  double dtheta = 0;           // spin phaseadvance stepwidth (option -r)
  BPM ELSAbpms[NBPMS];         // ELSAbpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR ELSAvcorrs[NVCORRS];    // ELSAvcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  BPM Ref_ELSAbpms[NBPMS];
  CORR Ref_ELSAvcorrs[NVCORRS];
  magnetvec dipols;            // use vector class for magnets and orbit; .name shows right labels
  magnetvec quads;
  magnetvec sexts;
  magnetvec vcorrs;
  CLOSEDORBIT bpmorbit;            // orbit at discrete positions (e.g. BPMs) for a specific time in elsa-cycle
  //  CLOSEDORBIT orbit;               // orbit interpolated from bpmorbit with n_samp sampling points
  TRAJECTORY trajectory;          // orbit of single particle (based on tracking)
  FIELDMAP B(n_samp);           // magnetic field along ring, [B]=1/m (missing factor gamma*m*c/e)
  int err=0;

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

  while ((opt = getopt(argc, argv, ":r:e:t:f:d:m:ah")) != -1) {
    switch(opt) {
    case 'r':
      dtheta = atof(optarg);
      fmax_res = int(360/dtheta / 2);
      break;
    case 'e':
      elsa = true;
      strncpy(spuren, optarg, 20);
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
      strncpy(Reference, optarg, 50);
      break;
    case 'a':
      allout = true;
      break;
    case 'h':
      cout << endl << "Bsupply HELP:" << endl;
      cout << "* First argument is project path." << endl;
      cout << "* -r [dtheta] estimates resonance strengths, stepwidth [dtheta]°" << endl;
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
  

  //initialize filenames
  FILENAMES file(argv[1], elsa, diff, sgt_access, spuren, Reference);
  cout << "Pfad: "<<file.path.c_str() << endl;


  //metadata for spectrum files
  METADATA metadata(file.path, elsa, diff, spuren, Reference);
  char madxLabels[100];
  snprintf(madxLabels, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
  metadata.madximport(madxLabels, file.import.c_str());
  circumference = strtod(metadata.getbyLabel("LENGTH").c_str(), NULL);
  if (circumference == 0) {
    cout << "ERROR: metadata: cannot read accelerator circumference from "<< file.import.c_str() << endl;
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
  madximport(file.import.c_str(), bpmorbit, dipols, quads, sexts, vcorrs);
  misalignments(file.misalign_dip.c_str(), dipols);
  trajectoryimport(file, trajectory, particle);

  // elsa=true: quad-&sext-strengths, BPM- & corrector-data from ELSA "Spuren"
  if (elsa) {
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors read"<<endl<<"  from "<<file.import.c_str() << endl;
    if (ELSAimport_magnetstrengths(quads, sexts, file.spuren.c_str()) != 0)
      return 1;
    ELSAimport(ELSAbpms, ELSAvcorrs, file.spuren.c_str());
    if (diff) ELSAimport(Ref_ELSAbpms, Ref_ELSAvcorrs, file.ref.c_str());
  }
  else {
    cout << "* "<<dipols.size()<<" dipoles, "<<quads.size()<<" quadrupoles, "
	 <<sexts.size()<<" sextupoles, "<<vcorrs.size()<<" correctors and "
	 <<bpmorbit.bpms()<<" BPMs read"<<endl<<"  from "<<file.import.c_str() << endl;
    cout << "* trajectory of particle "<<particle<<" read at "<<trajectory.bpms()
	 <<" observation points for "<<trajectory.turns()<<" turns"<<endl;
  }
  cout << "--------------------------------------------" << endl;



  // resonance strengths
  RESONANCES Res(dtheta, dipols.size());

 // magnetic spectrum (amplitudes & phases) up to fmax
  fmax_hc = int(dipols.size() / 2);
  SPECTRUM bx(fmax_x);
  SPECTRUM bz(fmax_z);
  SPECTRUM bs(fmax_s);
  SPECTRUM hc(fmax_hc); //harmcorr
  SPECTRUM res(fmax_res);

  

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
	   <<bpmorbit.size()<<" BPMs read"<<endl<<"  from "<<file.spuren.c_str() << endl;
    }
    //diff=true: read and subtract reference orbit & corrector data
    if (diff) {
	if (difference(file.ref.c_str(), t.get(i), bpmorbit, vcorrs, Ref_ELSAbpms, Ref_ELSAvcorrs, elsa) != 0)
	  return 1;
    }


    // interpolate orbit, calculate field distribution & spectrum
    //bpmorbit.interpol(orbit, n_samp);
    getfields(B, circumference, bpmorbit, dipols, quads, sexts, vcorrs, Res);
    getspectrum(bx, bz, res, B, circumference, Res);

    
    // generate output files
    if (allout) {
      //BPM data
      bpmorbit.out(file.out("bpms", t.tag(i)).c_str());
      trajectory.out(file.out("trajectory", t.tag(i)).c_str());
      //corrector data
      corrs_out(vcorrs, file.out("vcorrs", t.tag(i)).c_str());
      //orbit data (interpolated BPMs)
      //orbit.out(file.out("orbit", t.tag(i)).c_str());
      //field data
      fields_out(B, file.out("fields", t.tag(i)).c_str());
      //evaluated field data
      eval_out(bx, bz, n_samp, circumference, file.out("eval", t.tag(i)).c_str());
    }

    //export spectrum files for polarization-calculation
    exportfile(bx, metadata, "horizontal", file.spec("horizontal", t.tag(i)).c_str());
    exportfile(bz, metadata, "vertical", file.spec("vertical", t.tag(i)).c_str());
    // empty spectrum (fmax_s-1)
    exportfile(bs, metadata, "longitudinal", file.spec("longitudinal", t.tag(i)).c_str());

    //harmcorr data
    if (diff) {
      harmcorr(hc, vcorrs, quads, bpmorbit, dipols, circumference, file.out("harmcorr", t.tag(i)).c_str());
      exportfile(hc, metadata, "harmcorr", file.spec("harmcorr", t.tag(i)).c_str());
    }
    if (Res.on) {
      if (allout) Res.out(file.out("phaseadvance", t.tag(i)).c_str());
      exportfile(res, metadata, "resonances", file.spec("resonances", t.tag(i)).c_str());
    }

    cout << "--------------------------------------------" << endl;
  }
  
  cout << "Finished. (Run "<<file.path.c_str()<<"/inout/Bsupply"<<file.difftag.c_str()<<".gp for plots)" << endl << endl;

  return 0;
}


