/* libpalattice data type definitions
 * AccPair/AccTriple for 2D/3D quantities in accelerator coordinates (x,z)/(x,z,s)
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBPALATTICE__TYPES_HPP_
#define __LIBPALATTICE__TYPES_HPP_

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
  AccPair operator+(const AccPair second) const {
    AccPair tmp;
    tmp.x = this->x + second.x;
    tmp.z = this->z + second.z;
    return tmp;
  }
  AccPair operator-(const AccPair second) const {
    AccPair tmp;
    tmp.x = this->x - second.x;
    tmp.z = this->z - second.z;
    return tmp;
  }
  void operator+=(const AccPair second) {
    this->x += second.x;
    this->z += second.z;
  }
  void operator-=(const AccPair second) {
    this->x -= second.x;
    this->z -= second.z;
  }
  //multiplication&division
  template <typename T> AccPair operator*(const T num) {
    AccPair tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    return tmp;
  }
  template <typename T> AccPair operator/(const T num) {
    AccPair tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    return tmp;
  }
  template <typename T> void operator*=(const T num) {
    this->x *= num;
    this->z *= num;
  }
  template <typename T> void operator/=(const T num) {
    this->x /= num;
    this->z /= num;
  }
  //comparison
  bool operator==(const AccPair &o) const {
    if (this->x != o.x) return false;
    else if (this->z != o.z) return false;
    else return true;
  }
  bool operator!=(const AccPair &o) const {
    if (operator==(o) == true) return false;
    else return true;
  }

    //absolute value
  double abs() const {
    return std::sqrt(std::pow(x,2) + std::pow(z,2));
  }
  

  // tilt clockwise around s (longitudinal) axis
  AccPair tilt(const double tilt) const {
    AccPair tmp;
    tmp.x = this->x*cos(tilt) + this->z*sin(tilt);
    tmp.z = - this->x*sin(tilt) + this->z*cos(tilt);
    return tmp;
  }
};


class AccTriple : public AccPair {
public:
  double s;

  AccTriple() : s(0.) {x=0.; z=0.;}
  inline std::string header() const {return "x / m\t\t z / m\t\t s / m";}

  // addition and subraction
  AccTriple operator+(const AccTriple second) const {
    AccTriple tmp;
    tmp.x = this->x + second.x;
    tmp.z = this->z + second.z;
    tmp.s = this->s + second.s;
    return tmp;
  }
  AccTriple operator-(const AccTriple second) const {
    AccTriple tmp;
    tmp.x = this->x - second.x;
    tmp.z = this->z - second.z;
    tmp.s = this->s - second.s;
    return tmp;
  }
  void operator+=(const AccTriple second) {
    this->x += second.x;
    this->z += second.z;
    this->s += second.s;
  }
  void operator-=(const AccTriple second) {
    this->x -= second.x;
    this->z -= second.z;
    this->s -= second.s;
  }
  //multiplication&division
  template <typename T> AccTriple operator*(const T num) {
    AccTriple tmp;
    tmp.x = this->x * num;
    tmp.z = this->z * num;
    tmp.s = this->s * num;
    return tmp;
  }
  template <typename T> AccTriple operator/(const T num) {
    AccTriple tmp;
    tmp.x = this->x / num;
    tmp.z = this->z / num;
    tmp.s = this->s / num;
    return tmp;
  }
  template <typename T> void operator*=(const T num) {
    this->x *= num;
    this->z *= num;
    this->s *= num;
  }
  template <typename T> void operator/=(const T num) {
    this->x /= num;
    this->z /= num;
    this->s /= num;
  }
  //comparison
  bool operator==(const AccTriple &o) const {
    if (this->x != o.x) return false;
    else if (this->z != o.z) return false;
    else if (this->s != o.s) return false;
    else return true;
  }
  bool operator!=(const AccTriple &o) const {
    if (operator==(o) == true) return false;
    else return true;
  }

  //absolute value
  double abs() const {
    return std::sqrt(std::pow(x,2) + std::pow(z,2) + std::pow(s,2));
  }
  
  // tilt clockwise around s (longitudinal) axis
  AccTriple tilt(const double tilt) const {
    AccTriple tmp;
    tmp.x = this->x*cos(tilt) + this->z*sin(tilt);
    tmp.z = - this->x*sin(tilt) + this->z*cos(tilt);
    tmp.s = this->s;
    return tmp;
  }
};



  // exceptions
  class palatticeError : public std::runtime_error {
  public:
    palatticeError(std::string msg) : std::runtime_error(msg) {}
  };

  class palatticeFileError : public palatticeError {
  public:
    std::string filename;
    palatticeFileError(std::string file) : palatticeError("Cannot open "+file), filename(file) {}
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


namespace std {
  //define std::pow() & std::sqrt() for "Acc" data types
  inline pal::AccPair pow(const pal::AccPair &A, unsigned int n)
  {
    pal::AccPair ret;
    ret.x = pow(A.x,n);
    ret.z = pow(A.z,n);
    return ret;
  }
  inline pal::AccTriple pow(const pal::AccTriple &A, unsigned int n)
  {
    pal::AccTriple ret;
    ret.x = pow(A.x,n);
    ret.z = pow(A.z,n);
    ret.s = pow(A.s,n);
    return ret;
  }
  inline pal::AccPair sqrt(const pal::AccPair &A)
  {
    pal::AccPair ret;
    ret.x = sqrt(A.x);
    ret.z = sqrt(A.z);
    return ret;
  }
  inline pal::AccTriple sqrt(const pal::AccTriple &A)
  {
    pal::AccTriple ret;
    ret.x = sqrt(A.x);
    ret.z = sqrt(A.z);
    ret.s = sqrt(A.s);
    return ret;
  }
} //namespace std




#endif
/*__LIBPALATTICE__TYPES_HPP_*/
