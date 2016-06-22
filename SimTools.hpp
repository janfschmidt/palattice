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

#ifndef __LIBPALATTICE_SIMTOOLS_HPP_
#define __LIBPALATTICE_SIMTOOLS_HPP_

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <initializer_list>
#include "types.hpp"
#include "config.hpp"

#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
#include "SDDS/SDDS.h"
#endif

using namespace std;

namespace pal
{
  
  enum SimTool{madx,elegant};
  enum SimToolMode{online,offline};

  inline bool fileExists(const std::string& filename)
  {
    if (FILE *file = fopen(filename.c_str(), "r")) {
      fclose(file);
      return true;
    }
    else
      return false;
  }

  
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
  enum SimToolFileFormat{ascii,sdds};
#else
  enum SimToolFileFormat{ascii};
#endif


  class SddsColumnFilter {
  public:
    std::string column;
    double min;
    double max;
    bool on;
    SddsColumnFilter() : on(false) {}
    SddsColumnFilter(string _column, double _min, double _max) : column(_column), min(_min), max(_max), on(true) {}
    void set(string _column, double _min, double _max) {column=_column; min=_min; max=_max; on=true;}
    char* c_column() {return const_cast<char *>(column.c_str());}
  };

  
  // a table with columns accessible by column key (string) and row index (int).
  // values are set as string and allow get<T>(key,index) with any type T.
  //
  // alternatively it is connected to an SDDS table file via init_sdds() (previous table data is deleted!)
  // this data is then also accessible with get<T>(key,index) casted to any type T.
  class SimToolTable {
  protected:
    map< string, vector<string> > table;
    std::string tabname;
    bool sdds;
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
    std::shared_ptr<SDDS_TABLE> table_sdds;
    SddsColumnFilter sddsFilter;
    bool sddsFileIsOpen;
#endif
    template<class T> T get_sdds(unsigned int index, string key) const;
    
  public:
    SimToolTable(string _name="") : tabname(_name), sdds(false) {}
    ~SimToolTable() {};
    SimToolTable(const SimToolTable & other) = default;
    SimToolTable& operator=(const SimToolTable & other) = default;
    
    void push_back(string key, string value) {table[key].push_back(value);} //set data for "normal" mode
    void init_sdds(const string &filename, vector<string> columnKeys=vector<string>()); //connect with an SDDS file (previous data is deleted!)
    void init_sdds(const string &filename, std::initializer_list<string> columnKeys) {init_sdds(filename, vector<string>(columnKeys));}
    
    unsigned int rows() const;
    unsigned int columns() const;
    std::string name() const {return tabname;}
    bool sddsMode() const {return sdds;}

    template<class T> T get(unsigned int index, string keyX, string keyZ="", string keyS="") const; //keyZ,keyS for AccPair,AccTriple
    //short forms: getd = get<double> and gets = get<string>
    double getd(unsigned int index, string key) const {return get<double>(index,key);}
    string gets(unsigned int index, string key) const {return get<string>(index,key);}
    template<class T> T getParameter(const string &label);

    //special functions for sdds mode:
    //go to next page of SDDS file. previously applied filter is used
    void nextPage();
    //ignore all lines from SDDS file, if value in given column is out of given range
    //is immediately applied to current page 
    void filterRows(string column, double min, double max);
  };
  


  // an instance of madx or elegant: a "project" with a path and filenames.
  // it can run madx/elegant and read columns from their output files.
  class SimToolInstance {
  protected:
    bool executed;
    SimToolFileFormat preferedFormat;
    unsigned int trackingTurns;
    unsigned int trackingNumParticles;
    double trackingMomentum_MeV;
    bool trackingNumParticlesTouched;
    string tag;
    string _path;
    string file;     //mode=online: latticeInput, mode=offline: SimTool Output
    string runFile;
    string outCases(string madxExt, string eleExt) const {if(tool==madx) return outFile(madxExt); else if(tool==elegant) return outFile(eleExt); return "";}
    void replaceInFile(string variable, string value, string delim, string file);
    void replaceTagInFile(string name, string extension, string newTag, string file);

