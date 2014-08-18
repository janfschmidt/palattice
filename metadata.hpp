#ifndef __POLSIM__METADATA_HPP_
#define __POLSIM__METADATA_HPP_

#include <vector>
#include <string>
#include "gitversion.hpp"

using namespace std;

class METADATA {
private:
  vector<string> label;
  vector<string> entry;

public:
  METADATA();
  ~METADATA() {}
  int madximport(char *madxLabels, string madxfile);
  int elegantimport(char *elegantLabels, string elegantfile);
  void add(string inLabel, string inEntry);
  unsigned int size() const {return label.size();}
  string getLabel(unsigned int i) const;
  string getEntry(unsigned int i) const;
  string getbyLabel(string inLabel) const;
  void setbyLabel(string inLabel, string inEntry);

  string out(string delimiter) const; //formatted output, each line starting with delimiter
  unsigned int columnwidth() const;
  string timestamp() const;
};

#endif

/*__POLSIM__METADATA_HPP_*/
