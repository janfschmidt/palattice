/* === Interpolate Class ===
 * supplies values f(_x) for arbitrary _x for given arrays f and _x. uses gsl interpolation
 * _x is double, f can have several types.
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>

using namespace std;
using namespace pal;

// constructor
// ! user must provide appropriate _x and f(_x): !
// !  - _x[i] corresponding to f(_x)[i]          !
// !  - sorted by _x, increasing                !
template <class T>
Interpolate<T>::Interpolate(const gsl_interp_type *t, double periodIn, unsigned int sizeIn)
  : _x(sizeIn), f(sizeIn), headerString("value"), period(periodIn), ready(false), type(t)
{
  acc = gsl_interp_accel_alloc ();

  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  // if ( period!=0. && !periodic )
  //   cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}

template <class T>
Interpolate<T>::Interpolate(vector<double> xIn, vector<T> fIn, const gsl_interp_type *t, double periodIn)
  : _x(xIn), f(fIn), headerString("value"), type(t), period(periodIn), ready(false), headerString("value")
{
  acc = gsl_interp_accel_alloc ();

  if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
    periodic = true;
  else
    periodic = false;

  // if ( period!=0. && !periodic )
  //   cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}


// copy constructor
template <class T>
Interpolate<T>::Interpolate(const Interpolate &other)
  : _x(other._x), f(other.f), headerString(other.headerString), type(other.type), period(other.period), ready(false), headerString("value")
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
      cout << "WARNING: Interpolate::getSpline(): period of function should be set for periodic interpolation type "
	   << getType() << ". Assume last data point x=" << _x.back() << " as period." << endl;
      cout << "--- period can be set with class constructor." << endl;
      period = _x.back();
    }

    // for trajectory:
    // - data of additional turn needed to avoid extrapolation for non-periodic case
    // - due to additional turn data range exceeds period for periodic case
    // solution at the moment: dont set period to range, but set range to period (ignore additional points)
    // !! still problem: trajectory with periodic boundary: x=0 already in data -> repeats
    // ---
    if (range > period) {
      cout << "WARNING: Interpolate::getSpline(): period of function < data range (" << period <<"<"<< range
    	     << "). Use data within period only." << endl;
      unsigned int n_period = 0;
      for (;n_period < n; n_period++) {
	if (_x[n_period] > period) {
	  n_period--;
	  break;
	}
      }

      // add datapoint (based on period) to enlarge range
      if (dataMin()==0.) n = n_period +1; //avoid two datapoints at x=0
      else n = n_period + 2;
      xTmp = new double[n];
      fTmp = new double[n];
      cleanup = true;

      if (dataMin()!=0.) {//avoid two datapoints at x=0
	xTmp[0] = _x[n_period] - period;  // add datapoint BEFORE range (!interpMin/Max functions affected!)
	fTmp[0] = f_single[n_period];
	std::copy(_x.begin(), _x.begin()+n_period+1, xTmp+1);
	std::copy(f_single.begin(), f_single.begin()+n_period+1, fTmp+1);
      }
      else {
	std::copy(_x.begin(), _x.begin()+n_period+1, xTmp);
	std::copy(f_single.begin(), f_single.begin()+n_period+1, fTmp);
      }
      xTmp[n-1] = period + xTmp[1];   // add datapoint AFTER range (!interpMin/Max functions affected!)
      fTmp[n-1] = fTmp[1];
    }
    
    else if (range < period) {
      // add datapoint (based on period) to enlarge range
      n += 2;
      xTmp = new double[n];
      fTmp = new double[n];
      cleanup = true;
      
      xTmp[0] = _x.back() - period;  // add datapoint BEFORE range (!interpMin/Max functions affected!)
      fTmp[0] = f_single.back();
      std::copy(_x.begin(), _x.end(), xTmp+1);
      std::copy(f_single.begin(), f_single.end(), fTmp+1);
      xTmp[n-1] = period + xTmp[1];   // add datapoint AFTER range (!interpMin/Max functions affected!)
      fTmp[n-1] = fTmp[1];
    }
  } // (end periodic boundary conditions)

  else { // non periodic type
    xTmp = &(_x[0]); 
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
double Interpolate<T>::evalSpline(gsl_spline *s, double xIn) const
{
  double tmp;
  stringstream msg;
  if ( xIn >= interpMin() && xIn <= interpMax() )
    tmp =  gsl_spline_eval (s, xIn, acc);
  else {
    msg << "ERROR: Interpolate<T>::evalSpline(): f(x) Extrapolation instead of Interpolation requested @ x=" <<xIn<< endl;
    throw range_error(msg.str());
  }
  
  if (std::isnan(tmp)) {
    cout << "ERROR: Interpolate::evalSpline(): interpolation error at x="<<xIn<< endl
	 << "return 0.0 and continue";
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
    cout << "WARNING: Interpolate::init(): Interpolation initialisation called twice. Skip." << endl;
    return;
  }
  if (this->size() < 2) {
    stringstream msg;
    msg << "ERROR: Interpolate::init(): Interpolation not possible for only " << _x.size() << " datapoints. Skip.";
    throw std::runtime_error(msg.str());
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

template <class T>
T Interpolate<T>::interp(double xIn) const
{
  if (!ready) {
    throw libpalError("ERROR: Interpolate<>:interp_const(): Interpolation cannot be initialized by this const (!) function.");
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
  _x = xIn;
  f = fIn;
  if (periodIn != 0.) period = periodIn;

  reset();

  // if ( period!=0. && !periodic )
  //   cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
}




//get length of _x and f(_x)
template <class T>
unsigned int Interpolate<T>::size() const
{
  unsigned int s = _x.size();

  if(s != f.size()) {
    stringstream msg;
    msg << "ERROR: Interpolate::size(): unequal size of x (" << s << ") and f(x) (" << f.size()
         << ").";
    throw libpalError(msg.str());
  }
  else
    return s;
}



// lower limit for interpolation
template <class T>
double Interpolate<T>::interpMin() const
{
  if (periodic)
    return dataMax() - period; //dataMin(); //compare with Interpolate::init()
  else
    return dataMin();
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



// output of interpolated f(_x) to file
template <class T>
void Interpolate<T>::interp_out(double stepwidth, string filename)
{
  stringstream s;
  fstream file;
  const int w = 14;
  
  if (stepwidth == 0.) {
    throw libpalError("ERROR: Interpolate<T>::interp_out(): output stepwidth cannot be zero.");
  }
  else if (stepwidth < 0.)
    stepwidth = -stepwidth;
 
  //write text to s
  s << info.out("#");
  s <<"#"<<setw(w)<< "position" <<"\t\t"<< this->header() << endl;
  s <<setprecision(3);
  if (size() > 0) {
    double start;
    if (interpMin() < 0.) start = 0.;
    else start = interpMin();
    for (double pos=start; pos<=interpMax(); pos+=stepwidth) {
      s << resetiosflags(ios::scientific) << setiosflags(ios::fixed);
      s <<setw(w+1)<< pos;
      s << resetiosflags(ios::fixed) << setiosflags(ios::scientific);
      s <<setw(w)<< this->interp(pos)<< endl;
    }
  }

  //output of s
  if (filename == "")
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
      throw libpalFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote "<< filename  << endl;
  }
}




// ---- "defaults" for not implemented data types (see Interpolate.cpp for specialization) ----

template <class T>
void Interpolate<T>::initThis()
{
  throw libpalError("ERROR: Interpolate<>:initThis(): Interpolation is not implemented for this data type.");
}

template <class T>
T Interpolate<T>::interpThis(double xIn) const
{
  throw libpalError("ERROR: Interpolate<>:interpThis(): Interpolation is not implemented for this data type.");
}



// headline entry for "value" in output file (specialization for AccPair & AccTriple)
template <class T>
string Interpolate<T>::header() const
{
  return headerString;
}
