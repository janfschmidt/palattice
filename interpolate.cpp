/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <iostream>
#include <cmath>
#include "interpolate.hpp"

using namespace std;


// constructor
Interpolate::Interpolate(vector<double> *xIn, vector<double> *fIn, const gsl_interp_type *t, double periodIn)
  : x(xIn), f(fIn), type(t), period(periodIn), ready(false)
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

  acc = gsl_interp_accel_alloc ();
  spline = gsl_spline_alloc (type, n);

  old_error_handler = gsl_set_error_handler_off(); // ??
  

  // periodic boundary conditions
  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic) {

    double range = x->back() - x->front();

    if (period == 0.) {
      cout << "WARNING: Interpolate::init(): period of function should be set for periodic interpolation type "
	   << getType() << ". Assume last data point x=" << x->back() << " as period." << endl;
      cout << "--- period can be set with class constructor." << endl;
      period = x->back();
    }

    if (range > period) {
	cout << "WARNING: Interpolate::init(): period of function should be >= data range (" << range
	     << "). Assume range as period." << endl;
	period = range;
    }
    
    else if (range < period) {
      // add datapoint (based on period) to ensure periodic boundaries
      n += 1;
      double *xTmp = new double[n];
      double *fTmp = new double[n];
      
      //xTmp[0] = x->back() - period;  // add datapoint BEFORE range
      //fTmp[0] = f->back();
      std::copy(x->begin(), x->end(), xTmp);
      std::copy(f->begin(), f->end(), fTmp);
      xTmp[n-1] = period + xTmp[0];   // add datapoint AFTER range
      fTmp[n-1] = fTmp[0];
      
      //debug output:
      // for (unsigned int i=0; i<n; i++) {
      //   cout << xTmp[i] << "\t" << fTmp[i] << endl;
      // }
      // cout << "-----------------------" << endl;
      
      gsl_spline_init (spline, xTmp, fTmp, n);
      
      delete[] xTmp;
      delete[] fTmp;
    }
  } // (end periodic boundary conditions)

  
  else {
    gsl_spline_init (spline, &(x->at(0)), &(f->at(0)), n);
  }
  
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



void Interpolate::reset()
{
if (ready) {
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);
    ready = false;
  }
 else
   cout << "WARNING: Interpolate::reset(): nothing to reset." << endl;
}


//set type of interpolation
// void Interpolate::setType(const gsl_interp_type *t, double periodIn)
// {
//   type = t;
//   period = periodIn;
//   reset();
// }



//get length of x and f(x)
unsigned int Interpolate::size() const
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
