#ifndef __POLSIM__CONSTANTS_H_
#define __POLSIM__CONSTANTS_H_


/* program options */
#define ZERO_DISTANCE 1e-6  // used to compare positions/m in Accelerator (treated as equal if difference is smaller than this number)
#define MIN_AMPLITUDE 1e-10  // For smaller Amplitudes in Field Spectrum the phase is set to 0.0
#define VCPOS_WARNDIFF 0.05  //warning for larger VC pos.diff. in MadX & ELSA-Spuren

/* Accelerator properties */
#define NBPMS 32        // Number of BPMs for ELSA-Import (Spuren)
#define NVCORRS 32      // Number of VCs for ELSA-Import (Spuren)

/* physical constants */
#define SPEED_OF_LIGHT 299792458  // c [m/s]


#endif

/*__POLSIM__CONSTANTS_H_*/
