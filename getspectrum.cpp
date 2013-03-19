/* create spectrum of magnetic fields Bx & Bz along ring by FFT */
/* 31.01.2012 - J.Schmidt */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>
#include "constants.hpp"
#include "types.hpp"
#include "resonances.hpp"
#include "fieldmap.hpp"
#include "spectrum.hpp"
#include "getspectrum.hpp"

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

using namespace std;

int getspectrum (SPECTRUM &bx, SPECTRUM &bz, SPECTRUM &res, FIELDMAP &B, RESONANCES &Res)
{

  unsigned int i;
  unsigned int n_res = Res.size();
  double *BX = new double[2*B.size()];
  double *BZ = new double[2*B.size()];
  double *RES = new double[2*n_res];
  double dfreq;

  if (Res.n_turns != B.n_turns) {
    cout << "ERROR: getspectrum(): FIELDMAP and RESONANCES have different number of turns ("
	<<B.n_turns<<", "<<Res.n_turns<<")." << endl;
   return 1;
  }


  // set real parts
  for (i=0; i<B.size(); i++) {
    REAL(BX,i) = B.x(i);
    IMAG(BX,i) = 0;
    REAL(BZ,i) = B.z(i);
    IMAG(BZ,i) = 0;
  }
  for (i=0; i<n_res; i++) {
    REAL(RES,i) = Res.getkick(i);
    IMAG(RES,i) = 0;
  }

  dfreq = SPEED_OF_LIGHT/B.circumference/B.n_turns;
  fft(bx, BX, B.size(), B.size(), dfreq);
  fft(bz, BZ, B.size(), B.size(), dfreq);
  dfreq = 1.0/Res.n_turns;   //1.0/360;
  fft(res, RES, n_res, Res.ndipols()*Res.n_turns, dfreq); //normalize on # dipoles for correct kicks in-between

  delete[] BX;
  delete[] BZ;
  delete[] RES;

  return 0;
}




/* creates magnetic spectrum bx of field BX by GSL complex FFT */
int fft (SPECTRUM &bx, double *BX, int n_samp, int norm, double dfreq)
{

  unsigned int i;
  FREQCOMP btmp;
  gsl_fft_complex_wavetable * wavetable;
  gsl_fft_complex_workspace * workspace;
  wavetable = gsl_fft_complex_wavetable_alloc (n_samp);
  workspace = gsl_fft_complex_workspace_alloc (n_samp);
  gsl_fft_complex_forward (BX, 1, n_samp, wavetable, workspace);       /* transform BX */

  /*  write amplitude & phase to SPECTRUM */
  btmp.freq = 0.0;
  btmp.amp = sqrt( pow(REAL(BX,0),2) + pow(IMAG(BX,0),2) ) * 1.0/norm;
  btmp.phase = 0.0;
  bx.set(0, btmp);
  for (i=1; i<=bx.fmax; i++) {
    btmp.freq = i*dfreq;
    btmp.amp = sqrt( pow(REAL(BX,i),2) + pow(IMAG(BX,i),2) ) * 2.0/norm;
    btmp.phase = atan( IMAG(BX,i) / REAL(BX,i) );
    if (btmp.amp<MIN_AMPLITUDE) {                                     /* arbitrary phase for amp=0: set phase=0 */
      btmp.phase = 0.0;
    }
    else if (REAL(BX,i)<0.0 || (REAL(BX,i)==0.0 && IMAG(BX,i)<0.0)) {  /* adjust phase to [0,2pi] degree */
      btmp.phase += M_PI;
    }
    else if (REAL(BX,i)>0.0 && IMAG(BX,i)<=0.0) {
      btmp.phase += 2*M_PI;
    }
    bx.set(i, btmp);
  }

  gsl_fft_complex_wavetable_free (wavetable);
  gsl_fft_complex_workspace_free (workspace);

  return 0;
}




/* returns cos-evaluation of magnetic field BX at time t by using spectrum bx */
double eval(SPECTRUM bx, double t)
{
  double value=0.0;
  unsigned int f;

  for (f=0; f<=bx.fmax; f++) {
    value += bx.amp(f)*cos(bx.freq(f)*t + bx.phase(f));
  }

  return value;
}





/* create output file with evaluated field data */
int eval_out(SPECTRUM bx, SPECTRUM bz, unsigned int size, double circumference, const char *filename)
{
 unsigned int i;
 int w=12;
 double delta_s = circumference/n_samp;
 double delta_t = delta_s/SPEED_OF_LIGHT;
 fstream file;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: Cannot open " << filename << "." << endl;
   return 1;
 }
 
 file <<"# "<<setw(w)<< "s [m]" <<setw(w)<< "t [s]" <<setw(w)<< "Bx [1/m]" <<setw(w)<< "Bz [1/m]" <<setw(w)<< "Bs [1/m]" << endl;
 for (i=0; i<size; i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
   file <<setw(w+2)<< i*delta_s <<setw(w)<< i*delta_t <<setw(w)<< eval(bx, i*delta_t) <<setw(w)<< eval(bz, i*delta_t) <<setw(w)<< 0.0 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
