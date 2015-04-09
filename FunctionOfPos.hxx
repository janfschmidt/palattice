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

template <class T>
void FunctionOfPos<T>::circCheck()
{
  if (this->circ <= 0.) {
    stringstream msg;
    msg << "FunctionOfPos: circumference (" << this->circ << ") must be positive.";
    throw libpalError(msg.str());
  }
}

// constructor (set circumference & interpolation)
template <class T>
FunctionOfPos<T>::FunctionOfPos(double circIn, const gsl_interp_type *t)
  : Interpolate<T>::Interpolate(t,circIn), n_turns(1), circ(circIn)
{
  circCheck();
}

// constructor (set circumference from SimToolInstance)
template <class T>
FunctionOfPos<T>::FunctionOfPos(SimToolInstance &sim, const gsl_interp_type *t)
  : Interpolate<T>::Interpolate(t), n_turns(1), circ(sim.readCircumference())
{
  circCheck();
  this->period = circ; //set period (Interpolate class)
}

// constructor to additionally set EQUIDISTANT positions with given stepwidth for given #turns (increase speed for set())
// template <class T>
// FunctionOfPos<T>::FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, const gsl_interp_type *t)
//   : Interpolate<T>::Interpolate(t,circIn*turnsIn,samplesIn*turnsIn), pos(this->_x), value(this->f), n_turns(turnsIn), n_samples(samplesIn), circ(circIn)
// {
//   circCheck();

//   //initialize positions
//   double stepwidth = circ/samples();
//   for (unsigned int i=0; i<size(); i++) {
//     T empty = T();
//     pos[i] = sample(i)*stepwidth + (turn_by_index(i)-1)*circ;
//     value[i] =  empty;
//   }
// }

// constructor to set positions by vector
// template <class T>
// FunctionOfPos<T>::FunctionOfPos(double circIn, unsigned int samplesIn, unsigned int turnsIn, vector<double> posIn, const gsl_interp_type *t)
//   : Interpolate<T>::Interpolate(t,circIn*turnsIn, samplesIn*turnsIn), pos(this->_x), value(this->f), n_turns(turnsIn), n_samples(samplesIn), circ(circIn)
// {
//   circCheck();

//   if (n_samples != posIn.size()) {
//     stringstream msg;
//     msg << "Given number of samples (" <<n_samples<< ") does not match number of given positions (" <<posIn.size()<< ")";
//     throw invalid_argument(msg.str());
//   }
//   T empty = T();
//   for (unsigned int t=1; t<=turns(); t++) {
//     for (unsigned int i=0; i<samples(); i++) {
//       pos[index(i,t)] = posTotal(posIn[i], t);
//       value[index(i,t)] =  empty;
//     }
//   }
// }



//get index for pos[] & value[] from index(1turn) and turn
// template <class T>
// unsigned int FunctionOfPos<T>::index(unsigned int i, unsigned int turnIn) const
// {
//   stringstream msg;
//   if (i >= size()) {
//     msg << "Index "<<i<<" is out of range (" <<size()<< " elements).";
//     throw out_of_range(msg.str());
//   }
//   if (turnIn > turns()) {
//     msg << "Turn "<<turnIn<<" is out of range (" <<turns()<< " turns).";
//     throw out_of_range(msg.str());
//   }
//   if (turnIn > 1 && i >= samples()) {
//     msg << "Sample "<<i<<" is out of range (" <<samples()<< " samples).";
//     throw out_of_range(msg.str());
//   }
  
//   return samples()*(turnIn-1) + i;
// }



//get index for pos[] & value[] from pos(1turn) and turn
// template <class T>
// unsigned int FunctionOfPos<T>::index(double posIn, unsigned int turnIn) const
// {
//   unsigned int j;
//   double tmpPos = posTotal(posIn,turnIn);

//   for (j=0; j<size(); j++) {
//     if (abs(pos[j] - tmpPos) < ZERO_DISTANCE)
//       return j;
//     else if (pos[j] > tmpPos)
//       throw eNoData(j);
//   }

//   throw eNoData(j);
// }



// Turn
// template <class T>
// unsigned int FunctionOfPos<T>::turn_by_index(unsigned int i) const
// {
//   return int(i/samples() + ZERO_DISTANCE) + 1; // ensure return of next turn for i=samples
// }

