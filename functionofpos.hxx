/* === "Function of Position" Class ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * ! first "turn"   = 1  !
 * ! first "sample" = 0  !
 *
 */


#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;


// constructor
template <class T>
FunctionOfPos<T>::FunctionOfPos(const gsl_interp_type *t, double periodIn, double circIn)
  : Interpolate<T>::Interpolate(t,periodIn), pos(this->x), value(this->f), circ(circIn), n_turns(1), n_samples(0)
{
}



//get index for pos[] & value[] from index(1turn) and turn
template <class T>
unsigned int FunctionOfPos<T>::index(unsigned int i, unsigned int turnIn) const
{
  stringstream msg;
  if (i >= size()) {
    msg << "Index "<<i<<" is out of range (" <<size()<< " elements).";
    throw out_of_range(msg.str());
  }
  if (turnIn > turns()) {
    msg << "Turn "<<turnIn<<" is out of range (" <<turns()<< " turns).";
    throw out_of_range(msg.str());
  }
  if (turnIn > 1 && i >= samples()) {
    msg << "Sample "<<i<<" is out of range (" <<samples()<< " samples).";
    throw out_of_range(msg.str());
  }
  
  return samples()*(turnIn-1) + i;
}



//get index for pos[] & value[] from pos(1turn) and turn
template <class T>
unsigned int FunctionOfPos<T>::index(double posIn, unsigned int turnIn) const
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
template <class T>
unsigned int FunctionOfPos<T>::turn(unsigned int i) const
{
  return (i/samples()) + 1;
}

template <class T>
unsigned int FunctionOfPos<T>::turn(double posIn) const
{
  return int(posIn/circ) + 1;
}



// calculate Sample
template <class T>
unsigned int FunctionOfPos<T>::sample(unsigned int i) const
{
  if (i==0) return 0;     // avoid floating point exception
  else return i%samples();
}



// calculate pos within turn
template <class T>
double FunctionOfPos<T>::posInTurn(double pos) const
{
  return fmod(pos,circ);
}



// calculate "absolute pos" from "pos within turn" and turn
template <class T>
double FunctionOfPos<T>::posTotal(double posInTurn, unsigned int turnIn) const
{
  return  posInTurn + circ*(turnIn-1);
}




//-------- these functions depend on data. thus they can fail (program exit) -----------------------

// get sample ("index modulo turn") by pos, IF IT EXISTS
template <class T>
unsigned int FunctionOfPos<T>::getSample(double posIn) const
{
  unsigned int i;

  try {
    i = index(posIn, 1);
  }
  catch (eNoData &e) {
    cout << "ERROR: FunctionOfPos:getSample(): There is no Sample Point at pos=" << posIn << endl;
    exit(1);
  }

  return sample(i);
}



// get position
template <class T>
double FunctionOfPos<T>::getPos(unsigned int i, unsigned int turnIn) const
{
  try {
    return pos[index(i,turnIn)];
  }
  catch (out_of_range &e) {
    cout << "ERROR: FunctionOfPos:getPos(): " << e.what() << endl;
    exit(1);
  }
}

template <class T>
double FunctionOfPos<T>::getPosInTurn(unsigned int i, unsigned int turnIn) const
{
  return posInTurn( getPos(i,turnIn) );
}



// get Value
template <class T>
T FunctionOfPos<T>::get(unsigned int i, unsigned int turnIn) const
{
  try {
    return value[index(i,turnIn)];
  }
  catch (out_of_range &e) {
    cout << "ERROR: FunctionOfPos:get(): " << e.what() << endl;
    exit(1);
  }
}



// modify value at given index
template <class T>
void FunctionOfPos<T>::modify(T valueIn, unsigned int i, unsigned int turnIn)
{
  try {
    unsigned int tmpIndex = index(i,turnIn);
    value[tmpIndex] = valueIn;
    this->reset(); //reset interpolation
  }
  catch (out_of_range &e) {
    cout << "ERROR: FunctionOfPos:modify(): " << e.what() << endl;
    cout << "Please use set(value,position) to add new data." << endl;
    cout << "Data is not changed. Continue." << endl;
    return;
  }
}




// set value at given position. 
template <class T>
void FunctionOfPos<T>::set(T valueIn, double posIn, unsigned int turnIn)
{
  T empty = T();
  // add new turn(s)
  double newPos = posTotal(posIn,turnIn);
  unsigned int newTurn = turn(newPos);
  if (newTurn > turns()) {
    for (unsigned int t=turns(); t<newTurn; t++) {
      for (unsigned int i=0; i<samples(); i++) {
	pos.push_back( pos[i]+t*circ );
	value.push_back(empty);
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
    this->reset(); //reset interpolation
  }
  // ++++++++++++++++++++++++++++++++++++++
  // if pos does not exist, create new data
  // ++++++++++++++++++++++++++++++++++++++
  catch (eNoData &e) {
    vector<double>::iterator it_pos;
    typename vector<T>::iterator it_value;
    unsigned int newIndex;
    double newPosInTurn = posInTurn(posTotal(posIn,turnIn));
    unsigned int newSample = sample(e.index);
    // distinguish insert begin/end of turn
    if (newSample == 0) {
      if (samples() == 0)                       // (first data point at all)
	newSample = samples();
      else if (newPosInTurn > pos[samples()-1]) // end of turn
	newSample = samples();
    }
    // add sample point in each turn
    for (unsigned int t=turns(); t>0; t--) {
      newIndex = (t-1)*samples() + newSample;   // new data is inserted BEFORE this index
      it_pos = pos.begin() + newIndex;
      it_value = value.begin() + newIndex;
      pos.insert(it_pos, newPosInTurn+(t-1)*circ);
      value.insert(it_value, empty);
    }
    n_samples++;
    // call this function again to set value
    set(valueIn,posIn,turnIn);
    return;
  }
}



template <class T>
void FunctionOfPos<T>::clear()
{
  n_turns = 1;
  n_samples = 0;
  pos.clear();
  value.clear();
  this->reset(); //reset interpolation
}



// output to file
template <class T>
void FunctionOfPos<T>::out(char *filename) const
{
  fstream file;
  const int w = 14;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: FunctionOfPos:out(): Cannot open " << filename << "." << endl;
   return;
 }

 file <<"#"<<setw(w)<<"posInTurn"<<setw(w)<<"pos"<<setw(w)<<"value" << endl;
 file <<setprecision(3);
 
 for (unsigned int i=0; i<size(); i++) {
   file << resetiosflags(ios::scientific) << setiosflags(ios::fixed);
   file <<setw(w+1)<< posInTurn(i) <<setw(w)<< pos[i];
   file << resetiosflags(ios::fixed) << setiosflags(ios::scientific);
   file <<setw(w)<< value[i] << endl;
 }

 file.close();
 cout << "* Wrote "<< filename  << endl;
}



// test for existence of data
template <class T>
bool FunctionOfPos<T>::exists(double pos, unsigned int turnIn) const
{
  try {
    index(pos,turnIn);
  }
  catch (eNoData) {
    return false;
  }

  return true;
}
