/* === ResStrengths Class ===
 * compute the complex strengths of depolarizing resonances
 * from a lattice & orbit
 *
 * THIS IS EXPERIMENTAL
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
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
    const std::complex<double> im;
  
  public:
    const double a_gyro;   // electron gyromagnetic anomaly a = (g-2)/2
    
    Metadata info;
    ResStrengths(AccLattice *_lattice, FunctionOfPos<AccPair> *_orbit);
    ~ResStrengths() {}

    std::complex<double> calculate(double agamma);    // calculate res. strength freq. omega=agamma
    std::complex<double> operator[](double agamma);   // get res. strength (from cache, otherwise calculated)

    void print(double agammaStep, double agammaMax, string filename=""); //print all res. strengths (given step, up to given maximum)
  };

}//namespace pal

#endif
/*__LIBPALATTICE__RESSTRENGTHS_HPP_*/
