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


using namespace pal;

// static member definition
AccPair AccElement::zeroPair;
AccTriple AccElement::zeroTriple;

AccElement::AccElement(element_type _type, string _name, double _length)
  : type(_type),name(_name),length(_length),plane(noplane),family(nofamily)
{
  physLength = k0 = k1 = k2 = Qrf1 = dQrf = dpsi = 0;

  if (length < 0.) {
    stringstream msg;
    msg << "invalid length "<<length<<"<0 for AccElement " << name;
    throw libpalError(msg.str());
  }
  else if (length > 0.)
    this->checkPhysLength();
}



void AccElement::checkPhysLength()
{
  if (physLength==0.) {
    double pl = length - DEFAULT_LENGTH_DIFFERENCE; //calc. physLength from default setting (config.hpp)
    if (pl > 0)
      physLength = pl;
    else {
      cout <<"WARNING: DEFAULT_LENGTH_DIFFERENCE (config.hpp) > length for AccElement "<< this->name
	   << ". physical length set to zero." << endl;
      physLength = 0.;
    }
  }
  else if (physLength < 0.) {
    cout << "WARNING: AccElement::checkPhysLength():"<<endl
	 << "physical length was set to negative value "<<physLength<<" for " <<this->name
	 << ". sign changed." << endl;
    physLength *= -1;
  }
}


AccElement& AccElement::operator=(const AccElement* other)
{
  stringstream msg;

  if (type != other->type) {
    msg << "ERROR: AccElement::operator=(): Cannot assign Element of different type ("
	<< type_string() <<"/"<< other->type_string() <<")";
    throw libpalError(msg.str());
  }
  if (abs(length-other->length) > ZERO_DISTANCE) {
    msg << "ERROR: AccElement::operator=(): Cannot assign Element of different length ("
	<< length <<"/"<< other->length <<")";
    throw libpalError(msg.str());
  }

   this->name = other->name;
   this->plane = other->plane;
   this->family = other->family;
   this->dpsi = other->dpsi;
   this->k0 = other->k0;
   this->k1 = other->k1;
   this->k2 = other->k2;
   this->Qrf1 = other->Qrf1;
   this->dQrf = other->dQrf;

   return *this;
}


// Magnetic field amplitude factor for oscillating fields
// B(orbit,turn) = B(orbit) * rfFactor(turn)
// ! fixed starting phase
double AccElement::rfFactor(unsigned int turn) const
{
  if(Qrf1==0. && dQrf==0.)
    return 1.;

  double phi = turn*Qrf1 + turn*(turn+1)/2 * dQrf;
  return cos(2*M_PI*phi);
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

  s <<std::setw(w)<< name <<std::setw(w)<< type_string() <<std::setw(w) << length
    <<std::setw(w)<< k0 <<std::setw(w)<< k1 <<std::setw(w)<< k2 <<std::setw(w)<< dpsi << endl;

  return s.str();
}

// string output of header-line(s) for print()
string AccElement::printHeader() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< "Name" <<std::setw(w)<< "Type" <<std::setw(w)<< "Length/m"
    <<std::setw(w)<< "k0 / 1/m" <<std::setw(w)<< "k1 / 1/m^2" <<std::setw(w)<< "k2 / 1/m^3" 
    <<std::setw(w)<< "Rotation(s)/rad" << endl;

  return s.str();
}





// ========= magnetic field calculation =================

// Magnet
AccTriple Magnet::B() const
{
  AccTriple tmp; 
  if(plane==H) tmp.z=k0;       // H means a horizontally bending magnet (horizontal kick, VERTICAL magnetic field)
  else if(plane==V) tmp.x=k0;
  else if(plane==L) tmp.s=k0;
  else if(plane==noplane && k0!=0.) throw libpalError("Magnet::B(): k0 given, but plane (H,V,L) not specified");
  return tmp.tilt(dpsi);
}

AccTriple Magnet::B(const AccPair &orbit) const
{
  AccTriple tmp;
  //misalignment: tilt around s-Axis:
  // if B depends on orbit, two steps are required:
  // 1. rotate orbit to system of magnet (-dpsi)
  // 2. rotate calculated field back to lab frame (+dpsi)
  AccPair rotOrbit = orbit.tilt(- dpsi);
  
  //quadrupole & sextupole field (k1 & k2)
  tmp.x= k1*rotOrbit.z + k2*rotOrbit.x*rotOrbit.z;
  tmp.z= k1*rotOrbit.x + 0.5*k2*(rotOrbit.x*rotOrbit.x-rotOrbit.z*rotOrbit.z);
  tmp.s=0;
  
  if(family==D) tmp*=(-1.); //sign of k1,k2

  //dipole field (k0)
  if(k0!=0.) {
    if(plane==H) tmp.z+=k0;
    else if(plane==V) tmp.x+=k0;
    else if(plane==L) tmp.s+=k0;
    else throw libpalError("Magnet::B(orbit): k0 given, but plane (H,V,L) not specified");
  }
  
  return tmp.tilt(this->dpsi);
}




