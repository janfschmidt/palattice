/* === Spectrum Class ===
 * Frequency spectrum of arbitrary data.
 * Spectrum is calculated via gsl FFT in class constructor.
 * Data can be given as vector<double> (and a "length" in pos/time),
 * as FunctionOfPos<T> for an accelerator (automatic calc. of rev.freq. etc)
 * or as RESONANCES for resonance-strengths (Bsupply).
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <gsl/gsl_fft_real.h>
#include "Spectrum.hpp"

using namespace std;
using namespace pal;


// -------- constructors -----------


Spectrum::Spectrum(string _name, unsigned int fmaxrevIn, double ampcutIn)
  : fMax_rev(fmaxrevIn), ampcut(ampcutIn), turns(1), circ(1), norm(1), circUnit(meter)
{
  info.add("Spectrum name", _name);
}


Spectrum::Spectrum(string _name, vector<double> In, double c, unsigned int t, int _norm, unsigned int fmaxrevIn, double ampcutIn, unit u)
   : fMax_rev(fmaxrevIn), ampcut(ampcutIn), turns(t), circ(c), norm(_norm), circUnit(u)
{
  info.add("Spectrum name", _name);

  if (_norm == -1) norm = In.size(); // default normaization

  if (fMax() > In.size()/2.) {
    cout << "WARNING: Spectrum constructor: fmax = " <<fMax_rev<< " is to large." << endl
	 << "The " <<In.size()<< " given datapoints allow fmax = " <<  In.size()/2. << ", which is used instead." << endl;
    fMax_rev =  In.size()/2./turns;
  }
  
  fft(In);
}






// gsl complex FFT of double array
// writes data to b (vector<FREQCOMP>)
//void Spectrum::fft(double *data, unsigned int n)
void Spectrum::fft(vector<double> In)
{
  if (fMax() == 0) //no data, no FFT
    return;

  unsigned int i;
  unsigned int n = In.size();
  FREQCOMP btmp;

  double *data = new double[n];
  for (i=0; i<n; i++) {
    data[i] = In[i];
  }

  gsl_fft_real_wavetable * wavetable;
  gsl_fft_real_workspace * workspace;

  wavetable = gsl_fft_real_wavetable_alloc (n);
  workspace = gsl_fft_real_workspace_alloc (n);
  gsl_fft_real_transform (data, 1, n, wavetable, workspace);       // fourier transformation

  // constant component (freq=0):
  btmp.freq = 0.0;
  btmp.amp = sqrt( pow(real(0,data,n),2) + pow(imag(0,data,n),2) ) * 1.0/norm;
  btmp.phase = 0.0;
  if (btmp.amp >= ampcut) b.push_back(btmp);

  // all other freq:
  for (i=1; i<=fMax(); i++) {
    btmp.freq = i*dFreq();
    btmp.amp = sqrt( pow(real(i,data,n),2) + pow(imag(i,data,n),2) ) * 2.0/norm;
    btmp.phase = atan( imag(i,data,n) / real(i,data,n) );

    // arbitrary phase for amp~0. set phase=0:
    if (btmp.amp<MIN_AMPLITUDE) {
      btmp.phase = 0.0;
    }
    // adjust phase to [0,2pi] degree:
    else if (real(i,data,n)<0.0 || (real(i,data,n)==0.0 && imag(i,data,n)<0.0)) {
      btmp.phase += M_PI;
    }
    else if (real(i,data,n)>0.0 && imag(i,data,n)<=0.0) {
      btmp.phase += 2*M_PI;
    }

    if (btmp.amp >= ampcut) b.push_back(btmp);
  }

  delete[] data;
  gsl_fft_real_wavetable_free (wavetable);
  gsl_fft_real_workspace_free (workspace);

  return;
}



// real part of component i of halfcomplex array (created by gsl_real_fft)
double Spectrum::real(unsigned int i, double *hc, unsigned int n) const
{
  if (i>=n)
    throw palatticeError("Spectrum::real(): requested index i is out of given range n.");

  if (i == 0)
    return hc[0];
  else if (i <= n-i)
    return hc[2*i-1];
  else
    return hc[2*(n-i)-1];
}

// imaginary part of component i of halfcomplex array (created by gsl_real_fft)
double Spectrum::imag(unsigned int i, double *hc, unsigned int n) const
{
 if (i>=n)
    throw palatticeError("Spectrum::real(): requested index i is out of given range n.");

  if (i == 0)
    return 0.;
  else if (i < n-i)
    return hc[2*i];
  else if (i == n-i)
    return 0.;
  else
    return hc[2*(n-i)];
}



// revolution frequency
double Spectrum::f_rev() const
{
  switch(circUnit) {
  case meter:
    return GSL_CONST_MKSA_SPEED_OF_LIGHT/circ; //freq in Hz
  case second:
    return 1/circ;                             //freq in Hz
  case degree:
    return 1;                                  // freq in rev. harmonics
  }
  return 0; //to avoid warning
}



 // frequency stepwidth
double Spectrum::dFreq() const
{
  switch(circUnit) {
  case meter:
    return GSL_CONST_MKSA_SPEED_OF_LIGHT/(circ*turns); //freq in Hz
  case second:
    return 1/(circ*turns);                             //freq in Hz
  case degree:
    return 1./turns;                                   // freq in rev. harmonics
  }
  return 0; //to avoid warning
}



void Spectrum::push_back(FREQCOMP tmp)
{
  if (tmp.amp >= ampcut) b.push_back(tmp);
  return;
}



void Spectrum::setAmpcut(double ampcutIn)
{
  if (ampcutIn < ampcut)
    throw palatticeError("Spectrum::setAmpcut(): Amplitude Cut cannot be decreased. Use new Spectrum.");

  ampcut = ampcutIn;
  for (int i=size()-1; i>=0; i--) {
    if (amp(i) < ampcut)
      b.erase(b.begin()+i);
  }
}


void Spectrum::setFMax_rev(unsigned int fmaxrevIn)
{
  if (fMax_rev > fmaxrevIn)
    throw palatticeError("Spectrum::setFMax_rev():fmax cannot be increased. Use new Spectrum.");

  fMax_rev = fmaxrevIn;
  for (unsigned int i=0; i<size(); i++) {
    if( freq(i)/f_rev() > fMax() ) {
      b.erase(b.begin()+i, b.end());
      return;
    }
  }
}



void Spectrum::setLength(double length, unit u)
{
  if (size() > 0)
    throw palatticeError("Spectrum::setLength() Existing components would change their frequency!");
  //or: calculation of new freq. from new/old f_rev() etc.
  //or: clear()

  circ = length;
  turns = 1;
  circUnit = u;
  norm = 1;
}


// returns fourier series at time t based on this spectrum
double Spectrum::eval(double t) const
{
  double value=0.0;
  unsigned int f;

  for (f=0; f<size(); f++) {
    value += amp(f)*cos(2*M_PI*freq(f)*t + phase(f));
  }

  return value;
}




void Spectrum::print(string filename)
{
  fstream file;
  stringstream s;
  const int w = 14;

 //metadata
  updateMetadata();
  s << info.out("#");

 // write spectrum data
 if (circUnit == degree)
   s <<"#"<<setw(w+2)<<"Freq / rev.harm."<<setw(w)<<"Amp / mrad"<<setw(w)<<"Phase / deg" << endl;
 else
   s <<"#"<<setw(w+2)<<"Freq / Hz"<<setw(w)<<"Amp / 1/m"<<setw(w)<<"Phase / deg" << endl;
 
 for (unsigned int i=0; i<size(); i++) {
   s <<resetiosflags(ios::fixed)<<setiosflags(ios::scientific)<<showpoint<<setprecision(8);
   s <<setw(3+w)<< freq(i);
   s <<setprecision(6)<<setw(w)<< amp(i);
   s <<resetiosflags(ios::scientific)<<setiosflags(ios::fixed)<<setprecision(1);
   s <<setw(w)  << phase(i) * 360/(2*M_PI) << endl;
 }


 // output of s
 if (filename == "")
   cout << s.str();
 else {
   file.open(filename.c_str(), ios::out);
   if (!file.is_open()) {
     throw palatticeFileError(filename);
   }
   file << s.str();
   file.close();
   cout << "* Wrote [" <<setw(8)<< size()<< " frequency components] "<< filename  << endl;
 }

}




// create output file with evaluated field data (stepwitdh & max given for position s / m)
void Spectrum::eval_out(double stepwidth, double max, string filename)
{
  double s;
  int w=12;
  fstream file;
  
  if (stepwidth <= 0.) {
    stringstream msg;
   msg << "ERROR: Spectrum::eval_out(): output stepwidth must be > 0.0 (" <<stepwidth<< " is not)" <<endl;
   throw palatticeError(msg.str());
  }
 
  file.open(filename.c_str(), ios::out);
  if (!file.is_open()) {
    throw palatticeFileError(filename);
  }

  //metadata
  updateMetadata();
  file << info.out("#");

  file <<"# "<<setw(w)<< "s / m" <<setw(w)<< "t / s" <<setw(w)<< info.getbyLabel("Spectrum name")  << endl;
  for (s=0.0; s<=max; s+=stepwidth) {
    file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
    file <<setw(w+2)<< s <<setw(w)<< s/GSL_CONST_MKSA_SPEED_OF_LIGHT <<setw(w)<< eval(s/GSL_CONST_MKSA_SPEED_OF_LIGHT) << endl;
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return;
}


void Spectrum::updateMetadata()
{
  char tmp[30];
  if (circUnit != degree) {
    snprintf(tmp, 30, "%.3e", this->f_rev());
    info.add("rev. frequency / Hz", tmp);
  }
  snprintf(tmp, 30, "%d", this->fMax());
  info.add("max. frequency", tmp);
  snprintf(tmp, 30, "%.2e (%d cutted)", this->getAmpcut(), this->fMax()+1-this->size());
  info.add("cutted Amp <", tmp);
  if (info.getbyLabel("Spectrum name")=="harmcorr" || info.getbyLabel("Spectrum name")=="resonances") {
    info.add("WARNING:", "Phase NOT equal harmcorr in ELSA-CCS! (sign in cos)");
  }
}
