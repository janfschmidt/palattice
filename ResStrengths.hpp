/* ResStrengths Class
 * estimate the complex strengths of depolarizing resonances from a lattice & orbit
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
 *
 *
 * Based on formulas by Courant & Ruth from:
 * Courant, E. D., and Ronald D. Ruth.
 * "The acceleration of polarized protons in circular accelerators."
 * BNL–51270 and UC–28 and ISA–80–5 (1980).
 *
 * THIS IS EXPERIMENTAL. RESULTS NOT FULLY VERIFIED.
 */

#ifndef __LIBPALATTICE__RESSTRENGTHS_HPP_
#define __LIBPALATTICE__RESSTRENGTHS_HPP_

#include <map>
#include <complex>
#include "AccLattice.hpp"
#include "FunctionOfPos.hpp"
#include "Metadata.hpp"

namespace pal
{

  class ResStrengths {
  protected:
    const AccLattice *lattice;
    const FunctionOfPos<AccPair> *orbit;
    std::map<double,std::complex<double> > cache;
    const std::complex<double> im;                  // imaginary unit i (initialized in constructor)
    unsigned int nturns;
  
  public:
    const double a_gyro;   // electron gyromagnetic anomaly a = (g-2)/2
    
    Metadata info;
    ResStrengths(AccLattice *_lattice, FunctionOfPos<AccPair> *_orbit, unsigned int _nturns=1);
    ~ResStrengths() {}

    std::complex<double> calculate(double agamma);    // calculate res. strength freq. omega=agamma
    std::complex<double> operator[](double agamma);   // get res. strength (from cache, otherwise calculated)

    void print(double agammaStep, double agammaMax, string filename=""); //print all res. strengths (given step, up to given maximum)
  };

}//namespace pal

#endif
/*__LIBPALATTICE__RESSTRENGTHS_HPP_*/
