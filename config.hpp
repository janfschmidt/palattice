/* === libpalattice Configuration ===
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */

#ifndef __LIBPALATTICE__CONFIG_HPP_
#define __LIBPALATTICE__CONFIG_HPP_


// system commands:
#define MADXCOMMAND "madx"        // system command to run MadX
#define ELEGANTCOMMAND "elegant"  // system command to run Elegant


// minimum numeric values e.g. for comparisons:
#define COMPARE_DOUBLE_EQUAL 1e-10 // difference allowed for double to be accepted as equal (e.g. lattice compare)
#define ZERO_DISTANCE 1e-6         // used to compare positions/m in Accelerator (treated as equal if difference is smaller than this number)
#define MIN_AMPLITUDE 1e-10        // For smaller Amplitudes in Field Spectrum the phase is set to 0.0
#define MIN_EXPORT_TILT 1e-9       // smaller element tilts/rad are ignored during lattice export


// MadX/Elegant export:
#define EXPORT_LINE_ELEMENTS_PER_ROW 10 // number of elements per line in beamline export


#define VCPOS_WARNDIFF 0.05  // ELSAimport: warning for larger VC pos.diff. in MadX & ELSA-Spuren
#define DEFAULT_LENGTH_DIFFERENCE 0.09 // default for AccElement "effective-minus-physical" length in m (if no physical length is set)


#endif

/*__LIBPALATTICE__CONFIG_HPP_*/
