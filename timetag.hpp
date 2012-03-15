#ifndef __POLSIM__TIMETAG_HPP_
#define __POLSIM__TIMETAG_HPP_

#include <vector>
#include <string>

using namespace std;

class TIMETAG {

public:
  TIMETAG() : multi(false) {}
  TIMETAG(int t) : multi(false) {Tvec.push_back(t);}
  TIMETAG(char *tagfile);
  ~TIMETAG() {}
  void set(int t);
  void set(char *tagfile);
  int get(unsigned int i) const;
  unsigned int size() const {return Tvec.size();}
  string tag(unsigned int i) const;
  string label(unsigned int i) const;

private:
  vector<int> Tvec;
  bool multi;
};


#endif

/*__POLSIM__TIMETAG_HPP_*/
