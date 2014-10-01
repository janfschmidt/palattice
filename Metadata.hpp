#ifndef __LIBPAL__METADATA_HPP_
#define __LIBPAL__METADATA_HPP_

#include <vector>
#include <string>
#include "SimTools.hpp"
#include "libpalGitversion.hpp"

using namespace std;

namespace pal
{

class Metadata {
private:
  vector<string> label;
  vector<string> entry;

public:
  Metadata();
  ~Metadata() {}

  void add(string inLabel, string inEntry);         //add an entry. if label already exists, update entry
  void simToolImport(SimToolInstance &sim, string file="default", string labels="default");
  void madximport(string madxFile, string labels, SimToolMode m=online) {SimToolInstance mad(pal::madx,m,madxFile); simToolImport(mad,madxFile,labels);}
  void elegantimport(string eleFile, string labels, SimToolMode m=online) {SimToolInstance ele(pal::elegant,m,eleFile); simToolImport(ele,eleFile,labels);}

  void operator+=(Metadata &other);       //add other Metadata, without first 2 entries ("default" metadata)

  unsigned int size() const {return label.size();}
  string getLabel(unsigned int i) const;
  string getEntry(unsigned int i) const;
  string getbyLabel(string inLabel) const;

  string out(string delimiter) const; //formatted output, each line starting with delimiter
  unsigned int columnwidth() const;
  string timestamp() const;
};

} //namespace pal

#endif
/*__LIBPAL__METADATA_HPP_*/
