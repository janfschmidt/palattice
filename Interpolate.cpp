/* === Interpolate Class ===
 * supplies values f(_x) for arbitrary _x for given arrays f and _x. uses gsl interpolation
 * _x is double, f can have several types.
 * for each type the init() function must be implemented, because gsl interpolation needs double data type.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include "Interpolate.hpp"


// =========== template function specialization ============

//double
template <>
void Interpolate<double>::initThis()
{
  spline.push_back( getSpline(f) );
}

//double
template <>
double Interpolate<double>::interpThis(double xIn)
{
  double tmp;
  tmp = evalSpline(spline[0], xIn);
  return tmp;
}



//AccPair
template <>
void Interpolate<AccPair>::initThis()
{
  vector<double> tmpX, tmpZ;

  for (unsigned int i=0; i<f.size(); i++) {
    tmpX.push_back( f[i].x );
    tmpZ.push_back( f[i].z );
  }
  spline.push_back( getSpline(tmpX) ); // x: spline[0]
  spline.push_back( getSpline(tmpZ) ); // z: spline[1]
}

//AccPair
template <>
AccPair Interpolate<AccPair>::interpThis(double xIn)
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
  vector<double> tmpX, tmpZ, tmpS;

  for (unsigned int i=0; i<f.size(); i++) {
    tmpX.push_back( f[i].x );
    tmpZ.push_back( f[i].z );
    tmpS.push_back( f[i].s );
  }
  spline.push_back( getSpline(tmpX) ); // x: spline[0]
  spline.push_back( getSpline(tmpZ) ); // z: spline[1]
  spline.push_back( getSpline(tmpS) ); // s: spline[2]
}

//AccTriple
template <>
AccTriple Interpolate<AccTriple>::interpThis(double xIn)
{
  AccTriple tmp;
  tmp.x = evalSpline(spline[0], xIn);  // x: spline[0]
  tmp.z = evalSpline(spline[1], xIn);  // z: spline[1]
  tmp.s = evalSpline(spline[2], xIn);  // s: spline[2]
  return tmp;
}



// headline entry for "value" in output file
template <>
string Interpolate<AccPair>::header() const
{
  return this->f[0].header();
}
template <>
string Interpolate<AccTriple>::header() const
{
  return this->f[0].header();
}
