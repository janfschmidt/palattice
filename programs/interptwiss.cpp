/* -----interptwiss-----
 * interpolate twiss parameters from madx or elegant using libpal
 * ... and calculate spectrum (FFT)
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <getopt.h>
//#include <gsl/gsl_spline.h>
#include <libpal/libpal.hpp>

using namespace std;


int main ()
{
  string out = "interptwiss";
  double step=0.1;                     // interpolation output stepwidth
  pal::SimTool tool = pal::madx;       // choose madx or elegant
  pal::SimToolMode mode = pal::online; // SimTool is executed automatically


  // construct SimToolInstance for a lattice file
  pal::SimToolInstance sim(tool, mode, "/home/schmidt/ELSA/lattice/ELSA/elsa.madx");

  // construct FunctionOfPos container (here used for a twiss variable)
  // it reads the lattice circumference from the SimTool output files (runs SimTool, if mode=online)
  pal::FunctionOfPos<double> twiss(sim);

  //write the name of the twiss variable to a string vector
  //vector, because it allows 2 (3) columns for FunctionOfPos<AccPair> (<AccTriple>)
  vector<string> value;
  if (tool==madx) value.push_back("BETX");
  else if (tool==elegant) value.push_back("betax");

  // read a twiss variable from a SimTool output file (runs SimTool, if mode=online and not run before)
  twiss.readSimToolColumn(sim, sim.twiss(), "S", value);

  // twiss output
  twiss.print(out+".dat");

  // interpolated twiss output
  twiss.interp_out(step, out+"_interp.dat");

  // calculate spectrum (FFT) of twiss
  unsigned int fmax = 42; // maximum frequency in spectrum / rev. harmonics
  double cut = 0.1;       // frequencies with amplitude < cut are filtered out
  Spectrum fft = twiss.getSpectrum(fmax, cut);

  // spectrum output
  fft.print(out+".spectrum");


 // some info output examples
  cout <<endl<< "some info about FunctionOfPos<double> twiss:" << endl;
  cout << "circumference=" << twiss.circ << endl; 
  cout << "turns=" << twiss.turns() << endl; 
  cout << "samples=" << twiss.samples() << endl;
  cout << "data range: " << twiss.dataMin() <<" - "<< twiss.dataMax() << endl;
  cout << "interpolation range: " << twiss.interpMin() <<" - "<< twiss.interpMax() << endl;

  return 0;
}
