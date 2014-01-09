#ifndef __POLSIM__ELSAIMPORT_HPP_
#define __POLSIM__ELSAIMPORT_HPP_

#include "functionofpos.hpp"

int ELSAimport_bpms(BPM *ELSAbpms, const char *spurenFolder);

int ELSAimport_vcorrs(CORR *ELSAvcorrs, const char *spurenFolder);

int corrs_out(magnetvec vcorrs, const char *filename);

#endif

/*__POLSIM__ELSAIMPORT_HPP_*/
