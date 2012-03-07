#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

int difference(char *Ref_spurenFolder, char *spuren, char *Ref_spuren, double t, double corrlength, orbitvec &bpmorbit, magnetvec &vcorrs);

int harmcorr_out(magnetvec vcorrs, magnetvec dipols, char *filename);

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
