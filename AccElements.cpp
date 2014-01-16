/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <sstream>
#include <string>
#include <iomanip>
#include "AccElements.hpp"

// static member definition
AccPair AccElement::zeroPair;
AccTriple AccElement::zeroTriple;



AccElement& AccElement::operator=(const AccElement* other)
{
  stringstream msg;

  if (type != other->type) {
    msg << "ERROR: AccElement::operator=(): Cannot assign Element of different type ("
	<< type_string() <<"/"<< other->type_string() <<")";
    throw logic_error(msg.str());
  }

   this->name = other->name;
   this->length = other->length;
   this->dpsi = other->dpsi;
   this->strength = other->strength;

   return *this;
}


string AccElement::type_string() const
{
  switch (type) {
  case dipole:
    return "Dipole";
  case quadrupole:
    return "Quadrupole";
  case corrector:
    return "Corrector";
  case sextupole:
    return "Sextupole";
  case drift:
    return "Drift";
  }

  return "Please implement this type in AccElement::type_string()!";
}




// apply misalignments (e.g. for B) to initial AccTriple i
AccTriple AccElement::misalign(AccTriple i) const
{
  AccTriple f; //final

  // rotation around s (longitudinal axis) -> dpsi
  f.x = cos(dpsi)*i.x - sin(dpsi)*i.z;
  f.z = sin(dpsi)*i.x + cos(dpsi)*i.z;
  f.s = i.s;

  //implement other misalignments here, take care of commutation!

  return f;
}




// string output of (some) element properties
// ! if you change this function, !
// ! also modify printHeader()    !
string AccElement::print() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< this->name <<std::setw(w)<< this->type_string() <<std::setw(w)
    << this->length <<std::setw(w)<< this->strength <<std::setw(w)<< this->dpsi << endl;

  return s.str();
}

// string output of header-line(s) for print()
string AccElement::printHeader() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< "Name" <<std::setw(w)<< "Type" <<std::setw(w)<< "Length/m" <<std::setw(w)
    << "Strength/m^-1" <<std::setw(w)<< "Rotation(s)/rad" << endl;

  return s.str();
}





// ========= magnetic field calculation for all magnet types =================

// default B(), throw exception if orbit is needed as input
AccTriple AccElement::B() const
{
  string msg="B-Field of "+type_string()+" depends on orbit! Please provide argument.";
  throw std::invalid_argument(msg);
}
//default B(orbit), returns B=0
AccTriple AccElement::B(AccPair orbit) const
{
  return zeroTriple;
}


// Drift: B=0
AccTriple Drift::B() const
{
  return zeroTriple;
}


// Dipole
AccTriple Dipole::B() const
 {
   AccTriple tmp; 
   if(plane==V) tmp.z=strength;
   else if(plane==H) tmp.x=strength;
   return misalign(tmp);
 }


// Corrector
AccTriple Corrector::B() const
 {
   AccTriple tmp; 
   if(plane==V) tmp.z=strength;
   else if(plane==H) tmp.x=strength;
   return misalign(tmp);
 }


// Quadrupole
AccTriple Quadrupole::B(AccPair orbit) const
 {
   AccTriple tmp; 
   
   tmp.x=strength*orbit.z;
   //tmp.z=strength*orbit.x;
   tmp.z=0;
   
   if(family==D) tmp*=(-1.);
   
   return misalign(tmp);
 }


// Sextupole
AccTriple Sextupole::B(AccPair orbit) const
 {
   AccTriple tmp; 
   
   tmp.x=strength*orbit.x*orbit.z;
   //tmp.z=0.5*strength*(pow(orbit.x,2)-pow(orbit.z,2));
   tmp.z=0;
   
   if(family==D) tmp*=(-1.);
   
   return misalign(tmp);
 }
