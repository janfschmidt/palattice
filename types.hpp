#ifndef __POLSIM__TYPES_HPP_
#define __POLSIM__TYPES_HPP_

#include <string>
#include <vector>
#include <iostream>
#include "constants.hpp"

using namespace std;


// ----- accelerator coordinates (2D "Pair" & 3D "Tripel") -----

class AccPair {
public:
  double x;
  double z;
  
  AccPair() : x(0), z(0) {}

  //addition and subraction
  AccPair operator+(AccPair second) {
    AccPair tmp;
    tmp.x = this->x + second.x;
    tmp.z = this->z + second.z;
    return tmp;
  }
  AccPair operator-(AccPair second) {
    AccPair tmp;
    tmp.x = this->x - second.x;
    tmp.z = this->z - second.z;
    return tmp;
  }
  void operator+=(AccPair second) {
    this->x += second.x;
    this->z += second.z;
  }
  void operator-=(AccPair second) {
    this->x -= second.x;
    this->z -= second.z;
  }
};


class AccTripel {
public:
  double x;
  double z;
  double s;

  AccTripel() : x(0), z(0), s(0) {}

  // addition and subraction
  AccTripel operator+(AccTripel second) {
    AccTripel tmp;
    tmp.x = this->x + second.x;
    tmp.z = this->z + second.z;
    tmp.s = this->s + second.s;
    return tmp;
  }
  AccTripel operator-(AccTripel second) {
    AccTripel tmp;
    tmp.x = this->x - second.x;
    tmp.z = this->z - second.z;
    tmp.s = this->s - second.s;
    return tmp;
  }
  void operator+=(AccTripel second) {
    this->x += second.x;
    this->z += second.z;
    this->s += second.s;
  }
  void operator-=(AccTripel second) {
    this->x -= second.x;
    this->z -= second.z;
    this->s -= second.s;
  }
};

// overload << for output of "Acc" data types
inline ostream &operator<<(ostream &out, const AccPair &A)
{
  out << A.x << "\t" << A.z;
  return out;
}
inline ostream &operator<<(ostream &out, const AccTripel &A)
{
  out << A.x << "\t" << A.z << "\t" << A.s;
  return out;
}

// -------------------------------------------------------------




// classes
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
