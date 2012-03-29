#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

int difference(char *ReferenceFolder, unsigned int t, double corrlength, orbitvec &bpmorbit, magnetvec &vcorrs, bool elsa);

int harmcorr_out(magnetvec vcorrs, magnetvec quads, orbitvec orbit, magnetvec dipols, double sample, char *filename);

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
