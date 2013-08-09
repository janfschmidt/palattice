/* === "Function of Position" Classes ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include "functionofpos.hpp"



// constructor
FunctionOfPos::FunctionOfPos(gsl_interp_type *t, double periodIn, double circIn)
: Interpolate(t,periodIn), pos(&x), value(&f), circ(circIn), n_turns(1), n_samples(0)
{
}



//get index for pos[] & value[] from index(1turn) and turn
unsigned int FunctionOfPos::index(unsigned int i, unsigned int turn) const
{
  if (turn == 1)
    return i;
  else if (turn > 1)
    return samples()*(turn-1) + i - 1;   //("- 1" => starting at index 0)
    else  {
      cout << "ERROR: FunctionOfPos:index(): turn=0 is not defined. Set turn=1 (first turn)." << endl;
      return i;
    }
}



//get index for pos[] & value[] from pos(1turn) and turn
unsigned int FunctionOfPos::index(double pos, unsigned int turn) const
{
  unsigned int i;
  double tmpPos = getPos(pos,turn);

  for (i=0; i<size(); i++) {
    if (pos[i] == tmpPos)
      return i;
    else if (pos[i] > tmpPos)
      break;
  }
  cout << "WARNING: FunctionOfPos:index(): There is no Sample Point at pos=" << pos
       << "& turn=" << turn <<". Return next sample point at pos=" << pos[i] << endl;
  return i;
}



// Turn
unsigned int FunctionOfPos::getTurn(unsigned int i) const
{
  return (pos[i]%circ) + 1;
}

unsigned int FunctionOfPos::getTurn(double posIn) const
{
  return (posIn%circ) + 1;
}



// Sample (e.g. "which BPM?")
unsigned int FunctionOfPos::getSample(unsigned int i) const
{
  return i - (samples() * (getTurn(i)-1));
}

unsigned int FunctionOfPos::getSample(double posIn) const
{
  unsigned int i;
  for (i=0; i<size(); i++) {
    if (pos[i] == posIn)
      return getSample(i);
    else if (pos[i] > posIn) {
      break;
    }
  }
  cout << "WARNING: FunctionOfPos:getSample(): There is no Sample Point at pos=" << posIn
       << ". Return next sample point at pos=" << pos[i-1] << endl;
  return getSample(i);
}



// Position
double FunctionOfPos::getPos(unsigned int i, unsigned int turn) const
{
  return pos[index(i,turn)];
}

double FunctionOfPos::getPos(double pos1turn, unsigned int turn) const
{
  double tmpPos;
  
  if (turn == 1) tmpPos = pos1turn;
  else if (turn > 1) tmpPos = circ*(turn-1) + pos1turn;
  else {
    cout << "ERROR: FunctionOfPos:getPos(): turn=0 is not defined. Set turn=1 (first turn)." << endl;
    tmpPos = pos1turn;
  }
  
  return tmpPos;
}


// get Value
double FunctionOfPos::get(unsigned int i, unsigned int turn) const
{
  return value[index(i,turn)];
}

double FunctionOfPos::get(double pos, unsigned int turn) const
{
  //idee: index(pos,turn) throw exception wenn pos kein datenpunkt,
  //hier dann catch durch interp(pos)
    return value[index(pos,turn)];
}



// set Value
void set(double valueIn, unsigned int i, unsigned int turn)
{
  // wenn vorhanden, value[index(i,turn)] = valueIn
  // sonst insert
}



void set(double value, double pos, unsigned int turn)
{
    // wenn vorhanden, value[index(pos,turn)] = valueIn
  // sonst insert
}
