/* === Spectrum Class ===
 * Frequency spectrum of arbitrary data.
 * Spectrum is calculated via gsl FFT in class constructor.
 * Data can be given as vector<double> (and a "length" in pos/time),
 * as FunctionOfPos<T> for an accelerator (automatic calc. of rev.freq. etc)
 * or as RESONANCES for resonance-strengths (Bsupply).
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#ifndef __LIBPAL_SPECTRUM_HPP_
#define __LIBPAL_SPECTRUM_HPP_

#include <vector>
#include <string>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_const_mksa.h>
#include "Metadata.hpp"
#include "config.hpp"

using namespace std;


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

  void out(string filename="");
  void eval_out(double stepwidth, double max, string filename);
  void updateMetadata();
};




#endif
/*__LIBPAL_SPECTRUM_HPP_*/
