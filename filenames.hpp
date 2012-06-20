#ifndef __POLSIM__FILENAMES_HPP_
#define __POLSIM__FILENAMES_HPP_

#include <string>

using namespace std;

class FILENAMES {

public:
  FILENAMES() : project("uninitialized") {}
  FILENAMES(string path, bool elsa, bool diff, string spurenIn, string refIn);
  ~FILENAMES() {}
  string path() const {return project;}
  const char* import() const {return (project+s_import).c_str();}
  const char* misalign_dip() const {return (project+s_misalign_dip).c_str();}
  const char* spuren() const {return spurenpath.c_str();}
  const char* ref() const {return referencepath.c_str();}
  const char* out(string name, string t) const {return (project+"/inout/"+name+t+difftag+".dat").c_str();}
  const char* spec(string name, string t) const {return (project+"/inout/"+name+t+difftag+".spectrum").c_str();}



private:
  string project;
  string difftag;
  string spurenpath;
  string referencepath;
  string s_import;
  string s_misalign_dip;
  string s_import_ref;
};


#endif

/*__POLSIM__FILENAMES_HPP_*/
