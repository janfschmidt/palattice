#ifndef __POLSIM__GETFIELDS_HPP_
#define __POLSIM__GETFIELDS_HPP_

#include "resonances.hpp"
#include "types.hpp"

int getfields (FIELD *B, double circumference, orbitvec &orbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs, RESONANCES &Res);

int fields_out(FIELD *B, int n_samp, const char *filename);

#endif

/*__POLSIM__GETFIELDS_HPP_*/
