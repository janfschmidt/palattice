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
// template <class T>
// Interpolate<T>::Interpolate(const gsl_interp_type *t, double periodIn)
//   : headerString("value"), period(periodIn), ready(false), type(t)
// {
//   acc = gsl_interp_accel_alloc ();

//   if (type == gsl_interp_akima_periodic || type == gsl_interp_cspline_periodic)
//     periodic = true;
//   else
//     periodic = false;

//   // if ( period!=0. && !periodic )
//   //   cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
// }

template <class T>
Interpolate<T>::Interpolate(const gsl_interp_type *t, double periodIn, std::map<double,T> dataIn)
  : data(dataIn), headerString("value"), period(periodIn), ready(false), type(t)
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
  : data(other.data), headerString(other.headerString), type(other.type), period(other.period), ready(false), headerString("value")
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
gsl_spline* Interpolate<T>::getSpline(std::map<double,double> data1D)
{
  double *xTmp;
  double *fTmp;

  // periodic boundary conditions
  if (periodic) {

    if (period == 0.) {
      cout << "WARNING: Interpolate::getSpline(): period of function should be set for periodic interpolation type "
	   << getType() << ". Assume last data point x=" << dataMax() << " as period." << endl;
      cout << "--- period can be set with class constructor." << endl;
      period = dataMax();
    }

    if (dataRange() > period) {
      cout << "WARNING: Interpolate::getSpline(): period of function < data range (" << period <<"<"<< dataRange()
	   << "). Use data within period only." << endl;
    }

    // add datapoints to avoid extrapolation, if they do not exist already
    std::pair<double,double> begin = *data1D.begin();
    std::pair<double,double> lastInPeriod = *(--data1D.upper_bound(period));
    // add datapoint BEFORE range (!interpMin/Max functions affected!)
    data1D.insert( std::pair<double,double>(lastInPeriod.first-period, lastInPeriod.second) );
    // add datapoint AFTER range (!interpMin/Max functions affected!)
    data1D.insert( std::pair<double,double>(period+begin.first, begin.second) );
  }// (end periodic boundary conditions)
  
  // write data to double arrays for gsl spline
  unsigned int n = data1D.size();
  xTmp = new double[n];
  fTmp = new double[n];
  std::map<double,double>::const_iterator it=data1D.begin();
  for (unsigned int i=0; i<n; i++) {
    xTmp[i] = it->first;
    fTmp[i] = it->second;
    it++;
  }

  // initialize interpolation
  gsl_spline *newSpline = gsl_spline_alloc (type, n);
  gsl_spline_init (newSpline, xTmp, fTmp, n);

  delete[] xTmp;
  delete[] fTmp;
  
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
    msg << "ERROR: Interpolate<T>::evalSpline(): Extrapolation instead of Interpolation requested @ key=" <<xIn<< endl;
    throw range_error(msg.str());
  }
  
  if (std::isnan(tmp)) {
    cout << "ERROR: Interpolate::evalSpline(): interpolation error at key="<<xIn<< endl
	 << "return 0.0 and continue" << endl;
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
    msg << "ERROR: Interpolate::init(): Interpolation not possible for only " << data.size() << " datapoints. Skip.";
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
void Interpolate<T>::reset(std::map<double,T> dataIn, double periodIn)
{
  data = dataIn;
  if (periodIn != 0.) period = periodIn;

  reset();

  // if ( period!=0. && !periodic )
  //   cout << "WARNING: Interpolate::Interpolate(): period not used for non-periodic interpolation type." << endl;
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
