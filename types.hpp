#ifndef __POLSIM__TYPES_HPP_
#define __POLSIM__TYPES_HPP_

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>



enum simulationTool{madx,elegant};
enum madxLatticeType{line,sequence};
enum AccAxis {x,z,s};


// ----- accelerator coordinates (2D "Pair" & 3D "Triple") -----

class AccPair {
public:
  double x;
  double z;
  
  AccPair() : x(0.), z(0.) {}
  inline std::string header() const {return "x / m\t\t z / m";}

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
  //multiplication&division
AccPair operator*(double num) {
    AccPair tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    return tmp;
  }
  AccPair operator/(double num) {
    AccPair tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    return tmp;
  }
  void operator*=(double num) {
    this->x *= num;
    this->z *= num;
  }
  void operator/=(double num) {
    this->x /= num;
    this->z /= num;
  }
  // tilt clockwise around s (longitudinal) axis -> dpsi
  AccPair tilt(double dpsi) {
    AccPair tmp;
    tmp.x = this->x*cos(dpsi) + this->z*sin(dpsi);
    tmp.z = - this->x*sin(dpsi) + this->z*cos(dpsi);
    return tmp;
  }
};


class AccTriple : public AccPair {
public:
  double s;

  AccTriple() : s(0.) {x=0.; z=0.;}
  inline std::string header() const {return "x / m\t\t z / m\t\t s / m";}

  // addition and subraction
  AccTriple operator+(AccTriple second) {
    AccTriple tmp;
    tmp.x = this->x + second.x;
    tmp.z = this->z + second.z;
    tmp.s = this->s + second.s;
    return tmp;
  }
  AccTriple operator-(AccTriple second) {
    AccTriple tmp;
    tmp.x = this->x - second.x;
    tmp.z = this->z - second.z;
    tmp.s = this->s - second.s;
    return tmp;
  }
  void operator+=(AccTriple second) {
    this->x += second.x;
    this->z += second.z;
    this->s += second.s;
  }
  void operator-=(AccTriple second) {
    this->x -= second.x;
    this->z -= second.z;
    this->s -= second.s;
  }
  //multiplication&division
  AccTriple operator*(double num) {
    AccTriple tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    tmp.s = this->s * num;
    return tmp;
  }
  AccTriple operator/(double num) {
    AccTriple tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    tmp.s = this->s / num;
    return tmp;
  }
 void operator*=(double num) {
    this->x *= num;
    this->z *= num;
    this->s *= num;
  }
  void operator/=(double num) {
    this->x /= num;
    this->z /= num;
    this->s /= num;
  }
  // tilt clockwise around s (longitudinal) axis -> dpsi
  AccTriple tilt(double dpsi) {
    AccTriple tmp;
    tmp.x = this->x*cos(dpsi) + this->z*sin(dpsi);
    tmp.z = - this->x*sin(dpsi) + this->z*cos(dpsi);
    tmp.s = this->s;
    return tmp;
  }

};

// overload << for output of "Acc" data types
inline std::ostream &operator<<(std::ostream &out, const AccPair &A)
{
  out << A.x << "\t" << A.z;
  return out;
}
inline std::ostream &operator<<(std::ostream &out, const AccTriple &A)
{
  out << A.x << "\t" << A.z << "\t" << A.s;
  return out;
}


// -------------------------------------------------------------




// ELSA: Closed Orbit & Corrector-Kicks
class BPM_MS {
public:
  int ms;
  double x;
  double z;

  BPM_MS() : ms(0), x(0), z(0) {}
};

class BPM {
public:
  std::vector<BPM_MS> time;
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
  std::vector<CORR_MS> time;
  double pos;

  CORR() : pos(0) {}
};



#endif

/*__POLSIM__TYPES_HPP_*/
