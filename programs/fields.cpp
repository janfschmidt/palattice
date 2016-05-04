/* libpalattice expamle program: magnetic fields
 * get magnetic fields for a lattice, depending on closed orbit and calculate spectrum (FFT)
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

#include <libpalattice/AccLattice.hpp>
#include <libpalattice/FunctionOfPos.hpp>
#include <libpalattice/Field.hpp>
#include <libpalattice/ResStrengths.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc<2) {
    cout << "please give madx lattice file as argument." << endl;
    return 1;
  }
  string latticefile = argv[1];

  
  //import lattice and closed orbit
  pal::SimToolInstance sim(pal::madx, pal::online, latticefile);
  pal::AccLattice elsa(sim);

  pal::FunctionOfPos<pal::AccPair> orbit(sim);
  orbit.simToolClosedOrbit(sim);

  double s = 26.23;

  int samples = 16440;
  pal::Field B(elsa.circumference());


  //access field of element
  cout << "field of element (no edges) at s=" << s << "m: " << elsa[s]->B(orbit.interp(s)) << endl;
  //calc. field of one revolution
  B.set(elsa, orbit, samples, false);
  B.print("fields_noedge.dat");

  //access field of lattice (incl. edge fields, default)
  cout << "field of lattice (with edges) at s=" << s << "m: " << elsa.B(s, orbit.interp(s)) << endl;
  //calc. field of one revolution
  B.set(elsa, orbit, samples, true);
  B.print("fields_edge.dat");

  //Spectrum of horizontal (pal::x) field
  unsigned int fmax = 60;
  pal::Spectrum fft = B.getSpectrum(pal::x,fmax);  // Field already has equidistant sampling, so no new sampling for FFT needed (in contrast to interptwiss.cpp)
  fft.print("fields_x.spectrum");

  // depolarizing Resonance Strengths
  pal::ResStrengths res(&elsa, &orbit);
  std::cout << "ResStrength: res[5]=" << res[5] << std::endl;
  res.print(0.05,7, "fields.resonances");
  
  // test theta(s)
  // for (double s=0; s<164.4; s+=0.5) {
  //   std::cout << elsa[s]->name <<"\t"<< elsa.theta(s)*180./M_PI/15. << std::endl;
  // }

  return 0;
}
