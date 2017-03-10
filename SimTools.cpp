/* SimTools Classes
 * "interface" to simulation tools (currently MadX, Elegant)
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
 * SimToolInstance is connected to a lattice file and handles
 * setup and execution of the simulation tool.
 * SimToolTable contains columns of simulation tool results
 *
 * Elegant output can be read directly from binary SDDS files
 * if libSDDS1 (SDDSToolKit-devel package) is installed.
 * See README for details.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include "SimTools.hpp"
#include "types.hpp"
#include "config.hpp"

using namespace pal;
using namespace std;


//system call wrapper throwing std::system_error if return value=!0
void pal::system_throwing(const std::string& cmd)
{
  auto ret = system(cmd.c_str());
  if (ret!=0) {
    throw std::system_error(ret, std::system_category());
  }
}





//======================================================================================
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
//======================================================================================

unsigned int SimToolTable::rows() const
{
  if(sddsMode())
    return SDDS_CountRowsOfInterest(table_sdds.get());
  else
    return table.begin()->second.size();
}

unsigned int SimToolTable::columns() const
{
  if(sddsMode())
    return SDDS_CountColumnsOfInterest(table_sdds.get());
  else
    return table.size();
}


void SimToolTable::init_sdds(const string &filename, std::vector<string> columnKeys)
{
  sddsFileIsOpen = false;
  
  // SDDS_TABLE pointer with custom deleter
  table_sdds = std::shared_ptr<SDDS_TABLE>(new SDDS_TABLE, [this](SDDS_TABLE *t){
      if (sddsFileIsOpen) {
	if (SDDS_Terminate(t) !=1 )
	  std::cerr << "Error terminating SDDS table" << std::endl;
      }
      delete t;
    });

  // check file existence (without SDDS-Errors, which are not thread safe)
  if( !pal::fileExists(filename) ) {
    throw palatticeFileError(filename);
  }
  
   if ( SDDS_InitializeInput(table_sdds.get(),const_cast<char*>(filename.c_str())) != 1 ) {
     //SDDS_PrintErrors(stdout, SDDS_VERBOSE_PrintErrors);
     throw SDDSError();
   }
   
   sddsFileIsOpen = true;
  
  auto ret = SDDS_ReadPage(table_sdds.get());
  if( ret == -1 )
    throw SDDSPageError();
  else if (ret == 0)
    throw SDDSError();

  //select columns
   if (columnKeys.size() > 0) {
    std::stringstream tmp;
    for (auto &key : columnKeys)
      tmp << key << " ";
    SDDS_SetColumnFlags(table_sdds.get(),0); // unselect all columns first
    if( SDDS_SetColumnsOfInterest(table_sdds.get(), SDDS_NAMES_STRING, tmp.str().c_str()) != 1 )
    throw SDDSError();
   }

  sdds = true;
  tabname = filename;
  table.clear(); //delete non-sdds data
}

//go to next page of SDDS file. previously applied filter is used
void SimToolTable::nextPage()
{
  if(!sddsMode()) {
    std::cout << "SimToolTable::nextPage() - useless in non-SDDS-mode" << std::endl;
    return;
  }

  auto ret = SDDS_ReadPage(table_sdds.get());
  if( ret == -1 )
    throw SDDSPageError();
  else if (ret == 0)
    throw SDDSError();
  
  if (sddsFilter.on) {
    if ( SDDS_FilterRowsOfInterest(table_sdds.get(), sddsFilter.c_column(),sddsFilter.min,sddsFilter.max,SDDS_AND) == -1)
      throw SDDSError();
  }
}

//ignore all lines from SDDS file, if value in given column is out of given range
//is immediately applied to current page
void SimToolTable::filterRows(string column, double min, double max)
{
  if(!sddsMode()) {
    std::cout << "SimToolTable::filterRows() - not implemented in non-SDDS-mode" << std::endl;
    return;
  }

  sddsFilter.set(column,min,max);
  if ( SDDS_FilterRowsOfInterest(table_sdds.get(), sddsFilter.c_column(),sddsFilter.min,sddsFilter.max,SDDS_AND) == -1)
    throw SDDSError();
}

bool SimToolInstance::sddsMode() const
{
  if(preferedFormat==sdds && tool==elegant)
    return true;
  else
    return false;
}

//specialization for string parameter
// -> allow every data type to be read as string
template<>
string SimToolTable::getParameter(const string &label)
{
  if (!sdds)
    throw palatticeError("SimToolTable::getParameter(): Can only be used in SDDS mode. Use SimToolInstamce::readParameter() instead");

  auto dataIndex = SDDS_GetParameterIndex(table_sdds.get(), const_cast<char*>(label.c_str()));
  if (dataIndex == -1) throw SDDSError();
  auto dataType = SDDS_GetParameterType(table_sdds.get(), dataIndex);
  void* mem = SDDS_GetParameterByIndex(table_sdds.get(), dataIndex, NULL);
  if (mem == NULL) throw SDDSError();

  stringstream s;

  switch (dataType) {
  case SDDS_DOUBLE:
    s << *static_cast<double *>(mem);
    break;
  case SDDS_FLOAT:
    s << *static_cast<float *>(mem);
    break;
  case SDDS_ULONG:
    s << *static_cast<uint32_t *>(mem);
    break;
  case SDDS_LONG:
    s << *static_cast<int32_t *>(mem);
    break;
  case SDDS_USHORT:
    s << *static_cast<unsigned short *>(mem);
    break;
  case SDDS_SHORT:
    s << *static_cast<short *>(mem);
    break;
  case SDDS_CHARACTER:
    s << *static_cast<char *>(mem);
    break;
  case SDDS_STRING:
    s << string( *static_cast<char **>(mem) );
    break;
  }

  if(dataType == SDDS_STRING) {
    if(*((char**)mem)) free(*((char**)mem));
  }
  if(mem) free(mem);
  return s.str();
}

//specialization for string parameter
// -> allow every data type to be read as string
template<>
string SimToolTable::get_sdds(unsigned int index, string key) const
{
  auto dataIndex = SDDS_GetColumnIndex(table_sdds.get(), const_cast<char*>(key.c_str()));
  if (dataIndex == -1) throw SDDSError();
  auto dataType = SDDS_GetColumnType(table_sdds.get(), dataIndex);
  void* mem = SDDS_GetValueByIndex(table_sdds.get(), dataIndex, index, NULL);
  if (mem == NULL) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): No column \"" <<key<< "\" in SDDS table " << this->name();
    throw palatticeError(msg.str());
  }
  
  stringstream s;

  switch (dataType) {
  case SDDS_DOUBLE:
    s << *static_cast<double *>(mem);
    break;
  case SDDS_FLOAT:
    s << *static_cast<float *>(mem);
    break;
  case SDDS_ULONG:
    s << *static_cast<uint32_t *>(mem);
    break;
  case SDDS_LONG:
    s << *static_cast<int32_t *>(mem);
    break;
  case SDDS_USHORT:
    s << *static_cast<unsigned short *>(mem);
    break;
  case SDDS_SHORT:
    s << *static_cast<short *>(mem);
    break;
  case SDDS_CHARACTER:
    s << *static_cast<char *>(mem);
    break;
  case SDDS_STRING:
    s << string( *static_cast<char **>(mem) );
    break;
  }
  
  // DO NOT FREE MANUALLY HERE - causes double free by SDDS_Terminate !?!
  // if(dataType == SDDS_STRING) {
  //   if(*((char**)mem)) free(*((char**)mem));
  // }
  //free(mem); 

  return s.str();
}

//======================================================================================
#else
//======================================================================================

unsigned int SimToolTable::rows() const {return table.begin()->second.size();}

unsigned int SimToolTable::columns() const {return table.size();}

void SimToolTable::init_sdds(const string &filename, vector<string> columnKeys) {} //dummy

void SimToolTable::nextPage() {} //dummy

void SimToolTable::filterRows(string column, double min, double max) {} //dummy

bool SimToolInstance::sddsMode() const {return false;}


//======================================================================================
#endif
//======================================================================================







template<> AccPair SimToolTable::get(unsigned int i, string keyX, string keyZ, string) const
{
  if (keyZ.empty())
    throw palatticeError("SimToolTable::get<AccPair>: No key for z column given!");
  AccPair tmp;
  tmp.x = this->getd(i,keyX);
  tmp.z = this->getd(i,keyZ);
  return tmp;
}

template<> AccTriple SimToolTable::get(unsigned int i, string keyX, string keyZ, string keyS) const
{
  if (keyZ.empty())
    throw palatticeError("SimToolTable::get<AccPair>: No key for z column given!");
  if (keyS.empty())
    throw palatticeError("SimToolTable::get<AccPair>: No key for s column given!");
  AccTriple tmp;
  tmp.x = this->getd(i,keyX);
  tmp.z = this->getd(i,keyZ);
  tmp.s = this->getd(i,keyS);
  return tmp;
}



SimToolInstance::SimToolInstance(SimTool toolIn, SimToolMode modeIn, string fileIn, string fileTag)
  : executed(false), trackingTurns(0), trackingNumParticles(1), trackingMomentum(0.), trackingMomentum_MeV(0.),
    trackingBeamline(""), trackingNumParticlesTouched(false),
    tag(fileTag), defaultRunFile(true), tool(toolIn), mode(modeIn), verbose(false)
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

  if (tool==madx)
    runFile = "libpalattice.madx";
  else if (tool==elegant)
    runFile = "libpalattice.ele";

#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
  preferedFormat = sdds;
#else
  preferedFormat = ascii;
#endif
}

string SimToolInstance::filebase() const
{
  string filebase;
  if (mode==online && tool==madx)
    filebase = "madx";
  else if (mode==online && tool==elegant && !sddsMode())
    filebase = "elegant";
  else if (mode==online && sddsMode())
    filebase = stripExtension(runFile);
  else if (mode==offline) {
    //filebase: file without extension
    filebase = stripExtension(file);
  }
  if (mode==online && tag!="")
    filebase = filebase + "_" + tag;

  return filebase;
}

std:: string SimToolInstance::stripExtension(std::string f) const
{
  std::string::size_type f2 = f.find_first_of(".");
  if ( f2 != std::string::npos)
    return f.substr(0, f2);
  else
    return f;
}


string SimToolInstance::trajectory(unsigned int obs, unsigned int particle) const
{
  int n = 1024;
  char tmp[n];
  string out;
  
  if (sddsMode())
    snprintf(tmp, n, "%s%s%03d.w", path().c_str(), filebase().c_str(), obs);
  else if (tool == madx)
    snprintf(tmp, n, "%s%s.obs%04d.p%04d", path().c_str(), filebase().c_str(), obs, particle);
  else if (tool== elegant)
    snprintf(tmp, n, "%s%s.w%04d.p%d", path().c_str(), filebase().c_str(), obs, particle);
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
  int ret = system(cmd.str().c_str());
  if (ret != 0)
    throw palatticeError("SimToolInstance::replaceInFile(): Error executing sed");
}


// replace a tag in a filename in a madx/elegant file via sed.
// "[name]_[tag].[extension]", only [tag] is changed
void SimToolInstance::replaceTagInFile(string name, string extension, string newTag, string file)
{
  stringstream cmd;
  cmd << "cd "<< path() << "; ";
  cmd << "sed -i 's!"<<name<<".*."<<extension<<"!"<<name;
  if (newTag != "") cmd <<"_"<<newTag;
  cmd<<"."<<extension<<"!' "<<file;
  //cout << cmd.str() << endl;
  int ret = system(cmd.str().c_str());
  if (ret != 0)
    throw palatticeError("SimToolInstance::replaceTagInFile(): Error executing sed");
}

void SimToolInstance::setRampInFile(bool ramp, string file)
{
  stringstream cmd;
  cmd << "cd "<< path() << "; ";
  cmd << "sed -i 's%&insert_elements.*rampp.*&end%";
  if (ramp)
    cmd << "&insert_elements name=MY_START,element_def=\"ramp: rampp,waveform=\\\"ramp.sdds=t+pcentralFactor\\\"\" &end%'";
  else
    cmd << "! &insert_elements rampp_PLACEHOLDER_FOR_PALATTICE &end%'";
  cmd << " " << file;
  cout << cmd.str() << endl;
  int ret = system(cmd.str().c_str());
  if (ret != 0)
    throw palatticeError("SimToolInstance::setRampInFile(): Error executing sed");
}



// run madx/elegant (online mode only)
// single particle tracking is only done if trackingTurns!=0
//  (elegant does this automatically, for madx it is handled below)
void SimToolInstance::run()
{
  if (mode==offline)
    throw palatticeError("You cannot run "+tool_string()+" from a SimToolInstance in offline mode.");

  if (executed) return; // only run madx/elegant once

  stringstream cmd, runcmd, tmp;

 // copy madx/elegant file (if not existing)
  if (defaultRunFile) {
    cmd << "cp -n "<< pal::simToolPath() << "/" << runFile << " " << path();
    system_throwing(cmd.str());
  }

  // set lattice filename in runFile:
  tmp.str(std::string());
  tmp << "\""<<file<<"\"";
  if (tool== madx)
    replaceInFile("call, file", tmp.str(), ";", runFile);
  else if (tool== elegant)
    replaceInFile("lattice", tmp.str(), ",", runFile);

  // set output file tag in madx runFile:
  if (tool== madx) {
    replaceTagInFile("madx", "twiss", tag, runFile);      // twiss file
    replaceTagInFile("madx", "dipealign", tag, runFile);  // dipealign file
    replaceTagInFile("madx", "quadealign", tag, runFile); // quadealign file
    replaceInFile("ptc_track, file", filebase(), ",", runFile);        // "obs" files
  }

  // set tracking turns in runFile:
    tmp.str(std::string());
    tmp << trackingTurns;
    if (tool== madx)
      replaceInFile("turns", tmp.str(), ",", runFile);
    else if (tool== elegant)
      replaceInFile("n_passes", tmp.str(), ",", runFile);

  // set tracking number of particles in runFile:
  if (trackingNumParticlesTouched) {
    tmp.str(std::string());
    tmp << trackingNumParticles;
    replaceInFile("n_particles_per_bunch", tmp.str(), ",", runFile);
  }

  // set tracking momentum in runFile:
  if (trackingMomentum!=0. || trackingMomentum_MeV!=0.) {
    replaceInFile("p_central", std::to_string(trackingMomentum), ",", runFile);
    if (trackingMomentum == 0.)
      replaceInFile("p_central_mev", std::to_string(trackingMomentum_MeV), ",", runFile);
    else
      replaceInFile("p_central_mev", "0", ",", runFile);
  }

  // set tracking elegant beamline:
  if (!trackingBeamline.empty()) {
    replaceInFile("use_beamline", trackingBeamline, ",", runFile);
  }

  
  //run madx/elegant:
  runcmd << "cd "<< path() << "; ";
  if (tool== madx) {
    //avoid fatal error "cannot open input file: madx.observe"
    cmd.str(std::string());
    cmd << "cd "<<path()<<"; echo \"\" > madx.observe";
    system_throwing(cmd.str());
    // ---
    runcmd << MADXCOMMAND << " < ";
    cout << "Run MadX 1...";
  }
  else {
    runcmd << ELEGANTCOMMAND << " ";
    cout << "Run Elegant...";
  }
  runcmd << runFile;
  if (this->verbose)
    runcmd << " | tee " <<tool_string()<< ".log";
  else
    runcmd << " > " <<tool_string()<< ".log";
  cout << " (log: " << log() << ")" << endl;
  try {
    system_throwing(runcmd.str());
  } catch (std::system_error& e) {
    stringstream msg;
    msg << tool_string() << " Error! (return code "<<e.code()<<", see " << log() <<")";
    throw palatticeError(msg.str());
  }

  //elegant without sdds: run shell script for sdds to ascii conversion
  if ( !sddsMode() && tool==elegant ) {
    tmp.str(std::string());
    tmp << "cd "<<path()<<"; elegant2libpalattice ";
    if (tag=="") tmp << "none";
    else tmp << tag;
    tmp << " libpalattice ";
    if (trackingTurns !=0)
      tmp << "watchfiles";
    else
      tmp << "NO";
    system_throwing(tmp.str());
  }


  //MadX: additional commands to set tracking observation points at each BPM (element name including BPM)
  cmd.str(std::string());
  if (tool== madx && trackingTurns!=0) {
    //write BPMs as observation points to madx.observe
    cmd << "cd "<< path() << "; "
	<< "grep BPM " << lattice() << " | awk '{gsub(\"\\\"\",\"\",$2); print \"ptc_observe, place=\"$2\";\"}' > " 
	<< "madx.observe";
    system_throwing(cmd.str());
    
    // 2. madx run -> using madx.observe
    cout << "Run MadX 2... (log: " << log() << ")" << endl;
    int ret = system(runcmd.str().c_str());
    if (ret != 0) {
      stringstream msg;
      msg << tool_string() << " Error! (see " << log() <<")";
      throw palatticeError(msg.str());
    }
  }

  executed = true;
}

void SimToolInstance::forceRun()
{
  executed = false;
  run();
}



// read specified columns from a madx/elegant table format output file
// executes madx/elegant if mode=online and not yet executed
// if maxRows is given (!=0) reading is stopped after [maxRows] rows.
SimToolTable SimToolInstance::readTable(string filename, vector<string> columnKeys, unsigned int maxRows)
{
  SimToolTable table(filename);

  // run?
  if (!executed && mode==online)
    this->run();

  if(sddsMode()) {
    if (maxRows!=0) std::cout << "WARNING: SimToolInstance::readTable() - maxRows not implemented for SDDS mode." << std::endl;
    table.init_sdds(filename, columnKeys);
    return table;
  }
  

  fstream tabFile;
  if( pal::fileExists(filename) )
    tabFile.open(filename.c_str(), ios::in);
  else
    throw palatticeFileError(filename);

  // read column names (=> column position)
  string tmp;
  map<unsigned int,string> columnPos;
  unsigned int col = 0;
  bool readAllColumns = false;
  if (columnKeys.size() == 0)
    readAllColumns = true;
  vector<string> remainingColumnKeys(columnKeys);
    
  
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
      //check each column of headline
      while (!line.fail()) {
	line >> tmp;
	//check against all requested column keys
	if (readAllColumns) {
	  columnPos[col] = tmp;
	}
	else {
	  for (auto it=remainingColumnKeys.begin(); it!=remainingColumnKeys.end(); ++it) {
	    if (tmp == *it) {
	      columnPos[col] = tmp;
	      remainingColumnKeys.erase(it);
	      break;
	    }
	  }//check against column keys
	}
	col++;
      }//check each column of headline
      break;
    }//begin of headline
    tabFile >> tmp;
  }//look for headline

  //error checks
  if (tabFile.eof()) {
    stringstream msg;
    msg << "ERROR: SimToolInstance::readTable(): " << filename << " is not a valid "<<tool_string()<<" output file. No column headline found.";
    throw palatticeError(msg.str());
  }
  if (!readAllColumns && columnPos.size()!=columnKeys.size()) {
    stringstream msg;
    msg << "ERROR: SimToolInstance::readTable(): " <<columnPos.size()<< " of " <<columnKeys.size()<< " requested columns found in " << filename;
    if (columnPos.size()>0) {
      msg <<endl<< "  -> requested: ";
	for (unsigned int k=0; k<columnKeys.size(); k++)
	  msg <<"\""<< columnKeys[k] <<"\" ";
      msg <<endl<< "  -> found: ";
      for (map<unsigned int,string>::const_iterator it=columnPos.begin(); it!=columnPos.end(); ++it)
	msg <<"\""<< it->second <<"\"(column "<< it->first <<") ";
    }
    throw palatticeError(msg.str());
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
    msg << "ERROR: SimToolInstance::readTable(): " << filename << " has no valid "<<tool_string()<<" column headline.";
    throw palatticeError(msg.str());
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
string SimToolInstance::readParameter(const string &file, const string &label)
{
   if(sddsMode()) {
    return readParameter_sdds<string>(file, label);
  }
   
  fstream f;
  string tmp;
  string val;

  // run?
  if (!executed && mode==online)
    this->run();

  f.open(file.c_str(), ios::in);
  if (!f.is_open())
    throw palatticeFileError(file);

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
  msg << "pal::SimToolInstance::readParameter(): No parameter label "
      << label << " in " << file;
  throw palatticeError(msg.str());
}




// readParameter() implementations for some parameters (labels):
double SimToolInstance::readCircumference()
{
  if (sddsMode()) {
    SimToolTable tmp = readTable(orbit(), {"s"});
    return tmp.getd(tmp.rows()-1, "s");
  }
 
  string label;
  if (tool==madx)
    label = "LENGTH";
  else if (tool==elegant && !sddsMode())
    label = "circumference";
  double c = this->readParameter<double>(this->twiss(), label);
  return c;
}

double SimToolInstance::readGammaCentral()
{
  string label;
  if (tool==madx)
    label = "GAMMA";
  else if (tool==elegant)
    label = "pCentral";
  double c = this->readParameter<double>(this->twiss(), label);
  return c;
}

double SimToolInstance::readAlphaC()
{
  string label;
    if (tool==madx)
      label = "ALFA";
    else if (tool==elegant)
      label = "alphac";
  double c = this->readParameter<double>(this->twiss(), label);
  return c;
}
double SimToolInstance::readAlphaC2()
{
  string label;
  if (tool==madx) {
    std::cout << "WARNING: reading 2. order momentum compaction factor from Mad-X is not implemented" << std::endl;
    return 0.;
  }
  else if (tool==elegant)
    label = "alphac2";
  double c = this->readParameter<double>(this->twiss(), label);
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
    xLabel = "nux";
    zLabel = "nuy";
  }
    
  q.x = readParameter<double>(twiss(), xLabel);
  q.z = readParameter<double>(twiss(), zLabel);
  return q;
}

AccPair SimToolInstance::readChromaticity()
{
  AccPair c;
  string xLabel, zLabel;
  if (tool==madx) {
    xLabel = "DQ1";
    zLabel = "DQ2";
  }
  else if (tool==elegant) {
    xLabel = "dnux/dp";
    zLabel = "dnuy/dp";
  }
    
  c.x = readParameter<double>(twiss(), xLabel);
  c.z = readParameter<double>(twiss(), zLabel);
  return c;
}

AccTriple SimToolInstance::readDampingPartitionNumber_syli()
{
  AccTriple J;
  if (tool==madx) {
    double I2 = readParameter<double>(twiss(), "SYNCH_2");
    double I4 = readParameter<double>(twiss(), "SYNCH_4");
    J.x = 1. - I4/I2;
    J.z = 1.;
    J.s = 2 + I4/I2;
  }
  else if (tool==elegant) {
    J.x = readParameter<double>(twiss(), "Jx");
    J.z = readParameter<double>(twiss(), "Jy");
    J.s = readParameter<double>(twiss(), "Jdelta");
  }
  return J;
}



// if turns!=0 (default) single particle tracking is performed while madx/elegant run
void SimToolInstance::setTurns(unsigned int t)
{
  if (!executed || t!=trackingTurns) {
    trackingTurns=t;
    executed=false;
    }
}


void SimToolInstance::setNumParticles(unsigned int n)
{
  if (n!=trackingNumParticles) {
    
    //currently only implemented for elegant
    if (tool==madx)
      std::cout << "WARNING: set number of particles not implemented for madx. Please set manually in " << runFile << std::endl;
    
    trackingNumParticles=n;
    executed=false;
  }
  trackingNumParticlesTouched = true;
}


void SimToolInstance::setMomentum_MeV(double p_MeV)
{
  if (p_MeV!=trackingMomentum_MeV) {
    
    //currently only implemented for elegant
    if (tool==madx)
      std::cout << "WARNING: set momentum not implemented for madx. Please set manually in " << runFile << std::endl;
    
    trackingMomentum_MeV=p_MeV;
    executed=false;
  }
}

void SimToolInstance::setMomentum_betagamma(double p)
{
  if (p!=trackingMomentum) {
    
    //currently only implemented for elegant
    if (tool==madx)
      std::cout << "WARNING: set momentum not implemented for madx. Please set manually in " << runFile << std::endl;
    
    trackingMomentum=p;
    executed=false;
  }
}

void SimToolInstance::setElegantBeamline(const string& bl)
{
  if (!bl.empty()) {
    
    //currently only implemented for elegant
    if (tool==madx)
      std::cout << "WARNING: set beamline not implemented for madx." << std::endl;
    
    trackingBeamline=bl;
    executed=false;
  }
}




// EnergyRamp

void EnergyRamp::toFile(const std::string& filename) const
{
  auto dt = tStop / nSteps;
  double t = 0.;

  //======================================================================================
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
  //======================================================================================
  std::unique_ptr<SDDS_DATASET> tab(new SDDS_DATASET);
  if( SDDS_InitializeOutput(tab.get(),SDDS_BINARY,1,"palattice EnergyRamp","EnergyRamp",filename.c_str()) !=1 ||
      SDDS_DefineSimpleColumn(tab.get(),"t","s",SDDS_DOUBLE) != 1 || 
      SDDS_DefineSimpleColumn(tab.get(),"pcentralFactor","",SDDS_DOUBLE) != 1 || 
      SDDS_WriteLayout(tab.get())  != 1)
    {
      throw SDDSError();
    }
  SDDS_StartPage(tab.get(), nSteps+1);

  for (auto row=0u; row<=nSteps; row++) {
    if (SDDS_SetRowValues(tab.get(), SDDS_SET_BY_NAME | SDDS_PASS_BY_VALUE, row,
			  "t", t,
			  "pcentralFactor", ramp(t),
			  NULL) != 1) {
      throw SDDSError();
    }
    t += dt;
  }
  if (SDDS_WritePage(tab.get()) != 1)
    throw SDDSError();
  if (SDDS_Terminate(tab.get()) != 1)
    throw SDDSError();
  //======================================================================================
#else
  //======================================================================================
  ofstream csv;
  csv.open((filename+".csv").c_str());
  if(!csv.is_open())
    throw palatticeFileError(filename);
  for (auto row=0u; row<=nSteps; row++) {
    csv << t << "," << ramp(t) << std::endl;
    t += dt;
  }
  csv.close();

  std::stringstream s;
  s << "csv2sdds " << filename
    << ".csv -columnData=name=t,type=float,units=s -columnData=name=pcentralFactor,type=float "
    << filename;
  system_throwing(s.str().c_str());
  //======================================================================================
#endif
  //======================================================================================
}
