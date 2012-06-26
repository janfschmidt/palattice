#ifndef __POLSIM__DIFFERENCE_HPP_
#define __POLSIM__DIFFERENCE_HPP_

int difference(char *ReferenceFolder, unsigned int t, orbitvec &bpmorbit, magnetvec &vcorrs, BPM *Ref_ELSAbpms, CORR *Ref_ELSAvcorrs, bool elsa);

int harmcorr(SPECTRUM *hc, unsigned int fmax_hc, magnetvec vcorrs, magnetvec quads, orbitvec orbit, magnetvec dipols, double circumference, int n_samp, char *filename);

int harmcorr_out(double *HCvcorr, double *HCquad, double *HCsum, unsigned int nd, char *filename);

#endif

/*__POLSIM__DIFFERENCE_HPP_*/
