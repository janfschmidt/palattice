/* === Interpolate Class ===
 * stores arbitrary data values f(x) as a function of x
 * and allows for access to data f(x) at any x using gsl interpolation.
 * x is double, f can have several types (int, double, AccPair, AccTriple).
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
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
  virtual ~Interpolate();

  void init();
  T interp(double xIn);
  T interp(double xIn) const;
  void reset();                                // new initialization (for derived classes that can change data)
  void reset(std::map<double,T> dataIn, double periodIn=0.); // new initialization and new external data

  unsigned int size() const {return data.size();}
  double dataMin() const {return data.begin()->first;}            // minimum given _x 
  double dataMax() const {return data.rbegin()->first;}           // maximum given _x 
  double dataRange() const {return dataMax()-dataMin();}
  double interpMin() const;                             // lower limit for interpolation
  double interpMax() const;                             // upper limit for interpolation
  double interpRange() const;

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
