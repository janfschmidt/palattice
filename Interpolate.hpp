/* Interpolate Class
 * allows for access to data f(x) at any x using gsl interpolation.
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
 * x is double, f can have several types (int, double, AccPair, AccTriple).
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 */

#ifndef __LIBPALATTICE_INTERPOLATE_HPP_
#define __LIBPALATTICE_INTERPOLATE_HPP_

#include <vector>
#include <string>
#include <stdexcept>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include "types.hpp"
#include "Metadata.hpp"

namespace pal
{

template <class T=double>
class Interpolate {

protected:
  std::map<double,T> data;
  std::string headerString;
  double period;
  bool ready;
  bool periodic;

private:
  const gsl_interp_type *type;  // type of interpolation used (see GSL manual)
  gsl_interp_accel *acc;
  std::vector<gsl_spline*> spline;  //several splines for multidimensional data types

  gsl_spline* getSpline(std::map<double,double> data1D);
  double evalSpline(gsl_spline *s, double xIn) const;
  void initThis();
  T interpThis(double xIn) const;


public:
  Metadata info;

  Interpolate(const gsl_interp_type *t=gsl_interp_akima, double periodIn=0., std::map<double,T> dataIn=std::map<double,T>());
  Interpolate(const Interpolate &other);
  Interpolate(Interpolate &&other);
  Interpolate& operator=(const Interpolate &other);
  virtual ~Interpolate();

  // access data without interpolation
  T behind(double xIn) const;    // get data of smallest x with x > xIn
  T infrontof(double xIn) const; // get data of largest x with x <= xIn

  // manual initialization of interpolation
  void init();

  // access interpolated data
  // non const version initializes interpolation automatically if not done before
  // const version requires init() to be called before.
  T interp(double xIn);
  T interp(double xIn) const;

  // periodic interpolation: avoiding extrapolation my mapping xIn into interpRange:
  inline T interpPeriodic(double xIn)       { while(xIn<interpMin()) {xIn+=interpRange();} return interp(interpMin()+std::fmod(xIn-interpMin(), interpRange())); }
  inline T interpPeriodic(double xIn) const { while(xIn<interpMin()) {xIn+=interpRange();} return interp(interpMin()+std::fmod(xIn-interpMin(), interpRange()));}

  // reset initialization (for derived classes that can change data)
  void reset();
  void reset(std::map<double,T> dataIn, double periodIn=0.); // directly insert new external data

  // info
  unsigned int size() const {return data.size();}
  double dataMin() const {return data.begin()->first;}            // minimum given _x 
  double dataMax() const {return data.rbegin()->first;}           // maximum given _x 
  double dataRange() const {return dataMax()-dataMin();}
  double interpMin() const;                             // lower limit for interpolation
  double interpMax() const;                             // upper limit for interpolation
  double interpRange() const;                           // "length" of interpolation range

  const char * getType() const {return gsl_spline_name(spline[0]);}
  double getPeriod() const {return period;}

  // output
  virtual std::string header() const;
  void interp_out(double stepwidth, std::string filename="");
};


// template function specializations
// interpolation is only implemented for these data types
template<> void Interpolate<double>::initThis();
template<> double Interpolate<double>::interpThis(double xIn) const;
template<> void Interpolate<AccPair>::initThis();
template<> AccPair Interpolate<AccPair>::interpThis(double xIn) const;
template<> void Interpolate<AccTriple>::initThis();
template<> AccTriple Interpolate<AccTriple>::interpThis(double xIn) const;
template<> std::string Interpolate<AccPair>::header() const;
template<> std::string Interpolate<AccTriple>::header() const;

} //namespace pal

#include "Interpolate.hxx"

#endif
/*__LIBPALATTICE_INTERPOLATE_HPP_*/
