#ifndef __POLSIM__MADXIMPORT_HPP_
#define __POLSIM__MADXIMPORT_HPP_

int madximport(const char *filename, ORBIT &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs);

int misalignments(const char *filename, magnetvec &dipols);

#endif

/*__POLSIM__MADXIMPORT_HPP_*/
