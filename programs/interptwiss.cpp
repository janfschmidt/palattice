/* -----interptwiss-----
 * interpolate twiss parameters from madx or elegant using libpal
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <getopt.h>
#include <gsl/gsl_spline.h>
#include <libpal/libpal.hpp>

using namespace std;


int main ()
{
  string out = "interptwiss";
  double step=0.1;
  pal::SimTool tool = pal::madx;
  pal::SimToolMode mode = pal::online;


  // construct SimToolInstance for a lattice file
  pal::SimToolInstance sim(tool, mode, "/home/schmidt/ELSA/lattice/ELSA/elsa.madx");

  // read circumference from a SimTool output file (runs SimTool, if mode=online)
  double circ = sim.readCircumference();

  // read a twiss variable from a SimTool output file (runs SimTool, if mode=online)
  pal::FunctionOfPos<double> twiss(circ, gsl_interp_akima_periodic);
  vector<string> value;
  if (tool==madx) value.push_back("BETX");
  else if (tool==elegant) value.push_back("betax");
  twiss.readSimToolColumn(sim, sim.twiss(), "S", value);

  cout << "turns=" << twiss.turns() << endl; 


  // twiss output
  twiss.print(out+".dat");

  // interpolated twiss output
  twiss.interp_out(step, out+"_interp.dat");


  return 0;
}
