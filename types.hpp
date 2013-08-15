#ifndef __POLSIM__TYPES_HPP_
#define __POLSIM__TYPES_HPP_

#include <string>
#include <vector>
#include <iostream>
#include "constants.hpp"

using namespace std;


// transversal accelerator coordinates (2D)
class AccPair {
public:
  double x;
  double z;
  
  AccPair() : x(0), z(0) {}
  
  // overload << for output
  friend ostream &operator<<(ostream &out, const AccPair &A)
  {
    out << A.x << "\t" << A.z;
    return out;
  }
};

// accelerator coordinates (3D)
class AccTripel {
public:
  double x;
  double z;
  double s;

  AccTripel() : x(0), z(0), s(0) {}

  // overload << for output
  friend ostream &operator<<(ostream &out, const AccTripel &A)
  {
    out << A.x << "\t" << A.z << "\t" << A.s;
    return out;
  }
};



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



/* typedefs */
typedef vector<MAGNET> magnetvec;



#endif

/*__POLSIM__TYPES_HPP_*/
