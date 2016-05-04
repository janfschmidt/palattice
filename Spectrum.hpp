/* Spectrum Class
 * Frequency spectrum of arbitrary data calculated from input data via FFT.
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
 *
 *
 * Spectrum is calculated via gsl FFT in class constructor.
 * Data can be given as:
 * - vector<double> (and a "length" in pos or time)
 * - as FunctionOfPos<T> for an accelerator (automatic calculation of rev.freq. etc)
 */

#ifndef __LIBPALATTICE_SPECTRUM_HPP_
#define __LIBPALATTICE_SPECTRUM_HPP_

#include <vector>
#include <string>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_const_mksa.h>
#include "Metadata.hpp"
#include "config.hpp"

using namespace std;

namespace pal
{

enum unit{meter, second, degree};


class FREQCOMP {
public:
  double freq;
  double amp;
  double phase;

  FREQCOMP() : freq(0), amp(0), phase(0) {}
};



class Spectrum {

protected:
  vector<FREQCOMP> b;

  unsigned int fMax_rev;   // maximum frequency in rev. harmonics (fmax is highest INCLUDED frequency (**))
  double ampcut;           // amlitudes below this are ignored
  unsigned int turns;      // number of turns in ring
  double circ;             // accelerator ring circumference
  int norm;                // normalization for FFT (amp is devided by norm in fft())

private:
  unit circUnit;
  void fft(vector<double> data);
  double real(unsigned int i, double *halfcomplex, unsigned int n) const;
  double imag(unsigned int i, double *halfcomplex, unsigned int n) const;

public:
  Metadata info;

  Spectrum(string _name, unsigned int fmaxrevIn=30, double ampcut=0);
  Spectrum(string _name, vector<double> In, double circ, unsigned int turns, int _norm=-1, unsigned int fmaxrevIn=30, double ampcut=0, unit u=meter);
  ~Spectrum() {}
  
  inline FREQCOMP get(unsigned int i) const {return b[i];}
  inline double freq(unsigned int i) const {return b[i].freq;}
  inline double amp(unsigned int i) const {return b[i].amp;}
  inline double phase(unsigned int i) const {return b[i].phase;}
  inline double getAmpcut() const {return ampcut;}

  inline unsigned int fMax() const {return turns*fMax_rev;}   // expected/set maximum
  inline unsigned int size() const {return b.size();}         // actual #values (ampcut..)
  double f_rev() const;                                       // revolution frequency
  double dFreq() const;                                       // frequency stepwidth

  double eval(double t) const;        // fourier series at time t / s

  void setAmpcut(double ampcutIn);
  void setFMax_rev(unsigned int fmaxrevIn);
  void setLength(double length, unit u=meter);
  void push_back(FREQCOMP tmp);      // add FREQCOMP manually
  void clear() {b.clear();}

  void print(string filename="");
  void eval_out(double stepwidth, double maxPos, string filename); // fourier series output in [0,maxPos]
  void updateMetadata();
};

} //namespace pal


#endif
/*__LIBPALATTICE_SPECTRUM_HPP_*/
