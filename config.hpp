/* libpalattice Configuration
 */

#ifndef __LIBPALATTICE__CONFIG_HPP_
#define __LIBPALATTICE__CONFIG_HPP_


#include "systemconfig.hpp"       // system specific config, created by cmake


// system commands:
#define MADXCOMMAND "madx"        // system command to run MadX
#define ELEGANTCOMMAND "elegant"  // system command to run Elegant (e.g. use "mpirun -n 4 Pelegant" for 4 threads)


// minimum numeric values e.g. for comparisons:
#define COMPARE_DOUBLE_EQUAL 1e-10 // difference allowed for double to be accepted as equal (e.g. lattice compare)
#define ZERO_DISTANCE 1e-6         // used to compare positions/m in Accelerator (treated as equal if difference is smaller than this number)
#define MIN_AMPLITUDE 1e-10        // For smaller Amplitudes in Field Spectrum the phase is set to 0.0
#define MIN_EXPORT_TILT 1e-9       // smaller element tilts/rad are ignored during lattice export
#define MIN_EXPORT_DISPLACEMENT 1e-6 // smaller element shifts/m are ignored during lattice export


// MadX/Elegant export:
#define EXPORT_LINE_ELEMENTS_PER_ROW 10 // number of elements per line in beamline export
#define ELEGANT_SYNCH_RAD_SETTING 1     // def. of elegant dipole for export: synchrotron radiation (1=on, 0=off)
#define ELEGANT_DIPOLE_NKICKS 40        // def. of elegant dipole for export: n_kicks parameter
#define ELEGANT_QUADRUPOLE_NKICKS 20    // def. of elegant quadrupole for export: n_kicks parameter
#define ELEGANT_SEXTUPOLE_NKICKS 10     // def. of elegant sextupole for export: n_kicks parameter


#define VCPOS_WARNDIFF 0.05            // ELSAimport: warning for larger VC pos.diff. in MadX & ELSA-Spuren
#define DEFAULT_LENGTH_DIFFERENCE 0.09 // default for AccElement "effective-minus-physical" length in m (if no physical length is set)


#endif

/*__LIBPALATTICE__CONFIG_HPP_*/
