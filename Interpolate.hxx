/* Interpolate Class
 * allows for access to data f(x) at any x using gsl interpolation.
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * x is double, f can have several types (int, double, AccPair, AccTriple).
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>

using namespace std;
using namespace pal;

// constructor
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
  : data(other.data), headerString(other.headerString), type(other.type), periodic(other.periodic), period(other.period), ready(false), info(other.info)
{
  acc = gsl_interp_accel_alloc ();

  // by setting ready=false spline is initialized again before beeing used
}

// move constructor
template <class T>
Interpolate<T>::Interpolate(Interpolate &&other)
  : data(std::move(other.data)), headerString(std::move(other.headerString)), type(std::move(other.type)), periodic(std::move(other.periodic)), period(std::move(other.period)), ready(std::move(other.ready)), info(std::move(other.info))
{
  acc = other.acc;
  other.acc=nullptr;
  //  std::cout << "move it!" << std::endl;
}

// assignment operator
template<class T>
Interpolate<T>& Interpolate<T>::operator=(const Interpolate &other)
{
  data = other.data;
  headerString = other.headerString;
  type = other.type;
  periodic = other.periodic;
  period = other.period;
  ready = false;
  info = other.info;

  acc = gsl_interp_accel_alloc ();
  // by setting ready=false spline is initialized again before beeing used
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
    throw palatticeError("ERROR: Interpolate<>:interp_const(): Interpolation cannot be initialized by this const (!) function.");
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
    throw palatticeError("ERROR: Interpolate<T>::interp_out(): output stepwidth cannot be zero.");
  }
  else if (stepwidth < 0.)
    stepwidth = -stepwidth;
 
  //write text to s
  s << info.out("#");
  s <<"#"<<setw(w)<< "position" <<"\t\t"<< this->header() << endl;
  if (size() > 0) {
    double start;
    if (interpMin() < 0.) start = 0.;
    else start = interpMin();
    for (double pos=start; pos<=interpMax(); pos+=stepwidth) {
      s << resetiosflags(ios::scientific) << setiosflags(ios::fixed) <<setprecision(3);
      s <<setw(w+1)<< pos;
      s << resetiosflags(ios::fixed) << setiosflags(ios::scientific) <<setprecision(6);
      s <<setw(w)<< this->interp(pos)<< endl;
    }
  }

  //output of s
  if (filename == "")
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
      throw palatticeFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote "<< filename  << endl;
  }
}


// access data without interpolation
// get data of smallest x with x > xIn
template <class T>
T Interpolate<T>::behind(double xIn) const
{
  auto it = data.upper_bound(xIn);
  if (it==data.end()) {
    stringstream msg;
    msg << "Interpolate<>:behind(): " << xIn << " is out of data range.";
    throw palatticeError(msg.str());
  }
  return it->second;
}

// get data of largest x with x <= xIn
template <class T>
T Interpolate<T>::infrontof(double xIn) const
{
  auto it = data.lower_bound(xIn);
  if (it==data.begin()) {
    stringstream msg;
    msg << "Interpolate<>:infrontof(): " << xIn << " is out of data range.";
    throw palatticeError(msg.str());
  }
  it--;
  return it->second;
}



// ---- "defaults" for not implemented data types (see Interpolate.cpp for specialization) ----

template <class T>
void Interpolate<T>::initThis()
{
  throw palatticeError("ERROR: Interpolate<>:initThis(): Interpolation is not implemented for this data type.");
}

template <class T>
T Interpolate<T>::interpThis(double xIn) const
{
  throw palatticeError("ERROR: Interpolate<>:interpThis(): Interpolation is not implemented for this data type.");
}



// headline entry for "value" in output file (specialization for AccPair & AccTriple)
template <class T>
string Interpolate<T>::header() const
{
  return headerString;
}
