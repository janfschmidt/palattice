/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <cmath>
#include <iostream>
#include "functionofpos.hpp"

using namespace std;


// constructor
FunctionOfPos::FunctionOfPos(const gsl_interp_type *t, double periodIn, double circIn)
: Interpolate(t,periodIn), pos(&x), value(&f), circ(circIn), n_turns(1), n_samples(0)
{
}



//get index for pos[] & value[] from index(1turn) and turn
unsigned int FunctionOfPos::index(unsigned int i, unsigned int turnIn) const
{
  if (i >= size() || turnIn > turns())
    throw out_of_range("index or turn out of range.");

  if (turnIn > 1 && i >= samples())
    throw out_of_range("sample out of range.");
  
  return samples()*(turnIn-1) + i - 1;   //("- 1" => starting at index 0)
}



//get index for pos[] & value[] from pos(1turn) and turn
unsigned int FunctionOfPos::index(double posIn, unsigned int turnIn) const
{
  unsigned int j;
  double tmpPos = posTotal(posIn,turnIn);

  for (j=0; j<size(); j++) {
    if (pos[j] == tmpPos)
      return j;
    else if (pos[j] > tmpPos)
      throw eNoData(j);
  }

  throw eNoData(j);
}



// Turn
unsigned int FunctionOfPos::turn(unsigned int i) const
{
  return (i/samples()) + 1;
}

unsigned int FunctionOfPos::turn(double posIn) const
{
  return int(posIn/circ) + 1;
}



// calculate Sample
unsigned int FunctionOfPos::sample(unsigned int i) const
{
  return i%samples();
}



// calculate pos within turn
double FunctionOfPos::posInTurn(double pos) const
{
  return fmod(pos,circ);
}



// calculate "absolute pos" from "pos within turn" and turn
double FunctionOfPos::posTotal(double posInTurn, unsigned int turnIn) const
{
  return  posInTurn + circ*(turnIn-1);
}




//-------- these functions depend on data. thus they can fail (program exit) -----------------------

// get sample ("index modulo turn") by pos, IF IT EXISTS
unsigned int FunctionOfPos::getSample(double posIn) const
{
  unsigned int i;

  try {
    i = index(posIn);
  }
  catch (eNoData &e) {
    cout << "WARNING: FunctionOfPos:getSample(): There is no Sample Point at pos=" << posIn << endl;
    exit(1);
  }

  return getSample(i);
}



// get position
double FunctionOfPos::getPos(unsigned int i, unsigned int turnIn) const
{
  try {
    return pos[index(i,turnIn)];
  }
  catch (out_of_range) { //use what() !!
    cout << "ERROR: FunctionOfPos:getPos(): Index "<<i<<" turnIn "<<turnIn<<" is out of range ("
	 <<samples()<<" samples, " <<turns()<< " turns)" << endl;
    exit(1);
  }
}



// get Value
double FunctionOfPos::get(unsigned int i, unsigned int turnIn) const
{
  try {
    return value[index(i,turnIn)];
  }
  catch (out_of_range) { //use what() !!
    cout << "ERROR: FunctionOfPos:getPos(): Index "<<i<<" turn "<<turnIn<<" is out of range ("
	 <<samples()<<" samples, " <<turns()<< " turns)" << endl;
    exit(1);
  }
}

double FunctionOfPos::get(double pos, unsigned int turnIn) const
{
  //idee: index(pos,turnIn) throw exception wenn pos kein datenpunkt,
  //hier dann catch durch interp(pos)
    return value[index(pos,turnIn)];
}



// modify value
void FunctionOfPos::set(double valueIn, unsigned int i, unsigned int turnIn)
{
  try {
    unsigned int tmpIndex = index(i,turnIn);
    value[tmpIndex] = valueIn;
    reset(); //reset interpolation
  }
  catch (out_of_range) { //use what() !!
    cout << "ERROR: FunctionOfPos:set(): Element"
	 << " is out of range. Please use push_back(value,position) to append new data." << endl;
    cout << "Data is not changed. Continue." << endl;
    return;
  }
}




void FunctionOfPos::set(double valueIn, double posIn, unsigned int turnIn)
{
  // add new turn(s)
  double newPos = posTotal(posIn,turnIn);
  unsigned int newTurn = turn(newPos);
  if (newTurn > turns()) {
    for (unsigned int t=turns(); t<newTurn; t++) {
      for (unsigned int i=0; i<samples(); i++) {
	pos.push_back( pos[i]+t*circ );
	value.push_back(0.0);
      }
      n_turns++;
    }
  }
  
  try {
    unsigned int tmpIndex = index(posIn,turnIn);
    // +++++++++++++++++++++++++++++++++++++
    //if pos does exist, modify value
    // ++++++++++++++++++++++++++++++++++++++
    value[tmpIndex] = valueIn;
    reset(); //reset interpolation
  }
  // ++++++++++++++++++++++++++++++++++++++
  // if pos does not exist, create new data
  // ++++++++++++++++++++++++++++++++++++++
  catch (eNoData &e) {
    vector<double>::iterator it;
    double newPosInTurn = posInTurn(posTotal(posIn,turnIn));
    unsigned int newSample = sample(e.index);
    if (newSample == 0 && newPosInTurn > pos[samples()-1]) { // distinguish insert begin/end of turn
      newSample = samples();
    }
    // add sample point in each turn
    for (unsigned int t=turns(); t>0; t--) {
      it = pos.begin() + (t-1)*samples() + newSample;
      pos.insert(it, newPosInTurn+(t-1)*circ);
      value.insert(it, 0.0);
    }
    n_samples++;
    // call this function again to set value
    set(valueIn,posIn,turnIn);
    reset(); //reset interpolation
    return;
  }
}



void FunctionOfPos::clear()
{
  n_turns = 1;
  n_samples = 0;
  pos.clear();
  value.clear();
  reset(); //reset interpolation
}



// output to file
void FunctionOfPos::out(char *filename) const
{
  cout << "Bauen Sie dies ein!" << endl;
}



// test for existence of data
bool FunctionOfPos::exists(double pos, unsigned int turnIn=1) const
{
  try {
    index(pos,turnIn);
  }
  catch (eNoData) {
    return false;
  }

  return true;
}