template <class T>
unsigned int FunctionOfPos<T>::turn(double posIn) const
{
  return int(posIn/circ + ZERO_DISTANCE) + 1; // ensure return of next turn for pos=circ
}



// calculate Sample
// template <class T>
// unsigned int FunctionOfPos<T>::sample(unsigned int i) const
// {
//   if (i==0) return 0;     // avoid floating point exception
//   else return i%samples();
// }



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

template <class T>
unsigned int FunctionOfPos<T>::samplesInTurn(unsigned int turn) const
{
  unsigned int n=0;
  double posend = turn*circ;
  for ( const_FoPiterator it=data.lower_bound((turn-1)*circ); it!=data.end(); it++) {
    if (it->first < posend)
      n++;
    else
      break;
  }
  return n;
}




//-------- these functions depend on data. thus they can fail (program exit) -----------------------

// get sample ("index modulo turn") by pos, IF IT EXISTS
// template <class T>
// unsigned int FunctionOfPos<T>::getSample(double posIn) const
// {
//   unsigned int i;

//   try {
//     i = index(posIn, 1);
//   }
//   catch (eNoData &e) {
//     cout << "ERROR: FunctionOfPos:getSample(): There is no Sample Point at pos=" << posIn << endl;
//     exit(1);
//   }

//   return sample(i);
// }



// get position
// template <class T>
// double FunctionOfPos<T>::getPos(unsigned int i) const
// {
//   try {
//     return pos[index(i,turnIn)];
//   }
//   catch (out_of_range &e) {
//     cout << "ERROR: FunctionOfPos:getPos(): " << e.what() << endl;
//     exit(1);
//   }
// }

// template <class T>
// double FunctionOfPos<T>::getPosInTurn(unsigned int i, unsigned int turnIn) const
// {
//   return posInTurn( getPos(i,turnIn) );
// }



// get Value
template <class T>
T FunctionOfPos<T>::get(unsigned int i) const
{
  if (i >= data.size()) {
    std::stringstream msg;
    msg << "FunctionOfPos<T>::get(): index" << i << "out of data range (" << data.size() <<")";
    throw libpalError(msg.str());
  }
  FoPiterator it;
  for (unsigned int k=0; k<i; k++) {
    it++;
  }
  return it->second;
}



// // modify value at given index
// template <class T>
// void FunctionOfPos<T>::modify(T valueIn, unsigned int i, unsigned int turnIn)
// {
//   try {
//     unsigned int tmpIndex = index(i,turnIn);
//     value[tmpIndex] = valueIn;
//     this->reset(); //reset interpolation
//   }
//   catch (out_of_range &e) {
//     cout << "ERROR: FunctionOfPos:modify(): " << e.what() << endl;
//     cout << "Please use set(value,position) to add new data." << endl;
//     cout << "Data is not changed. Continue." << endl;
//     return;
//   }
// }




// set value at given position. 
// template <class T>
// void FunctionOfPos<T>::set(T valueIn, double posIn, unsigned int turnIn)
// {
//   double newPos = posTotal(posIn,turnIn);
//   unsigned int newTurn = turn(newPos);
//   // add new turn(s)
//   if (newTurn > turns()) {
//     T empty = T();
//     for (unsigned int t=turns(); t<newTurn; t++) {
//       for (unsigned int i=0; i<samples(); i++) {
// 	pos.push_back( pos[i]+t*circ );
// 	value.push_back(empty);
//       }
//       n_turns++;
//     }
//   }
  
//   try {
//     unsigned int tmpIndex = index(posIn,turnIn); // TEUER!!!!!
//     // +++++++++++++++++++++++++++++++++++++
//     //if pos does exist, modify value
//     // ++++++++++++++++++++++++++++++++++++++
//     value[tmpIndex] = valueIn;
//     this->reset(); //reset interpolation
//   }
//   // ++++++++++++++++++++++++++++++++++++++
//   // if pos does not exist, create new data
//   // ++++++++++++++++++++++++++++++++++++++
//   catch (eNoData &e) {
//     T empty = T();
//     vector<double>::iterator it_pos;
//     typename vector<T>::iterator it_value;
//     unsigned int newIndex;
//     double newPosInTurn = posInTurn(posTotal(posIn,turnIn));
//     unsigned int newSample = sample(e.index);
//     // distinguish insert begin/end of turn
//     if (newSample == 0) {
//       if (samples() == 0)                       // (first data point at all)
// 	newSample = samples();
//       else if (newPosInTurn > pos[samples()-1]) // end of turn
// 	newSample = samples();
//     }
//     // add sample point in each turn
//     for (unsigned int t=turns(); t>0; t--) {
//       newIndex = (t-1)*samples() + newSample;   // new data is inserted BEFORE this index
//       it_pos = pos.begin() + newIndex;
//       it_value = value.begin() + newIndex;
//       pos.insert(it_pos, newPosInTurn+(t-1)*circ);
//       value.insert(it_value, empty);
//     }
//     n_samples++;
//     // now set value
//     unsigned int tmpIndex = index(posIn,turnIn);
//     value[tmpIndex] = valueIn;
//     this->reset(); //reset interpolation
//     return;
//   }
// }

