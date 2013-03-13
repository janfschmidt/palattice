#ifndef __POLSIM__MADXIMPORT_HPP_
#define __POLSIM__MADXIMPORT_HPP_

#include "filenames.hpp"

int madximport(const char *filename, ORBIT &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs);

int misalignments(const char *filename, magnetvec &dipols);

int trajectoryimport(const FILENAMES files, ORBIT &trajectory, unsigned int particle);

#endif

/*__POLSIM__MADXIMPORT_HPP_*/
