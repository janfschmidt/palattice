#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

#include "functionofpos.hpp"

int difference(const char *ReferenceFolder, unsigned int t, FunctionOfPos<AccPair> &bpmorbit, magnetvec &vcorrs, BPM *Ref_ELSAbpms, CORR *Ref_ELSAvcorrs, bool elsa);

int harmcorr(magnetvec vcorrs, magnetvec quads, FunctionOfPos<AccPair> &orbit, magnetvec dipols, const char *filename);

int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, const char *filename);

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