//set value at given position. 
template <class T>
void FunctionOfPos<T>::set(T valueIn, double posIn, unsigned int turnIn) {
  std::pair<FoPiterator,bool> ret;
  double pos = posTotal(posIn,turnIn);

  // insert data
  ret = data.insert( std::pair<double,T>(pos, valueIn) );
  // increase n_turns if necessary
  unsigned int t = turn(pos);
  if (t > turns()) n_turns = t;

  // if insert failed (key already existed), replace value
  if (ret.second == false)
    ret.first->second = valueIn;
}


template <class T>
void FunctionOfPos<T>::clear()
{
  n_turns = 1;
  data.clear();
  this->reset(); //reset interpolation
}



// erase data of last turn, reduces turns by 1
template <class T>
void FunctionOfPos<T>::pop_back_turn()
{
  data.erase( data.lower_bound( posTotal(0.,n_turns) ), data.end());
  n_turns -= 1;
}



// print FunctionOfPos.  If no filename is given, print to stdout
template <class T>
void FunctionOfPos<T>::print(string filename) const
{
  stringstream s;
  fstream file;
  const int w = 14;

  //write text to s
 s << this->info.out("#");
 s <<"#"<<setw(w)<<"pos / m"<<setw(w)<<"posInTurn"<<setw(w)<<"turn"<<"\t"<< this->header() << endl;
 s <<setprecision(3);
 
 for (const_FoPiterator it=data.begin(); it!=data.end(); it++) {
   s << resetiosflags(ios::scientific) << setiosflags(ios::fixed);
   s <<setw(w+1)<< it->first <<setw(w)<< posInTurn(it->first) <<setw(w)<< turn(it->first);
   s << resetiosflags(ios::fixed) << setiosflags(ios::scientific);
   s <<setw(w)<< it->second << endl;
 }

 //output of s
 if (filename == "")
   cout << s.str();
 else {
   file.open(filename.c_str(), ios::out);
   if (!file.is_open())
     throw libpalFileError(filename);
   file << s.str();
   file.close();
   cout << "* Wrote "<< filename  << endl;
 }
}


// test for existence of data
template <class T>
bool FunctionOfPos<T>::exists(double pos, unsigned int turnIn) const
{
  return data.count(posTotal(pos,turnIn));
}



// test for compatibility with other
// compatible means:
//   1. circumferences are equal
//   2. same number of turns or "other" has only 1 turn  (+= or -= add/sub. this 1 turn to/from each turn)
template <class T>
bool FunctionOfPos<T>::compatible(FunctionOfPos<T> &o, bool verbose) const
{
  if ( turns() == 0 || o.turns() == 0 ) {
    if (verbose)
      cout <<endl<<"=== FunctionOfPos<T> objects are not compatible, zero turns are not allowed. ===" << endl;
    return false;
  }
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
  // if ( turns()!=o.turns() && o.turns()==1 ) {
  //   // check if all turns have same number of samples as the turn of o
  //   unsigned int osamples = o.samplesInTurn(1);
  //   for (unsigned int t=1; t<=turns(); t++) {
  //     if (samplesInTurn(t)!=osamples) {
  // 	if (verbose) {
  // 	  cout <<endl<< "=== FunctionOfPos<T> objects are not compatible! ===" << endl;
  // 	  cout << "Number of samples is different in turn " <<t<< "("<<samplesInTurn(t) <<"/"<<osamples <<")." << endl;
  // 	}
  // 	return false;
  //     }
  //   }
  // }
  // if ( turns()==o.turns() ) {
  //   // check if all turns have same number of samples
  //   for (unsigned int t=1; t<=turns(); t++) {
  //     if (samplesInTurn(t)!=o.samplesInTurn(t)) {
  // 	if (verbose) {
  // 	  cout <<endl<< "=== FunctionOfPos<T> objects are not compatible! ===" << endl;
  // 	  cout << "Number of samples is different in turn " <<t<< "("<<samplesInTurn(t) <<"/"<<o.samplesInTurn(t) <<")." << endl;
  // 	}
  // 	return false;
  //     }
  //   }
  // }

  return true;
}



