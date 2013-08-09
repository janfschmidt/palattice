/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __FUNCTIONOFPOS_HPP_
#define __FUNCTIONOFPOS_HPP_

#include "interpolate.hpp"



class FunctionOfPos : public Interpolate {

protected:
  vector<double> &pos;                        //position: absolute, increasing with turn
  vector<double> &value;

  double circ;                                //circumference of accelerator
  unsigned int n_turns;                       //number of turns
  unsigned int n_samples;                     //number of samples per turn


private:
  FunctionOfPos(vector<double> *posIn); //...
  unsigned int index(unsigned int i, unsigned int turn) const; //get index for pos[] & value[] from index(1turn) and turn
  unsigned int index(double pos, unsigned int turn) const; //get index for pos[] & value[] from pos(1turn) and turn


public:
  FunctionOfPos(gsl_interp_type *t=gsl_interp_akima, double periodIn=0., double circIn=164.4);
  ~FunctionOfPos() {}

  unsigned int turns() const {return n_turns;}
  unsigned int samples() const {return n_samples;}
  unsigned int size() const {return value->size();}

  unsigned int getTurn(unsigned int i) const;
  unsigned int getTurn(double pos) const;
  unsigned int getSample(unsigned int i) const;
  unsigned int getSample(double pos) const;

  double getPos(unsigned int i, unsigned int turn=1) const;//get pos by index or by index(1turn) and turn
  double getPos(double pos1turn, unsigned int turn=1) const; //get pos by pos1turn and turn
  double get(unsigned int i, unsigned int turn=1) const;   //get value by index or by index(1turn) and turn
  double get(double pos, unsigned int turn=1) const;       //get value by pos or by pos(1turn) and turn

  void set(double valueIn, unsigned int i, unsigned int turn=1);//set value by index or by index(1turn) and turn
  void set(double valueIn, double pos, unsigned int turn=1);    //set value by pos or by pos(1turn) and turn
  void clear();

  void out(char *filename) const;
};



#endif
/*__FUNCTIONOFPOS_HPP_*/
