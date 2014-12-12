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
string pal::axis_string(AccAxis a)
{
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
void FunctionOfPos<AccPair>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn)
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
    double pos = tab.get<double>(posColumn,i);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    pair.x = tab.getd(valColumn[0],i);
    pair.z = tab.getd(valColumn[1],i);
    this->set(pair, pos);
  }
  //if (s.tool==elegant) this->pop_back_turn(); //elegant: always entry with s=circumference due to drifts. Avoid additional turn.

 //metadata
  info.simToolImport(s);
  info.add("Data Source file", file);
  info.add("read Parameters", valColumn[0]+", "+valColumn[1]);
}
template <>
void FunctionOfPos<AccTriple>::readSimToolColumn(SimToolInstance &s, string file, string posColumn, vector<string> valColumn)
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
    double pos = tab.get<double>(posColumn,i);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    triple.x = tab.getd(valColumn[0],i);
    triple.z = tab.getd(valColumn[1],i);
    triple.s = tab.getd(valColumn[2],i);
    this->set(triple, pos);
  }
  //if (s.tool==elegant) this->pop_back_turn(); //elegant: always entry with s=circumference due to drifts. Avoid additional turn.

 //metadata
  info.simToolImport(s);
  info.add("Data Source file", file);
  info.add("read Parameters", valColumn[0]+", "+valColumn[1]+", "+valColumn[2]);
}





// ---------------- Orbit import ---------------------
//import closed orbit from madx twiss file or elegant .clo file
// --------------------------------------------------------------------------------
// position of orbit-data at END of quadrupole (according to s in file).
// Corresponds to BPM behind the Quadrupole
// --------------------------------------------------------------------------------
template <>
void FunctionOfPos<AccPair>::simToolClosedOrbit(SimToolInstance &s)
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

  //read columns from file (execute madx/elegant if mode=online)
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

  //init interpolation
    if (!this->ready) {
    this->init();
  }

  //stdout info
  cout  << "* "<<samples()<<" BPMs read"<<endl
	<<"  from "<<orbitFile << endl;
}




//import single particle trajectory from madx/elegant tracking "obs"/"watch" files
template <>
void FunctionOfPos<AccPair>::simToolTrajectory(SimToolInstance &s, unsigned int particle)
{
  //SimTool columns
  vector<string> columns;
  if (s.tool == pal::madx) {
    columns.push_back("S");
    columns.push_back("TURN");
    columns.push_back("X");
    columns.push_back("Y");
  }
  else if (s.tool == pal::elegant) {
    // s: parameter in file header
    columns.push_back("Pass");
    columns.push_back("x");
    columns.push_back("y");
  }


  //for chosen particle: read turns, samples and positions
  //then modify() can be used below instead of set() ->  much faster for large datasets


  cout << "* Initializing trajectory... " << endl;  
  unsigned int obs;
  unsigned int turnsRead;
  SimToolTable tab_first;
  vector<double> obsPos;
  if (s.tool==pal::madx) obs=1;
  else if (s.tool==pal::elegant) obs=0;
  string trajFile=s.trajectory(obs,particle);

  tab_first = s.readTable(trajFile, columns);

  //read turns from first obs file (and position)
  if (s.tool==pal::madx) {
    turnsRead = tab_first.get<unsigned int>("TURN",tab_first.rows()-1);
    obsPos.push_back(tab_first.getd("S",0)); //read position s from first line
  }
  else if (s.tool==pal::elegant) {
    turnsRead = tab_first.get<unsigned int>("Pass",tab_first.rows()-1);
    obsPos.push_back(s.readParameter<double>(trajFile,"position_s/m")); //read position s from parameter in file header
  }

  if (s.mode==pal::online && turnsRead != s.turns())
    cout << "LIBPAL WARNING: FunctionOfPos<AccPair>::simToolTrajectory(): "
	 <<turnsRead<< " turns read from "<<s.tool_string()<<" files, but "
	 <<s.turns()<<" turns set in SimToolInstance" << endl;

  //read positions from all other obs files
  obs++;
  SimToolTable tab;
  //iterate all existing obs files:
  while (true) {
    trajFile=s.trajectory(obs,particle);
    try {
      if (s.tool==pal::madx) {
	tab = s.readTable(trajFile, columns, 1); //read first line
	obsPos.push_back(tab.getd("S",0));
      }
      else if (s.tool==pal::elegant) {
	obsPos.push_back(s.readParameter<double>(trajFile,"position_s/m")); //read parameter in file header
      }
    }
    catch (libpalFileError) {
      if (s.tool==pal::elegant) obsPos.pop_back(); //elegant: obs-file at lattice end only needed for last turn
      break;
    }
    obs++;
  }
  
  //resize & set positions s
  AccPair empty;
  this->n_turns = turnsRead + 1;      //last turn to avoid extrapolation, hidden later
  this->n_samples = obsPos.size();
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
  //madx & obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  AccPair otmp;
  unsigned int turn;
  if (s.tool==pal::madx) obs=1;
  else if (s.tool==pal::elegant) obs=0;
  //iterate all existing obs files:
  while (true) {
    // read table from file:
    trajFile=s.trajectory(obs,particle);
    try {
      tab = s.readTable(trajFile, columns);
    }
    catch (libpalFileError) {
      obs--;
      break;
    }
    // write table rows to FunctionOfPos:
    if (s.tool==pal::elegant && obs==this->samples()) //elegant: obs-file at lattice end only needed for last turn
      break;
    for (unsigned int i=0; i<tab.rows(); i++) {
      if (s.tool==pal::madx) {
	turn = tab.get<unsigned int>("TURN",i);
	otmp.x = tab.getd("X",i);
	otmp.z = tab.getd("Y",i);
	if (obs==1) { //see comment above ("madx & obs0001")
	  turn += 1;
	}
      }
      else if (s.tool==pal::elegant) {
	turn = tab.get<unsigned int>("Pass",i);
	otmp.x = tab.getd("x",i);
	otmp.z = tab.getd("y",i);
      }
      
      // modify(): by index (obs) and not by pos -> faster than set() !
      if (s.tool==pal::madx) this->modify(otmp, obs-1, turn);
      else if (s.tool==pal::elegant) this->modify(otmp, obs, turn);
    }
    obs++;
  }
  //last turn has only one "real" entry: at the begin of the lattice to avoid extrapolation
  //it is read from:
  //- obs0001 for madx (see comment above)
  //- dedicated last obs (at lattice end) for elegant (implemented below)
  if (s.tool==pal::elegant) {
    turn = tab.get<unsigned int>("Pass",tab.rows()-1) + 1;
    otmp.x = tab.getd("x",tab.rows()-1);
    otmp.z = tab.getd("y",tab.rows()-1);
    this->modify(otmp, 0, turn);
  }
  this->hide_last_turn();

   //metadata
  info.add("Trajectory from", s.tool_string());
  info.add("Tr. Source path", s.path());
  stringstream stmp;
  stmp << particle;
  info.add("particle number", stmp.str());
  stmp.str(std::string());
  stmp << samples();
  info.add("number of obs. points", stmp.str());

  //info stdout
  cout << "* trajectory of particle "<<particle<<" read at "<<samples()
       <<" observation points for "<<turns()<<" turns"<<endl;
  this->print("traj.tmp");
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
      throw libpalError(msg);
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






