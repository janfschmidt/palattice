/* === Magnetic Field Class ===
 * magnetic field distribution (3D) along an accelerator ring
 * implemented as FunctionOfPos<AccTriple> with additional function(s)
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 */


#ifndef __BSUPPLY__FIELD_HPP_
#define __BSUPPLY__FIELD_HPP_

#include "types.hpp"
#include "functionofpos.hpp"


class Field : public FunctionOfPos<AccTriple> {

public:
  // use FunctionOfPos constructor:
  Field(double circIn=164.4, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.)
    : FunctionOfPos(circIn, t, periodIn) {}
  ~Field() {}

  // additional functions for Field:
  int magnetlengths(magnetvec &dipols, const char *filename) const;

};

#endif
/*__BSUPPLY__FIELD_HPP_*/
