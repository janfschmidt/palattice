#ifndef __POLSIM__ELSAIMPORT_HPP_
#define __POLSIM__ELSAIMPORT_HPP_

#include "orbit.hpp"

int ELSAimport(BPM *ELSAbpms, CORR *ELSAvcorrs, const char *spurenFolder);

int ELSAimport_magnetstrengths(magnetvec &quads, magnetvec &sexts, const char *spurenFolder);

int ELSAimport_bpms(BPM *ELSAbpms, const char *spurenFolder);

int ELSAimport_vcorrs(CORR *ELSAvcorrs, const char *spurenFolder);

int ELSAimport_getbpmorbit(BPM *ELSAbpms, ORBIT &bpmorbit, unsigned int t);

int ELSAimport_getvcorrs(CORR *ELSAvcorrs, magnetvec &vcorrs, unsigned int t);

int bpms_out(ORBIT bpmorbit, const char *filename);

int corrs_out(magnetvec vcorrs, const char *filename);

#endif

/*__POLSIM__ELSAIMPORT_HPP_*/
