/* "Function of Position" Class
 * contain any value(pos) for an accelerator ring, e.g. Twiss-, orbit- or field-data.
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * !!! Convention:
 * !!! first "turn"   = 1
 * !!! first "sample" = 0
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
  if (this->circumference() < 0.) {
    stringstream msg;
    msg << "FunctionOfPos: circumference (" << this->circumference() << ") must be positive.";
    throw palatticeError(msg.str());
  }
}

// constructor (set circumference & interpolation)
template <class T>
FunctionOfPos<T>::FunctionOfPos(double circIn, const gsl_interp_type *t)
  : Interpolate<T>::Interpolate(t,circIn), n_turns(1), circ(circIn), verbose(false)
{
  circCheck();
  this->info.add("Circumference (set manually)", circ);
}

// constructor (set circumference from SimToolInstance)
template <class T>
FunctionOfPos<T>::FunctionOfPos(SimToolInstance &sim, const gsl_interp_type *t)
  : Interpolate<T>::Interpolate(t), n_turns(1), circ(sim.readCircumference()), verbose(false)
{
  circCheck();
  this->period = circ; //set period (Interpolate class)
  this->info.add("Circumference", circ);
}





template <class T>
unsigned int FunctionOfPos<T>::turn(double posIn) const
{
  return int(posIn/circumference() + ZERO_DISTANCE) + 1; // ensure return of next turn for pos=circ
}





// calculate pos within turn
template <class T>
double FunctionOfPos<T>::posInTurn(double pos) const
{
  double tmp = fmod(pos,circumference());
  if ( abs(tmp-circumference()) < ZERO_DISTANCE ) return 0.0; // pos=circ is always returned as pos=0 in next turn
  else return tmp;
}



// calculate "absolute pos" from "pos within turn" and turn
template <class T>
double FunctionOfPos<T>::posTotal(double posInTurn, unsigned int turnIn) const
{
  if(turnIn==0) throw palatticeError("FunctionOfPos<T>::posTotal: turn < 1 is invalid!");
  return  posInTurn + circumference()*(turnIn-1);
}

template <class T>
unsigned int FunctionOfPos<T>::samplesInTurn(unsigned int turn) const
{
  if(turn==0) throw palatticeError("FunctionOfPos<T>::samplesInTurn: turn < 1 is invalid!");
  
  unsigned int n=0;
  double posend = turn*circumference();
  for ( const_FoPiterator it=data.lower_bound((turn-1)*circumference()); it!=data.end(); it++) {
    if (it->first < posend)
      n++;
    else
      break;
  }
  return n;
}

template <class T>
T FunctionOfPos<T>::mean() const
{
  T sum = T();
  for (auto &d : data)
    sum += d.second;
  return sum/data.size();
}

template <class T>
T FunctionOfPos<T>::rms() const
{
  T sum = T();
  for (auto &d : data)
    sum += std::pow(d.second,2);
  return std::sqrt( sum/data.size() );
}

template <class T>
T FunctionOfPos<T>::stddev() const
{
  T sum = T();
  T mean = this->mean();
  for (auto &d : data)
    sum += std::pow(d.second-mean, 2);
  return std::sqrt( sum/data.size() );
}





// get Value
template <class T>
T FunctionOfPos<T>::get(unsigned int i) const
{
  if (i >= data.size()) {
    std::stringstream msg;
    msg << "FunctionOfPos<T>::get(): index" << i << "out of data range (" << data.size() <<")";
    throw palatticeError(msg.str());
  }
  FoPiterator it;
  for (unsigned int k=0; k<i; k++) {
    it++;
  }
  return it->second;
}






//set value at given position.
//map::insert adds new value if keys are not EXACTLY equal,
//though "twin-data points" can occur due to numeric accuracy.
//they do not harm, so are accepted to keep set()-performance
template <class T>
void FunctionOfPos<T>::set(T valueIn, double posIn, unsigned int turnIn) {
  std::pair<FoPiterator,bool> ret;
  double pos = posTotal(posIn,turnIn);

  // insert data
  ret = data.insert( std::move(std::pair<double,T>(pos, valueIn)) );
  // increase n_turns if necessary
  unsigned int t = turn(pos);
  if (t > turns()) n_turns = t;

  // if insert failed (key already existed), replace value
  if (ret.second == false)
    ret.first->second = valueIn;

  this->reset(); //reset interpolation
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
 
 for (const_FoPiterator it=data.begin(); it!=data.end(); it++) {
   s << resetiosflags(ios::scientific) << setiosflags(ios::fixed) <<setprecision(3);
   s <<setw(w+1)<< it->first <<setw(w)<< posInTurn(it->first) <<setw(w)<< turn(it->first);
   s << resetiosflags(ios::fixed) << setiosflags(ios::scientific) <<setprecision(6);
   s <<setw(w)<< it->second << endl;
 }

 //output of s
 if (filename == "")
   cout << s.str();
 else {
   file.open(filename.c_str(), ios::out);
   if (!file.is_open())
     throw palatticeFileError(filename);
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
bool FunctionOfPos<T>::compatible(FunctionOfPos<T> &o) const
{
  if ( turns() == 0 || o.turns() == 0 ) {
    if (verbose)
      cout <<endl<<"=== FunctionOfPos<T> objects are not compatible, zero turns are not allowed. ===" << endl;
    return false;
  }
  if ( circumference() != o.circumference()) {
    if (verbose) {
      cout <<endl<<"=== FunctionOfPos<T> objects are not compatible! ===" << endl;
      cout << "Circumferences are different ("<<circumference() <<"/"<<o.circumference() <<"). So I guess it is not the same accelerator." << endl;
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

  // tests for equal number of samples. not needed, because += and -= use interpolation of other objects data.

  return true;
}






// ---------------- operators -------------------------
template <class T>
void FunctionOfPos<T>::operator+=(FunctionOfPos<T> &other)
{
  if (!compatible(other))
    throw invalid_argument("Addition of FunctionOfPos<T> objects not possible (incompatible circumference or number of turns).");

  FoPiterator it = data.begin();
  if (other.turns()==1 && turns()>1) {    // special case: add this 1 turn of other to each turn
    for (; it!=data.end(); it++)
      it->second += other.interp( posInTurn(it->first) );
  }
  else {                                  // "usual case": same number of turns
    for (; it!=data.end(); it++)
      it->second += other.interp( it->first);
  }

  this->reset();  // reset interpolation

  string tmp = other.info.getbyLabel("Data Source file");
  if (tmp != "NA") {
    this->info.add("added Orbit", tmp);
  }
  this->info.addStatistics(mean(),stddev());
}

template <class T>
void FunctionOfPos<T>::operator-=(FunctionOfPos<T> &other)
{
  if (!compatible(other))
    throw invalid_argument("Addition of FunctionOfPos<T> objects not possible (incompatible circumference or number of turns).");

  FoPiterator it = data.begin();
  if (other.turns()==1 && turns()>1) {    // special case: add this 1 turn of other to each turn
    for (; it!=data.end(); it++)
      it->second -= other.interp( posInTurn(it->first) );
  }
  else {                                  // "usual case": same number of turns
    for (; it!=data.end(); it++)
      it->second -= other.interp( it->first);
  }

  this->reset();  // reset interpolation

  string tmp = other.info.getbyLabel("Data Source file");
  if (tmp != "NA") {
    this->info.add("subtracted Orbit", tmp);
  }
  this->info.addStatistics(mean(),stddev());
}

// construct Spectrum (FFT) from this FunctionOfPos
// uses getVector() to generate 1D input data
template <class T>
Spectrum FunctionOfPos<T>::getSpectrum(double stepwidth, AccAxis axis, unsigned int fmaxrevIn, double ampcutIn, string name) const
{
  if (name=="") name = axis_string(axis);
  vector<double> data = this->getVector(stepwidth, axis);
  Spectrum s(name, data, circumference(), turns(), data.size(), fmaxrevIn, ampcutIn);
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








// import a column of data from usual madx/elegant table files like twiss, closed-orbit etc.
template <class T>
void FunctionOfPos<T>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, string valX, string valZ, string valS)
{
  SimToolTable tab;
  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valX);
  if (!valZ.empty()) columns.push_back(valZ);
  if (!valS.empty()) columns.push_back(valS);

  tab = s.readTable(file, columns);
  
  T tmp;
  auto rows = tab.rows();
  
  for (unsigned int i=0; i<rows; i++) {
    double pos = tab.get<double>(i,posColumn);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circumference()) <= 0.0001) continue;

    tmp = tab.get<T>(i, valX, valZ, valS);
    this->set(tmp, pos);
  }
  //if (s.tool==elegant) this->pop_back_turn(); //elegant: always entry with s=circumference due to drifts. Avoid additional turn.

 //metadata
  stringstream stmp;
  this->info.simToolImport(s);
  this->info.add("Data Source file", file);
  stmp << valX;
  if (!valZ.empty()) stmp << ", " << valZ;
  if (!valS.empty()) stmp << ", " << valS;
  this->info.add("read Parameters", stmp.str());
  this->info.addStatistics(mean(),stddev());
  
  this->headerString = valX; //output column header. For 2D/3D AccPair/AccTriple.header() is used
}



//import single particle data from madx/elegant tracking "obs"/"watch" files
template <class T>
void FunctionOfPos<T>::readSimToolParticleColumn(SimToolInstance &s, unsigned int particle, string valX, string valZ, string valS)
{
  if (particle == 0)
    throw palatticeError("readSimToolParticleColumn: "+s.tool_string()+" particle number < 1 is invalid");
  
  this->clear(); //delete old data

  auto columns = getTrajectoryColumns(s, valX,valZ,valS);

  //for chosen particle: read data from all observation points
  //-----------------------------------------------------------------------------------------------------
  //madx & obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  unsigned int obs = 0;
  if (s.tool==pal::madx) obs=1;

  //iterate all existing obs files:
  while (true) {
    string trajFile=s.trajectory(obs,particle);
    if(verbose) cout << "reading file " << trajFile << "\r" << std::flush;
    try {
      SimToolTable tab = s.readTable(trajFile, columns);
      if (s.sddsMode()) {
	tab.filterRows("particleID", particle, particle);
      }
    
      double obsPos = readObsPos(s, tab, trajFile);
 
      // write table rows to FunctionOfPos:
      unsigned int turn;
      // ------
      if (s.sddsMode()) {
	while(true) {
	  turn = tab.getParameter<unsigned int>("Pass") + 1;      
	  T otmp = tab.get<T>(0,valX,valZ,valS); //only 1 row per particle
	  this->set(otmp, obsPos, turn);
	  try {
	    tab.nextPage();
	  }
	  catch(SDDSPageError) {
	    break; //next obs point (=next file)
	  }
	}
      }
      // ------
      else {
	for (unsigned int i=0; i<tab.rows(); i++) {
	  if (s.tool==pal::madx) {
	    turn = tab.get<unsigned int>(i,"TURN");
	    if (obs==1) { //see comment above ("madx & obs0001")
	      turn += 1;
	    }
	  }
	  else if (s.tool==pal::elegant) {
	    turn = tab.get<unsigned int>(i,"Turn"); // +1 included in elegant2libpalattice.sh
	  }
	  T otmp = tab.get<T>(i, valX, valZ, valS);
	  this->set(otmp, obsPos, turn);
	}
      }
      // ------
      obs++;
    }
    catch (palatticeFileError) { //thrown by readTable if file not found
      break;
    }
  }

  hide_last_turn(); //last data point is at begin of next turn (pos=0), but this should not be shown as additional turn
  if (this->periodic)
    this->period = circumference() * n_turns;
  writeTrajectoryMetadata(s,particle, valX,valZ,valS);
}



template <class T>
vector<string> FunctionOfPos<T>::getTrajectoryColumns(const SimToolInstance &s, const string &valX, const string &valZ, const string &valS) const
{
  vector<string> columns;
  if (s.tool == pal::madx) {
    columns.push_back("S");
    columns.push_back("TURN");
  }
  else if (s.tool == pal::elegant && !s.sddsMode()) {
    // s: parameter in file header
    columns.push_back("Turn");
  }
  columns.push_back(valX);
  if (!valZ.empty()) columns.push_back(valZ);
  if (!valS.empty()) columns.push_back(valS);
  return std::move(columns);
}

template <class T>
double FunctionOfPos<T>::readObsPos(SimToolInstance &s, SimToolTable &tab, const string &trajFile) const
{
  double obsPos;
  if (s.tool==pal::madx) {
    obsPos = tab.getd(0,"S");
  }
  else if (s.sddsMode()) {
    obsPos = tab.getParameter<double>("s");
  }
  else if (s.tool==pal::elegant) {
    obsPos = s.readParameter<double>(trajFile,"position_s/m");
  }
  else {
    throw palatticeError("Illegal SimTool in FunctionOfPos<T>::readObsPos");
  }
  return std::move(obsPos);
}

template <class T>
void FunctionOfPos<T>::writeTrajectoryMetadata(SimToolInstance &s, unsigned int particle, const string &valX, const string &valZ, const string &valS)
{
  stringstream stmp;
  this->info.add("Single Particle Data from", s.tool_string());
  this->info.add("Data Source path", s.path());
  stmp << particle;
  this->info.add("particle number", stmp.str());
  stmp.str(std::string());
  stmp << valX;
  if (!valZ.empty()) stmp << ", " << valZ;
  if (!valS.empty()) stmp << ", " << valS;
  this->info.add("read Parameters", stmp.str());
  stmp.str(std::string());
  stmp << turns();
  this->info.add("number of turns", stmp.str());
  stmp.str(std::string());
  stmp << samplesInTurn(1);
  this->info.add("number of obs. points", stmp.str());
  this->info.addStatistics(mean(),stddev());
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
