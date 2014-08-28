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
#include <typeinfo>


using namespace std;
using namespace pal;


// constructor (set circumference[default 164.4] & interpolation)
template <class T>
FunctionOfPos<T>::FunctionOfPos(double circIn, const gsl_interp_type *t, double periodIn)
  : Interpolate<T>::Interpolate(t,periodIn), pos(this->_x), value(this->f), n_turns(1), n_samples(0), circ(circIn)
{
}

// constructor to additionally set EQUIDISTANT positions with given stepwidth for given #turns (increase speed for set())
template <class T>
FunctionOfPos<T>::FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, const gsl_interp_type *t, double periodIn)
  : Interpolate<T>::Interpolate(t,periodIn,samplesIn*turnsIn), pos(this->_x), value(this->f), n_turns(turnsIn), n_samples(samplesIn), circ(circIn)
{
  //initialize positions
  double stepwidth = circ/samples();
  for (unsigned int i=0; i<size(); i++) {
    T empty = T();
    pos[i] = sample(i)*stepwidth + (turn_by_index(i)-1)*circ;
    value[i] =  empty;
  }
}

// constructor to set positions by vector
template <class T>
FunctionOfPos<T>::FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, vector<double> posIn, const gsl_interp_type *t, double periodIn)
  : Interpolate<T>::Interpolate(t,periodIn, samplesIn*turnsIn), pos(this->_x), value(this->f), n_turns(turnsIn), n_samples(samplesIn), circ(circIn)
{
  if (n_samples != posIn.size()) {
    stringstream msg;
    msg << "Given number of samples (" <<n_samples<< ") does not match number of given positions (" <<posIn.size()<< ")";
    throw invalid_argument(msg.str());
  }
  T empty = T();
  for (unsigned int t=1; t<=turns(); t++) {
    for (unsigned int i=0; i<samples(); i++) {
      pos[index(i,t)] = posTotal(posIn[i], t);
      value[index(i,t)] =  empty;
    }
  }
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
    if (abs(pos[j] - tmpPos) < ZERO_DISTANCE)
      return j;
    else if (pos[j] > tmpPos)
      throw eNoData(j);
  }

  throw eNoData(j);
}



// Turn
template <class T>
unsigned int FunctionOfPos<T>::turn_by_index(unsigned int i) const
{
  return int(i/samples() + ZERO_DISTANCE) + 1; // ensure return of next turn for i=samples
}

