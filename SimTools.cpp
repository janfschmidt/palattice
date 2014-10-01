#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "SimTools.hpp"
#include "types.hpp"
#include "simToolPath.hpp"
#include "config.hpp"

using namespace pal;
using namespace std;


SimToolInstance::SimToolInstance(SimTool toolIn, SimToolMode modeIn, string fileIn)
  : executed(false), trackingTurns(0), tool(toolIn), mode(modeIn)
{

  //path: path of fileIn
  //file: fileIn without path
  string::size_type f = fileIn.find_last_of("/");
  if (f != string::npos) {
    _path = fileIn.substr(0,f+1);
    file = fileIn.substr(f+1,string::npos);
  }
  else {
    _path = "./";
    file = fileIn;
  }

  if (mode==online && tool==madx)
    filebase = "madx";
  else if (mode==online && tool==elegant)
    filebase = "elegant";
  else if (mode==offline) {
    //filebase: file without extension
    string::size_type f2 = file.find_first_of(".");
    if ( f2 != string::npos)
      filebase = file.substr(0, f2);
    else
      filebase = file;
  }


  if (tool==madx)
    runFile = "libpal.madx";
  else if (tool==elegant)
    runFile = "libpal.ele";
}




string SimToolInstance::trajectory(unsigned int obs, unsigned int particle) const
{
  int n = 1024;
  char tmp[n];
  string out;
  if (tool == madx)
    snprintf(tmp, n, "%s%s.obs%04d.p%04d", path().c_str(), filebase.c_str(), obs, particle);
  else if (tool== elegant)
    snprintf(tmp, n, "%s%s.w%04d.p%d", path().c_str(), filebase.c_str(), obs, particle);
  out = tmp;

  return out;
}



// replace a command in a madx/elegant file via sed.
// "[variable] = x[delim]" ---> "[variable] = [value][delim]"
void SimToolInstance::replaceInFile(string variable, string value, string delim, string file)
{
  stringstream cmd;
  cmd << "cd "<< path() << "; ";
  cmd << "sed -i 's!"<<variable<<" = .*"<<delim<<"!"<<variable<<" = "<<value<<delim<<"!' "<<file;
  //cout << cmd.str() << endl;
  system(cmd.str().c_str());
}


// run madx/elegant (online mode only)
// single particle tracking is only done if trackingTurns!=0
//  (elegant does this automatically, for madx it is handled below)
void SimToolInstance::run()
{
  if (mode==offline)
    throw libpalError("You cannot run madx/elegant from a SimToolInstance in offline mode.");

  if (executed) return; // only run madx/elegant once

  stringstream cmd, runcmd, tmp;

 // copy madx/elegant file (if not existing)
  cmd << "cp -n "<< pal::simToolPath() << "/" << runFile << " " << path();
  system(cmd.str().c_str());

  // set lattice filename in runFile:
  tmp.str(std::string());
  tmp << "\""<<file<<"\"";
  if (tool== madx)
    replaceInFile("call, file", tmp.str(), ";", runFile);
  else if (tool== elegant)
    replaceInFile("lattice", tmp.str(), ",", runFile);

  // set tracking turns in runFile:
  if (trackingTurns != 0) {
    tmp.str(std::string());
    tmp << trackingTurns;
    if (tool== madx)
      replaceInFile("turns", tmp.str(), ",", runFile);
    else if (tool== elegant)
      replaceInFile("n_passes", tmp.str(), ",", runFile);
  }
  
  //run madx/elegant:
  runcmd << "cd "<< path() << "; ";
  if (tool== madx) {
    //avoid fatal error "cannot open input file: madx.observe"
    cmd.str(std::string());
    cmd << "cd "<<path()<<"; echo \"\" > madx.observe";
    system(cmd.str().c_str());
    // ---
    runcmd << MADXCOMMAND << " < ";
    cout << "Run MadX 1...";
  }
  else {
    runcmd << ELEGANTCOMMAND << " ";
    cout << "Run Elegant...";
  }
  runcmd << runFile << " >> " << log();
  cout << " (log: " << log() << ")" << endl;
  system(runcmd.str().c_str());


  //MadX: additional commands to set tracking observation points at each BPM (element name including BPM)
  cmd.str(std::string());
  if (tool== madx && trackingTurns!=0) {
    //write BPMs as observation points to madx.observe
    cmd << "cd "<< path() << "; "
	<< "grep BPM " << lattice() << " | awk '{gsub(\"\\\"\",\"\",$2); print \"ptc_observe, place=\"$2\";\"}' > " 
	<< "madx.observe";
    system(cmd.str().c_str());
    
    // 2. madx run -> using madx.observe
    cout << "Run MadX 2... (log: " << log() << ")" << endl;
    system(runcmd.str().c_str());
  }

  executed = true;
}




