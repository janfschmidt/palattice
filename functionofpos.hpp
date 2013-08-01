/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#ifndef __FUNCTIONOFPOS_HPP_
#define __FUNCTIONOFPOS_HPP_

#include "interpolate.hpp"



class FunctionOfPos : public Interpolate {

private:
  vector<double> posdata;
  vector<double> value;
  vector<double> *pos;                        //position (absolute, increasing with turn)

  double circ;                                //circumference of accelerator
  unsigned int n_turns;                       //number of turns
  unsigned int n_samples;                     //number of samples per turn

  FunctionOfPos(vector<double> *p); //...


public:
  FunctionOfPos() : circ(164.4), n_turns(1), n_bpms(0) {pos = &posdata;}
  FunctionOfPos(double circumference, gsl_interp_type *t=gsl_interp_akima) : circ(164.4), n_turns(1), n_bpms(0);
  ~FunctionOfPos() {}

  unsigned int turns() const {return n_turns;}
  unsigned int samples() const {return n_samples;}
  unsigned int size() const {return value.size();}

  unsigned int getTurn(unsigned int i) const;
  unsigned int getTurn(double pos) const;
  unsigned int getSample(unsigned int i, unsigned int turn=1) const;
  unsigned int getSample(double pos, unsigned int turn=1) const;

  double getPos(unsigned int i, unsigned int turn=1) const;
  double get(unsigned int i, unsigned int turn=1) const;   //get value by index or by index(1turn) and turn
  double get(double pos, unsigned int turn=1) const;       //get value by pos or by pos(1turn) and turn

  void set(double value, unsigned int i, unsigned int turn=1);//set value by index or by index(1turn) and turn
  void set(double value, double pos, unsigned int turn=1);    //set value by pos or by pos(1turn) and turn
  void clear();

  void out(char *filename) const;
};



#endif
/*__FUNCTIONOFPOS_HPP_*/
