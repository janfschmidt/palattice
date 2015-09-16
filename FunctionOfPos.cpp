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
 * !!! first "turn"   = 1
 * !!! first "sample" = 0
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
  return "Please implement this AccAxis in axis_string() in FunctionOfPos.cpp.";
}


// =========== template specialization ============

//double
template <>
vector<double> FunctionOfPos<double>::getVector(double stepwidth,AccAxis axis) const
{
  // axis not needed, if only 1D values exist.
  vector<double> out;
  
  //data values, no interpolation
  if (stepwidth == 0.) {
    for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
      out.push_back( it->second );
  }
  //interpolation: equidistant data values
  else {
    for (double pos=0; pos<turns()*circ; pos+=stepwidth)
      out.push_back( this->interp(pos) );
  }
   return out;
}

//int
template <>
vector<double> FunctionOfPos<int>::getVector(double stepwidth,AccAxis axis) const
{ 
  // axis not needed, if only 1D values exist.
  vector<double> out;
  
  //data values, no interpolation
  if (stepwidth == 0.) {
    for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
      out.push_back( double(it->second) );
  }
  //interpolation: equidistant data values
  else {
    for (double pos=0; pos<turns()*circ; pos+=stepwidth)
      out.push_back( double(this->interp(pos)) );
  }
   return out;
 
}

//AccPair
template <>
vector<double> FunctionOfPos<AccPair>::getVector(double stepwidth,AccAxis axis) const
{
  vector<double> out;

  switch(axis) {
  case s:
    throw invalid_argument("FunctionOfPos<AccPair>::getVector(): s coordinate is not defined for AccPair. Use AccTriple instead.");
    break;
  case x:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.x );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circ; pos+=stepwidth)
	out.push_back( this->interp(pos).x );
    }
    break;
  case z:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.z );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circ; pos+=stepwidth)
	out.push_back( this->interp(pos).z );
    }
    break;
  }

  return out;
}

//AccTriple
template <>
vector<double> FunctionOfPos<AccTriple>::getVector(double stepwidth,AccAxis axis) const
{
  vector<double> out;

  switch(axis) {
  case s:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.s );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circ; pos+=stepwidth)
	out.push_back( this->interp(pos).s );
    }
    break;
  case x:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.x );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circ; pos+=stepwidth)
	out.push_back( this->interp(pos).x );
    }
    break;
  case z:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.z );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circ; pos+=stepwidth)
	out.push_back( this->interp(pos).z );
    }
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
    throw palatticeError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);
  columns.push_back(valColumn[1]);

  SimToolTable tab = s.readTable(file, columns);
  AccPair pair;

  for (unsigned int i=0; i<tab.rows(); i++) {
    double pos = tab.get<double>(i,posColumn);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    pair.x = tab.getd(i,valColumn[0]);
    pair.z = tab.getd(i,valColumn[1]);
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
    throw palatticeError(msg.str());
  }

  vector<string> columns;
  columns.push_back(posColumn);
  columns.push_back(valColumn[0]);
  columns.push_back(valColumn[1]);
  columns.push_back(valColumn[2]);

  SimToolTable tab = s.readTable(file, columns);
  AccTriple triple;

  for (unsigned int i=0; i<tab.rows(); i++) {
    double pos = tab.get<double>(i,posColumn);
    // values at pos=circ are ignored to avoid #turns problem
    // see simToolTrajectory() for another solution
    if (fabs(pos-circ) <= 0.0001) continue;

    triple.x = tab.getd(i,valColumn[0]);
    triple.z = tab.getd(i,valColumn[1]);
    triple.s = tab.getd(i,valColumn[2]);
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
  this->clear(); //delete old data

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
    if (s.tool==pal::madx && oTable.gets(i,"KEYWORD") == "\"QUADRUPOLE\"") {
      otmp.x = oTable.getd(i,"X");
      otmp.z = oTable.getd(i,"Y");
      this->set(otmp, oTable.getd(i,"S"));
    }
    else if (s.tool==pal::elegant && oTable.gets(i,"ElementType") == "QUAD") {
      otmp.x = oTable.getd(i,"x");
      otmp.z = oTable.getd(i,"y");
      this->set(otmp, oTable.getd(i,"s"));
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
  cout  << "* "<<size()<<" BPMs read"<<endl
	<<"  from "<<orbitFile << endl;
}










//import closed orbit from ELSA BPM-measurement at time t/ms
template <>
void FunctionOfPos<AccPair>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t)
{
  int i;
  char msg[1024];
  AccPair otmp;

  this->clear(); //delete old-BPM-data
  
  for (i=0; i<NBPMS; i++) {
    if (t > spuren.bpms[i].time.size()) {
      snprintf(msg, 1024, "ERROR: FunctionOfPos::elsaClosedOrbit: No ELSA BPM%02d data available for %d ms.\n", i+1, t);
      throw palatticeError(msg);
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




template<>
void FunctionOfPos<AccPair>::simToolTrajectory(SimToolInstance &s, unsigned int particle)
{
  cout << "Initializing trajectory... " << endl;
  
  if (s.tool==pal::madx)
    readSimToolParticleColumn(s,particle,"X","Y");
  else if (s.tool==pal::elegant)
    readSimToolParticleColumn(s,particle,"x","y");

  cout << "* Trajectory of particle "<<particle<<" read at "<<samplesInTurn(1)
       <<" observation points for "<<turns()<<" turns"<<endl;
}

