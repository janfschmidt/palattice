/* AccElements Classes
 * Elements of an Accelerator (e.g. magnets) used by the "AccLattice" Container.
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


#ifndef __LIBPALATTICE_ACCELEMENTS_HPP_
#define __LIBPALATTICE_ACCELEMENTS_HPP_

#include <iostream>
#include <string>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "SimTools.hpp"
#include "types.hpp"
#include "config.hpp"
#include <gsl/gsl_const_mksa.h>

using namespace std;

namespace pal
{

  enum element_type{dipole=0, quadrupole=1, corrector=2, sextupole=3, cavity=4, multipole=5, marker=6, monitor=7, rcollimator=8, solenoid=9, drift=10}; //! keep dipole as first, drift as last
  enum element_plane{H,V,noplane};    //horizontal,vertical
                                        //used for export and filtering only, NO INFLUENCE ON FIELD B()!
  enum element_family{F,D,nofamily};    //focus,defocus, CHANGES SIGN OF FIELD!

  template <typename T=std::string> T simToolConditional(T madx, T elegant, SimTool t);
  string filterCharactersForLaTeX(string in);
  string type_string(element_type t);
  string type_string(element_type t, SimTool tool);

// abstract base class
class AccElement {
protected:
  static AccPair zeroPair;
  static AccTriple zeroTriple;
  double physLength;      // physical length (used for edge field calculation (pal::AccLattice::B()) / m

  // following data can be accessed and modified. Only type and length of an element must not be changed.
public:
  const element_type type;
  string name;
  const double length;    // effective length (field length) / m
  element_plane plane;
  element_family family;
  AccTriple k0;          // magnet strength. 1/R / m^-1 for each axis (x,z,s).
                         // direction: e.g. k0.x corresponds to B.x and thus causes a vertical kick
  double k1;             // magnet strength. k / m^-2
  double k2;             // magnet strength. m / m^-3
  double Qrf1;           // RF magnet tune for turn=1
  double dQrf;           // RF magnet tune change per turn (linear frequency sweep)

  //edge angles / rad
  //used only for import/export and integrated field B_int(orbit)
  double e1;
  double e2;
  
  //alignment errors:
  double tilt;           //rotation around s axis / rad
  AccPair displacement;  //transverse displacement in x and z direction / m

  //rectangular aperture, only used for rcollimator!
  AccPair halfWidth;    //half width of the beam pipe / m

  // cavity stuff
  double volt;          // RF amplitude / V
  double freq;          // RF frequency / Hz

  AccElement(element_type _type, string _name, double _length);
  virtual ~AccElement() {};
  virtual AccElement& operator=(const AccElement* other);
  virtual bool operator==(const AccElement &other) const;
  virtual bool operator!=(const AccElement &other) const;
  virtual AccElement* clone() const =0;

  // physical length (used for edge field calculation (pal::AccLattice::B()) / m
  void setPhysLength(double pl) {physLength = pl; this->checkPhysLength();}
  void setPhysLength() {physLength = 0.; this->checkPhysLength();}
  double getPhysLength() const {return physLength;}
  double dl() const {return fabs(length - getPhysLength());}  // difference of (effective) length and physical length / m

  // magnetic field
  virtual AccTriple B() const =0;
  virtual AccTriple B(const AccPair &orbit) const =0;

  // integral magnetic field
  virtual AccTriple B_int() const {return B() * length;}
  virtual AccTriple B_int(const AccPair &orbit) const {return B(orbit) * (length + orbit.x*(tan(e1)+tan(e2)) );} //edges: length depends on x
  //RF magnets (oscillating fields)
  double rfFactor(unsigned int turn) const; // Magnetic field amplitude factor for oscillating fields
  AccTriple B_rf(unsigned int turn) const {return B() * rfFactor(turn);}
  AccTriple B_rf(unsigned int turn, const AccPair &orbit) const {return B(orbit) * rfFactor(turn);}

  // get magnetic field kick angle
  AccTriple kick_mrad() const {return B() * 1000 * length;}
  AccTriple kick_mrad(const AccPair &orbit) const {return B(orbit) * 1000 * length;}
  // set magnetic field by kick angle (1/R from kick angle, straight length l)
  void hkick_mrad(double kick) {k0.z = std::sin(kick) / length;}
  void vkick_mrad(double kick) {k0.x = std::sin(kick) / length;}

  // synchrotron radiation
  //critical photon energy (in 3 units) at electron beam energy given as gamma:
  virtual double syli_Ecrit_Joule(const double& ) const {return 0;}
  virtual double syli_Ecrit_keV(const double& ) const {return 0;}
  virtual double syli_Ecrit_gamma(const double& ) const {return 0;}
  //mean number of photons emmited in this magnet by electron beam with energy given as gamma:
  virtual double syli_meanPhotons(const double& ) const {return 0;}

  // compare element name with pattern (can include 1 wildcard *)
  bool nameMatch(const vector<string> &nameList) const;  // true if matches any entry in list
  bool nameMatch(const string &pattern) const;                 // true if matches pattern 

  // string output of element type name (in simtool if given)
  string type_string() const {return pal::type_string(type);}
  string type_string(SimTool t) const {return pal::type_string(type, t);}

  string print() const;          // string output of (some) element properties
  string printHeader() const;    // string output of header-line(s) for print()
  virtual string printSimTool(SimTool t) const =0;  // string output of element definition in elegant or madx format
  virtual string printLaTeX() const =0;  // string output of element definition in LaTeX format (tikz-palattice package)

  
protected:
  void checkPhysLength(); // check for valid value and (re-)calculate physLength from default (config.hpp)
  string printNameType(SimTool t) const {return name + " : " + type_string(t);}
  string printTilt(SimTool t) const;
  string printDisplace() const;
  string printEdges() const;
  string printStrength() const;
  string printAperture(SimTool t) const;
  string printRF(SimTool t) const;
  string printSyli(SimTool t) const;
  string printNKicks(SimTool t) const;
  string rfMagComment() const;
};


  // no magnet, B-Field=0 (abstract)
  class NoMagnet : public AccElement {
  public:
    NoMagnet(element_type _type, string _name, double _length)
      : AccElement(_type,_name,_length) {}
    
    virtual NoMagnet* clone() const =0;
    
    virtual AccTriple B() const {return zeroTriple;}
    virtual AccTriple B(const AccPair&) const {return B();}
    virtual string printSimTool(SimTool t) const;
    virtual string printLaTeX() const =0;
  };
  
  
// drift
class Drift : public NoMagnet {
public:
  Drift(string _name="drift", double _length=0.)
    : NoMagnet(drift,_name,_length) {}
  ~Drift() {}
  
  virtual Drift* clone() const {return new Drift(*this);}

  virtual string printLaTeX() const;
};

  // marker
  class Marker : public NoMagnet {
  public:
    Marker(string _name)
      : NoMagnet(marker,_name,0.0) {}
    ~Marker() {}
    
    virtual Marker* clone() const {return new Marker(*this);}

    virtual string printSimTool(SimTool t) const;
    virtual string printLaTeX() const;
  };

   // monitor
  class Monitor : public NoMagnet {
  public:
    Monitor(string _name, double _length)
      : NoMagnet(monitor,_name,_length) {}
    ~Monitor() {}
    
    virtual Monitor* clone() const {return new Monitor(*this);}

    virtual string printSimTool(SimTool t) const;
    virtual string printLaTeX() const;
  };


// cavity (E-Field not implemented)
class Cavity : public NoMagnet {
public:
  Cavity(string _name, double _length)
    : NoMagnet(cavity,_name,_length) {}
  ~Cavity() {}

  virtual Cavity* clone() const {return new Cavity(*this);}
  
  virtual string printSimTool(SimTool t) const;
  virtual string printLaTeX() const;
};

  
  // rectangular collimator
  class Rcollimator : public NoMagnet {
  public:
    Rcollimator(string _name, double _length)
      : NoMagnet(rcollimator,_name,_length) {}
    ~Rcollimator() {}

    virtual Rcollimator* clone() const {return new Rcollimator(*this);}
  
    virtual string printSimTool(SimTool t) const;
    virtual string printLaTeX() const;
  };


  // magnet (abstract, due to export)
  class Magnet : public AccElement {
  protected:
    const bool rf;
  public:
    Magnet(element_type _type, string _name, double _length, bool _rf=false)
      : AccElement(_type,_name,_length), rf(_rf) {}

    virtual Magnet* clone() const =0;

    virtual AccTriple B() const;
    virtual AccTriple B(const AccPair &orbit) const;
    virtual double syli_Ecrit_Joule(const double& gamma) const;
    virtual double syli_Ecrit_keV(const double& gamma) const;
    virtual double syli_Ecrit_gamma(const double& gamma) const;
    virtual double syli_meanPhotons(const double& gamma) const;
    
    virtual string printSimTool(SimTool t) const =0;
    virtual string printLaTeX() const =0;
  };


  // multipole. forbids B() without orbit argument.
  class Multipole : public Magnet {
  protected:
    Multipole(element_type _type, string _name, double _length)
      : Magnet(_type,_name,_length) {}
  public:
    Multipole(string _name, double _length, element_family _family=F)
      : Magnet(multipole,_name,_length) {family=_family;}

    virtual Multipole* clone() const {return new Multipole(*this);}

    virtual AccTriple B() const;
    virtual string printSimTool(SimTool t) const;
    virtual string printLaTeX() const;
  };




//-------- magnet types -----------

class Dipole : public Magnet {
public:
  Dipole(string _name, double _length, element_plane p=H, double _k0=0.);
  Dipole(string _name, double _length, AccTriple _k0);
  ~Dipole() {}

  virtual Dipole* clone() const {return new Dipole(*this);}

  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};

class Corrector : public Magnet {
public:
  Corrector(string _name, double _length, element_plane p=noplane, double _k0=0.);
  Corrector(string _name, double _length, AccTriple _k0);
  ~Corrector() {}

  virtual Corrector* clone() const {return new Corrector(*this);}

  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};

class Solenoid : public Magnet {
public:
  Solenoid(string _name, double _length, double _k0=0.)
    : Magnet(solenoid,_name,_length) {k0.s = _k0;}
  ~Solenoid() {}

  virtual Solenoid* clone() const {return new Solenoid(*this);}

  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};


// ====== ATTENTION ====================================================================
// Change of "element_family" (F <-> D) changes sign of magnetic field (k1,k2) !
// So either set "element_family" and use absolute values for "k"
// or always use default "element_family" (F) and set negative "k" for D magnets.
// =====================================================================================

class Quadrupole : public Multipole {
public:
  Quadrupole(string _name, double _length, element_family _family=F, double _k1=0.)
    : Multipole(quadrupole,_name,_length) {family=_family; k1=_k1;}
  ~Quadrupole() {}

  virtual Quadrupole* clone() const {return new Quadrupole(*this);}

  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};

class Sextupole : public Multipole {
public:
  Sextupole(string _name, double _length, element_family _family=F, double _k2=0.)
    : Multipole(sextupole,_name,_length) {family=_family; k2=_k2;}
  ~Sextupole() {}

  virtual Sextupole* clone() const {return new Sextupole(*this);}

  string printSimTool(SimTool t) const;
  string printLaTeX() const;
};



string getLaTeXDrift(double driftlength); // Drift element for LaTeX (used by Drift::printLaTeX and AccLattice::latexexport)

} //namespace pal



// function template implementation
template <typename T>
T pal::simToolConditional(T madx, T elegant, pal::SimTool t)
{
  switch(t) {
  case pal::madx:
    return madx;
  case pal::elegant:
    return elegant;
  default:
    throw palatticeError("pal::simToolConditional(): unknown SimTool");
  }
}


#endif
/*__LIBPALATTICE_ACCELEMENTS_HPP_*/
