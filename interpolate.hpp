/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __INTERPOLATE_HPP_
#define __INTERPOLATE_HPP_

#include <vector>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

using namespace std;

class Interpolate {

protected:
  vector<double> x;      // x data
  vector<double> f;      // f(x) data

private:
  const gsl_interp_type *type;  // type of interpolation used (see GSL manual)
  bool periodic;
  double period;

  gsl_interp_accel *acc;
  gsl_spline *spline;
  bool ready;


public:
  // ! user must provide appropriate x and f(x): !
  // !  - x[i] corresponding to f(x)[i]          !
  // !  - sorted by x, increasing                !
  Interpolate(const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  Interpolate(vector<double> xIn, vector<double> fIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  Interpolate(const Interpolate &other);
  ~Interpolate();

  void init();
  double interp(double x);
  void reset();                                       // new initialization (for child-classes that can change x and f)
  void reset(vector<double> xIn, vector<double> fIn, double periodIn=0.); // new initialization and new external data

  unsigned int size() const;
  double dataMin() const {return x.front();}            // minimum given x 
  double dataMax() const {return x.back();}             // maximum given x 
  double dataRange() const {return x.back()-x.front();}
  double interpMin() const;                             // lower limit for interpolation
  double interpMax() const;                             // upper limit for interpolation
  double interpRange() const;

  const char * getType() const {return gsl_spline_name(spline);}
  double getPeriod() const {return period;}
};

#endif
/*__INTERPOLATE_HPP_*/
