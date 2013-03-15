#ifndef __POLSIM__GETFIELDS_HPP_
#define __POLSIM__GETFIELDS_HPP_

#include "resonances.hpp"
#include "types.hpp"

int getfields (FIELDMAP &B, double circumference, CLOSEDORBIT bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs, RESONANCES &Res);

int fields_out(FIELDMAP B, const char *filename);

#endif

/*__POLSIM__GETFIELDS_HPP_*/
