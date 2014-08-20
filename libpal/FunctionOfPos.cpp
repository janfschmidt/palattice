/* === "Function of Position" Class ===
 * contain any value(pos) for an accelerator ring, e.g. orbit- or field-data.
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * ! first "turn"   = 1  !
 * ! first "sample" = 0  !
 *
 */

#include <string>
#include <sstream>
#include "FunctionOfPos.hpp"

using namespace std;
using namespace pal;

//AccAxis string output
string pal::axis_string(AccAxis a) {
  switch (a) {
  case x:
    return "horizontal";
  case z:
    return "vertical";
  case s:
    return "longitudinal";
  }
  return "Please implement this AccAxis in axis_string() in types.hpp.";
}



// =========== template specialization ============


template <>
vector<double> FunctionOfPos<double>::getVector(AccAxis axis) const
{
  // axis not needed, if only 1D values exist.
  unsigned int i;
  vector<double> out;
  
   for (i=0; i<size(); i++)
     out.push_back( value[i] );
   
   return out;
}


template <>
vector<double> FunctionOfPos<int>::getVector(AccAxis axis) const
{
  // axis not needed, if only 1D values exist.
  unsigned int i;
  vector<double> out;
  
   for (i=0; i<size(); i++)
     out.push_back( int(value[i]) );
   
   return out;
}


template <>
vector<double> FunctionOfPos<AccPair>::getVector(AccAxis axis) const
{
  unsigned int i;
  vector<double> out;

  switch(axis) {
  case s:
    throw invalid_argument("FunctionOfPos<AccPair>::getVector(): s coordinate is not defined for AccPair. Use AccTriple instead.");
    break;
  case x:
    for (i=0; i<size(); i++)
      out.push_back(value[i].x);
    break;
  case z:
    for (i=0; i<size(); i++)
      out.push_back(value[i].z);
    break;
  }

  return out;
}


template <>
vector<double> FunctionOfPos<AccTriple>::getVector(AccAxis axis) const
{
  unsigned int i;
  vector<double> out;

  switch(axis) {
  case s:
    for (i=0; i<size(); i++)
      out.push_back(value[i].s);
    break;
  case x:
    for (i=0; i<size(); i++)
      out.push_back(value[i].x);
    break;
  case z:
    for (i=0; i<size(); i++)
      out.push_back(value[i].z);
    break;
  }

  return out;
}





// ---------------- Orbit import ---------------------
//import closed orbit from madx twiss file
template <>
void FunctionOfPos<AccPair>::madxClosedOrbit(const char *madxTwissFile)
{
  // --------------------------------------------------------------------------------
  // position of orbit-data at END of quadrupole (according to s in madx twiss file).
  // Corresponds to BPM behind the Quadrupole
  // --------------------------------------------------------------------------------

  string tmp;
  string expectedColumns[11] = {"KEYWORD","NAME","S","X","Y","L","ANGLE","K1L","K2L","VKICK","HKICK"};
  unsigned int j;
  double pos, k1l; 
  string s, x, y;
  char *p, *pp, *ppp;
  fstream madxTwiss;
  AccPair otmp;

  madxTwiss.open(madxTwissFile, ios::in);
  if (!madxTwiss.is_open()) {
    cout << "ERROR: FunctionOfPos::madxClosedOrbit(): Cannot open " << madxTwissFile << endl;
    exit(1);
  }

  while (!madxTwiss.eof()) {
    madxTwiss >> tmp;

    // --- check correct column order by comparing headline ---------------------
    if (tmp == "*") {
      for (j=0; j<11; j++) {
	madxTwiss >> tmp;
	//cout << "debug: madx=>" <<tmp<< " expected=>" <<expectedColumns[j] << endl; //debug
	if (tmp != expectedColumns[j]) {
	  cout << "ERROR: FunctionOfPos::madxClosedOrbit(): Unexpected columns (or column order) in " << madxTwissFile << endl;
	  exit(1);
	}
      }
    }
    // --------------------------------------------------------------------------

    if (tmp == "\"QUADRUPOLE\"") {
      madxTwiss >> tmp >> s >> x >> y >> tmp >> tmp >> k1l;
      pos = strtod(s.c_str(), &p);
      otmp.x = strtod(x.c_str(), &pp);
      otmp.z = strtod(y.c_str(), &ppp);
      if (*p!=0 || *pp!=0 || *ppp!=0) {
	cout << "WARNING: FunctionOfPos::madxClosedOrbit(): No correct BPM data @ s="
	     <<s<< "("<<x<<", "<<y<<"). Ignore it and continue." << endl;
      }
      else
	this->set(otmp, pos);
    }
  }

  //metadata
  info.add("Closed Orbit from", "MAD-X");
  info.add("Orbit Source file", madxTwissFile);
}




