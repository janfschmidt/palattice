/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include "interpolate.hpp"


// constructor
Interpolate::Interpolate(double *xIn, double *fIn, gsl_interp_type *t=gsl_interp_akima, double periodIn)
  : x(xIn), fIn(f), type(t), period(periodIn), ready(false)
{

}



// destructor
Interpolate::~Interpolate()
{
  if (ready) {
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);
  }

}



//initialize interpolation
void Interpolate::init()
{
  if (ready) {
    cout << "ERROR: Interpolate::init: Interpolation initialisation called twice. Skip." << endl;
    return;
  }

  unsigned int n = size();
  gsl_error_handler_t *old_error_handler;

  // periodic boundary conditions
  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic) {
    n += 2;
  } // (end periodic boundary conditions)

  acc = gsl_interp_accel_alloc ();
  spline = gsl_spline_alloc (type, n);

  old_error_handler = gsl_set_error_handler_off(); // ??
  

  // periodic boundary conditions
  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic) {

    if (period == 0.) {
      cout << "WARNING: Interpolate::init(): period of function should be set for periodic interpolation type "
	   << getType() << ". Assume last data point x=" << x->back() << " as period." << endl;
      cout << "--- period can be set with Interpolate::setType() or class constructor." << endl;
      period = x->back();
    }
    else if (period < x->back()) {
      cout << "WARNING: Interpolate::init(): period of function should be >= last data point x=" << x->back()
	   << ". Assume last data point as period." << endl;
      period = x->back();
    }

    double *xTmp = new double[n];
    double *fTmp = new double[n];

    xTmp[0] = x->back() - period;   // add initial entry to avoid extrapolation
    fTmp[0] = f->back();
    std::copy(x->begin(), x->end(), &xTmp[1]);
    std::copy(f->begin(), f->end(), &fTmp[1]);
    xTmp[n-1] = period + x->front(); // add final entry for periodic boundary condition
    fTmp[n-1] = f->front();

    gsl_spline_init (spline, xTmp, fTmp, n);

    delete[] xTmp;
    delete[] fTmp;

  } // (end periodic boundary conditions)


  gsl_spline_init (spline, &(x->at(0)), &(f->at(0)), n);
  
  gsl_set_error_handler(old_error_handler); // ??
  
  ready = true;
}



// get interpolated value f(xIn)
double Interpolate::interp(double xIn)
{
  double tmp;
  if (!ready) {
    init();
  }

  // if (xIn > x->back()) {
  //   cout << "ERROR: Interpolate::interp(): x=" <<xIn<< " is to large for interpolation." <<endl;
  //   return 0.;
  // }

  tmp = gsl_spline_eval (spline, xIn, acc);

  if (isnan(tmp)) {
    cout << "ERROR: Interpolate::interp(): interpolation error at x="<<xIn<< endl;
    return 0.;
  }
  else
    return tmp;
}



//set type of interpolation
void Interpolate::setType(gsl_interp_type *t)
{
  type = t;
  reset();
}



//get length of x and f(x)
unsigned int size() const
{
  unsigned int s = x->size();

  if(s != f->size()) {
    cout << "ERROR: Interpolate::size(): unequal size of x (" << s << ") and f(x) (" << f->size()
         << ")." << endl;
    return 0;
  }
  else
    return s;
}
