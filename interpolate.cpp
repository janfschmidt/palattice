/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <iostream>
#include <cmath>
#include "interpolate.hpp"

using namespace std;


// constructor
// ! user must provide appropriate x and f(x): !
// !  - x[i] corresponding to f(x)[i]          !
// !  - sorted by x, increasing                !
Interpolate::Interpolate(const gsl_interp_type *t, double periodIn)
  : type(t), period(periodIn), ready(false)
{
  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}

Interpolate::Interpolate(vector<double> xIn, vector<double> fIn, const gsl_interp_type *t, double periodIn)
  : x(xIn), f(fIn), type(t), period(periodIn), ready(false)
{
  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}


// copy constructor
Interpolate::Interpolate(const Interpolate &other)
  : x(other.x), f(other.f), type(other.type), period(other.period), ready(false)
{
  // by setting ready=false spline and acc are initialized again before beeing used
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
    cout << "ERROR: Interpolate::init(): Interpolation initialisation called twice. Skip." << endl;
    return;
  }

  unsigned int n = size();
  double *xTmp;
  double *fTmp;
  bool cleanup = false;
  

  // periodic boundary conditions
  if (periodic) {

    double range = dataRange();

    if (period == 0.) {
      cout << "WARNING: Interpolate::init(): period of function should be set for periodic interpolation type "
	   << getType() << ". Assume last data point x=" << x.back() << " as period." << endl;
      cout << "--- period can be set with class constructor." << endl;
      period = x.back();
    }

    if (range > period) {
	cout << "WARNING: Interpolate::init(): period of function should be >= data range (" << range
	     << "). Assume range as period." << endl;
	period = range;
    }
    
    else if (range < period) {
      // add datapoint (based on period) to ensure periodic boundaries
      n += 1;
      xTmp = new double[n];
      fTmp = new double[n];
      cleanup = true;
      
      //xTmp[0] = x.back() - period;  // add datapoint BEFORE range (!interpMin/Max functions affected!)
      //fTmp[0] = f.back();
      std::copy(x.begin(), x.end(), xTmp);
      std::copy(f.begin(), f.end(), fTmp);
      xTmp[n-1] = period + xTmp[0];   // add datapoint AFTER range
      fTmp[n-1] = fTmp[0];
    }
  } // (end periodic boundary conditions)

  else { // non periodic type
    xTmp = &(x[0]); 
    fTmp = &(f[0]);
  }


  // initialize interpolation
  acc = gsl_interp_accel_alloc ();
  spline = gsl_spline_alloc (type, n);
  gsl_spline_init (spline, xTmp, fTmp, n);
  ready = true;

  if (cleanup) {
    delete[] xTmp;
    delete[] fTmp;
  }
}



// get interpolated value f(xIn)
double Interpolate::interp(double xIn)
{
  double tmp;
  if (!ready) {
    init();
  }

  // if (xIn > x.back()) {
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



// change of data and Interpolation reset
void Interpolate::reset(vector<double> xIn, vector<double> fIn, double periodIn)
{
  x = xIn;
  f = fIn;
  if (periodIn != 0.) period = periodIn;

  if (ready) {
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);
    ready = false;
  }

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}




//get length of x and f(x)
unsigned int Interpolate::size() const
{
  unsigned int s = x.size();

  if(s != f.size()) {
    cout << "ERROR: Interpolate::size(): unequal size of x (" << s << ") and f(x) (" << f.size()
         << ")." << endl;
    return 0;
  }
  else
    return s;
}



// lower limit for interpolation
double Interpolate::interpMin() const
{
  return dataMin(); //compare with Interpolate::init()
}

// upper limit for interpolation
double Interpolate::interpMax() const
{
  if (periodic)
    return period + dataMin(); //compare with Interpolate::init()
  else
    return dataMax();
}

double Interpolate::interpRange() const
{
  if (periodic)
    return period;
  else
    return dataRange();
}