template <class T>
unsigned int FunctionOfPos<T>::turn(double posIn) const
{
  return int(posIn/circ + ZERO_DISTANCE) + 1; // ensure return of next turn for pos=circ
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
  double tmp = fmod(pos,circ);
  if ( abs(tmp-circ) < ZERO_DISTANCE ) return 0.0; // pos=circ is always returned as pos=0 in next turn
  else return tmp;
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
  double newPos = posTotal(posIn,turnIn);
  unsigned int newTurn = turn(newPos);
  // add new turn(s)
  if (newTurn > turns()) {
    T empty = T();
    for (unsigned int t=turns(); t<newTurn; t++) {
      for (unsigned int i=0; i<samples(); i++) {
	pos.push_back( pos[i]+t*circ );
	value.push_back(empty);
      }
      n_turns++;
    }
  }
  
  try {
    unsigned int tmpIndex = index(posIn,turnIn); // TEUER!!!!!
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
    T empty = T();
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
    // now set value
    unsigned int tmpIndex = index(posIn,turnIn);
    value[tmpIndex] = valueIn;
    this->reset(); //reset interpolation
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



// erase data of last turn, reduces turns by 1
template <class T>
void FunctionOfPos<T>::pop_back_turn()
{
  for (unsigned int i=0; i<samples(); i++) {
    pos.pop_back();
    value.pop_back();
  }
  n_turns -= 1;
}

// reduce turns by one (only do this, if you need pos=0. value to avoid extrapolation for non-periodic function!)
template <class T>
void FunctionOfPos<T>::hide_last_turn()
{
  n_turns -= 1;
}



// headline entry for "value" in output file (specialization for AccPair & AccTriple)
template <class T>
string FunctionOfPos<T>::header() const
{
  return "value";
}

// output to file
template <class T>
void FunctionOfPos<T>::out(const char *filename) const
{
  fstream file;
  const int w = 14;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: FunctionOfPos:out(): Cannot open " << filename << "." << endl;
   return;
 }

 file << info.out("#");
 file <<"#"<<setw(w)<<"pos / m"<<setw(w)<<"posInTurn"<<setw(w)<<"turn"<<"\t"<< this->header() << endl;
 file <<setprecision(3);
 
 for (unsigned int i=0; i<size(); i++) {
   file << resetiosflags(ios::scientific) << setiosflags(ios::fixed);
   file <<setw(w+1)<< pos[i] <<setw(w)<< getPosInTurn(i) <<setw(w)<< turn_by_index(i);
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



// test for compatibility with other
// compatible means:
//   1. circumferences are equal
//   2. same number of turns or "other" has only 1 turn (+= or -= add/sub. this 1 turn to/from each turn)
template <class T>
bool FunctionOfPos<T>::compatible(FunctionOfPos<T> &o, bool verbose) const
{
  if ( circ != o.circ) {
    if (verbose) {
      cout <<endl<<"=== FunctionOfPos<T> objects are not compatible! ===" << endl;
      cout << "Circumferences are different ("<<circ <<"/"<<o.circ <<"). So I guess it is not the same accelerator." << endl;
    }
    return false;
  }
  if ( turns()!=o.turns() && o.turns()>1 ) { // special case: o.turns=1: use this 1 turn with each turn (e.g. closed orbit)
    if (verbose) {
      cout <<endl<< "=== FunctionOfPos<T> objects are not compatible! ===" << endl;
      cout << "Number of turns is different ("<<turns() <<"/"<<o.turns() <<")." << endl;
    }
    return false;
  }

  return true;
}



// import a column of data from a madx/elegant file.
// valColumn usually has 1 entry, 2 for AccPair(x,z), 3 for AccTriple(x,z,s)
template <class T>
void FunctionOfPos<T>::readSimToolColumn(SimToolInstance s, string file, string posColumn, vector<string> valColumn)
{
  if (valColumn.size() != 1) {
    stringstream msg;
    msg << "FunctionOfPos::readSimToolColumn(): valColumn should have 1(not "
	<<valColumn.size()<<") entry for 1D data type " << typeid(T).name();
    throw libpalError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);

  SimToolTable tab = s.readTable(file, columns);

  for (unsigned int i=0; i<tab.rows(); i++)
    this->set(tab.get<T>(valColumn[0],i), tab.get<double>(posColumn,i));
}


// import a column of data from a madx/elegant file. if a latticeFile is given, madx/elegant is executed.
template <class T>
void FunctionOfPos<T>::readSimToolColumn(SimTool t, string file, string posColumn, vector<string> valColumn, string latticeFile)
{
  if (latticeFile=="")
    SimToolInstance s(t, pal::offline, file);
  else
    SimToolInstance s(t, pal::online, latticeFile);

  readSimToolColumn(s, file, posColumn, valColumn);
}





// ---------------- operators -------------------------
template <class T>
void FunctionOfPos<T>::operator+=(FunctionOfPos<T> &other)
{
  if (!compatible(other))
    throw invalid_argument("Addition of FunctionOfPos<T> objects not possible (incompatible # or pos of samples).");

  if (other.turns()==1 && turns()>1) {    // special case: add this 1 turn of other to each turn
    for (unsigned int i=0; i<size(); i++)
      value[i] += other.interp( getPosInTurn(i) );
  }
  else {                                  // "usual case": same number of turns
    for (unsigned int i=0; i<size(); i++)
      value[i] += other.interp( getPos(i) );
  }

  this->reset();  // reset interpolation

  string tmp = other.info.getbyLabel("Orbit Source file");
  if (tmp != "NA") {
    this->info.add("added Orbit", tmp);
  }
}

template <class T>
void FunctionOfPos<T>::operator-=(FunctionOfPos<T> &other)
{
  if (!compatible(other))
    throw invalid_argument("Subtraction of FunctionOfPos<T> objects not possible (incompatible # or pos of samples).");
  
  if (other.turns()==1 && turns()>1) {    // special case: subtract this 1 turn of other from each turn
    for (unsigned int i=0; i<size(); i++)
      value[i] -= other.interp( getPosInTurn(i) );
  }
  else {                                  // "usual case": same number of turns
    for (unsigned int i=0; i<size(); i++)
      value[i] -= other.interp( getPos(i) );
  }
  
  this->reset();  // reset interpolation

  string tmp = other.info.getbyLabel("Orbit Source file");
  if (tmp != "NA") {
    this->info.add("subtracted Orbit", tmp);
  }
}




// ----------- defaults for template specialization (FunctionOfPos.cpp)

// construct Spectrum (FFT) from this FunctionOfPos
// uses getVector() to generate 1D input data
template <class T>
Spectrum FunctionOfPos<T>::getSpectrum(AccAxis axis, unsigned int fmaxrevIn, double ampcutIn) const
{
  vector<double> data = this->getVector(axis);
  Spectrum s(axis_string(axis), data, circ, turns(), data.size(), fmaxrevIn, ampcutIn);
  // copy metadata to Spectrum
  for (unsigned int i=2; i<this->info.size(); i++)
    s.info.add(this->info.getLabel(i), this->info.getEntry(i));
  return s;
}



// get all values as double-vector, axis for multidim. data (-> template specialization)
template <class T>
vector<double> FunctionOfPos<T>::getVector(AccAxis axis) const
{
  stringstream s;
  s << "FunctionOfPos<T>::getVector(): get vector<double> from vector<T> is not implemented for data type "
    << typeid(T).name();
  throw logic_error(s.str());
}




//orbit import is defined only for T=AccPair (-> template specialization)
template <class T>
void FunctionOfPos<T>::simToolClosedOrbit(SimToolInstance sim)
{
  stringstream s;
  s << "FunctionOfPos<T>::simToolClosedOrbit() is not implemented for data type " << typeid(T).name()
    << ". It is only defined for T=AccPair.";
  throw logic_error(s.str());
}

template <class T>
void FunctionOfPos<T>::madxTrajectory(string path, unsigned int particle)
{
  stringstream s;
  s << "FunctionOfPos<T>::madxTrajectory() is not implemented for data type " << typeid(T).name()
    << ". It is only defined for T=AccPair.";
  throw logic_error(s.str());
}


template <class T>
void FunctionOfPos<T>::elegantTrajectory(string path, unsigned int particle)
{
  stringstream s;
  s << "FunctionOfPos<T>::elegantTrajectory() is not implemented for data type " << typeid(T).name()
    << ". It is only defined for T=AccPair.";
  throw logic_error(s.str());
}

template <class T>
void FunctionOfPos<T>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t)
{
  stringstream s;
  s << "FunctionOfPos<T>::elsaClosedOrbit() is not implemented for data type " << typeid(T).name()
    << ". It is only defined for T=AccPair.";
  throw logic_error(s.str());
}


// returns filename for trajectory files of particle p from madx or elegant at observation point obs
template <class T>
string FunctionOfPos<T>::trajectoryFile(string path, SimTool s, unsigned int obs, unsigned int p) const
{
  char tmp[512];
  string out;
  if (s == madx)
    sprintf(tmp, "%s/madx.obs%04d.p%04d", path.c_str(), obs, p);
  else // elegant
    sprintf(tmp, "%s/elegant.w%03d.p%d", path.c_str(), obs, p);
  out = tmp;

  return out;
}
