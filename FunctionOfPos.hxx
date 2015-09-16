/* === "Function of Position" Class ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
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
    throw palatticeError(msg.str());
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





template <class T>
unsigned int FunctionOfPos<T>::turn(double posIn) const
{
  return int(posIn/circ + ZERO_DISTANCE) + 1; // ensure return of next turn for pos=circ
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
  ret = data.insert( std::pair<double,T>(pos, valueIn) );
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

  // tests for equal number of samples. not needed, because += and -= use interpolation of other objects data.
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
    throw palatticeError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);

  SimToolTable tab = s.readTable(file, columns);

  for (unsigned int i=0; i<tab.rows(); i++) {
    double pos = tab.get<double>(i,posColumn);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    this->set(tab.get<T>(i,valColumn[0]), pos);
  }

  //metadata
  this->info.simToolImport(s);
  this->info.add("Data Source file", file);
  this->info.add("read Parameter", valColumn[0]);
  this->headerString = valColumn[0];
}

// as above, but no need to fill vector
template <class T>
void FunctionOfPos<T>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, string valX, string valZ, string valS)
{
  std::vector<std::string> valColumn { valX };
  if (!valZ.empty()) valColumn.push_back(valZ);
  if (!valS.empty()) valColumn.push_back(valS);
  readSimToolColumn(s, file, posColumn, std::move(valColumn));
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





//import single particle data from madx/elegant tracking "obs"/"watch" files
template <class T>
void FunctionOfPos<T>::readSimToolParticleColumn(SimToolInstance &s, unsigned int particle, string valX, string valZ, string valS)
{
  this->clear(); //delete old data

  //SimTool columns
  vector<string> columns;
  if (s.tool == pal::madx) {
    columns.push_back("S");
    columns.push_back("TURN");
  }
  else if (s.tool == pal::elegant) {
    // s: parameter in file header
    columns.push_back("Pass");
  }

  columns.push_back(valX);
  if (!valZ.empty()) columns.push_back(valZ);
  if (!valS.empty()) columns.push_back(valS);

  unsigned int obs;
  std::string trajFile;
  SimToolTable tab;


  //for chosen particle: read data from all observation points
  //-----------------------------------------------------------------------------------------------------
  //madx & obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  T otmp;
  unsigned int turn;
  double obsPos;
  if (s.tool==pal::madx) obs=1;
  else if (s.tool==pal::elegant) obs=0;
  //iterate all existing obs files:
  while (true) {
    // read table from file:
    trajFile=s.trajectory(obs,particle);
    try {
      tab = s.readTable(trajFile, columns);
    }
    catch (palatticeFileError) {
      obs--;
      break;
    }
    // read obs position
    if (s.tool==pal::madx)
      obsPos = tab.getd(0,"S");
    else if (s.tool==pal::elegant)
      obsPos = s.readParameter<double>(trajFile,"position_s/m"); // read parameter in file header
    // write table rows to FunctionOfPos:
    //    if (s.tool==pal::elegant && obs==this->samples()) //elegant: obs-file at lattice end only needed for last turn
    //  break;
    for (unsigned int i=0; i<tab.rows(); i++) {
      if (s.tool==pal::madx) {
	turn = tab.get<unsigned int>(i,"TURN");
	// otmp.x = tab.getd("X",i);
	// otmp.z = tab.getd("Y",i);
	if (obs==1) { //see comment above ("madx & obs0001")
	  turn += 1;
	}
      }
      else if (s.tool==pal::elegant) {
	turn = tab.get<unsigned int>(i,"Pass");
	// otmp.x = tab.getd("x",i);
	// otmp.z = tab.getd("y",i);
      }
      otmp = tab.get<T>(i, valX, valZ, valS);
      this->set(otmp, obsPos, turn);
    }
    obs++;
  }
  //last turn has only one "real" entry: at the begin of the lattice to avoid extrapolation
  //it is read from:
  //- obs0001 for madx (see comment above)
  //- dedicated last obs (at lattice end) for elegant (implemented below)
  if (s.tool==pal::elegant) {
    turn = tab.get<unsigned int>(tab.rows()-1,"Pass") + 1;
    // otmp.x = tab.getd("x",tab.rows()-1);
    // otmp.z = tab.getd("y",tab.rows()-1);
    otmp = tab.get<T>(tab.rows()-1, valX, valZ, valS);
    this->set(otmp, 0, turn);
  }
  this->hide_last_turn();
  //this->pop_back_turn();
  if(this->periodic) this->period=circ*n_turns;

   //metadata
  this->info.add("Single Particle Data from", s.tool_string());
  this->info.add("Data Source path", s.path());
  stringstream stmp;
  stmp << particle;
  this->info.add("particle number", stmp.str());
  stmp.str(std::string());
  stmp << turns();
  this->info.add("number of turns", stmp.str());
  stmp.str(std::string());
  stmp << samplesInTurn(1);
  this->info.add("number of obs. points", stmp.str());
}