//import single particle trajectory from madx tracking "obs" files at each quadrupole
template <>
void FunctionOfPos<AccPair>::madxTrajectory(string path, unsigned int particle)
{
  unsigned int obs=1;
  string tmp="init";
  unsigned int turn, theTurn; // madx column variables
  double x, y, s;
  fstream madxFile;
  AccPair otmp;
  vector<double> obsPos;

  //for chosen particle: read turns, samples and positions
  //then modify() can be used below instead of set() ->  much faster for large datasets
  cout << "* Initializing trajectory... " << endl;
  madxFile.open(trajectoryFile(path,madx,obs,particle).c_str(), ios::in);
  while ( madxFile.is_open() ) {
    //go to end of "header" / first data line
    while (tmp != "$") {
      madxFile >> tmp;
      if (madxFile.eof()) {
  	cout << "ERROR: madximport.cpp: Wrong data format in " << trajectoryFile(path,madx,obs,particle) << endl;
  	exit(1);
      }
    }
    getline(madxFile, tmp);
    //obs1: read all lines to get #turns
    while (!madxFile.eof()) {
      madxFile >> tmp >> turn >> x >> tmp >> y >> tmp >> tmp >> tmp >> s;
      getline(madxFile, tmp);
      if (madxFile.eof()) break;
      if (obs==1) { //see comment above
  	theTurn = turn + 1;
      }
      else // all other obs: only read s from first line
	break;
    }
    madxFile.close();
    obsPos.push_back(s);
    obs++;
    madxFile.open(trajectoryFile(path,madx,obs,particle).c_str(), ios::in);
  }
  //resize & set positions s
  AccPair empty;
  n_turns = theTurn;
  n_samples = obsPos.size();
  pos.resize(turns()*samples());
  value.resize(turns()*samples());
  for (unsigned int t=1; t<=turns(); t++) {
    for (unsigned int i=0; i<samples(); i++) {
      pos[index(i,t)] = posTotal(obsPos[i], t);
      value[index(i,t)] =  empty;
    }
  }
  

  //for chosen particle: read data from all observation points
  //-----------------------------------------------------------------------------------------------------
  //obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  obs=1;
  madxFile.open(trajectoryFile(path,madx,obs,particle).c_str(), ios::in);
  while ( madxFile.is_open() ) {
    
    //go to end of "header" / first data line
    while (tmp != "$") {
      madxFile >> tmp;
      if (madxFile.eof()) {
  	cout << "ERROR: madximport.cpp: Wrong data format in " << trajectoryFile(path,madx,obs,particle) << endl;
  	exit(1);
      }
    }
    getline(madxFile, tmp);
    //read trajectory data
    while (!madxFile.eof()) {
      madxFile >> tmp >> turn >> x >> tmp >> y >> tmp >> tmp >> tmp >> s;
      getline(madxFile, tmp);
      if (madxFile.eof()) break;
      
      if (obs==1) { //see comment above
  	turn += 1;
      }
      otmp.x = x;
      otmp.z = y;

      //this->set(otmp, s, turn);
      this->modify(otmp, obs-1, turn); // by index (obs) and not by pos -> faster than set() !
      //cout << " set turn " <<turn << endl; //debug
    }

    madxFile.close();
    obs++;
    madxFile.open(trajectoryFile(path,madx,obs,particle).c_str(), ios::in);
  }
  this->hide_last_turn();

  //metadata
  info.add("Trajectory from", "MAD-X");
  info.add("Tr. Source path", path);
  stringstream stmp;
  stmp << particle;
  info.add("particle number", stmp.str());
  stmp.str(std::string());
  stmp << obs;
  info.add("number of obs. points", stmp.str());
}



