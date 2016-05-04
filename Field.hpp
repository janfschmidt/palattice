/* Magnetic Field Class
 * magnetic field distribution (3D) along an accelerator ring
 * implemented as FunctionOfPos<AccTriple> with additional function(s)
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBPALATTICE__FIELD_HPP_
#define __LIBPALATTICE__FIELD_HPP_

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
  ~Field() {}

  
  void set(AccLattice &lattice, FunctionOfPos<AccPair> &orbit, unsigned int n_samples, bool edgefields=true); // set all magnetic field values from lattice and orbit

  int magnetlengths(AccLattice &lattice, const char *filename) const;

  // overwrite getSpectrum: equidistant sampling given, no need to set stepwidth
  Spectrum getSpectrum(AccAxis axis=x, unsigned int fmaxrev=30, double ampcut=0., string name="") const
  {return FunctionOfPos<AccTriple>::getSpectrum(0,axis,fmaxrev,ampcut,name);}
  Spectrum getSpectrum(unsigned int fmaxrev=30, double ampcut=0., string name="") const
  {return FunctionOfPos<AccTriple>::getSpectrum(0,fmaxrev,ampcut,name);}

};

} //namespace pal

#endif
/*__LIBPALATTICE__FIELD_HPP_*/
