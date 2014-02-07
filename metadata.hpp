#ifndef __POLSIM__METADATA_HPP_
#define __POLSIM__METADATA_HPP_

#include <vector>
#include <string>
#include "types.hpp"
#include "spectrum.hpp"

using namespace std;

class METADATA {
private:
  vector<string> label;
  vector<string> entry;

public:
  METADATA() {METADATA::add("created at", timestamp());}
  METADATA(string path, bool elsa, simulationTool s, bool diff, char *spuren, char *Ref_spuren);
  ~METADATA() {}
  int madximport(char *madxLabels, const char *madxfile);
  int elegantimport(char *elegantLabels, const char *elegantfile);
  void add(string inLabel, string inEntry);
  unsigned int size() const;
  string getLabel(unsigned int i) const;
  string getEntry(unsigned int i) const;
  string getbyLabel(string inLabel) const;
  void setbyLabel(string inLabel, string inEntry);

  string get(Spectrum s, string tag="") const;
  unsigned int columnwidth() const;

private:
  string timestamp() const;
};


#endif

/*__POLSIM__METADATA_HPP_*/
