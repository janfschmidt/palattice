#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

#include "types.hpp"
#include "constants.hpp"
#include "ELSAimport.hpp"
//#include "madximport.hpp"
#include "functionofpos.hpp"
#include "AccLattice.hpp"

int difference(const char *ReferenceFile, unsigned int t, FunctionOfPos<AccPair> &bpmorbit, AccLattice &lattice, BPM *Ref_ELSAbpms, CORR *Ref_ELSAvcorrs, bool elsa);

/*
int harmcorr(magnetvec vcorrs, magnetvec quads, FunctionOfPos<AccPair> &orbit, magnetvec dipols, const char *filename);

int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, const char *filename);
*/

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
