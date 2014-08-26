/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#ifndef __LIBPAL_ACCELEMENTS_HPP_
#define __LIBPAL_ACCELEMENTS_HPP_

#include <iostream>
#include <string>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "SimTools.hpp"
#include "types.hpp"

using namespace std;

namespace pal
{

enum element_type{dipole, quadrupole, corrector, rfdipole, sextupole, cavity, drift}; //! keep dipole as first and drift as last
enum element_plane{H,V,L,noplane};    //horizontal,vertical,longitudinal
enum element_family{F,D,nofamily};     //focus,defocus

// abstract base class
class AccElement {
protected:
  static AccPair zeroPair;
  static AccTriple zeroTriple;

  // data entries are allowed to be public: AccLattice class uses const AccElement!
  // (everybody is allowed to modify an Element which is not mounted in a Lattice)
public:
  string name;
  double length;
  const element_type type;
  const element_plane plane;
  const element_family family;
  double strength;
  double Qrf0;
  double dQrf;
  
  //alignment errors:
  double dpsi; //rotation around s axis in rad


  AccElement(string _name, double _length,element_type _type, double _strength=0., element_plane _plane=noplane, element_family _family=nofamily)
    : name(_name),length(_length),type(_type),plane(_plane),family(_family),strength(_strength),Qrf0(0.),dQrf(0.),dpsi(0.) {}
  virtual ~AccElement() {};
  virtual AccElement& operator=(const AccElement* other);

  virtual AccElement* clone() const =0;

  // magnetic field with all arguments, has to be implemented in each derived class
  virtual AccTriple B(AccPair orbit, unsigned int turn) const =0;

  // magnetic field defaults with less arguments. throws exception.
  // => implement the "wanted" cases in derived classes
  virtual AccTriple B() const;
  virtual AccTriple B(AccPair orbit) const;
  virtual AccTriple B(unsigned int turn) const;

  double hKick_mrad() const {return 1000 * B().x * length;}
  double hKick_mrad(AccPair orbit) const {return 1000 * B(orbit).x * length;}

  bool nameMatch(vector<string> &nameList) const; // true if element name matches entry in List (can include 1 wildcard *)
  bool nameMatch(string &pattern) const;           // true if element name matches pattern (can include 1 wildcard *)

  string type_string() const;    // string output of element type

  string print() const;          // string output of (some) element properties
  virtual string printSimTool(SimTool t) const =0;  // string output of element definition in elegant or madx format
  virtual string printLaTeX() const =0;  // string output of element definition in LaTeX format (using lattice package by Jan Schmidt <schmidt@physik.uni-bonn.de>)
  string printHeader() const;    // string output of header-line(s) for print()

};



// drift
class Drift : public AccElement {
public:
  Drift(string _name="nothing", double _length=0.)
    : AccElement(_name,_length,drift) {}
  ~Drift() {}
  
  virtual Drift* clone() const {return new Drift(*this);}

  virtual AccTriple B() const;
  virtual AccTriple B(AccPair orbit) const {return B();}
  virtual AccTriple B(unsigned int turn) const {return B();}
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B();}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};



// cavity (no B-field)
class Cavity : public AccElement {
public:
  Cavity(string _name, double _length=0.)
    : AccElement(_name,_length,cavity) {}
  ~Cavity() {}

  virtual Cavity* clone() const {return new Cavity(*this);}
  
  virtual AccTriple B() const;
  virtual AccTriple B(AccPair orbit) const {return B();}
  virtual AccTriple B(unsigned int turn) const {return B();}
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B();}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};



// two abstract magnet classes:
//  PlaneMagnet - homogeneous field in one plane e.g. dipole,corrector (occur as horizontal,vertical,longitudinal)
//     -> e.g. H means a horizontally bending magnet (horizontal kick, VERTICAL magnetic field)
//  FamilyMagnet - field in multiple plane e.g. quad, sext (occur as focus and defocus)

class PlaneMagnet : public AccElement {
public:
  PlaneMagnet(string _name, double _length,element_type _type, element_plane _plane, double _strength=0.)
    : AccElement(_name,_length,_type,_strength, _plane, nofamily) {}
};

class FamilyMagnet : public AccElement {
public:
  FamilyMagnet(string _name, double _length,element_type _type, element_family _family, double _strength=0.)
    : AccElement(_name,_length,_type,_strength, noplane, _family) {}
};







//-------- definition of magnet types -----------

class Dipole : public PlaneMagnet {
public:
  Dipole(string _name, double _length, double _strength=0., element_plane _plane=H)
    : PlaneMagnet(_name,_length,dipole,_plane,_strength) {}
  ~Dipole() {}

  virtual Dipole* clone() const {return new Dipole(*this);}

  virtual AccTriple B() const;
  virtual AccTriple B(AccPair orbit) const {return B();}
  virtual AccTriple B(unsigned int turn) const {return B();}
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B();}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};


class Corrector : public PlaneMagnet {
public:
  Corrector(string _name, double _length, double _strength=0., element_plane _plane=H)
    : PlaneMagnet(_name,_length,corrector,_plane,_strength) {}
  ~Corrector() {}

  virtual Corrector* clone() const {return new Corrector(*this);}

  virtual AccTriple B() const;
  virtual AccTriple B(AccPair orbit) const {return B();}
  virtual AccTriple B(unsigned int turn) const {return B();}
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B();}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};


class RFdipole : public PlaneMagnet {
public:
  RFdipole(string _name, double _length, double _strength=0., element_plane _plane=H)
    : PlaneMagnet(_name,_length,rfdipole,_plane,_strength) {}
  ~RFdipole() {}

  virtual RFdipole* clone() const {return new RFdipole(*this);}

  virtual AccTriple B(unsigned int turn) const;
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B(turn);}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};



// ====== ATTENTION ====================================================================
// Change of "element_family" (F <-> D) changes sign of "B()" !
// So either set "element_family" and use absolute values for "strength"
// or always use default "element_family" (F) and set negative "strength" for D magnets.
// =====================================================================================

class Quadrupole : public FamilyMagnet {
public:
  Quadrupole(string _name, double _length, double _strength=0., element_family _family=F)
    : FamilyMagnet(_name,_length,quadrupole,_family,_strength) {}
  ~Quadrupole() {}

  virtual Quadrupole* clone() const {return new Quadrupole(*this);}

  virtual AccTriple B(AccPair orbit) const;
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B(orbit);}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};


class Sextupole : public FamilyMagnet {
public:
  Sextupole(string _name, double _length, double _strength=0., element_family _family=F)
    : FamilyMagnet(_name,_length,sextupole,_family,_strength) {}
  ~Sextupole() {}

  virtual Sextupole* clone() const {return new Sextupole(*this);}

  virtual AccTriple B(AccPair orbit) const;
  virtual AccTriple B(AccPair orbit, unsigned int turn) const {return B(orbit);}
  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};


string getLaTeXDrift(double driftlength); // Drift element for LaTeX (used by Drift::printLaTeX and AccLattice::latexexport)

} //namespace pal

#endif
/*__LIBPAL_ACCELEMENTS_HPP_*/
