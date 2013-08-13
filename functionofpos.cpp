/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include "functionofpos.hpp"

using namespace std;


// constructor
FunctionOfPos::FunctionOfPos(gsl_interp_type *t, double periodIn, double circIn)
: Interpolate(t,periodIn), pos(&x), value(&f), circ(circIn), n_turns(1), n_samples(0)
{
}



//get index for pos[] & value[] from index(1turn) and turn
unsigned int FunctionOfPos::index(unsigned int i, unsigned int turn) const
{
  if (i >= size() || turn > turns())
    throw out_of_range();

  if (turn > 1 && i >= samples())
    throw out_of_range();
  
  return samples()*(turn-1) + i - 1;   //("- 1" => starting at index 0)
}



//get index for pos[] & value[] from pos(1turn) and turn
unsigned int FunctionOfPos::index(double pos, unsigned int turn) const
{
  unsigned int j;
  double tmpPos = posTotal(pos,turn);

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
  return (i%samples()) + 1;
}

unsigned int FunctionOfPos::turn(double posIn) const
{
  return (posIn%circ) + 1;
}



// calculate Sample (e.g. "which BPM?")
unsigned int FunctionOfPos::sample(unsigned int i) const
{
  return i - (samples() * (getTurn(i)-1));
}



// calculate pos within turn
double FunctionOfPos::posInTurn(double pos) const
{
  return pos - ((pos%circ) * circ);
}



// calculate "absolute pos" from "pos within turn" and turn
double FunctionOfPos::posTotal(double posInTurn, unsigned int turn) const
{
  return  posInTurn + circ*(turn-1);
}



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
double FunctionOfPos::getPos(unsigned int i, unsigned int turn) const
{
  try {
    return pos[index(i,turn)];
  }
  catch (out_of_range) {
    cout << "ERROR: FunctionOfPos:getPos(): Index "<<i<<" turn "<<turn<<" is out of range ("
	 <<samples()<<" samples, " <<turns()<< " turns)" << endl;
    exit(1);
  }
}



// get Value
double FunctionOfPos::get(unsigned int i, unsigned int turn) const
{
  try {
    return value[index(i,turn)];
  }
  catch (out_of_range) {
    cout << "ERROR: FunctionOfPos:getPos(): Index "<<i<<" turn "<<turn<<" is out of range ("
	 <<samples()<<" samples, " <<turns()<< " turns)" << endl;
    exit(1);
  }
}

double FunctionOfPos::get(double pos, unsigned int turn) const
{
  //idee: index(pos,turn) throw exception wenn pos kein datenpunkt,
  //hier dann catch durch interp(pos)
    return value[index(pos,turn)];
}



// modify value
void set(double valueIn, unsigned int i, unsigned int turn)
{
  try {
    unsigned int tmpIndex = index(i,turn);
  }
  catch (out_of_range) {
    cout << "ERROR: FunctionOfPos:set(): Element "<<tmpIndex
	 << " is out of range. Please use push_back(value,position) to append new data." << endl;
    cout << "Data is not changed. Continue." << endl;
    return;
  }

  value[tmpIndex] = valueIn;
}

void set(double valueIn, double posIn, unsigned int turn)
{
  try {
    unsigned int tmpIndex = index(posIn,turn);
  }
  catch (eNoData) {
    cout << "ERROR: FunctionOfPos:set(): There is no data for pos="
	 <<posIn<< " and turn=" <<turn<< endl;
    cout << "Data is not changed. Continue." << endl;
    return;
  }

  value[tmpIndex] = valueIn; 
}



void FunctionOfPos::push_back(double valueIn, double posIn, unsigned int turn)
{

  pos.push_back(posIn);
  value.push_back(valueIn);
}
