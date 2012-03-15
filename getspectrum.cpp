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
#include "getspectrum.hpp"

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

using namespace std;

int getspectrum (SPECTRUM *bx, SPECTRUM *bz, FIELD *B, int n_samp, int fmax_x, int fmax_z, double circumference)
{

  int i;
  double *BX = new double[2*n_samp];
  double *BZ = new double[2*n_samp];

  //memset(BX, 0, 2*n_samp*sizeof(double));
  //memset(BZ, 0, 2*n_samp*sizeof(double));

  for (i=0; i<n_samp; i++) { /* set real parts */
    REAL(BX,i) = B[i].x;
    IMAG(BX,i) = 0;
    REAL(BZ,i) = B[i].z;
    IMAG(BZ,i) = 0;
  }

  fft(bx, BX, n_samp, fmax_x, circumference);
  fft(bz, BZ, n_samp, fmax_z, circumference);

  delete[] BX;
  delete[] BZ;

  return 0;
}




/* creates magnetic spectrum bx of field BX by GSL complex FFT */
int fft (SPECTRUM *bx, double *BX, int n_samp, int fmax, double circumference)
{

  int i;
  gsl_fft_complex_wavetable * wavetable;
  gsl_fft_complex_workspace * workspace;
  wavetable = gsl_fft_complex_wavetable_alloc (n_samp);
  workspace = gsl_fft_complex_workspace_alloc (n_samp);
  gsl_fft_complex_forward (BX, 1, n_samp, wavetable, workspace);       /* transform BX */

  /*  write amplitude & phase to SPECTRUM */
  bx[0].omega = 0.0;
  bx[0].amp = sqrt( pow(REAL(BX,0),2) + pow(IMAG(BX,0),2) ) * 1.0/n_samp;
  bx[0].phase = 0.0;
  for (i=1; i<=fmax; i++) {
    bx[i].omega = i*2*M_PI*SPEED_OF_LIGHT/circumference;
    bx[i].amp = sqrt( pow(REAL(BX,i),2) + pow(IMAG(BX,i),2) ) * 2.0/n_samp;
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
    value += bx[f].amp*cos(bx[f].omega*t + bx[f].phase);
  }

  return value;
}





/* create output file with evaluated field data */
int eval_out(SPECTRUM *bx, SPECTRUM *bz, int fmax_x, int fmax_z, int n_samp, double circumference, char *filename)
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
 
 file <<setw(w)<< "s [m]" <<setw(w)<< "Bx [1/m]" <<setw(w)<< "Bz [1/m]" << endl;
 for (i=0; i<n_samp; i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
   file <<setw(w)<< i*delta_s <<setw(w)<< eval(bx, fmax_x, i*delta_t) <<setw(w)<< eval(bz, fmax_z, i*delta_t) << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
