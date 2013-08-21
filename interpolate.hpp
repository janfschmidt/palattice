/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * x is double, f can have several types.
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

using namespace std;

template <class T=double>
class Interpolate {

protected:
  vector<double> x;      // x data
  vector<T> f;           // f(x) data

private:
  const gsl_interp_type *type;  // type of interpolation used (see GSL manual)
  bool periodic;
  double period;

  gsl_interp_accel *acc;
  vector<gsl_spline*> spline;  //several splines for multidimensional data types
  bool ready;

  gsl_spline* getSpline(vector<double> f_single);
  double evalSpline(gsl_spline *s, double xIn);
  void initThis();
  T interpThis(double xIn);


public:
  // ! user must provide appropriate x and f(x): !
  // !  - x[i] corresponding to f(x)[i]          !
  // !  - sorted by x, increasing                !
  Interpolate(const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  Interpolate(vector<double> xIn, vector<T> fIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  Interpolate(const Interpolate &other);
  ~Interpolate();

  void init();
  T interp(double xIn);
  void reset();                                // new initialization (for child-classes that can change x and f)
  void reset(vector<double> xIn, vector<T> fIn, double periodIn=0.); // new initialization and new external data

  unsigned int size() const;
  double dataMin() const {return x.front();}            // minimum given x 
  double dataMax() const {return x.back();}             // maximum given x 
  double dataRange() const {return x.back()-x.front();}
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
template<> void Interpolate<AccTripel>::initThis();
template<> AccTripel Interpolate<AccTripel>::interpThis(double xIn);


#include "interpolate.hxx"

#endif
/*__INTERPOLATE_HPP_*/
