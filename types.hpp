#ifndef __LIBPAL__TYPES_HPP_
#define __LIBPAL__TYPES_HPP_

#include <string>
#include <iostream>
#include <stdexcept>
#include <cmath>


namespace pal
{

enum MadxLatticeType{line,sequence};
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
  template <typename T> AccPair operator*(T num) {
    AccPair tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    return tmp;
  }
  template <typename T> AccPair operator/(T num) {
    AccPair tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    return tmp;
  }
  template <typename T> void operator*=(T num) {
    this->x *= num;
    this->z *= num;
  }
  template <typename T> void operator/=(T num) {
    this->x /= num;
    this->z /= num;
  }
  // tilt clockwise around s (longitudinal) axis -> dpsi
  AccPair tilt(double dpsi) const {
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
  template <typename T> AccTriple operator*(T num) {
    AccTriple tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    tmp.s = this->s * num;
    return tmp;
  }
  template <typename T> AccTriple operator/(T num) {
    AccTriple tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    tmp.s = this->s / num;
    return tmp;
  }
  template <typename T> void operator*=(T num) {
    this->x *= num;
    this->z *= num;
    this->s *= num;
  }
  template <typename T> void operator/=(T num) {
    this->x /= num;
    this->z /= num;
    this->s /= num;
  }
  // tilt clockwise around s (longitudinal) axis -> dpsi
  AccTriple tilt(double dpsi) const {
    AccTriple tmp;
    tmp.x = this->x*cos(dpsi) + this->z*sin(dpsi);
    tmp.z = - this->x*sin(dpsi) + this->z*cos(dpsi);
    tmp.s = this->s;
    return tmp;
  }
};



  // exceptions
  class libpalError : public std::runtime_error {
  public:
    libpalError(std::string msg) : std::runtime_error(msg) {}
  };

  class libpalFileError : public libpalError {
  public:
    libpalFileError(std::string file) : libpalError("Cannot open "+file) {}
  };


} //namespace pal


// overload << for output of "Acc" data types
inline std::ostream &operator<<(std::ostream &out, const pal::AccPair &A)
{
  out << A.x << "\t" << A.z;
  return out;
}
inline std::ostream &operator<<(std::ostream &out, const pal::AccTriple &A)
{
  out << A.x << "\t" << A.z << "\t" << A.s;
  return out;
}




#endif
/*__LIBPAL__TYPES_HPP_*/
