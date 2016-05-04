/* libpalattice expamle program: twiss parameter interpolation
 * interpolate twiss parameters from madx or elegant and calculate spectrum (FFT)
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <getopt.h>
#include <libpalattice/FunctionOfPos.hpp>
#include <libpalattice/Spectrum.hpp>

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

  //write the name of the twiss variable(s)
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
  cout << "circumference=" << twiss.circumference() << endl; 
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
  fft.eval_out(step,twiss.circumference(),out+".eval");

  return 0;
}
