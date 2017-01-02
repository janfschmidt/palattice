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

#include "ResStrengths.hpp"

ResStrengths::ResStrengths(AccLattice *_lattice, FunctionOfPos<AccPair> *_orbit, unsigned int _nturns)
  : lattice(_lattice), orbit(_orbit), im(std::complex<double> (0,1)), nturns(_nturns), a_gyro(0.001159652)
{
  info.add("Description", "strengths of depolarizing resonances (complex numbers)");
  info.add("turns used for res. strength calc.", nturns);
  info += lattice->info;
  info += orbit->info;
}


// get res. strength from cache if possible, else calculate and add to cache
std::complex<double> ResStrengths::operator[](double agamma)
{
  std::map<double,std::complex<double> >::const_iterator it = cache.find(agamma);
  if (it != cache.end())
    return it->second;
  else
    return calculate(agamma);
}



// calculate res. strength freq. omega=agamma
// based on Courant-Ruth formalism using the magnetic fields B(orbit)
// Fields are NOT expressed by linear approx. of particle motion as by Courant-Ruth and DEPOL code,
// but the magnetic fields are used directly.
// !!! At the moment the orbit/field inside a magnet is assumed to be constant.
// !!! edge focusing of dipoles is not included in AccLattice, but horizontal edge fields is
//     calculated here. Longitudinal component is not implemented.
std::complex<double> ResStrengths::calculate(double agamma)
{
  std::complex<double> epsilon (0,0);

  for (unsigned int turn=1; turn<=nturns; turn++) {
    for (AccLattice::const_iterator it=lattice->begin(); it!=lattice->end(); ++it) {
      //field from Thomas-BMT equation:
      // omega = (1+agamma) * B_x - i * (1+a) * B_s
      // assume particle velocity parallel to s-axis, B is already normalized to rigidity (BR)_0 = p_0/e
      std::complex<double> omega = (1+agamma)*it.element()->B(orbit->interp(it.pos())).x - im * (1+a_gyro)*it.element()->B(orbit->interp(it.pos())).s;

      // dipole
      if (it.element()->type == dipole) {
	double R = 1/it.element()->k0.z; // bending radius
	// horizontal component of fringe field due to edge angle (e1,e2):
	//   Bx*l = k*z*l = - tan(e1)/R*z1 - tan(e2)/R*z2
	// (longitudinal fringe field not implemented)
	omega -= (1+agamma) * ( tan(it.element()->e1)/R * orbit->interp(it.begin()).z
				+ tan(it.element()->e2)/R * orbit->interp(it.end()).z );
	// dipole: epsilon = 1/2pi * omega * R/(i*agamma) * (e^{i*agamma*theta2}-e^{i*agamma*theta1})
	epsilon += 1/(2*M_PI) * omega * R/(im*agamma) * (std::exp(im*agamma*(lattice->theta(it.end())+turn*2*M_PI)) - std::exp(im*agamma*(lattice->theta(it.begin())+turn*2*M_PI)));
      }
      // all others: epsilon = 1/2pi * e^{i*agamma*theta} *  omega * l
      else {
	epsilon += 1/(2*M_PI) * std::exp(im*agamma*(lattice->theta(it.pos())+turn*2*M_PI)) * omega * it.element()->length;
      }
    }//lattice
  }//turn
  
  cache.insert( std::pair<double,std::complex<double> >(agamma,epsilon/double(nturns)) );
  return epsilon/double(nturns);
}



// print res. strength for each agamma in [agammaStep, 2*agammaStep, ..., agammaMax]
void ResStrengths::print(double agammaStep, double agammaMax, string filename)
{
  std::fstream file;
  std::stringstream s;
  const int w = 16;
  std::complex<double> epsilon;

 //metadata
  s << info.out("#");

 // header
  s <<"#"<<std::setw(w)<<"agamma"<<std::setw(w)<< "real(epsilon)" <<std::setw(w)<< "imag(epsilon)" <<std::setw(w)<< "abs(epsilon)" << std::endl;
 
  for (double agamma = agammaStep; agamma<=agammaMax; agamma+=agammaStep) {
    epsilon = operator[](agamma);
    s <<resetiosflags(ios::scientific)<<setiosflags(ios::fixed)<<setprecision(4);
    s <<std::setw(1+w)<< agamma;
    s <<resetiosflags(ios::fixed)<<setiosflags(ios::scientific)<<showpoint<<setprecision(5);
    s <<std::setw(w)<< epsilon.real() <<std::setw(w)<< epsilon.imag() <<std::setw(w)<< std::abs(epsilon)
      << std::endl;
  }

 // output of s
 if (filename == "")
   cout << s.str();
 else {
   file.open(filename.c_str(), ios::out);
   if (!file.is_open()) {
     throw palatticeFileError(filename);
   }
   file << s.str();
   file.close();
   cout << "* Wrote " << filename  << std::endl;
 }

}
