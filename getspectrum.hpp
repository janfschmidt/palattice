#ifndef __POLSIM__GETSPECTRUM_HPP_
#define __POLSIM__GETSPECTRUM_HPP_


int getspectrum (SPECTRUM *bx, SPECTRUM *bz, FIELD *B, int n_samp, int fmax_x, int fmax_z, double circumference);

int fft (SPECTRUM *bx, double *BX, int n_samp, int fmax, double circumference);

double eval(SPECTRUM *bx, int f_max, double t);

int eval_out(SPECTRUM *bx, SPECTRUM *bz, int fmax_x, int fmax_z, int n_samp, double circumference, const char *filename);


#endif

/*__POLSIM__GETSPECTRUM_HPP_*/
