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

  unsigned int wildcardPos = pattern.find("*");

  if (wildcardPos != string::npos) { // if wildcard * occurs in pattern
    if (this->name.substr(0,wildcardPos) != pattern.substr(0,wildcardPos)) {//match before wildcard
      return false;
    }
    string afterString = pattern.substr(wildcardPos+1);
    if (afterString.size() == 0) {
      return true;
    }
    unsigned int afterPos = this->name.find( afterString ); //first pos after wildcard in name
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
  case rfdipole:
    return "RF-Dipole";
  case sextupole:
    return "Sextupole";
  case cavity:
    return "Cavity";
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

// default B-Field functions, throw exceptions
AccTriple AccElement::B() const
{
  string msg="B-Field of "+type_string()+" may depend on orbit or turn! Please provide argument.";
  throw std::invalid_argument(msg);
}
AccTriple AccElement::B(AccPair orbit) const
{
  string msg="B-Field of "+type_string()+" may depend on turn! Please provide argument.";
  throw std::invalid_argument(msg);
}
AccTriple AccElement::B(unsigned int turn) const
{
  string msg="B-Field of "+type_string()+" may depend on orbit! Please provide argument.";
  throw std::invalid_argument(msg);
}
// AccTriple AccElement::B(AccPair orbit, unsigned int turn) const
// {
//   string msg="B-Field of "+type_string()+" not implemented.";
//   throw std::invalid_argument(msg);
// }



// Drift: B=0
AccTriple Drift::B() const
{
  return zeroTriple;
}

// Cavity: B=0
AccTriple Cavity::B() const
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


// RFdipole
AccTriple RFdipole::B(unsigned int turn) const
 {
   AccTriple tmp;
   double phi = turn*Qrf0 + turn*(turn+1)/2 * dQrf;
   double rfStrength = strength * cos(2*M_PI*phi);
   if(plane==V) tmp.z = rfStrength;
   else if(plane==H) tmp.x = rfStrength;
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




// ============ printSimTool (elegant or madx) ======================
string Drift::printSimTool(simulationTool t) const
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "DRIF";
  else
    nameInTool = "DRIFT";

  s << name <<" : "<< nameInTool <<", "
    <<"L="<< length <<";"<<endl;
  return s.str();
}

string Cavity::printSimTool(simulationTool t) const
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "RFCA";
  else
    nameInTool = "RFCAVITY";

  s << name <<" : "<< nameInTool <<", "
    <<"L="<< length <<", "
    <<"VOLT=0.0";
  if (t==madx)
    s << ", HARMON=0;";
  s << " ! Voltage and frequency not implemented in pole/Bsupply. Please set manually." <<endl;
  return s.str();
}


string Dipole::printSimTool(simulationTool t) const
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "CSBEND";
  else
    nameInTool = "SBEND";

  s << name <<" : "<< nameInTool <<", "
    <<"L="<< length <<", "
    <<"ANGLE="<< strength*length;
  if (t == elegant) {
    s <<", e1=0, e2=0, "   //not implemented
      <<"synch_rad=1, isr=1, use_rad_dist=0";
  }
  s <<";"<< endl;
  return s.str();
}

string Corrector::printSimTool(simulationTool t) const
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "KICK";
  else
    nameInTool = "KICKER";
 
  if (plane == H)
    s << name <<" : V"<< nameInTool <<", "; //! plane==H => horizontal field => vertical kick
  else if (plane == V)
    s << name <<" : H"<< nameInTool <<", ";
  s <<"L="<< length <<", "
    <<"KICK="<< strength
    <<";"<< endl;
  return s.str();
}

string RFdipole::printSimTool(simulationTool t) const // same as corrector
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "KICK";
  else
    nameInTool = "KICKER";
 
  if (plane == H)
    s << name <<" : V"<< nameInTool <<", "; //! plane==H => horizontal field => vertical kick
  else if (plane == V)
    s << name <<" : H"<< nameInTool <<", ";
  s <<"L="<< length <<", "
    <<"KICK="<< strength
    <<"; ! This is a RFDipole in pole/Bsupply"<< endl;
  return s.str();
}

string Quadrupole::printSimTool(simulationTool t) const
{
  stringstream s;
  string nameInTool;
  if (t == elegant)
    nameInTool = "QUAD";
  else
    nameInTool = "QUADRUPOLE";

  s << name <<" : "<< nameInTool <<", "
    <<"L="<< length <<", ";
  if (family == F)
    s  <<"K1="<< strength;
  else if (family == D)
    s  <<"K1="<< -strength;
  if (t == elegant) {
    s <<", fringe_type=fixed-strength, ffringe=0";
  }
  s <<";"<< endl;
  return s.str();
}

string Sextupole::printSimTool(simulationTool t) const
{
 stringstream s;
 string nameInTool;
  if (t == elegant)
    nameInTool = "SEXT";
  else
    nameInTool = "SEXTUPOLE";

  s << name <<" : "<< nameInTool <<", "
    <<"L="<< length <<", ";
  if (family == F)
    s  <<"K2="<< strength;
  else if (family == D)
    s  <<"K2="<< -strength;
  s <<";"<< endl;
  return s.str();
}




// ============ printLaTeX ==========================
string Drift::printLaTeX() const
{
  return getLaTeXDrift(length);
}

string Cavity::printLaTeX() const
{
  stringstream s;
  s << "\\cavity{"<< name <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Dipole::printLaTeX() const
{
  stringstream s;
  s << "\\dipole{"<< name <<"}{"<< length <<"}{"<< strength*length*180/M_PI <<"}" << endl;
  return s.str();
}

string Corrector::printLaTeX() const
{
  stringstream s;
  s << "\\kicker{"<< name <<"}{"<< length <<"}" << endl;
  return s.str();
}

string RFdipole::printLaTeX() const // same as corrector
{
 stringstream s;
  s << "\\kicker{"<< name <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Quadrupole::printLaTeX() const
{
  stringstream s;
  s << "\\quadrupole{"<< name <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Sextupole::printLaTeX() const
{
 stringstream s;
  s << "\\sextupole{"<< name <<"}{"<< length <<"}" << endl;
  return s.str();
}



// Drift element for LaTeX (used by Drift::printLaTeX and AccLattice::latexexport)
//maximum drift length 2.9m in lattice package -> split longer drifts
string getLaTeXDrift(double driftlength)
{
  if (fabs(driftlength) < 1e-6)
    return "";

  std::stringstream s;
  int factor = driftlength/2.9 +1;
    if (factor > 1) { 
      driftlength /= factor;
      for (int i=0; i<factor; i++)
	s << "\\drift{" << driftlength << "}"<< endl;
    }
    else {
      s << "\\drift{" << driftlength << "}"<< endl; //drift
    }
    return s.str();
}
