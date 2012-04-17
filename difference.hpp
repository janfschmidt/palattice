#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

int difference(char *ReferenceFolder, unsigned int t, double corrlength, orbitvec &bpmorbit, magnetvec &vcorrs, bool elsa);

int harmcorr(SPECTRUM *hc, int fmax_x, magnetvec vcorrs, magnetvec quads, orbitvec orbit, magnetvec dipols, double circumference, int n_samp, char *filename);

int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, char *filename);

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
