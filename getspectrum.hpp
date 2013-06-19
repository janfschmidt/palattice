#ifndef __POLSIM__GETSPECTRUM_HPP_
#define __POLSIM__GETSPECTRUM_HPP_

#include "resonances.hpp"
#include "spectrum.hpp"
#include "fieldmap.hpp"

int getspectrum (SPECTRUM &bx, SPECTRUM &bz, SPECTRUM &res, FIELDMAP &B, RESONANCES &Res);

int fft (SPECTRUM &bx, double *BX, int n_samp, int norm, double dfreq);


#endif

/*__POLSIM__GETSPECTRUM_HPP_*/
