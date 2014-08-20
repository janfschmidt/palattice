#ifndef __POLSIM__FILENAMES_HPP_
#define __POLSIM__FILENAMES_HPP_

#include <string>
#include "libpal.hpp"

using namespace std;

class FILENAMES {

public:
  FILENAMES() : path("uninitialized") {}
  FILENAMES(string pathIn, pal::simulationTool _simTool, bool elsa, bool diff, bool sgt_access, string spurenIn, string refIn);
  ~FILENAMES() {}
  string out(string name, string t) const {return (path+"/inout/"+name+t+difftag+".dat");}
  string spec(string name, string t) const {return (path+"/inout/"+name+t+difftag+".spectrum");}
  string path;
  string path_simTool;
  string lattice;
  string orbit;
  string misalign_dip;
  string spuren;
  string spuren_ref;
  string lattice_ref;
  string misalign_dip_ref;
  string orbit_ref;
  string difftag;
  pal::simulationTool simTool;


private:
  string file_lattice;
  string file_orbit;
  string file_misalign_dip;
  string file_misalign_dip_ref;
  string file_lattice_ref;
  string file_orbit_ref;
};


#endif

/*__POLSIM__FILENAMES_HPP_*/
