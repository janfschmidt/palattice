/* === Interpolate Class ===
 * stores arbitrary data values f(x) as a function of x
 * and allows for access to data f(x) at any x using gsl interpolation.
 * x is double, f can have several types (int, double, AccPair, AccTriple).
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include "Interpolate.hpp"

using namespace pal;


// =========== template function specialization ============

//double
template <>
void Interpolate<double>::initThis()
{
  spline.push_back( getSpline(data) );
}

//double
template <>
double Interpolate<double>::interpThis(double xIn) const
{
  double tmp;
  tmp = evalSpline(spline[0], xIn);
  return tmp;
}



//AccPair
template <>
void Interpolate<AccPair>::initThis()
{
  std::map<double,double> tmpX, tmpZ;

  for (std::map<double,AccPair>::const_iterator it=data.begin(); it!=data.end(); it++) {
    tmpX.insert( std::pair<double,double>(it->first, it->second.x) );
    tmpZ.insert( std::pair<double,double>(it->first, it->second.z) );
  }
  spline.push_back( getSpline(tmpX) ); // x: spline[0]
  spline.push_back( getSpline(tmpZ) ); // z: spline[1]
}

//AccPair
template <>
AccPair Interpolate<AccPair>::interpThis(double xIn) const
{
  AccPair tmp;
  tmp.x = evalSpline(spline[0], xIn);  // x: spline[0]
  tmp.z = evalSpline(spline[1], xIn);  // z: spline[1]
  return tmp;
}



//AccTriple
template <>
void Interpolate<AccTriple>::initThis()
{
  std::map<double,double> tmpX, tmpZ, tmpS;

  for (std::map<double,AccTriple>::const_iterator it=data.begin(); it!=data.end(); it++) {
    tmpX.insert( std::pair<double,double>(it->first, it->second.x) );
    tmpZ.insert( std::pair<double,double>(it->first, it->second.z) );
    tmpS.insert( std::pair<double,double>(it->first, it->second.s) );
  }
  spline.push_back( getSpline(tmpX) ); // x: spline[0]
  spline.push_back( getSpline(tmpZ) ); // z: spline[1]
  spline.push_back( getSpline(tmpS) ); // s: spline[2]
}

//AccTriple
template <>
AccTriple Interpolate<AccTriple>::interpThis(double xIn) const
{
  AccTriple tmp;
  tmp.x = evalSpline(spline[0], xIn);  // x: spline[0]
  tmp.z = evalSpline(spline[1], xIn);  // z: spline[1]
  tmp.s = evalSpline(spline[2], xIn);  // s: spline[2]
  return tmp;
}



// headline entry for "value" in output file
template <>
std::string Interpolate<AccPair>::header() const
{
  return this->data.begin()->second.header();
}

template <>
std::string Interpolate<AccTriple>::header() const
{
  return this->data.begin()->second.header();
}
