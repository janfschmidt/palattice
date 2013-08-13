/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __FUNCTIONOFPOS_HPP_
#define __FUNCTIONOFPOS_HPP_

#inlcude <exception>
#include <stdexcept>
#include "interpolate.hpp"



class FunctionOfPos : public Interpolate {

protected:
  // "data" (stored in Interpolate members)
  vector<double> &pos;                        //position: absolute, increasing with turn
  vector<double> &value;

  const double circ;                          //circumference of accelerator
  unsigned int n_turns;                       //number of turns
  unsigned int n_samples;                     //number of samples per turn


private:
  FunctionOfPos(vector<double> *posIn); //...
  unsigned int index(unsigned int i, unsigned int turn) const; //get index for pos[] & value[] from index(1turn) and turn
  unsigned int index(double pos, unsigned int turn) const; //get index for pos[] & value[] from pos(1turn) and turn. returns -1 if there is no data for given pos


public:
  FunctionOfPos(gsl_interp_type *t=gsl_interp_akima, double periodIn=0., double circIn=164.4);
  ~FunctionOfPos() {}

  unsigned int turns() const {return n_turns;}
  unsigned int samples() const {return n_samples;}
  unsigned int size() const {return value.size();}
  double posMax() const {return pos.back();}

  unsigned int turn(unsigned int i) const;
  unsigned int turn(double pos) const;
  unsigned int sample(unsigned int i) const;
  double posInTurn(double pos) const;
  double posTotal(double posInTurn, unsigned int turn) const;

  //-------- these functions depend on data. thus they can fail (program exit) -----------------------
  unsigned int getSample(double pos) const;                  //get sample ("index modulo turn") by pos, IF IT EXISTS
  double getPos(unsigned int i, unsigned int turn=1) const;  //get pos-DATA by index or by index(1turn) and turn
  double get(unsigned int i, unsigned int turn=1) const;     //get value-DATA by index or by index(1turn) and turn
  double get(double pos, unsigned int turn=1) const;         //get value-DATA by pos or by pos(1turn) and turn

  // these functions modify data
  void set(double valueIn, unsigned int i, unsigned int turn=1);  //modify value by index or by index(1turn) and turn
  void set(double valueIn, double pos, unsigned int turn=1);      //modify value by pos or by pos(1turn) and turn
  void push_back(double valueIn, double pos, unsigned int turn=1);//add value&pos or value&pos(1turn)&turn
  void clear();

  void out(char *filename) const;  //output to file
};




// exceptions
class eNoData : public std::exception {
public:
  const unsigned int index;

  eNoData(unsigned int indexIn) : index(indexIn) {};
};



#endif
/*__FUNCTIONOFPOS_HPP_*/
