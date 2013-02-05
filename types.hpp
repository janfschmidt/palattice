#ifndef __POLSIM__TYPES_HPP_
#define __POLSIM__TYPES_HPP_

#include <string>
#include <vector>
#include "constants.hpp"

using namespace std;

/* classes */
class BPM_MS {
public:
  int ms;
  double x;
  double z;

  BPM_MS() : ms(0), x(0), z(0) {}
};

class BPM {
public:
  vector<BPM_MS> time;
  double pos;

  BPM() : pos(0) {}
};

class CORR_MS {
public:
  int ms;
  double kick;

  CORR_MS() : ms(0), kick(0) {}
};

class CORR {
public:
  vector<CORR_MS> time;
  double pos;

  CORR() : pos(0) {}
};

class MAGNET {
public:
  string name;
  double start;
  double end;
  double length;
  double strength;
  double dpsi;   //alignment error: rotation around s

  MAGNET() : start(0), end(0), length(0), strength(0), dpsi(0) {}
};

class FIELD {
public:
  double pos;
  string name;
  double x;
  double z;
  double theta; //spin phaseadvance in degree

  FIELD() : pos(0), x(0), z(0), theta(0) {}
};

class ORBIT {
public:
  double pos;
  double x;
  double z;

  ORBIT() : pos(0), x(0), z(0) {}
};

class SPECTRUM {
public:
  double freq;
  double amp;
  double phase;

  SPECTRUM() : freq(0), amp(0), phase(0) {}
};




/* typedefs */
typedef vector<MAGNET> magnetvec;
typedef vector<ORBIT> orbitvec;
typedef vector<FIELD> fieldvec;



#endif

/*__POLSIM__TYPES_HPP_*/
