#ifndef __LIBPAL_SIMTOOLS_HPP_
#define __LIBPAL_SIMTOOLS_HPP_

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
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

  public:
    void push_back(string key, string value) {table[key].push_back(value);}
    template<class T> T get(string key, unsigned int index);
    unsigned int rows() const {return table.begin()->second.size();}
    unsigned int columns() const {return table.size();}

    //short forms: getf = get<double> and gets = get<string>
    double getd(string key, unsigned int index) {return get<double>(key,index);}
    string gets(string key, unsigned int index) {return get<string>(key,index);}
  };
  


  // an instance of madx or elegant: a "project" with a path and filenames.
  // it can run madx/elegant and read columns from their output files.
  class SimToolInstance {
  protected:
    bool executed;
    unsigned int trackingTurns;
    string tag;
    string _path;
    string file;     //mode=online: latticeInput, mode=offline: SimTool Output
    string filebase; //output filename without extension
    string runFile;
    string outCases(string madxExt, string eleExt) const {if(tool==madx) return outFile(madxExt); else if(tool==elegant) return outFile(eleExt); return "";}
    void replaceInFile(string variable, string value, string delim, string file);
    void replaceTagInFile(string name, string extension, string newTag, string file);

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
    void setTurns(unsigned int t) {trackingTurns=t; executed=false;}  // if turns!=0 (default) single particle tracking is performed while madx/elegant run
    void setRunFile(string file) {runFile = file;}
    SimToolTable readTable(string file, vector<string> columnKeys, unsigned int maxRows=0); // read specified columns from a madx/elegant table format output file (reading stopped after [maxRows] rows, if !=0)
    template<class T> T readParameter(string file, string label); // read specified parameter from file header

    // readParameter() implementations for some parameters (labels):
    double readCircumference();
    AccPair readTune();

    string tool_string() const {if (tool==madx) return "Mad-X"; else if (tool==elegant) return "Elegant"; else return "";}

    //filenames
    string inFile() const {return _path+file;} //mode=online: latticeInput, mode=offline: SimTool Output
    string outFile(string extension) const {return _path+filebase+"."+extension;}
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

  
} //namespace pal





//SimToolTable template function implementation:
template<class T>
T pal::SimToolTable::get(string key, unsigned int index)
{
  map< string, vector<string> >::iterator it = table.find(key);
  if (it == table.end()) {
    stringstream msg;
    msg << "ERROR: pal::SimToolTable::get<T>(): No key \"" <<key<< "\" in this table.";
    throw libpalError(msg.str());
  }

  stringstream s(table[key][index]);
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
    throw libpalFileError(file);

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
  throw libpalError(msg.str());
}


#endif
/*__LIBPAL_SIMTOOLS_HPP_*/

