#include "libpal/AccLattice.hpp"
#include "libpal/FunctionOfPos.hpp"
#include "libpal/Field.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
  //import lattice and closed orbit
  pal::SimToolInstance sim(pal::elegant, pal::online, "../lattice/ELSA/elsa.lte");
  pal::AccLattice elsa("ELSA", sim);

  pal::FunctionOfPos<pal::AccPair> orbit(sim);
  orbit.simToolClosedOrbit(sim);

  double s = 26.23;

  int samples = 16440;
  pal::Field B(elsa.circumference(), samples);


  //access field of element
  cout << "field of element at s=" << s << "m: " << elsa[s]->B(orbit.interp(s)) << endl;
  //calc. field of one revolution
  B.set(elsa, orbit, samples, false);
  B.print("fields_noedge.dat");

  //access field of lattice (incl. edge fields, default)
  cout << "field of lattice at s=" << s << "m: " << elsa.B(s, orbit.interp(s)) << endl;
  //calc. field of one revolution
  B.set(elsa, orbit, samples, true);
  B.print("fields_edge.dat");

  //Spectrum of horizontal (pal::x) field
  unsigned int fmax = 60;
  pal::Spectrum fft = B.getSpectrum(pal::x,fmax);  // Field already has equidistant sampling, so no new sampling for FFT needed (in contrast to interptwiss.cpp)
  fft.print("fields_x.spectrum");

  return 0;
}
