/* === Interpolate Class ===
 * supplies values f(_x) for arbitrary _x for given arrays f and _x. uses gsl interpolation
 * _x is double, f can have several types.
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __INTERPOLATE_HPP_
#define __INTERPOLATE_HPP_

#include <vector>
#include <stdexcept>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include "types.hpp"



template <class T=double>
class Interpolate {

protected:
  std::vector<double> _x;      // _x data
  std::vector<T> f;           // f(_x) data

private:
  const gsl_interp_type *type;  // type of interpolation used (see GSL manual)
  bool periodic;
  double period;

  gsl_interp_accel *acc;
  std::vector<gsl_spline*> spline;  //several splines for multidimensional data types
  bool ready;

  gsl_spline* getSpline(std::vector<double> f_single);
  double evalSpline(gsl_spline *s, double xIn);
  void initThis();
  T interpThis(double xIn);


public:
  // ! user must provide appropriate _x and f(x): !
  // !  - _x[i] corresponding to f(_x)[i]          !
  // !  - sorted by _x, increasing                !
  Interpolate(const gsl_interp_type *t=gsl_interp_akima, double periodIn=0., unsigned int sizeIn=0);
  Interpolate(std::vector<double> xIn, std::vector<T> fIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  Interpolate(const Interpolate &other);
  ~Interpolate();

  void init();
  T interp(double xIn);
  void reset();                                // new initialization (for child-classes that can change _x and f)
  void reset(std::vector<double> xIn, std::vector<T> fIn, double periodIn=0.); // new initialization and new external data

  unsigned int size() const;
  double dataMin() const {return _x.front();}            // minimum given _x 
  double dataMax() const {return _x.back();}             // maximum given _x 
  double dataRange() const {return _x.back()-_x.front();}
  double interpMin() const;                             // lower limit for interpolation
  double interpMax() const;                             // upper limit for interpolation
  double interpRange() const;

  const char * getType() const {return gsl_spline_name(spline[0]);}
  double getPeriod() const {return period;}

  void interp_out(double stepwidth, const char *filename);
};


// template function specializations
// interpolation is only implemented for these data types
template<> void Interpolate<double>::initThis();
template<> double Interpolate<double>::interpThis(double xIn);
template<> void Interpolate<AccPair>::initThis();
template<> AccPair Interpolate<AccPair>::interpThis(double xIn);
template<> void Interpolate<AccTriple>::initThis();
template<> AccTriple Interpolate<AccTriple>::interpThis(double xIn);


#include "interpolate.hxx"

#endif
/*__INTERPOLATE_HPP_*/
