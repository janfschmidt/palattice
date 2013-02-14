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
#include "getspectrum.hpp"

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

using namespace std;

int getspectrum (SPECTRUM *bx, SPECTRUM *bz, SPECTRUM *res, FIELDMAP &B, int fmax_x, int fmax_z, int fmax_res, double circumference, RESONANCES &Res)
{

  unsigned int i;
  unsigned int n_res = Res.size();
  double *BX = new double[2*B.n_samp];
  double *BZ = new double[2*B.n_samp];
  double *RES = new double[2*n_res];
  double dfreq;

  // set real parts
  for (i=0; i<B.n_samp; i++) {
    REAL(BX,i) = B.x(i);
    IMAG(BX,i) = 0;
    REAL(BZ,i) = B.z(i);
    IMAG(BZ,i) = 0;
  }
  for (i=0; i<n_res; i++) {
    REAL(RES,i) = Res.getkick(i);
    IMAG(RES,i) = 0;
  }

  dfreq = SPEED_OF_LIGHT/circumference;
  fft(bx, BX, B.n_samp, B.n_samp, fmax_x, dfreq);
  fft(bz, BZ, B.n_samp, B.n_samp, fmax_z, dfreq);
  dfreq = 1.0;   //1.0/360;
  fft(res, RES, n_res, Res.ndipols(), fmax_res, dfreq); //normalize on # dipoles for correct kicks in-between

  delete[] BX;
  delete[] BZ;
  delete[] RES;

  return 0;
}




/* creates magnetic spectrum bx of field BX by GSL complex FFT */
int fft (SPECTRUM *bx, double *BX, int n_samp, int norm, int fmax, double dfreq)
{

  int i;
  gsl_fft_complex_wavetable * wavetable;
  gsl_fft_complex_workspace * workspace;
  wavetable = gsl_fft_complex_wavetable_alloc (n_samp);
  workspace = gsl_fft_complex_workspace_alloc (n_samp);
  gsl_fft_complex_forward (BX, 1, n_samp, wavetable, workspace);       /* transform BX */

  /*  write amplitude & phase to SPECTRUM */
  bx[0].freq = 0.0;
  bx[0].amp = sqrt( pow(REAL(BX,0),2) + pow(IMAG(BX,0),2) ) * 1.0/norm;
  bx[0].phase = 0.0;
  for (i=1; i<=fmax; i++) {
    bx[i].freq = i*dfreq;
    bx[i].amp = sqrt( pow(REAL(BX,i),2) + pow(IMAG(BX,i),2) ) * 2.0/norm;
    bx[i].phase = atan( IMAG(BX,i) / REAL(BX,i) );
    if (bx[i].amp<MIN_AMPLITUDE) {                                     /* arbitrary phase for amp=0: set phase=0 */
      bx[i].phase = 0.0;
    }
    else if (REAL(BX,i)<0.0 || (REAL(BX,i)==0.0 && IMAG(BX,i)<0.0)) {  /* adjust phase to [0,2pi] degree */
      bx[i].phase += M_PI;
    }
    else if (REAL(BX,i)>0.0 && IMAG(BX,i)<=0.0) {
      bx[i].phase += 2*M_PI;
    }
  }

  gsl_fft_complex_wavetable_free (wavetable);
  gsl_fft_complex_workspace_free (workspace);

  return 0;
}




/* returns cos-evaluation of magnetic field BX at time t by using spectrum bx */
double eval(SPECTRUM *bx, int fmax, double t)
{
  double value=0.0;
  int f;

  for (f=0; f<=fmax; f++) {
    value += bx[f].amp*cos(bx[f].freq*t + bx[f].phase);
  }

  return value;
}





/* create output file with evaluated field data */
int eval_out(SPECTRUM *bx, SPECTRUM *bz, int fmax_x, int fmax_z, int n_samp, double circumference, const char *filename)
{
 int i;
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
 for (i=0; i<n_samp; i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
   file <<setw(w+2)<< i*delta_s <<setw(w)<< i*delta_t <<setw(w)<< eval(bx, fmax_x, i*delta_t) <<setw(w)<< eval(bz, fmax_z, i*delta_t) <<setw(w)<< 0.0 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
