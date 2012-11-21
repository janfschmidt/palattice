#ifndef __POLSIM__FILENAMES_HPP_
#define __POLSIM__FILENAMES_HPP_

#include <string>

using namespace std;

class FILENAMES {

public:
  FILENAMES() : path("uninitialized") {}
  FILENAMES(string pathIn, bool elsa, bool diff, bool sgt_access, string spurenIn, string refIn);
  ~FILENAMES() {}
  string out(string name, string t) const {return (path+"/inout/"+name+t+difftag+".dat");}
  string spec(string name, string t) const {return (path+"/inout/"+name+t+difftag+".spectrum");}
  string path;
  string import;
  string misalign_dip;
  string spuren;
  string ref;
  string difftag;


private:
  string file_import;
  string file_misalign_dip;
  string file_import_ref;
};


#endif

/*__POLSIM__FILENAMES_HPP_*/
