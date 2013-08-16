#ifndef __POLSIM__MADXIMPORT_HPP_
#define __POLSIM__MADXIMPORT_HPP_

#include "filenames.hpp"
#include "functionofpos.hpp"

int madximport(const char *filename, FunctionOfPos<AccPair> &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs);

int misalignments(const char *filename, magnetvec &dipols);

int trajectoryimport(const FILENAMES files, FunctionOfPos<AccPair> &trajectory, unsigned int particle);

#endif

/*__POLSIM__MADXIMPORT_HPP_*/
