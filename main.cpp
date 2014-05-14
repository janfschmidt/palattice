
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
#include "AccLattice.hpp"
#include "timetag.hpp"
#include "filenames.hpp"
#include "resonances.hpp"
#include "metadata.hpp"
#include "ELSAimport.hpp"
#include "spectrum.hpp"
#include "field.hpp"
#include "functionofpos.hpp"
#include "gitversion.hpp"

using namespace std;



void usage()
{
  cout << endl << "Bsupply HELP:" << endl;
  cout << "* First argument is project path." << endl;
  cout << "* -n [n_samp] sets number of sampling points (per rev.) for field calculation." << endl;
  cout << "* -p [particle no] enables import of single particle trajectory from madx (ptc_track) or elegant." << endl;
  cout << "* -s [madx/elegant] set simulation tool whose output is used for lattice/orbit/tracking import." << endl;
  cout << "* -r [dtheta] estimates resonance strengths, stepwidth [dtheta]°" << endl;
  cout << "* -e [spuren] enables ELSA-mode, Spuren as argument (path: [project]/ELSA-Spuren/) " << endl;
  cout << "* -f [fmax] sets maximum frequency for B-Field spectrum (in rev. harmonics)" << endl;
  cout << "* -F [fmax] sets maximum frequency for resonance-spectrum (-r) output (in rev. harmonics)" << endl;
  cout << "* -c [minamp] sets minimum amplitude for B-Field spectrum, others are cutted" << endl;
  cout << "* -C [minamp] sets minimum amplitude for resonance-spectrum (-r), others are cutted" << endl;
  cout << "* -t [time] sets time of ELSA cycle to evaluate BPMs and correctors (in ms)" << endl;
  cout << "* -m [tagfile] multiple times of ELSA cycle evaluated. Times listed in [tagfile]" << endl;
  cout << "* -a enables all output files (orbit, fields, correctors, ...)" << endl;
  cout << "* -d [reference] enables difference-mode, where a reference orbit and corrector kicks are subtracted:"  << endl;
  cout << "     in ELSA-mode [reference] are Spuren ([project]/ELSA-Spuren/)," << endl;
  cout << "     else a MadX-twiss file ([project]/madx/[reference])" << endl;
  cout << "     or the name of elegant output files ([project]/elegant/[reference].clo & [project]/elegant/[reference].param)." << endl;
  cout << "     MadX or elegant is chosen by -s option." << endl;
  cout << "* -h displays this help" << endl << endl;
}




