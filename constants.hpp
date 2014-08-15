#ifndef __POLSIM__CONSTANTS_H_
#define __POLSIM__CONSTANTS_H_


/* acclattice options */
#define ZERO_DISTANCE 1e-6  // used to compare positions/m in Accelerator (treated as equal if difference is smaller than this number)
#define MIN_AMPLITUDE 1e-10  // For smaller Amplitudes in Field Spectrum the phase is set to 0.0
#define VCPOS_WARNDIFF 0.05  //warning for larger VC pos.diff. in MadX & ELSA-Spuren


#endif

/*__POLSIM__CONSTANTS_H_*/
