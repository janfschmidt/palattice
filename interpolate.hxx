/* === Interpolate Class ===
 * supplies values f(x) for arbitrary x for given arrays f and x. uses gsl interpolation
 * x is double, f can have several types.
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>

using namespace std;


// constructor
// ! user must provide appropriate x and f(x): !
// !  - x[i] corresponding to f(x)[i]          !
// !  - sorted by x, increasing                !
template <class T>
Interpolate<T>::Interpolate(const gsl_interp_type *t, double periodIn)
  : type(t), period(periodIn), ready(false)
{
  acc = gsl_interp_accel_alloc ();

  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}

template <class T>
Interpolate<T>::Interpolate(vector<double> xIn, vector<T> fIn, const gsl_interp_type *t, double periodIn)
  : x(xIn), f(fIn), type(t), period(periodIn), ready(false)
{
  acc = gsl_interp_accel_alloc ();

  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}


// copy constructor
template <class T>
Interpolate<T>::Interpolate(const Interpolate &other)
  : x(other.x), f(other.f), type(other.type), period(other.period), ready(false)
{
  // by setting ready=false spline and acc are initialized again before beeing used
}



// destructor
template <class T>
Interpolate<T>::~Interpolate()
{
  gsl_interp_accel_free (acc);

  if (ready) {
    for (unsigned int i=0; i<spline.size(); i++)
      gsl_spline_free (spline[i]);
  }
}



//initialize interpolation for double type data
template <class T>
gsl_spline* Interpolate<T>::getSpline(vector<double> f_single)
{
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
      std::copy(f_single.begin(), f_single.end(), fTmp);
      xTmp[n-1] = period + xTmp[0];   // add datapoint AFTER range
      fTmp[n-1] = fTmp[0];
    }
  } // (end periodic boundary conditions)

  else { // non periodic type
    xTmp = &(x[0]); 
    fTmp = &(f_single[0]);
  }


  // initialize interpolation
  gsl_spline *newSpline = gsl_spline_alloc (type, n);
  gsl_spline_init (newSpline, xTmp, fTmp, n);

  if (cleanup) {
    delete[] xTmp;
    delete[] fTmp;
  }

  return newSpline;
}




// evaluate spline s at xIn (double type result)
template <class T>
double Interpolate<T>::evalSpline(gsl_spline *s, double xIn)
{
  double tmp;
  tmp =  gsl_spline_eval (s, xIn, acc);

if (isnan(tmp)) {
    cout << "ERROR: Interpolate::interp(): interpolation error at x="<<xIn<< endl;
    return 0.;
  }
  else
    return tmp;
}




// initialize interpolation
template <class T>
void Interpolate<T>::init()
{
  if (ready) {
    cout << "ERROR: Interpolate::init(): Interpolation initialisation called twice. Skip." << endl;
    return;
  }

  initThis(); // specialized function for each data type

  ready = true;
}



// get interpolated value f(xIn)
template <class T>
T Interpolate<T>::interp(double xIn)
{
  if (!ready) {
    init();
  }

  return interpThis(xIn);
}



// interpolation reset
template <class T>
void Interpolate<T>::reset()
{
  if (ready) {
    for (unsigned int i=0; i<spline.size(); i++)
      gsl_spline_free (spline[i]);
    ready = false;
  }
  // else
  //   cout << "INFO: Interpolate::reset(): nothing to reset." << endl;
}



// change of data and Interpolation reset
template <class T>
void Interpolate<T>::reset(vector<double> xIn, vector<T> fIn, double periodIn)
{
  x = xIn;
  f = fIn;
  if (periodIn != 0.) period = periodIn;

  reset();

  if ( period!=0. && !periodic )
    cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}




//get length of x and f(x)
template <class T>
unsigned int Interpolate<T>::size() const
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
template <class T>
double Interpolate<T>::interpMin() const
{
  return dataMin(); //compare with Interpolate::init()
}

// upper limit for interpolation
template <class T>
double Interpolate<T>::interpMax() const
{
  if (periodic)
    return period + dataMin(); //compare with Interpolate::init()
  else
    return dataMax();
}

template <class T>
double Interpolate<T>::interpRange() const
{
  if (periodic)
    return period;
  else
    return dataRange();
}



// output of interpolated f(x) to file
template <class T>
void Interpolate<T>::interp_out(double stepwidth, const char *filename)
{
  int w=12;
  fstream file;
  
  if (stepwidth == 0.) {
    cout << "ERROR: Interpolate<T>::interp_out(): output stepwidth cannot be zero." <<endl;
    return;
  }
  else if (stepwidth < 0.)
    stepwidth = -stepwidth;
 
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: ORBIT::interp_out(): Cannot open " << filename << "." << endl;
    return;
  }
  
  file <<setw(w)<< "x" <<setw(w)<< "f(x)" << endl;
  file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
  if (interpRange() > 0) {
    for (double s=interpMin(); s<=interpMax(); s+=stepwidth) {
      file <<setw(w)<< s <<setw(w)<< this->interp(s)<< endl;
    }
  }

  file.close();
  cout << "* Wrote " << filename  << endl;
}




// ---- "defaults" for not implemented data types (see interpolate.cpp for specialization) ----

template <class T>
void Interpolate<T>::initThis()
{
  cout << "ERROR: Interpolate<>:initThis(): Interpolation is not implemented for this data type." << endl;
  exit(1);
}

template <class T>
T Interpolate<T>::interpThis(double xIn)
{
  cout << "ERROR: Interpolate<>:interpThis(): Interpolation is not implemented for this data type." << endl;
  exit(1);
}
