/* === Magnetic Field Class ===
 * magnetic field distribution (3D) along an accelerator ring
 * implemented as FunctionOfPos<AccTriple> with additional function(s)
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 */


#ifndef __LIBPAL__FIELD_HPP_
#define __LIBPAL__FIELD_HPP_

#include "types.hpp"
#include "FunctionOfPos.hpp"
#include "AccLattice.hpp"

namespace pal
{

class Field : public FunctionOfPos<AccTriple> {

public:
  // use FunctionOfPos constructors:
  Field(double circIn=164.4, const gsl_interp_type *t=gsl_interp_akima)
    : FunctionOfPos(circIn, t) {}
  Field(double circIn, unsigned int samplesIn, unsigned int turnsIn, const gsl_interp_type *t=gsl_interp_akima)
    : FunctionOfPos(circIn, samplesIn, turnsIn, t) {}
  ~Field() {}

  
  void set(AccLattice &lattice, FunctionOfPos<AccPair> &orbit, double n_samples); // set all magnetic field values from lattice and orbit

  int magnetlengths(AccLattice &lattice, const char *filename) const;
};

} //namespace pal

#endif
/*__LIBPAL__FIELD_HPP_*/
