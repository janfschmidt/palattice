/* === "Function of Position" Class ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * ! first "turn"   = 1  !
 * ! first "sample" = 0  !
 *
 */

#ifndef __LIBPAL_FUNCTIONOFPOS_HPP_
#define __LIBPAL_FUNCTIONOFPOS_HPP_

#include <exception>
#include <stdexcept>
#include "Interpolate.hpp"
#include "Spectrum.hpp"
#include "ELSASpuren.hpp"
#include "Metadata.hpp"
#include "types.hpp"
#include "SimTools.hpp"
#include "config.hpp"

namespace pal
{

template <class T=double>
class FunctionOfPos : public Interpolate<T> {

protected:
  // "data" (stored in Interpolate members, rename for this context)
  std::vector<double> &pos;                        //position: absolute, increasing with turn
  std::vector<T> &value;

  unsigned int n_turns;                       //number of turns (initialized as 1)
  unsigned int n_samples;                     //number of samples per turn (initialized as zero)

  // returns filename for trajectory files of particle p from madx or elegant at observation point obs:
  string trajectoryFile(string path, SimTool s, unsigned int obs, unsigned int p) const;



private:
  unsigned int index(unsigned int i, unsigned int turn) const; //get index for pos[] & value[] from index(1turn) and turn
  unsigned int index(double pos, unsigned int turn) const; //get index for pos[] & value[] from pos(1turn) and turn
  void hide_last_turn(); // reduce turns by one (only do this, if you need pos=0. value to avoid extrapolation for non-periodic function!)


public:
  const double circ;                          //circumference of accelerator
  Metadata info;

  FunctionOfPos(double circIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, vector<double> posIn, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
  ~FunctionOfPos() {}

  unsigned int turns() const {return n_turns;}
  unsigned int samples() const {return n_samples;}
  unsigned int size() const {return value.size();}
  double posMax() const {return pos.back();}

  unsigned int turn_by_index(unsigned int i) const;
  unsigned int turn(double pos) const;
  unsigned int sample(unsigned int i) const;
  double posInTurn(double posTotal) const;
  double posTotal(double posInTurn, unsigned int turn) const;

  //-------- these functions depend on data. thus they can fail (program exit) -----------------------
  unsigned int getSample(double pos) const;                  //get sample ("index modulo turn") by pos, IF IT EXISTS
  double getPos(unsigned int i, unsigned int turn=1) const;  //get pos-DATA by index or by index(1turn) and turn
  double getPosInTurn(unsigned int i, unsigned int turn=1) const;
  T get(unsigned int i, unsigned int turn=1) const;          //get value-DATA by index or by index(1turn) and turn
  vector<double> getVector(AccAxis axis=x) const;            //get vector of values (choose axis for multidim.)

  // >>> to get value by position use interp(double pos) (inherited from Interpolate) <<<

  // these functions modify data
  void modify(T valueIn, unsigned int i, unsigned int turn=1); //modify value by index or by index(1turn) and turn
  void set(T valueIn, double pos, unsigned int turn=1);        //set (existing or new) value by pos or by pos(1turn) and turn
  //   !! set() is much slower than modify(). you can initialize equidistant positions by samples&turns in constructor & use modify.
  void clear();
  void pop_back_turn();  // erase data of last turn, reduces turns by 1

  void readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn); // import a column of data from a madx/elegant file. valColumn usually has 1 entry, 2 for AccPair(x,z), 3 for AccTriple(x,z,s)
  void readSimToolColumn(SimTool t, string file, string posColumn, vector<string> valColumn, string latticeFile=""); //if a latticeFile is given, madx/elegant is executed.

  // orbit import
  void simToolClosedOrbit(SimToolInstance &s);                  //import closed orbit from madx (twiss file) or elegant (clo file)
  void madxClosedOrbit(string madxFile, SimToolMode m=online) {SimToolInstance mad(pal::madx, m, madxFile); simToolClosedOrbit(mad);}
  void elegantClosedOrbit(string elegantFile, SimToolMode m=online) {SimToolInstance ele(pal::elegant, m, elegantFile); simToolClosedOrbit(ele);}
  void elsaClosedOrbit(ELSASpuren &spuren, unsigned int t);    //import closed orbit from ELSA measurement at time t/ms
  // trajectory import
  void simToolTrajectory(SimToolInstance &s, unsigned int particle); //import single particle trajectory from madx/elegant tracking "obs"/"watch" files
  void madxTrajectory(string madxFile, unsigned int particle, SimToolMode m=online) {SimToolInstance mad(pal::madx, m, madxFile); simToolTrajectory(mad,particle);}
  void elegantTrajectory(string elegantFile, unsigned int particle, SimToolMode m=online) {SimToolInstance ele(pal::elegant, m, elegantFile); simToolTrajectory(ele,particle);}

  // tests
  bool exists(double pos, unsigned int turn=1) const; // is there data at pos?
  bool compatible(FunctionOfPos<T> &other, bool verbose=true) const; // can I add/subract with other? (data at same pos?)

  // output to file
  std::string header() const;
  void out(const char *filename) const;

  // operators
  void operator+=(FunctionOfPos<T> &other);
  void operator-=(FunctionOfPos<T> &other);

  // construct Spectrum (FFT) from this FunctionOfPos (for 1D values, chosen by axis)
  // axis is ignored for 1D data (int or double)
  Spectrum getSpectrum(AccAxis axis=x, unsigned int fmaxrevIn=30, double ampcut=0.) const;

};


// template function specializations
template<> vector<double> FunctionOfPos<double>::getVector(AccAxis axis) const;
template<> vector<double> FunctionOfPos<int>::getVector(AccAxis axis) const;
template<> vector<double> FunctionOfPos<AccPair>::getVector(AccAxis axis) const;
template<> vector<double> FunctionOfPos<AccTriple>::getVector(AccAxis axis) const;
template<> void FunctionOfPos<AccPair>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn);
template<> void FunctionOfPos<AccTriple>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn);
template<> void FunctionOfPos<AccPair>::simToolClosedOrbit(SimToolInstance &s);
  template<> void FunctionOfPos<AccPair>::simToolTrajectory(SimToolInstance &s, unsigned int particle);
template<> void FunctionOfPos<AccPair>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t);
template<> string FunctionOfPos<AccPair>::header() const;
template<> string FunctionOfPos<AccTriple>::header() const;


string axis_string(AccAxis a);


// exceptions
class eNoData : public std::exception {
public:
  const unsigned int index;

  eNoData(unsigned int In) : index(In) {};
};

} //namespace pal


#include "FunctionOfPos.hxx"

#endif
/*__LIBPAL_FUNCTIONOFPOS_HPP_*/
