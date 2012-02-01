#ifndef __POLSIM__GETORBIT_HPP_
#define __POLSIM__GETORBIT_HPP_

/*
#ifdef __cplusplus
extern "C" {
#endif 
*/

int getorbit (orbitvec &orbit , double circumference, orbitvec &bpmorbit, int n_samp);

int orbit_out(orbitvec &orbit, char *filename);


/*
#ifdef __cplusplus
}
#endif 
*/

#endif

/*__POLSIM__GETORBIT_HPP_*/
