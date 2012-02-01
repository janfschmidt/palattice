#ifndef __POLSIM__METADATA_HPP_
#define __POLSIM__METADATA_HPP_

#include <vector>
#include <string>

using namespace std;

class METADATA {
private:
  vector<string> label;
  vector<string> entry;

public:
  METADATA();
  int madximport(char *madxLabels, char *madxfile);
  void add(string inLabel, string inEntry);
  int size() const;
  string getLabel(int i) const;
  string getEntry(int i) const;
  string getbyLabel(string inLabel) const;

private:
  string timestamp() const;
};


#endif

/*__POLSIM__METADATA_HPP_*/
