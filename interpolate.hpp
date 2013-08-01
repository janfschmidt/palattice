/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __INTERPOLATE_HPP_
#define __INTERPOLATE_HPP_

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>


class Interpolate {

private:
  vector<double> *const x;      //pointer to x data
  vector<double> *const f;      //pointer to f(x) data
  gsl_interp_type *type;  // type of interpolation used (see GSL manual)
  double period;

  gsl_interp_accel *acc;
  gsl_spline *spline;
  bool ready;


public:
  Interpolate(vector<double> *xIn, vector<double> *fIn, gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  ~Interpolate() {}

  void init();
  double interp(double x);
  void reset() {ready = false;}

  void setType(gsl_interp_type *t, double periodIn=0.);
  const char * getType() const {return gsl_spline_name(spline);}

  unsigned int size() const;
  double period() const {return period;}
};

#endif
/*__INTERPOLATE_HPP_*/