    template<class T> T readParameter_sdds(const string &file, const string &label);
    std::string stripExtension(std::string f) const;

  public:
    const SimTool tool;
    const SimToolMode mode;
    bool verbose;

    //SimToolInstance constructor:
    //-fileIn: mode=online: latticeInput, mode=offline: SimTool Output
    //-fileTag: added to output files to distinguish "runs". needed for mode=online only.
    SimToolInstance(SimTool tool, SimToolMode mode, string fileIn, string fileTag="");
    ~SimToolInstance() {}

    void run();
    unsigned int turns() const {return trackingTurns;}
    unsigned int numParticles() const {return trackingNumParticles;}
    double momentum_MeV() const {return trackingMomentum_MeV;}
    
    void setTurns(unsigned int t);  // if turns!=0 (default) single particle tracking is performed while madx/elegant run
    void setNumParticles(unsigned int n);
    void setMomentum_MeV(double p_MeV);
    
    void setRunFile(string file) {runFile = file;}
    void setPreferedFileFormat(SimToolFileFormat f) {preferedFormat = f; executed = false;}

    // read specified columns from a madx/elegant table format output file
    // - if columnKeys is not given, all columns are read
    // - reading stopped after [maxRows] rows, if !=0
    SimToolTable readTable(string file, vector<string> columnKeys=vector<string>(), unsigned int maxRows=0);
    SimToolTable readTable(string file, std::initializer_list<string> columnKeys, unsigned int maxRows=0) {return readTable(file, vector<string>(columnKeys), maxRows);}
    // read specified parameter from file
    template<class T> T readParameter(const string &file, const string &label);

    // readParameter() implementations for some parameters (labels):
    double readCircumference();
    double readGammaCentral();
    double readAlphaC();       // momentum compaction factor
    double readAlphaC2();      // second order momemtum compaction factor
    AccPair readTune();
    AccPair readChromaticity();
    AccTriple readDampingPartitionNumber_syli();

    string tool_string() const {if (tool==madx) return "madx"; else if (tool==elegant) return "elegant"; else return "";}
    string mode_string() const {if (mode==online) return "online"; else if (mode==offline) return "offline"; else return "";}

    //filenames
    string filebase() const;                   //output filename without extension
    string inFile() const {return _path+file;} //mode=online: latticeInput, mode=offline: SimTool Output
    string outFile(string extension) const {return _path+filebase()+"."+extension;}
    string log() const {return _path+tool_string()+".log";}
    string path() const {return _path;}
    string lattice() const {return outCases("twiss", "param");}
    string twiss() const {return outCases("twiss", "twi");}
    string orbit() const {return outCases("twiss", "clo");}
    string trajectory(unsigned int obs, unsigned int particle) const;

    bool runDone() const {return executed;}
    bool sddsMode() const;
  };


  //template function specialization
  template<> string SimToolInstance::readParameter(const string &file, const string &label);
  template<> AccPair SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;
  template<> AccTriple SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;


//======================================================================================
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
//======================================================================================
  
  template<> string SimToolTable::getParameter(const string &label);
  template<> string SimToolTable::get_sdds(unsigned int index, string key) const;

  class SDDSError : public std::exception
  {
    virtual const char* what() const throw()
    {
      SDDS_PrintErrors(stdout,SDDS_VERBOSE_PrintErrors);
      return "SDDS ERROR description above";
    }
  };

  class SDDSPageError : public SDDSError {
    virtual const char* what() const throw()
    {
      return "End of SDDS file reached.";
    }
  };
  
//======================================================================================
  #else
//======================================================================================

  typedef std::exception SDDSError; //dummy
  typedef std::runtime_error SDDSPageError; //dummy
  
//======================================================================================
  #endif
//======================================================================================


