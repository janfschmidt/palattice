/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#ifndef __ACCELEMENTS_HPP_
#define __ACCELEMENTS_HPP_

#include <iostream>
#include <string>
#include <cmath>
#include <stdexcept>
#include "types.hpp"

using namespace std;


enum element_type{dipole,quadrupole,corrector,sextupole,drift};
enum element_plane{H,V,L};    //horizontal,vertical,longitudinal
enum element_family{F,D};     //focus,defocus

// abstract base class
class AccElement {
protected:
  static AccPair zeroPair;
  static AccTriple zeroTriple;

public:
  string name;
  double length;
  const element_type type;
  
  //alignment errors:
  double dpsi; //rotation around s axis


  AccElement(string _name, double _length,element_type _type)
    : name(_name),length(_length),type(_type),dpsi(0.) {}

  virtual AccElement* clone() const =0;

  virtual AccTriple B() const
  {string msg="B-Field of "+type_string()+" depends on orbit! Please provide argument."; throw std::invalid_argument(msg);}
  virtual AccTriple B(AccPair orbit) const {return zeroTriple;}

  void misalign(double _dpsi) {dpsi = _dpsi;}

  string type_string() const;

};



// drift
class Drift : public AccElement {
public:
  Drift(string _name="nothing", double _length=0.)
    : AccElement(_name,_length,drift) {}
 ~Drift() {}
  
  virtual Drift* clone() const {return new Drift(*this);}

  virtual AccTriple B() const {return zeroTriple;}
};



// abstract magnet class
class Magnet : public AccElement {
public:
  double strength;

  Magnet(string _name, double _length, element_type _type, double _strength=0.)
    : AccElement(_name,_length,_type), strength(_strength) {}
};




// two magnet classes:
//  PlaneMagnet - homogeneous field in one plane e.g. dipole,corrector (occur as horizontal,vertical,longitudinal)
//  FamilyMagnet - field in multiple plane e.g. quad, sext (occur as focus and defocus)

class PlaneMagnet : public Magnet {
protected:
  const element_plane plane;

  PlaneMagnet(string _name, double _length,element_type _type, element_plane _plane, double _strength=0.)
    : Magnet(_name,_length,_type,_strength), plane(_plane) {}

};

class FamilyMagnet : public Magnet {
protected:
  const element_family family;

  FamilyMagnet(string _name, double _length,element_type _type, element_family _family, double _strength=0.)
    : Magnet(_name,_length,_type,_strength), family(_family) {}

};






//-------- definition of magnet types -----------

class Dipole : public PlaneMagnet {
public:
  Dipole(string _name, double _length, double _strength=0., element_plane _plane=V)
    : PlaneMagnet(_name,_length,dipole,_plane,_strength) {}
  ~Dipole() {}

  virtual Dipole* clone() const {return new Dipole(*this);}

  virtual AccTriple B() const {AccTriple tmp; if(plane==V) tmp.z=strength; else if(plane==H) tmp.x=strength; return tmp;}
  virtual AccTriple B(AccPair orbit) const {return B();}

};



class Quadrupole : public FamilyMagnet {
public:
  Quadrupole(string _name, double _length, double _strength=0., element_family _family=F)
    : FamilyMagnet(_name,_length,quadrupole,_family,_strength) {}
  ~Quadrupole() {}

  virtual Quadrupole* clone() const {return new Quadrupole(*this);}

  virtual AccTriple B(AccPair orbit) const {AccTriple tmp; tmp.x=strength*orbit.z; tmp.z=strength*orbit.x; if(family==D) tmp*=(-1.); return tmp;}

};


class Sextupole : public FamilyMagnet {
public:
  Sextupole(string _name, double _length, double _strength=0., element_family _family=F)
    : FamilyMagnet(_name,_length,sextupole,_family,_strength) {}
  ~Sextupole() {}

  virtual Sextupole* clone() const {return new Sextupole(*this);}

  virtual AccTriple B(AccPair orbit) const {AccTriple tmp; tmp.x=strength*orbit.x*orbit.z; tmp.z=0.5*strength*(pow(orbit.x,2)-pow(orbit.z,2)); if(family==D) tmp*=(-1); return tmp;}

};



#endif
/*__ACCELEMENTS_HPP_*/
