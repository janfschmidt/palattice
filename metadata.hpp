#ifndef __POLSIM__METADATA_HPP_
#define __POLSIM__METADATA_HPP_

#include <vector>
#include <string>
#include "gitversion.hpp"

using namespace std;

class Metadata {
private:
  vector<string> label;
  vector<string> entry;

public:
  Metadata();
  ~Metadata() {}

  void add(string inLabel, string inEntry);         //add an entry. if label already exists, update entry
  int madximport(string madxLabels, string madxfile);
  int elegantimport(string elegantLabels, string elegantfile);

  void operator+=(Metadata &other);       //add other Metadata, without first 2 entries ("default" metadata)

  unsigned int size() const {return label.size();}
  string getLabel(unsigned int i) const;
  string getEntry(unsigned int i) const;
  string getbyLabel(string inLabel) const;

  string out(string delimiter) const; //formatted output, each line starting with delimiter
  unsigned int columnwidth() const;
  string timestamp() const;
};

#endif

/*__POLSIM__METADATA_HPP_*/
