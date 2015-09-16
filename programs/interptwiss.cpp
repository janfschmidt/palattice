/* ===libpalattice expamle program: interptwiss===
 * interpolate twiss parameters from madx or elegant using libpalattice
 * ... and calculate spectrum (FFT)
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include <iostream>
#include <getopt.h>
#include <libpalattice/libpalattice.hpp>

using namespace std;


int main (int argc, char *argv[])
{
  if (argc<2) {
    cout << "please give elegant lattice file as argument." << endl;
    return 1;
  }
  string latticefile = argv[1];

  
  string out = "twiss";
  pal::SimTool tool = pal::elegant;       // choose madx or elegant
  pal::SimToolMode mode = pal::online; // SimTool is executed automatically


  // construct SimToolInstance for a lattice file
  pal::SimToolInstance sim(tool, mode, latticefile);

  // construct FunctionOfPos container (here used for a twiss variable)
  // it reads the lattice circumference from the SimTool output files (runs SimTool, if mode=online)
  pal::FunctionOfPos<double> twiss(sim);
  //pal::FunctionOfPos<pal::AccPair> twiss(sim);

  //write the name of the twiss variable to a string vector
  //vector, because it allows 2 (3) columns for FunctionOfPos<AccPair> (<AccTriple>)
  string s;
  string valueX, valueZ;
  if (tool==madx) {
    s = "S";    
    valueX = "BETX";
    //valueZ = "BETY";
  }
  else if (tool==elegant) {
    s = "s";
    valueX = "betax";
    //valueZ = "betay";
  }

  // read a twiss variable from a SimTool output file (runs SimTool, if mode=online and not run before)
  // 1-3 value strings can be given (2 for AccPair, 3 for AccTriple, 1 else)
  // needless ones are ignored. missing ones cause throwing a palatticeError
  twiss.readSimToolColumn(sim, sim.twiss(), s, valueX, valueZ);

 // some info output examples
  cout << endl<< "some info about FunctionOfPos<double> twiss:" << endl;
  cout << "circumference=" << twiss.circ << endl; 
  cout << "turns=" << twiss.turns() << endl; 
  cout << "samples=" << twiss.samplesInTurn(1) << endl;
  cout << "data range: " << twiss.dataMin() <<" - "<< twiss.dataMax() << endl;
  cout << "interpolation range: " << twiss.interpMin() <<" - "<< twiss.interpMax() << endl << endl;

  // twiss output
  twiss.print(out+".dat");

  // interpolated twiss output
  double step=0.1;
  double pos = 42.42;
  cout << "interpolated data at pos=" << pos <<": " << twiss.interp(pos) << endl;
  twiss.interp_out(step, out+"_interp.dat");

  // calculate spectrum (FFT) of twiss
  unsigned int fmax = 42; // maximum frequency in spectrum / rev. harmonics
  double cut = 0.05;      // frequencies with amplitude < cut are filtered out
  Spectrum fft = twiss.getSpectrum(step, pal::x, fmax, cut);

  // spectrum output
  fft.print(out+".spectrum");
  fft.eval_out(step,twiss.circ,out+".eval");

  return 0;
}
