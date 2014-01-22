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
#include "functionofpos.hpp"



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
}




//import single particle trajectory from madx tracking "obs" files at each quadrupole
template <>
void FunctionOfPos<AccPair>::madxTrajectory(const FILENAMES files, unsigned int particle)
{
 unsigned int obs=1;
  string tmp="init";
  unsigned int turn; // madx column variables
  double x, y, s;
  fstream madx;
  AccPair otmp;

  //for chosen particle read data from all observation points
  //-----------------------------------------------------------------------------------------------------
  //obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  madx.open(files.tracking(obs,particle).c_str(), ios::in);
  while ( madx.is_open() ) {
    
    //go to end of "header" / first data line
    while (tmp != "$") {
      madx >> tmp;
      if (madx.eof()) {
	cout << "ERROR: madximport.cpp: Wrong data format in " << files.tracking(obs,particle) << endl;
	exit(1);
      }
    }
    getline(madx, tmp);
    //read trajectory data
    while (!madx.eof()) {
      madx >> tmp >> turn >> x >> tmp >> y >> tmp >> tmp >> tmp >> s;
      getline(madx, tmp);
      if (madx.eof()) break;
      
      if (obs==1) { //see comment above
	turn += 1;
      }
      otmp.x = x;
      otmp.z = y;
      this->set(otmp, s, turn);
    }

    madx.close();
    obs++;
    madx.open(files.tracking(obs,particle).c_str(), ios::in);
  }
  this->hide_last_turn();

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
}



//import closed orbit from ELSA BPM-measurement at time t/ms
template <>
void FunctionOfPos<AccPair>::elsaClosedOrbit(BPM *ELSAbpms, unsigned int t)
{
  int i;
  char msg[1024];
  AccPair otmp;

  this->clear(); //delete old-BPM-data (from madx or previous t)
  
  for (i=0; i<NBPMS; i++) {
    if (t > ELSAbpms[i].time.size()) {
      snprintf(msg, 1024, "ERROR: ELSAimport.cpp: No ELSA BPM%02d data available for %d ms.\n", i+1, t);
      throw std::runtime_error(msg);
    }

    otmp.x = ELSAbpms[i].time[t].x / 1000; // unit mm -> m
    otmp.z = ELSAbpms[i].time[t].z / 1000;
    this->set(otmp, ELSAbpms[i].pos);
  }
}
