/* magnetic field spectrum */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include "types.hpp"
#include "constants.hpp"
#include "spectrum.hpp"

using namespace std;



//copy constructor
// SPECTRUM::SPECTRUM(const SPECTRUM &other)
//   : fmax(other.fmax), turns(other.turns)
// {
//   unsigned int i;

//   b = new FREQCOMP[this->size()];

//   for (i=0; i<this->size(); i++) {
//     b[i].freq = other.freq(i);
//     b[i].amp = other.amp(i);
//     b[i].phase = other.phase(i);
//   }
// }



void SPECTRUM::push_back(FREQCOMP tmp)
{
  if (tmp.amp >= ampcut) b.push_back(tmp);
}


// returns fourier series at time t based on this spectrum
double SPECTRUM::eval(double t) const
{
  double value=0.0;
  unsigned int f;

  for (f=0; f<size(); f++) {
    value += amp(f)*cos(freq(f)*t + phase(f));
  }

  return value;
}


// create output file with evaluated field data (stepwitdh & max given for position s / m)
int SPECTRUM::eval_out(double stepwidth, double max, const char *filename) const
{
  double s;
  int w=12;
  fstream file;
  
  if (stepwidth <= 0.) {
    cout << "ERROR: SPECTRUM::eval_out(): output stepwidth must be > 0.0 (" <<stepwidth<< " is not)" <<endl;
    return 1;
  }
 
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: SPECTRUM::eval_out(): Cannot open " << filename << "." << endl;
    return 1;
  }

  file <<"# "<<setw(w)<< "s [m]" <<setw(w)<< "t [s]" <<setw(w)<< "B [1/m]"  << endl;
  for (s=0.0; s<=max; s+=stepwidth) {
    file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
    file <<setw(w+2)<< s <<setw(w)<< s/SPEED_OF_LIGHT <<setw(w)<< eval(s/SPEED_OF_LIGHT) << endl;
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}
