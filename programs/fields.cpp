/* ===libpalattice expamle program: fields===
 * get magnetic fields for a lattice, depending on closed orbit
 * ... and calculate spectrum (FFT)
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include "libpalattice/AccLattice.hpp"
#include "libpalattice/FunctionOfPos.hpp"
#include "libpalattice/Field.hpp"
#include "libpalattice/ResStrengths.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  //import lattice and closed orbit
  string latticefile = argv[1];
  pal::SimToolInstance sim(pal::madx, pal::online, latticefile);
  pal::AccLattice elsa("ELSA", sim);

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
  std::cout << res[5] << std::endl;
  res.print(0.05,7, "fields.resonances");
  
  // test theta(s)
  // for (double s=0; s<164.4; s+=0.5) {
  //   std::cout << elsa[s]->name <<"\t"<< elsa.theta(s)*180./M_PI/15. << std::endl;
  // }

  return 0;
}