int main (int argc, char *argv[])
{
  //-----------------------------
  bool sgt_access=true;       //special option for elsa-mode:
                               //if 1, spuren are read from /sgt/elsa/data/bpm/ instead of [project]/ELSA-Spuren/
  //-----------------------------
  unsigned int n_samp;         // number of sampling points along ring for magn. field strengths
  bool  default_n_samp = true;
  unsigned int i, ftmp;
  unsigned int fmax_x = 25;      // max Frequency used for magnetic field spectrum (in revolution harmonics)
  unsigned int fmax_z = 25;
  unsigned int fmax_s = 0;
  unsigned int fmax_res = 0;
  unsigned int particle = 1;    // particle counter (for individual fields based on multi-particle tracking)
  TIMETAG t(530);               // moment(s) of elsa-cycle (ms)
  bool tracking = false;        // true: single particle trajectory imported from madx (ptc_track) or elegant results
  bool elsa = false;            // true: orbit, correctors, k & m read from /sgt/elsa/bpm/...
  bool diff = false;            // true: "harmcorr mode", calculate difference of two "Spuren"...
  bool allout = false;          // true: additional output files (orbit, field, bpms, ...)
  char spuren[20] = "dummy";
  char Reference[50] = "dummy";
  double circumference=0;
  double ampcut_x = 1e-5;     // minimum amplitudes for magnetic field spectra
  double ampcut_z = 1e-6;     // be carefull: ampcut_z can change spin tune !
  double ampcut_res=0;          // minimum amplitude for resonance spectrum (option -c)
  double dtheta = -1;           // spin phaseadvance stepwidth (option -r)
  BPM ELSAbpms[NBPMS];         // ELSAbpms[0]=BPM01, ELSAbpms[31]=BPM32
  CORR ELSAvcorrs[NVCORRS];    // ELSAvcorrs[0]=VC01, ELSAvcorrs[31]=VC32
  BPM Ref_ELSAbpms[NBPMS];
  CORR Ref_ELSAvcorrs[NVCORRS];
  FunctionOfPos<AccPair> *orbit;
  simulationTool simTool = madx;  // lattice/orbit/tracking from madx or elegant? (option -s)

  //for getopt():
  int opt, warnflg=0, conflictflg=0;
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
      usage();
      return 1;
    }
  }

  while ((opt = getopt(argc, argv, ":n:p:s:r:e:t:f:F:c:C:d:m:ah")) != -1) {
    switch(opt) {
    case 'n':
      default_n_samp = false;
      n_samp = atoi(optarg);
      break;
    case 'p':
      tracking = true;
      particle = atoi(optarg);
      break;
    case 's':
      if (strcmp(optarg,"madx") == 0) simTool = madx;
      else if (strcmp(optarg,"elegant") == 0) simTool = elegant;
      else cout << "WARNING: invalid input for simulation tool (-s). Default is used. See -h for help." << endl;
      break;
    case 'r':
      dtheta = atof(optarg);
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
      fmax_z = fmax_x;
      break;
    case 'F':
      fmax_res = atoi(optarg);
      break;
    case 'c':
      ampcut_x = atof(optarg);
      ampcut_z = ampcut_x;
      break;
    case 'C':
      ampcut_res = atof(optarg);
      break;
    case 'd':
      diff = true;
      strncpy(Reference, optarg, 50);
      break;
    case 'a':
      allout = true;
      break;
    case 'h':
      usage();
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
    cout << "ERROR: do not use both options -t and -m." << endl;
    usage();
    return 1;
  }
  if (warnflg && !elsa) {
    cout << endl;
    cout << "================================================================================" << endl;
    cout << "WARNING: options -t and -m are only used in ELSA-mode (-e). Use -h for help." << endl;
    cout << "================================================================================" << endl;
    t.set(0); // reset to single time to disable multi-mode
  }
  if ((fmax_res+ampcut_res)!=0 && dtheta==-1) {
    cout << endl;
    cout << "=======================================================================================" << endl;
    cout << "WARNING: options -F and -C are only used for resonance spectrum (-r). Use -h for help." << endl;
    cout << "=======================================================================================" << endl;
 }
  

  //initialize filenames
  FILENAMES file(argv[1], simTool, elsa, diff, sgt_access, spuren, Reference);
  cout << "Pfad: "<<file.path.c_str() << endl;


  //metadata for spectrum files
  METADATA metadata(file.path, elsa, simTool, diff, spuren, Reference);
  //metadata.add("POLE version", gitversion());
  char tmp[100];
  if (simTool == madx) {
    snprintf(tmp, 100, "TITLE,LENGTH,ORIGIN,PARTICLE");
    metadata.madximport(tmp, file.lattice.c_str());
  circumference = strtod(metadata.getbyLabel("LENGTH").c_str(), NULL);
  }
  else {
    metadata.add("ORIGIN", "Elegant");
    snprintf(tmp, 100, "circumference,pCentral/m_e*c,tune:Qx,tune:Qz");
    metadata.elegantimport(tmp, file.lattice.c_str());
    circumference = strtod(metadata.getbyLabel("circumference").c_str(), NULL);
  }
  if (circumference == 0) {
    cout << "ERROR: metadata: cannot read accelerator circumference from "<< file.lattice << endl;
    return 1;
  }
  // add revolution freq. to metadata
  stringstream frev;
  frev << GSL_CONST_MKSA_SPEED_OF_LIGHT/circumference;
  metadata.add("rev.frequency/Hz", frev.str());

  // calculate default sampling points along ring (n_samp)
  double stepwidth = 0.001;  // in m
  if (default_n_samp) n_samp = circumference/stepwidth;
  
  // check fmax and n_samp
  if (fmax_x >= n_samp/2 || fmax_z >= n_samp/2) {
    cout << "ERROR: The maximum frequency is to large to be calculated with "<<n_samp<<" sampling points." << endl;
    return 1;
  }
  
  // write n_samp to metadata
  snprintf(tmp, 100, "%d points per turn", n_samp);
  metadata.add("Field sampling", tmp);
  
  
  // initialize Lattice
  AccLattice lattice(circumference, end); // refPos=end used by MAD-X
  AccLattice Ref_lattice(circumference, end);

  // initialize orbit
  FunctionOfPos<AccPair> bpmorbit(circumference, gsl_interp_akima_periodic, circumference);
  FunctionOfPos<AccPair> Ref_bpmorbit(circumference, gsl_interp_akima_periodic, circumference);
  FunctionOfPos<AccPair> trajectory(circumference, gsl_interp_akima);



  // output
  cout << endl;
  cout << "--------------------------------------------" << endl;
  cout << "Bsupply: calculate magnetic field & spectrum" << endl;
  if (elsa) cout << "         ELSA-mode" << endl;
  if (diff) cout << "         harmcorr analysis (difference-mode)" << endl;
  cout << "                    version:" <<endl<<gitversion() << endl;
  cout << "--------------------------------------------" << endl;
  cout << "* "<<n_samp<<" sampling points along ring" << endl;
  cout << "* maximum frequency used for B-field evaluation:  Bx->"<<fmax_x <<", Bz->"<<fmax_z << endl;
  cout << "* frequency components cutted if amplitude below: Bx->"<<ampcut_x<<" 1/m, Bz->"<<ampcut_z<< " 1/m" << endl;


  // read lattice (magnet positions,strengths,misalignments) and  particle orbit
  if (simTool == madx) {
    lattice.madximport(file.lattice.c_str());
    lattice.madximportMisalignments(file.misalign_dip.c_str());
    bpmorbit.madxClosedOrbit(file.orbit.c_str());
  }
  else { //elegant
    lattice.elegantimport(file.lattice.c_str());
    //lattice.madximportMisalignments(file.misalign_dip.c_str());
    bpmorbit.elegantClosedOrbit(file.orbit.c_str());
  }

  // elsa=true: quad-&sext-strengths, BPM- & corrector-data from ELSA "Spuren"
  if (elsa) {
    try {
      lattice.setELSAoptics(file.spuren.c_str());
	} catch (std::runtime_error &e) {
      cout << e.what() << endl;
      return 1; 
    }
    // get orbit and vcorrs from ELSA "Spuren" (for all times t) 
    ELSAimport_bpms(ELSAbpms, file.spuren.c_str());
    ELSAimport_vcorrs(ELSAvcorrs, file.spuren.c_str());
    if (diff) {
      ELSAimport_bpms(Ref_ELSAbpms, file.orbit_ref.c_str());
      ELSAimport_vcorrs(Ref_ELSAvcorrs, file.lattice_ref.c_str());
      Ref_lattice = lattice; // take names,positions,lengths
    }
  }

  cout << "* "<<lattice.size(dipole)<<" dipoles, "<<lattice.size(quadrupole)<<" quadrupoles, "
       <<lattice.size(sextupole)<<" sextupoles, "<<lattice.size(corrector)<<" kickers read"<<endl
       <<"  from "<<file.lattice<<endl
       << "* "<<bpmorbit.samples()<<" BPMs(@Quad) read"<<endl
       <<"  from "<<file.orbit << endl;

  if (tracking) {
    if (simTool == madx)
      trajectory.madxTrajectory(file, particle);
    else //elegant
      trajectory.elegantTrajectory(file, particle);
    cout << "* trajectory of particle "<<particle<<" read at "<<trajectory.samples()
	 <<" observation points for "<<trajectory.turns()<<" turns"<<endl;
  }
  cout << "--------------------------------------------" << endl;



  // magnetic field along ring, [B]=1/m (factor gamma*m*c/e multiplied in TBMTsolver)
  Field B(circumference, n_samp, trajectory.turns());

  // resonance strengths
  RESONANCES Res(dtheta, lattice.size(dipole), trajectory.turns());
  // check fmax for resonance strengths (if not set by -F (or to large): set to maximum)
  if (dtheta != -1) {
    ftmp = int(abs(360/dtheta / 2.0));
    if (fmax_res==0 || fmax_res>ftmp)  fmax_res = ftmp;
  }

  


  // following part of the program can be executed multiple times for different times t,
  // which is only relevant for ELSA-mode. Else, the loop is executed only once (t.size()=1).
  for(i=0; i<t.size(); i++) {

    // ELSA-mode: read orbit & corrector data for time t
    if (elsa) {
      if (t.get(i) < 0) {
	cout << "ERROR: t = "<<t.get(i)<<" < 0 is no valid moment of ELSA cycle." << endl;
	return 1;
      }
      metadata.setbyLabel("Time in cycle", t.label(i));
      try {
	bpmorbit.elsaClosedOrbit(ELSAbpms, t.get(i));
	lattice.setELSACorrectors(ELSAvcorrs, t.get(i));
      } catch (exception &e) {
	cout << e.what();
	exit(1);
      }
      cout << "* "<<t.label(i)<<": "<<lattice.size(corrector)<<" correctors and "
	   <<bpmorbit.samples()<<" BPMs read"<<endl<<"  from "<<file.spuren << endl;
    }

    // difference-mode: read reference orbit & corrector data
    if (diff) {
      if (elsa) {
	Ref_bpmorbit.elsaClosedOrbit(Ref_ELSAbpms, t.get(i));
	Ref_lattice.setELSACorrectors(Ref_ELSAvcorrs, t.get(i));
	cout << "* "<<t.label(i)<<": ";
	  }
      else {
	if (simTool == madx) {
	  Ref_bpmorbit.madxClosedOrbit(file.orbit_ref.c_str());
	  Ref_lattice.madximport(file.lattice_ref.c_str());
	  Ref_lattice.madximportMisalignments(file.misalign_dip_ref.c_str());
	}
	else { //elegant
	  Ref_bpmorbit.elegantClosedOrbit(file.orbit_ref.c_str());
	  Ref_lattice.elegantimport(file.lattice_ref.c_str());
	}
	cout << "* ";
      }
      cout <<Ref_lattice.size(dipole)<<" dipoles, "<<Ref_lattice.size(quadrupole)<<" quadrupoles, "
	   <<Ref_lattice.size(sextupole)<<" sextupoles, "<<Ref_lattice.size(corrector)<<" kickers read"<<endl
	   <<"  from "<<file.lattice_ref<<endl
	   <<"* "<<Ref_bpmorbit.samples()<<" BPMs(@Quad) read"<<endl
	   <<"  from "<< file.orbit_ref << endl;
    }
    
    // difference-mode: subtract reference orbit & corrector data
    if (diff) {
      bpmorbit -= Ref_bpmorbit;
      lattice.subtractCorrectorStrengths(Ref_lattice);
      lattice.subtractMisalignments(Ref_lattice);
    }

    // ======= most important feature: =======
    // calculate field distribution & spectrum

    // if tracking is used (tracking-mode), orbit = trajectory + closed orbit
    // else, orbit = closed orbit
    if (tracking) {
      trajectory += bpmorbit; // add closed orbit for every turn (trajectory coord. relative to C.O.)
      orbit = &trajectory;
    }
    else {
      orbit = &bpmorbit;
    }

    cout << "Calculate field distribution..." << endl;
    B.set(lattice, *orbit, n_samp);        // calculate field distributions (s)
    Res.set(lattice, *orbit);      // calculate Resonances (theta)

    cout << "Calculate spectra (FFT)..." << endl;
    Spectrum bx = B.getSpectrum(x, fmax_x, ampcut_x);
    Spectrum bz = B.getSpectrum(z, fmax_z, ampcut_z);
    Spectrum bs = B.getSpectrum(s, fmax_s);
    Spectrum res = Res.getSpectrum(fmax_res, ampcut_res);
    cout << "--------------------------------------------" << endl;
    // =======================================





    // generate output files
    if (allout) {
      //lattice
      lattice.print(file.out("lattice", t.tag(i)).c_str());
      //BPM data
      bpmorbit.out(file.out("bpms", t.tag(i)).c_str());
      trajectory.out(file.out("trajectory", t.tag(i)).c_str());
      //corrector data
      lattice.print(corrector, file.out("vcorrs", t.tag(i)).c_str());
      //orbit data (interpolated BPMs)
      bpmorbit.interp_out(0.1, file.out("interp_bpms", t.tag(i)).c_str());
      //field data (if tracking: file may be to large)
      if (!tracking)
	B.out(file.out("fields", t.tag(i)).c_str());
      //evaluated field data
      bx.eval_out(0.1, B.circ, file.out("eval_x", t.tag(i)).c_str());
      bz.eval_out(0.1, B.circ, file.out("eval_z", t.tag(i)).c_str());
      //check dipole lengths
      B.magnetlengths(lattice, file.out("dipolelengths", t.tag(i)).c_str());
    }

    //export spectrum files for polarization-calculation (TBMTsolver)
    bx.out( file.spec("horizontal", t.tag(i)).c_str(), metadata.get(bx, "horizontal") );
    bz.out( file.spec("vertical", t.tag(i)).c_str(), metadata.get(bz, "vertical") );
    bs.out( file.spec("longitudinal", t.tag(i)).c_str(), metadata.get(bs, "longitudinal") );
    if (Res.on) {
      //resonance spectrum (=resonance strengths)
      res.out( file.spec("resonances", t.tag(i)).c_str(), metadata.get(res, "resonances") );
      //kicks as function of spin phaseadvance theta (total, vcorr, quad)
      Res.out(file.out("resonances", t.tag(i)).c_str());
    }


    cout << "--------------------------------------------" << endl;
  }

  cout << "Finished. (Output in "<<file.path<<"/inout/)" << endl << endl;

  return 0;
}


