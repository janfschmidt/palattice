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


template <class T=double>
class FunctionOfPos : public Interpolate<T> {

protected:
  // "data" (stored in Interpolate members, rename for this context)
  vector<double> &pos;                        //position: absolute, increasing with turn
  vector<T> &value;

  unsigned int n_turns;                       //number of turns (initialized as 1)
  unsigned int n_samples;                     //number of samples per turn (initialized as zero)


private:
  unsigned int index(unsigned int i, unsigned int turn) const; //get index for pos[] & value[] from index(1turn) and turn
  unsigned int index(double pos, unsigned int turn) const; //get index for pos[] & value[] from pos(1turn) and turn. returns -1 if there is no data for given pos


public:
  const double circ;                          //circumference of accelerator

  FunctionOfPos(double circIn=164.4, const gsl_interp_type *t=gsl_interp_akima, double periodIn=0.);
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
  T get(unsigned int i, unsigned int turn=1) const;     //get value-DATA by index or by index(1turn) and turn

  // >>> to get value by position use interp(double pos) (inherited from Interpolate) <<<

  // these functions modify data
  void modify(T valueIn, unsigned int i, unsigned int turn=1); //modify value by index or by index(1turn) and turn
  void set(T valueIn, double pos, unsigned int turn=1);        //set (existing or new) value by pos or by pos(1turn) and turn
  void clear();
  void pop_back_turn(); // erase data of last turn, reduces turns by 1

  // tests
  bool exists(double pos, unsigned int turn=1) const; // is there data at pos?
  bool compatible(FunctionOfPos<T> &other, bool verbose=true) const; // can I add/subract with other? (data at same pos?)

  // output to file
  void out(const char *filename) const;

  // operators
  void operator+=(FunctionOfPos<T> &other);
  void operator-=(FunctionOfPos<T> &other);
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