// read specified columns from a madx/elegant table format output file
// executes madx/elegant if mode=online and not yet executed
// if maxRows is given (!=0) reading is stopped after [maxRows] rows.
SimToolTable SimToolInstance::readTable(string filename, vector<string> columnKeys, unsigned int maxRows)
{
  SimToolTable table;

  // run?
  if (!executed && mode==online)
    this->run();

  fstream tabFile;
  tabFile.open(filename.c_str(), ios::in);
  if (!tabFile.is_open()) {
    throw libpalFileError(filename);
  }


  // read column names (=> column position)
  string tmp;
  map<unsigned int,string> columnPos;
  unsigned int col = 0;
  tabFile >> tmp;
  //look for headline:
  while (!tabFile.eof()) {
    // begin of headline:
    if ((tool==madx && tmp=="*") || (tool==elegant && tmp=="***")) {
      if (tool==elegant)
	getline(tabFile, tmp); //line break ("***" in line above header)
      //read headline:
      getline(tabFile, tmp);
      stringstream line(tmp);
      //check single columns of headline
      while (!line.fail()) {
	line >> tmp;
	//check against all requested column keys
	for (unsigned int k=0; k<columnKeys.size(); k++) {
	  if (tmp == columnKeys[k]) {
	    columnPos[col] = tmp;
	    break;
	  }
	}//check against column keys
	col++;
	if (columnPos.size() == columnKeys.size()) //break if all keys found
	  break;
      }//check single columns of headline
      break;
    }//begin of headline
    tabFile >> tmp;
  }//look for headline

  //error checks
  if (tabFile.eof()) {
    stringstream msg;
    msg << "ERROR: SimToolInstance::readTable(): " << filename << " is not a valid madx/elegant output file. No column headline found.";
    throw libpalError(msg.str());
  }
  if (columnPos.size() != columnKeys.size()) {
    stringstream msg;
    msg << "ERROR: SimToolInstance::readTable(): " <<columnPos.size()<< " of " <<columnKeys.size()<< " requested columns found in " << filename;
    if (columnPos.size()>0) {
      msg <<endl<< "found: ";
      for (map<unsigned int,string>::const_iterator it=columnPos.begin(); it!=columnPos.end(); ++it)
	msg <<"\""<< it->second <<"\"(column "<< it->first <<") ";
    }
    throw libpalError(msg.str());
  }

  //skip additional headline(s)
  if (tool== madx) {
    while (tmp.substr(0,2) != "$ ")
    getline(tabFile, tmp);
  }
  else if (tool== elegant) {
    while (tmp.substr(0,5) != "-----")
      getline(tabFile, tmp);
  }
  if (tabFile.eof()) {
    stringstream msg;
    msg << "ERROR: SimToolInstance::readTable(): " << filename << " has no valid madx/elegant column headline.";
    throw libpalError(msg.str());
  }


  //read data:

  bool stop=false;
  if (maxRows!=0) stop=true;
  unsigned int nLines=0;

  //iterate lines
  while (!tabFile.eof()) {
    getline(tabFile, tmp);
    stringstream line(tmp);
    unsigned int col = 0;
    //iterate requested columns
    for (map<unsigned int,string>::const_iterator it=columnPos.begin(); it!=columnPos.end(); ++it) {
      //go to next requested column
      for (unsigned int i=col; i<=it->first; i++,col++) {
	line >> tmp;
      }
      if (tabFile.eof()) break;
      table.push_back(it->second, tmp);
    }//iterate requested columns
    nLines++;
    if (stop && nLines == maxRows)
      break;
  }//iterate lines
  
  return table;
}



//specialization for string parameter -> accept space in parameter
template<>
string SimToolInstance::readParameter(string file, string label)
{
  fstream f;
  string tmp;
  string val;

  // run?
  if (!executed && mode==online)
    this->run();

  f.open(file.c_str(), ios::in);
  if (!f.is_open())
    throw libpalFileError(file);

  while(!f.eof()) {
    f >> tmp;
    if (tmp == label) {
      if (tool==madx) f >> tmp; //skip type/length info column
      getline(f, val);
      //remove leading space in val
      string::size_type newStart = val.find_first_not_of(" ");
      if (newStart != string::npos)
	val = val.substr(newStart);
	return val;
    }
    else if ((tool==madx && tmp=="*") || (tool==elegant && tmp=="***"))
      break;
  }
  stringstream msg;
  msg << "ERROR: pal::SimToolInstance::readParameter(): No parameter label "
      << label << "in " << file;
  throw libpalError(msg.str());
}



// readParameter() implementations for some parameters (labels):
double SimToolInstance::readCircumference()
{
  string label;
  if (tool==madx)
    label = "LENGTH";
  else if (tool==elegant)
    label = "circumference";
  double c = this->readParameter<double>(this->lattice(), label);
  return c;
}

AccPair SimToolInstance::readTune()
{
  AccPair q;
  string xLabel, zLabel;
  if (tool==madx) {
    xLabel = "Q1";
    zLabel = "Q2";
  }
  else if (tool==elegant) {
    xLabel = "tune:Qx";
    zLabel = "tune:Qz";
  }
  q.x = readParameter<double>(lattice(), xLabel);
  q.z = readParameter<double>(lattice(), zLabel);
  return q;
}
