#ifndef __BSUPPLY__GETSPECTRUM_HPP_
#define __BSUPPLY__GETSPECTRUM_HPP_

#include "resonances.hpp"
#include "spectrum.hpp"
#include "fieldmap.hpp"
#include "field.hpp"

int getspectrum (SPECTRUM &bx, SPECTRUM &bz, SPECTRUM &res, Field &B, RESONANCES &Res);

int fft (SPECTRUM &bx, double *BX, int n_samp, int norm, double dfreq);


#endif

/*__BSUPPLY__GETSPECTRUM_HPP_*/
