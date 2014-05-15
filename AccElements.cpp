/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
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




// true if element name matches entry in List (can include 1 wildcard *)
bool AccElement::nameMatch(vector<string> &nameList) const
{
  for (unsigned int i=0; i<nameList.size(); i++) {
    if (nameMatch(nameList[i]))
      return true;
  }
  return false;
}


// true if element name matches pattern (can include 1 wildcard *)
bool AccElement::nameMatch(string &pattern) const
{

  if (this->name.size() < pattern.size()-1)
    return false;

  int wildcardPos = pattern.find("*");

  if (wildcardPos != string::npos) { // if wildcard * occurs in pattern
    if (this->name.substr(0,wildcardPos) != pattern.substr(0,wildcardPos)) {//match before wildcard
      return false;
    }
    string afterString = pattern.substr(wildcardPos+1);
    if (afterString.size() == 0) {
      return true;
    }
    int afterPos = this->name.find( afterString ); //first pos after wildcard in name
    if (afterPos == string::npos) { //afterString not found in name
      return false;
    }
    else if (afterString.size() < this->name.substr(afterPos).size()) {
      return false;
    }
  }
  else {
    if (this->name != pattern)       // if no wildcard occurs
      return false;
  }

  return true; 
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
    << "Strength*" <<std::setw(w)<< "Rotation(s)/rad" << endl;

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
   return tmp.tilt(this->dpsi);
 }


// Corrector
AccTriple Corrector::B() const
 {
   AccTriple tmp; 
   if(plane==V) tmp.z=strength;
   else if(plane==H) tmp.x=strength;
   return tmp.tilt(this->dpsi);
 }



//misalignment: tilt around s-Axis:
// if B depends on orbit, two steps are required:
// 1. rotate orbit to system of magnet (-dpsi)
// 2. rotate calculated field back to lab frame (+dpsi)

// Quadrupole
AccTriple Quadrupole::B(AccPair orbit) const
 {
   AccTriple tmp; 
   AccPair rotOrbit = orbit.tilt(- this->dpsi);

   tmp.x=strength*rotOrbit.z;
   tmp.z=strength*rotOrbit.x;
   tmp.s=0;
   
   if(family==D) tmp*=(-1.);
   
   return tmp.tilt(this->dpsi);
 }


// Sextupole
AccTriple Sextupole::B(AccPair orbit) const
 {
   AccTriple tmp;
   AccPair rotOrbit = orbit.tilt(- this->dpsi);
   
   tmp.x=strength*rotOrbit.x*rotOrbit.z;
   tmp.z=0.5*strength*(pow(rotOrbit.x,2)-pow(rotOrbit.z,2));
   tmp.s=0;
   
   if(family==D) tmp*=(-1.);
   
   return tmp.tilt(this->dpsi);
 }
