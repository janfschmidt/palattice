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
#include "libsddsi/sdds-interface.hpp"
#include "types.hpp"

using namespace std;

namespace pal
{

  enum SimTool{madx,elegant};
  enum SimToolMode{online,offline};
  

  // a table with columns accessible by column key (string) and row index (int).
  // values are set as string and allow get<T>(key,index) with any type T.
  class SimToolTable {
  protected:
    map< string, vector<string> > table;
    std::string tabname;

  public:
    SimToolTable(std::string _name="") : tabname(_name) {}
    void push_back(string key, string value) {table[key].push_back(value);}
    template<class T> T get(unsigned int index, string keyX, string keyZ="", string keyS="") const; //keyZ,keyS for AccPair,AccTriple
    
    unsigned int rows() const {return table.begin()->second.size();}
    unsigned int columns() const {return table.size();}
    std::string name() const {return tabname;}

    //short forms: getd = get<double> and gets = get<string>
    double getd(unsigned int index, string key) const {return get<double>(index,key);}
    string gets(unsigned int index, string key) const {return get<string>(index,key);}
  };
  


  // an instance of madx or elegant: a "project" with a path and filenames.
  // it can run madx/elegant and read columns from their output files.
  class SimToolInstance {
  protected:
    bool executed;
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

  public:
    const SimTool tool;
    const SimToolMode mode;
    bool verbose;
    bool SDDS;

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
    SimToolTable readTable(string file, vector<string> columnKeys, unsigned int maxRows=0); // read specified columns from a madx/elegant table format output file (reading stopped after [maxRows] rows, if !=0)
    template<class T> T readParameter(string file, string label); // read specified parameter from file header

    SimToolTable readSDDSTable(string file, map<string, sddsi::sdds_type> columnKeys);

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

    bool runDone() const {return executed;}
  };


  //SimToolInstance template function specialization
  template<> string SimToolInstance::readParameter(string file, string label);
  template<> AccPair SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;
  template<> AccTriple SimToolTable::get(unsigned int index, string keyX, string keyZ, string keyS) const;

  
} //namespace pal





//SimToolTable template function implementation:
//keyZ & keyS ignored for 1D data (used with AccPair&AccTriple specializations)
template<class T>
T pal::SimToolTable::get(unsigned int index, string key, string keyZ, string keyS) const
{
  map< string, vector<string> >::const_iterator it = table.find(key);
  if (it == table.end()) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): No column \"" <<key<< "\" in table " << this->name();
    throw palatticeError(msg.str());
  }

  if (index >= table.at(key).size()) {
    stringstream msg;
    msg << "pal::SimToolTable::get<T>(): row " <<index<< ">=" <<table.at(key).size()
	<< "rows in column \"" <<key<< "\" in table " << this->name();
    throw std::out_of_range(msg.str());
  }
  stringstream s(table.at(key)[index]);
  T value;
  s >> value;
  return value;
}


//SimToolInstance template function implementation:
template<class T>
T pal::SimToolInstance::readParameter(string file, string label)
{
  fstream f;
  string tmp;
  T val;

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


#endif
/*__LIBPALATTICE_SIMTOOLS_HPP_*/

