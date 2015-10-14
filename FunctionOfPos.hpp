/* === "Function of Position" Class ===
 * contain any physical data, value(pos), for an accelerator ring, e.g. orbit- or field-data.
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 *
 * ! first "turn"   = 1  !
 * ! first "sample" = 0  !
 *
 */

#ifndef __LIBPALATTICE_FUNCTIONOFPOS_HPP_
#define __LIBPALATTICE_FUNCTIONOFPOS_HPP_

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
  using Interpolate<T>::data;
  typedef typename std::map<double,T>::iterator FoPiterator;
  typedef typename std::map<double,T>::const_iterator const_FoPiterator;
  
  //std::map<double,T> data -> is inherited from Interpolate<T>
  unsigned int n_turns;                 //number of turns (initialized as 1)
  double circ;                          //circumference of accelerator


private:
  void hide_last_turn() {n_turns-=1;} // reduce turns by one (only do this, if you need pos=0. value to avoid extrapolation for non-periodic function!)
  void circCheck();


public:
  //Metadata info -> is inherited from Interpolate<T>

  FunctionOfPos(double circIn, const gsl_interp_type *t=gsl_interp_akima);
  FunctionOfPos(SimToolInstance &sim, const gsl_interp_type *t=gsl_interp_akima_periodic); //get circ from SimToolInstance
  FunctionOfPos(const FunctionOfPos &other) = default;
  FunctionOfPos(FunctionOfPos &&other) = default;
  FunctionOfPos& operator=(const FunctionOfPos &other) = default;
  ~FunctionOfPos() {}

  double circumference() const {return circ;}
  unsigned int turns() const {return n_turns;}
  unsigned int size() const {return data.size();}
  double posMax() const {return data.rbegin()->first;}

  unsigned int turn(double pos) const;
  double posInTurn(double posTotal) const;
  double posTotal(double posInTurn, unsigned int turn) const;
  unsigned int samplesInTurn(unsigned int turn) const;

  //statistics of data (no interpolation used)
  T mean() const;
  T rms() const;

  // these functions depend on data. thus they can throw palatticeError exception
  T get(unsigned int i) const;          //get value-DATA by index or by index(1turn) and turn
  vector<double> getVector(double stepwidth=0.1, AccAxis axis=x) const;  //get vector of equidistant values (choose axis for multidim.)
  // T interp(double pos) -> inherited from Interpolate<T> allows access of data by position

  // these functions modify data
  void set(T valueIn, double pos, unsigned int turn=1);        //set (existing or new) value by pos or by pos(1turn) and turn
  void clear();
  void pop_back_turn();  // erase data of last turn, reduces turns by 1

  void readSimToolColumn(SimToolInstance &s, string file, string posColumn, string valX, string valZ="", string valS=""); // import a column of data from usual madx/elegant table files like twiss, closed-orbit etc.
  void readSimToolParticleColumn(SimToolInstance &s, unsigned int particle, string valX, string valZ="", string valS=""); // import single particle data from madx/elegant tracking "obs"/"watch" files

  // orbit import
  void simToolClosedOrbit(SimToolInstance &s);                  //import closed orbit from madx (twiss file) or elegant (clo file)
  void madxClosedOrbit(string madxFile, SimToolMode m=online) {SimToolInstance mad(pal::madx, m, madxFile); simToolClosedOrbit(mad);}
  void elegantClosedOrbit(string elegantFile, SimToolMode m=online) {SimToolInstance ele(pal::elegant, m, elegantFile); simToolClosedOrbit(ele);}
  void elsaClosedOrbit(ELSASpuren &spuren, unsigned int t);    //import closed orbit from ELSA measurement at time t/ms
  // trajectory import
  void simToolTrajectory(SimToolInstance &s, unsigned int particle); //import single particle trajectory from madx/elegant tracking "obs"/"watch" files
  void madxTrajectory(string madxFile, unsigned int particle, SimToolMode m=online) {SimToolInstance mad(pal::madx, m, madxFile); simToolTrajectory(mad,particle);} //if m=offline, file is only used to get path & output filenames without extension
  void elegantTrajectory(string elegantFile, unsigned int particle, SimToolMode m=online) {SimToolInstance ele(pal::elegant, m, elegantFile); simToolTrajectory(ele,particle);}

  // tests
  bool exists(double pos, unsigned int turn=1) const; // is there data at pos?
  bool compatible(FunctionOfPos<T> &other, bool verbose=true) const; // can I add/subract with other? (data at same pos?)

  // output
  void print(string filename="") const;

  // operators
  // interpolated values of other are used,
  // so datapoints can be different, but circumference & number of turns have to match.
  // exclusion: other has 1 turn only. then this one turn is used with every turn
  void operator+=(FunctionOfPos<T> &other);
  void operator-=(FunctionOfPos<T> &other);

  // construct Spectrum (FFT) from this FunctionOfPos (for 1D values, chosen by axis).
  // axis is ignored for 1D data (int or double)
  // FFT is done with equidistant data (given stepwidth in m) from interpolation.
  // if stepwidth=0, data is taken without interpolation.
  // default Spectrum name is axis_string(axis) (e.g. "horizontal" for x).
  Spectrum getSpectrum(double stepwidth=0.1, AccAxis axis=x, unsigned int fmaxrev=30, double ampcut=0., string name="") const;
  //1D version without axis. default name is this->header()
  Spectrum getSpectrum(double stepwidth=0.1, unsigned int fmaxrev=30, double ampcut=0., string name="") const;
};


// template function specializations
template<> vector<double> FunctionOfPos<double>::getVector(double stepwidth,AccAxis axis) const;
template<> vector<double> FunctionOfPos<int>::getVector(double stepwidth,AccAxis axis) const;
template<> vector<double> FunctionOfPos<AccPair>::getVector(double stepwidth,AccAxis axis) const;
template<> vector<double> FunctionOfPos<AccTriple>::getVector(double stepwidth,AccAxis axis) const;
template<> void FunctionOfPos<AccPair>::simToolClosedOrbit(SimToolInstance &s);
template<> void FunctionOfPos<AccPair>::simToolTrajectory(SimToolInstance &s, unsigned int particle);
template<> void FunctionOfPos<AccPair>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t);


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
/*__LIBPALATTICE_FUNCTIONOFPOS_HPP_*/