  class sddsDummyCalled : public palatticeError {
  public:
    sddsDummyCalled()
      : palatticeError("FATAL! Dies hätte nicht passieren dürfen!\nSDDS dummy function should NEVER be called.") {}
  };
  
} //namespace pal





//SimToolTable template function implementation:
//keyZ & keyS ignored for 1D data (used with AccPair&AccTriple specializations)
template<class T>
inline T pal::SimToolTable::get(unsigned int index, string key, string keyZ, string keyS) const
{
  if (index >= this->rows()) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): row " <<index<< ">=" <<this->rows()
	<< " rows in column \"" <<key<< "\" in table " << this->name();
    throw std::out_of_range(msg.str());
  }

  if(sddsMode()) {
    return get_sdds<T>(index,key);
  }

  map< string, vector<string> >::const_iterator it = table.find(key);
  if (it == table.end()) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): No column \"" <<key<< "\" in table " << this->name();
    throw palatticeError(msg.str());
  }

  stringstream s(table.at(key)[index]);
  T value;
  s >> value;
  return value;
}



//SimToolInstance template function implementation:
template<class T>
T pal::SimToolInstance::readParameter(const string &file, const string &label)
{
   // run?
  if (!executed && mode==online)
    this->run();
  
  if(sddsMode()) {
    return readParameter_sdds<T>(file, label);
  }
  
  fstream f;
  string tmp;
  T val;

  f.open(file.c_str(), ios::in);
  if (!f.is_open())
    throw palatticeFileError(file);

  while(!f.eof()) {
    f >> tmp;
    if (tmp == label) {
      if (tool==madx) f >> tmp; //skip type/length info column
      f >> val;
      return val;
    }
    else if ((tool==madx && tmp=="*") || (tool==elegant && tmp=="***"))
      break;
  }
  stringstream msg;
  msg << "ERROR: pal::SimToolInstance::readParameter(): No parameter label "
      << label << " in " << file;
  throw palatticeError(msg.str());
}









//======================================================================================
#ifdef LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY
//======================================================================================

template<class T>
inline T pal::SimToolTable::get_sdds(unsigned int index, string key) const
{
  void* mem = SDDS_GetValue(table_sdds.get(), const_cast<char*>(key.c_str()), index, NULL);
  if (mem == NULL) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): No column \"" <<key<< "\" in SDDS table " << this->name();
    throw palatticeError(msg.str());
  }
  T ret = *static_cast<T *>(mem);
  free(mem);
  return ret;
}

template<class T>
inline T pal::SimToolTable::getParameter(const string &label)
{
  if (!sdds)
    throw palatticeError("SimToolTable::getParameter(): Can only be used in SDDS mode. Use SimToolInstance::readParameter() instead");

  void* mem = SDDS_GetParameter(table_sdds.get(), const_cast<char*>(label.c_str()), NULL);
  if (mem == NULL) throw SDDSError();
  T ret = *static_cast<T *>(mem);
  
  free(mem);
  return ret;
}

template<class T>
inline T pal::SimToolInstance::readParameter_sdds(const string &file, const string &label)
{
  SimToolTable t;
  t.init_sdds(file);
  return t.getParameter<T>(label);
}

//======================================================================================
#else
//======================================================================================
// dummy function implementations. they should never be called, because sddsMode() is
// always false if LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY is not defined.

template<class T>
inline T pal::SimToolTable::get_sdds(unsigned int index, string key) const {throw sddsDummyCalled();}

template<class T>
inline T pal::SimToolTable::getParameter(const string &label) {throw sddsDummyCalled();}

template<class T>
inline T pal::SimToolInstance::readParameter_sdds(const string &file, const string &label) {throw sddsDummyCalled();}
  
//======================================================================================
#endif
//======================================================================================


#endif
/*__LIBPALATTICE_SIMTOOLS_HPP_*/

