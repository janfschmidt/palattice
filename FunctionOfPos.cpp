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




// valColumn usually has 1 entry, 2 for AccPair(x,z), 3 for AccTriple(x,z,s)
template <>
void FunctionOfPos<AccPair>::readSimToolColumn(SimToolInstance s, string file, string posColumn, vector<string> valColumn)
{
  if (valColumn.size() != 2) {
    stringstream msg;
    msg << "FunctionOfPos::readSimToolColumn(): valColumn should have 2(not "
	<<valColumn.size()<<") entries for 2D data type AccPair";
    throw libpalError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);
  columns.push_back(valColumn[1]);

  SimToolTable tab = s.readTable(file, columns);
  AccPair pair;

  for (unsigned int i=0; i<tab.rows(); i++) {
    pair.x = tab.getd(valColumn[0],i);
    pair.z = tab.getd(valColumn[1],i);
    this->set(pair, tab.getd(posColumn,i));
  }
}
template <>
void FunctionOfPos<AccTriple>::readSimToolColumn(SimToolInstance s, string file, string posColumn, vector<string> valColumn)
{
  if (valColumn.size() != 3) {
    stringstream msg;
    msg << "FunctionOfPos::readSimToolColumn(): valColumn should have 2(not "
	<<valColumn.size()<<") entries for 3D data type AccTriple";
    throw libpalError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);
  columns.push_back(valColumn[1]);
  columns.push_back(valColumn[2]);

  SimToolTable tab = s.readTable(file, columns);
  AccTriple triple;

  for (unsigned int i=0; i<tab.rows(); i++) {
    triple.x = tab.getd(valColumn[0],i);
    triple.z = tab.getd(valColumn[1],i);
    triple.s = tab.getd(valColumn[2],i);
    this->set(triple, tab.getd(posColumn,i));
  }
}





// ---------------- Orbit import ---------------------
//import closed orbit from madx twiss file or elegant .clo file
// --------------------------------------------------------------------------------
// position of orbit-data at END of quadrupole (according to s in file).
// Corresponds to BPM behind the Quadrupole
// --------------------------------------------------------------------------------
template <>
void FunctionOfPos<AccPair>::simToolClosedOrbit(SimToolInstance s)
{
  string orbitFile=s.orbit();

  //SimTool columns
  vector<string> columns;
  if (s.tool == pal::madx) {
    columns.push_back("S");
    columns.push_back("KEYWORD");
    columns.push_back("X");
    columns.push_back("Y");
  }
  else if (s.tool == pal::elegant) {
    columns.push_back("s");
    columns.push_back("ElementType");
    columns.push_back("x");
    columns.push_back("y");
  }

  //read columns from file (execute madx if mode=online)
  SimToolTable oTable;
  oTable = s.readTable(orbitFile, columns);

  //orbit at quadrupoles
  AccPair otmp;
  for (unsigned int i=0; i<oTable.rows(); i++) {
    if (s.tool==pal::madx && oTable.gets("KEYWORD",i) == "\"QUADRUPOLE\"") {
      otmp.x = oTable.getd("X",i);
      otmp.z = oTable.getd("Y",i);
      this->set(otmp, oTable.getd("S",i));
    }
    else if (s.tool==pal::elegant && oTable.gets("ElementType",i) == "QUAD") {
      otmp.x = oTable.getd("x",i);
      otmp.z = oTable.getd("y",i);
      this->set(otmp, oTable.getd("s",i));
    }
  }

  //metadata
  info.add("Closed Orbit from", s.tool_string());
  info.add("Orbit Source file", orbitFile);
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



