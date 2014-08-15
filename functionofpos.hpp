/* === "Function of Position" Class ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * ! first "turn"   = 1  !
 * ! first "sample" = 0  !
 *
 */

#ifndef __FUNCTIONOFPOS_HPP_
#define __FUNCTIONOFPOS_HPP_

#include <exception>
#include <stdexcept>
#include "interpolate.hpp"
#include "spectrum.hpp"
#include "types.hpp"



template <class T=double>
class FunctionOfPos : public Interpolate<T> {

protected:
  // "data" (stored in Interpolate members, rename for this context)
  std::vector<double> &pos;                        //position: absolute, increasing with turn
  std::vector<T> &value;

  unsigned int n_turns;                       //number of turns (initialized as 1)
  unsigned int n_samples;                     //number of samples per turn (initialized as zero)

  // returns filename for trajectory files of particle p from madx or elegant at observation point obs:
  string trajectoryFile(string path, simulationTool s, unsigned int obs, unsigned int p) const;



private:
  unsigned int index(unsigned int i, unsigned int turn) const; //get index for pos[] & value[] from index(1turn) and turn
  unsigned int index(double pos, unsigned int turn) const; //get index for pos[] & value[] from pos(1turn) and turn
  void hide_last_turn(); // reduce turns by one (only do this, if you need pos=0. value to avoid extrapolation for non-periodic function!)


public:
  const double circ;                          //circumference of accelerator

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

  // orbit import
  void madxClosedOrbit(const char *madxTwissFile);                   //import closed orbit from madx twiss file
  void madxTrajectory(string path, unsigned int particle); //import single particle trajectory from madx tracking "obs" files at each quadrupole
  void elegantClosedOrbit(const char *elegantCloFile);               //import closed orbit from elegant .clo file
  void elegantTrajectory(string path, unsigned int particle); //import single particle trajectory from elegant tracking "watch" files at each quadrupole
  void elsaClosedOrbit(BPM *ELSAbpms, unsigned int t);               //import closed orbit from ELSA measurement at time t/ms

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
  Spectrum getSpectrum(AccAxis axis=x, unsigned int fmaxrevIn=30, double ampcut=0.) const;
};




// exceptions
class eNoData : public std::exception {
public:
  const unsigned int index;

  eNoData(unsigned int In) : index(In) {};
};




#include "functionofpos.hxx"

#endif
/*__FUNCTIONOFPOS_HPP_*/