// Multipole
AccTriple Multipole::B() const
{
  string msg="Multipole::B(): B-Field of "+type_string()+" depends on orbit! Please provide argument.";
  throw libpalError(msg);
}


 




// ============ printSimTool (elegant or madx) ======================
string AccElement::nameInTool(string name_madx, string name_elegant, SimTool t) const
{
  switch(t) {
  case madx:
    return name_madx;
  case elegant:
    return name_elegant;
  default:
    throw libpalError("AccElement::nameInTool(): unknown SimTool");
  }
  return "unknown SimTool";
}

string AccElement::rfComment() const
{
  stringstream s;
  if (Qrf1!=0. || dQrf!=0.)
    s << "! RF magnet in libpal (Qrf1=" << Qrf1 << ", dQrf=" << dQrf << ")";

  return s.str();
}

string Drift::printSimTool(SimTool t) const
{
  stringstream s;

  s << name <<" : "<< nameInTool("DRIFT","DRIF",t) <<", "
    <<"L="<< length <<";"<<endl;
  return s.str();
}

string Cavity::printSimTool(SimTool t) const
{
  stringstream s;

  s << name <<" : "<< nameInTool("RFCAVITY","RFCA",t) <<", "
    <<"L="<< length <<", "
    <<"VOLT=0.0";
  if (t==madx)
    s << ", HARMON=0;";
  s << " ! Voltage and frequency not implemented in libpal. Please set manually." <<endl;
  return s.str();
}


string Dipole::printSimTool(SimTool t) const
{
  stringstream s;

  s << name <<" : "<< nameInTool("SBEND","CSBEND",t) <<", "
    <<"L="<< length <<", "
    <<"ANGLE="<< k0*length;
  if (t == elegant) {
    s <<", TILT="<< dpsi;
    s <<", e1=0, e2=0, "   //not implemented
      <<"synch_rad=1, isr=1, use_rad_dist=0";
  }
  s <<";"<< rfComment() << endl;
  return s.str();
}

string Corrector::printSimTool(SimTool t) const
{
  stringstream s;
 
  if (plane == V)
    s << name <<" : V"<< nameInTool("KICKER","KICK",t) <<", "; // plane==V => vertical kick
  else if (plane == H)
    s << name <<" : H"<< nameInTool("KICKER","KICK",t) <<", ";
  s <<"L="<< length <<", "
    <<"KICK="<< asin(k0*length);
  if (t == elegant && dpsi!=0.)
    s <<", TILT="<< dpsi;
  s  <<";"<< rfComment() << endl;
  return s.str();
}


string Quadrupole::printSimTool(SimTool t) const
{
  stringstream s;

  s << name <<" : "<< nameInTool("QUADRUPOLE","QUAD",t) <<", "
    <<"L="<< length <<", ";
  if (family == F)
    s  <<"K1="<< k1;
  else if (family == D)
    s  <<"K1="<< -k1;
  if (t == elegant) {
    s <<", TILT=" <<dpsi;
    s <<", fringe_type=fixed-strength, ffringe=0";
  }
  s <<";"<< rfComment() << endl;
  return s.str();
}

string Sextupole::printSimTool(SimTool t) const
{
 stringstream s;

  s << name <<" : "<< nameInTool("SEXTUPOLE","SEXT",t) <<", "
    <<"L="<< length <<", ";
  if (family == F)
    s  <<"K2="<< k2;
  else if (family == D)
    s  <<"K2="<< -k2;
  if (t == elegant && dpsi!=0.)
    s <<", TILT="<< dpsi;
  s <<";"<< rfComment() << endl;
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
  s << "\\dipole{"<< name <<"}{"<< length <<"}{"<< k0*length*180/M_PI <<"}" << endl;
  return s.str();
}

string Corrector::printLaTeX() const
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
string pal::getLaTeXDrift(double driftlength)
{
  if (fabs(driftlength) < 1e-6)
    return "";

  std::stringstream s;
  s << "\\drift{" << driftlength << "}"<< endl; //drift
  return s.str();
}
