/* === SimTools Classes ===
 * classes to handle simulation tools (currently MadX, Elegant)
 * SimToolInstance is connected to a lattice file and can be executed,
 * SimToolTable contains columns of simulation tool results
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#ifndef __LIBPALATTICE_SIMTOOLS_HPP_
#define __LIBPALATTICE_SIMTOOLS_HPP_

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include "SDDS/SDDS.h"
#include "types.hpp"

using namespace std;

namespace pal
{

  enum SimTool{madx,elegant};
  enum SimToolMode{online,offline};


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
    std::shared_ptr<SDDS_TABLE> table_sdds;
    SddsColumnFilter sddsFilter;
    bool sdds;

    template<class T> T get_sdds(unsigned int index, string key) const;
    

  public:
    SimToolTable(std::string _name="") : tabname(_name), sdds(false) {}
    ~SimToolTable() {};
    SimToolTable(const SimToolTable & other) = default;
    SimToolTable& operator=(const SimToolTable & other) = default;
    
    void push_back(string key, string value) {table[key].push_back(value);} //set data for "normal" mode
    void init_sdds(const string &filename, vector<string> columnKeys=vector<string>()); //connect with an SDDS file (previous data is deleted!)
    
    unsigned int rows() const {if(sdds) return SDDS_CountRowsOfInterest(table_sdds.get()); else return table.begin()->second.size();}
    unsigned int columns() const {if(sdds) return SDDS_CountColumnsOfInterest(table_sdds.get()); else return table.size();}
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
    bool sdds;
    unsigned int trackingTurns;
    unsigned int trackingNumParticles;
    bool trackingTurnsTouched;
    bool trackingNumParticlesTouched;
    string tag;
    string _path;
    string file;     //mode=online: latticeInput, mode=offline: SimTool Output
    string runFile;
    string outCases(string madxExt, string eleExt) const {if(tool==madx) return outFile(madxExt); else if(tool==elegant) return outFile(eleExt); return "";}
    void replaceInFile(string variable, string value, string delim, string file);
    void replaceTagInFile(string name, string extension, string newTag, string file);

    template<class T> T readParameter_sdds(const string &file, const string &label);

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
    
    void setTurns(unsigned int t);  // if turns!=0 (default) single particle tracking is performed while madx/elegant run
    void setNumParticles(unsigned int n);
    void setRunFile(string file) {runFile = file;}
    void set_sddsMode(bool s) {sdds = s;}

    // read specified columns from a madx/elegant table format output file
    // - if columnKeys is not given, all columns are read
    // - reading stopped after [maxRows] rows, if !=0
    SimToolTable readTable(string file, vector<string> columnKeys=vector<string>(), unsigned int maxRows=0);
    // read specified parameter from file
    template<class T> T readParameter(const string &file, const string &label);

    // readParameter() implementations for some parameters (labels):
    double readCircumference();
    AccPair readTune();

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
    string trajectory_sdds(unsigned int obs) const;

    bool runDone() const {return executed;}
    bool sddsMode() const {if(sdds && tool==elegant) return true; else return false;}
  };


  //template function specialization
  template<> string SimToolInstance::readParameter(const string &file, const string &label);
  template<> AccPair SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;
  template<> AccTriple SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;
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
  
} //namespace pal





//SimToolTable template function implementation:
//keyZ & keyS ignored for 1D data (used with AccPair&AccTriple specializations)
template<class T>
T pal::SimToolTable::get(unsigned int index, string key, string keyZ, string keyS) const
{
  if (index >= this->rows()) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): row " <<index<< ">=" <<this->rows()
	<< " rows in column \"" <<key<< "\" in table " << this->name();
    throw std::out_of_range(msg.str());
  }

  if(sdds) {
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

template<class T>
T pal::SimToolTable::get_sdds(unsigned int index, string key) const
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
      << label << "in " << file;
  throw palatticeError(msg.str());
}



template<class T>
T pal::SimToolInstance::readParameter_sdds(const string &file, const string &label)
{
  SimToolTable t;
  t.init_sdds(file);
  return t.getParameter<T>(label);
}

template<class T>
T pal::SimToolTable::getParameter(const string &label)
{
  if (!sdds)
    throw palatticeError("SimToolTable::getParameter(): Can only be used in SDDS mode. Use SimToolInstamce::readParameter() instead");

  void* mem = SDDS_GetParameter(table_sdds.get(), const_cast<char*>(label.c_str()), NULL);
  if (mem == NULL) throw SDDSError();
  T ret = *static_cast<T *>(mem);
  
  free(mem);
  return ret;
}


#endif
/*__LIBPALATTICE_SIMTOOLS_HPP_*/