//import closed orbit from elegant .clo file
template<>
void FunctionOfPos<AccPair>::elegantClosedOrbit(const char *elegantCloFile)
{
 // --------------------------------------------------------------------------------
  // position of orbit-data at END of quadrupole (according to s in elegant .clo file).
  // Corresponds to BPM behind the Quadrupole
  // --------------------------------------------------------------------------------

  string tmp;
  string s, x, y;
  double pos;
  char *p, *pp, *ppp;
  fstream elegantClo;
  AccPair otmp;

  elegantClo.open(elegantCloFile, ios::in);
  if (!elegantClo.is_open()) {
    cout << "ERROR: FunctionOfPos::elegantClosedOrbit(): Cannot open " << elegantCloFile << endl;
    exit(1);
  }

  while (!elegantClo.eof()) {
    elegantClo >> tmp;

    if (tmp == "QUAD") {
      elegantClo >> s >> x >> y;
      pos = strtod(s.c_str(), &p);
      otmp.x = strtod(x.c_str(), &pp);
      otmp.z = strtod(y.c_str(), &ppp);
      if (*p!=0 || *pp!=0 || *ppp!=0) {
	cout << "WARNING: FunctionOfPos::elegantClosedOrbit(): No correct BPM data @ s="
	     <<s<< "("<<x<<", "<<y<<"). Ignore it and continue." << endl;
      }
      else
	this->set(otmp, pos);
    }
  }
  //metadata
  info.add("Closed Orbit from", "Elegant");
  info.add("Orbit Source file", elegantCloFile);
}




//import single particle trajectory from elegant tracking "watch" files at each quadrupole
template <>
void FunctionOfPos<AccPair>::elegantTrajectory(string path, unsigned int particle)
{
  unsigned int watch=0;
  string tmp="init";
  unsigned int turn, p;
  double x, y, s;
  fstream elegantFile;
  AccPair otmp;

  elegantFile.open(trajectoryFile(path,elegant,watch,particle).c_str(), ios::in);
  while ( elegantFile.is_open() ) {
    
    // header with parameters (*** marks end of parameters, three header lines below it)
    while (tmp != "***") {
      elegantFile >> tmp;
      // read position s
      if (tmp == "position_s/m") {
	elegantFile >> s;
      }
      if (elegantFile.eof()) {
	cout << "ERROR: FunctionOfPos::elegantTrajectory(): Wrong data format in " << trajectoryFile(path,elegant,watch,particle) << endl;
	exit(1);
      }
    }
    for (int i=0; i<4; i++)   getline(elegantFile, tmp);

    //read trajectory data
    while (!elegantFile.eof()) {
      elegantFile >> x >> tmp >> y >> tmp >> tmp >> tmp >> p >> turn;
      getline(elegantFile, tmp);
      if (elegantFile.eof()) break;

      if (p != particle) {
	cout << "ERROR: FunctionOfPos::elegantTrajectory(): particleID in " << trajectoryFile(path,elegant,watch,particle) 
	     << " does not match the input particle ("<<p<<"/"<<particle<<")"<< endl;
	exit(1);
      }
      
      otmp.x = x;
      otmp.z = y;
      this->set(otmp, s, turn);
    }

    elegantFile.close();
    watch++;
    elegantFile.open(trajectoryFile(path,elegant,watch,particle).c_str(), ios::in);
  }
  this->hide_last_turn();

  //metadata
  info.add("Trajectory from", "Elegant");
  info.add("Tr. Source path", path);
  stringstream stmp;
  stmp << particle;
  info.add("particle number", stmp.str());
  stmp.str(std::string());
  stmp << watch;
  info.add("number of watch points", stmp.str());
}





//import closed orbit from ELSA BPM-measurement at time t/ms
template <>
void FunctionOfPos<AccPair>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t)
{
  int i;
  char msg[1024];
  AccPair otmp;

  this->clear(); //delete old-BPM-data (from madx or previous t)
  
  for (i=0; i<NBPMS; i++) {
    if (t > spuren.bpms[i].time.size()) {
      snprintf(msg, 1024, "ERROR: FunctionOfPos::elsaClosedOrbit: No ELSA BPM%02d data available for %d ms.\n", i+1, t);
      throw std::runtime_error(msg);
    }

    otmp.x = spuren.bpms[i].time[t].x / 1000; // unit mm -> m
    otmp.z = spuren.bpms[i].time[t].z / 1000;
    this->set(otmp, spuren.bpms[i].pos);
  }

 //metadata
 stringstream spureninfo;
 spureninfo << t << "ms in "<<spuren.spurenFolder;
 info.add("ELSA Closed Orbit from", spureninfo.str());
}




// headline entry for "value" in output file
template <>
string FunctionOfPos<AccPair>::header() const
{
  return this->value[0].header();
}
template <>
string FunctionOfPos<AccTriple>::header() const
{
  return this->value[0].header();
}



