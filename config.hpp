#ifndef __LIBPAL__CONFIG_HPP_
#define __LIBPAL__CONFIG_HPP_


// acclattice options
#define ZERO_DISTANCE 1e-6   // used to compare positions/m in Accelerator (treated as equal if difference is smaller than this number)
#define MIN_AMPLITUDE 1e-10  // For smaller Amplitudes in Field Spectrum the phase is set to 0.0
#define MIN_EXPORT_TILT 1e-9 // smaller element tilts/rad are ignored during lattice export
#define VCPOS_WARNDIFF 0.05  //warning for larger VC pos.diff. in MadX & ELSA-Spuren
#define DEFAULT_LENGTH_DIFFERENCE 0.09 // default for AccElement "effective-minus-physical" length in m (if no physical length is set)
#define EXPORT_LINE_ELEMENTS_PER_ROW 10

// system commands
#define MADXCOMMAND "madx"        // system command to run MadX
#define ELEGANTCOMMAND "elegant"  // system command to run Elegant

#endif

/*__LIBPAL__CONFIG_HPP_*/
