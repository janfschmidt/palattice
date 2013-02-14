/* magnetic field spectrum */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "types.hpp"
#include "spectrum.hpp"

using namespace std;



//copy constructor
SPECTRUM::SPECTRUM(const SPECTRUM &other)
  : fmax(other.fmax), turns(other.turns)
{
  unsigned int i;

  b = new FREQCOMP[this->size()];

  for (i=0; i<this->size(); i++) {
    b[i].freq = other.freq(i);
    b[i].amp = other.amp(i);
    b[i].phase = other.phase(i);
  }
}



int SPECTRUM::set(unsigned int i, FREQCOMP tmp)
{
  b[i].freq = tmp.freq;
  b[i].amp = tmp.amp;
  b[i].phase = tmp.phase;

  return 0;
}
