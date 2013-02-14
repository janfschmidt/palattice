#ifndef __POLSIM__GETSPECTRUM_HPP_
#define __POLSIM__GETSPECTRUM_HPP_

#include "resonances.hpp"

int getspectrum (SPECTRUM *bx, SPECTRUM *bz, SPECTRUM *res, FIELDMAP &B, int fmax_x, int fmax_z, int fmax_res, double circumference, RESONANCES &Res);

int fft (SPECTRUM *bx, double *BX, int n_samp, int norm, int fmax, double dfreq);

double eval(SPECTRUM *bx, int f_max, double t);

int eval_out(SPECTRUM *bx, SPECTRUM *bz, int fmax_x, int fmax_z, int n_samp, double circumference, const char *filename);


#endif

/*__POLSIM__GETSPECTRUM_HPP_*/
