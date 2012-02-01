#ifndef __POLSIM__GETFIELDS_HPP_
#define __POLSIM__GETFIELDS_HPP_

int getfields (FIELD *B, double circumference, orbitvec &orbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs);

int fields_out(FIELD *B, int n_samp, char *filename);

#endif

/*__POLSIM__GETFIELDS_HPP_*/