// import a column of data from a madx/elegant file.
// valColumn usually has 1 entry, 2 for AccPair(x,z), 3 for AccTriple(x,z,s)
template <class T>
void FunctionOfPos<T>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn)
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

  for (unsigned int i=0; i<tab.rows(); i++) {
    double pos = tab.get<double>(posColumn,i);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    this->set(tab.get<T>(valColumn[0],i), pos);
  }

  //metadata
  this->info.simToolImport(s);
  this->info.add("Data Source file", file);
  this->info.add("read Parameter", valColumn[0]);
  this->headerString = valColumn[0];
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
    throw invalid_argument("Addition of FunctionOfPos<T> objects not possible (incompatible circumference or number of turns).");

  const_FoPiterator it = data.begin();
  if (other.turns()==1 && turns()>1) {    // special case: add this 1 turn of other to each turn
    for (; it!=data.end(); it++)
      it->second += other.interp( posInTurn(it->first) );
  }
  else {                                  // "usual case": same number of turns
    for (; it!=data.end(); it++)
      it->second += other.interp( it->first);
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
    throw invalid_argument("Addition of FunctionOfPos<T> objects not possible (incompatible circumference or number of turns).");

  const_FoPiterator it = data.begin();
  if (other.turns()==1 && turns()>1) {    // special case: add this 1 turn of other to each turn
    for (; it!=data.end(); it++)
      it->second -= other.interp( posInTurn(it->first) );
  }
  else {                                  // "usual case": same number of turns
    for (; it!=data.end(); it++)
      it->second -= other.interp( it->first);
  }

  this->reset();  // reset interpolation

  string tmp = other.info.getbyLabel("Orbit Source file");
  if (tmp != "NA") {
    this->info.add("subtracted Orbit", tmp);
  }
}

// construct Spectrum (FFT) from this FunctionOfPos
// uses getVector() to generate 1D input data
template <class T>
Spectrum FunctionOfPos<T>::getSpectrum(double stepwidth, AccAxis axis, unsigned int fmaxrevIn, double ampcutIn, string name) const
{
  if (name=="") name = axis_string(axis);
  vector<double> data = this->getVector(stepwidth, axis);
  Spectrum s(name, data, circ, turns(), data.size(), fmaxrevIn, ampcutIn);
  // copy metadata to Spectrum
  for (unsigned int i=2; i<this->info.size(); i++)
    s.info.add(this->info.getLabel(i), this->info.getEntry(i));
  return s;
}
template <class T>
Spectrum FunctionOfPos<T>::getSpectrum(double stepwidth, unsigned int fmaxrevIn, double ampcutIn, string name) const
{
  if (name=="") name = this->header()+"-spectrum";
  return getSpectrum(stepwidth, pal::x, fmaxrevIn, ampcutIn, name);
}




// ----------- defaults for template specialization (FunctionOfPos.cpp)

// get all values as double-vector, axis for multidim. data (-> template specialization)
template <class T>
vector<double> FunctionOfPos<T>::getVector(double stepwidth,AccAxis axis) const
{
  stringstream s;
  s << "FunctionOfPos<T>::getVector(): get vector<double> from vector<T> is not implemented for data type "
    << typeid(T).name();
  throw logic_error(s.str());
}




//orbit import is defined only for T=AccPair (-> template specialization)
template <class T>
void FunctionOfPos<T>::simToolClosedOrbit(SimToolInstance &sim)
{
  stringstream s;
  s << "FunctionOfPos<T>::simToolClosedOrbit() is not implemented for data type " << typeid(T).name()
    << ". It is only defined for T=AccPair.";
  throw logic_error(s.str());
}

template <class T>
void FunctionOfPos<T>::simToolTrajectory(SimToolInstance &sim, unsigned int particle)
{
  stringstream s;
  s << "FunctionOfPos<T>::simToolTrajectory() is not implemented for data type " << typeid(T).name()
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
