#ifndef __LIBPAL_SIMTOOLS_HPP_
#define __LIBPAL_SIMTOOLS_HPP_

#include <string>
#include <sstream>
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
  };
  


  // an instance of madx or elegant: a "project" with a path and filenames.
  // it can run madx/elegant and read columns from their output files.
  class SimToolInstance {
  protected:
    bool executed;
    unsigned int trackingTurns;
    string _path;
    string file;     //mode=online: latticeInput, mode=offline: SimTool Output
    string filebase; //output filename without extension
    string runFile;
    string outCases(string madxExt, string eleExt) const {if(t==madx) return outFile(madxExt); else if(t==elegant) return outFile(eleExt); return "";}
    void replaceInFile(string variable, string value, string delim, string file);

  public:
    const SimTool t;
    const SimToolMode m;

    SimToolInstance(SimTool tool, SimToolMode mode, string fileIn);
    ~SimToolInstance() {}

    void run();
    SimToolTable readTable(string file, vector<string> columnKeys);

    //filenames
    string inFile() const {return _path+file;} //mode=online: latticeInput, mode=offline: SimTool Output
    string outFile(string extension) const {return _path+filebase+"."+extension;}
    string log() const {return outFile("log");}
    string path() const {return _path;}
    string lattice() const {return outCases("twiss", "param");}
    string twiss() const {return outCases("twiss", "twi");}
    string orbit() const {return outCases("twiss", "clo");}
    string trajectory(unsigned int obs, unsigned int particle) const;

    bool runDone() const {return executed;}
  };
  
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


#endif
/*__LIBPAL_SIMTOOLS_HPP_*/

