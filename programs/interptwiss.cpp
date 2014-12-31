/* -----interptwiss-----
 * libpal example program
 * interpolate twiss parameters from madx or elegant using libpal
 * ... and calculate spectrum (FFT)
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <getopt.h>
#include <libpal/libpal.hpp>

using namespace std;


int main ()
{
  string out = "twiss";
  pal::SimTool tool = pal::elegant;       // choose madx or elegant
  pal::SimToolMode mode = pal::online; // SimTool is executed automatically
  string latticefile = "../lattice/ELSA/elsa.lte";


  // construct SimToolInstance for a lattice file
  pal::SimToolInstance sim(tool, mode, latticefile);

  // construct FunctionOfPos container (here used for a twiss variable)
  // it reads the lattice circumference from the SimTool output files (runs SimTool, if mode=online)
  pal::FunctionOfPos<double> twiss(sim);
  //pal::FunctionOfPos<pal::AccPair> twiss(sim);

  //write the name of the twiss variable to a string vector
  //vector, because it allows 2 (3) columns for FunctionOfPos<AccPair> (<AccTriple>)
  string s;
  vector<string> value;
  if (tool==madx) {
    s = "S";    
    value.push_back("BETX");
    //value.push_back("BETY");
  }
  else if (tool==elegant) {
    s = "s";
    value.push_back("betax");
    //value.push_back("betay");
  }

  // read a twiss variable from a SimTool output file (runs SimTool, if mode=online and not run before)
  twiss.readSimToolColumn(sim, sim.twiss(), s, value);

 // some info output examples
  cout << endl<< "some info about FunctionOfPos<double> twiss:" << endl;
  cout << "circumference=" << twiss.circ << endl; 
  cout << "turns=" << twiss.turns() << endl; 
  cout << "samples=" << twiss.samples() << endl;
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
