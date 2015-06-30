/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <cmath>
#include "AccElements.hpp"


using namespace pal;

// static member definition
AccPair AccElement::zeroPair;
AccTriple AccElement::zeroTriple;

AccElement::AccElement(element_type _type, string _name, double _length)
  : type(_type),name(_name),length(_length),plane(noplane),family(nofamily)
{
  physLength = k1 = k2 = Qrf1 = dQrf = dpsi = 0;

  if (length < 0.) {
    stringstream msg;
    msg << "invalid length "<<length<<"<0 for AccElement " << name;
    throw palatticeError(msg.str());
  }
  else if (length > 0.)
    this->checkPhysLength();
}


Dipole::Dipole(string _name, double _length, element_plane p, double _k0)
  : Magnet(dipole,_name,_length)
{
  plane = p;
  switch(plane) {
  case V:
    k0.x = _k0;
    break;
  case H:
    k0.z = _k0;
    break;
  case L:
    k0.s = _k0;
    break;
  case noplane:
    k0.x = k0.z = _k0;
    break;
  }
}

Dipole::Dipole(string _name, double _length, AccTriple _k0)
  : Magnet(dipole,_name,_length) {k0 = _k0;}

Corrector::Corrector(string _name, double _length, element_plane p, double _k0)
  : Magnet(corrector,_name,_length)
{
  plane=p;
  switch(plane) {
  case V:
    k0.x = _k0;
    break;
  case H:
    k0.z = _k0;
    break;
  case L:
    k0.s = _k0;
    break;
  case noplane:
    k0.x = k0.z = _k0;
    break;
  }
}

Corrector::Corrector(string _name, double _length, AccTriple _k0)
  : Magnet(corrector,_name,_length) {k0 = _k0;}


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
    throw palatticeError(msg.str());
  }
  if (abs(length-other->length) > ZERO_DISTANCE) {
    msg << "ERROR: AccElement::operator=(): Cannot assign Element of different length ("
	<< length <<"/"<< other->length <<")";
    throw palatticeError(msg.str());
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


bool AccElement::operator==(const AccElement &o) const
{
  if (o.type != type) return false;
  else if (o.name != name) return false;
  else if (std::fabs(o.length-length) > ZERO_DISTANCE) return false;
  else if ((k0-o.k0).abs() > COMPARE_DOUBLE_EQUAL) return false;
  else if (std::fabs(o.k1-k1) > COMPARE_DOUBLE_EQUAL) return false;
  else if (std::fabs(o.k2-k2) > COMPARE_DOUBLE_EQUAL) return false;
  else if (std::fabs(o.dpsi-dpsi) > COMPARE_DOUBLE_EQUAL) return false;
  // libpal internal variables
  else if (o.plane != plane) return false;
  else if (o.family != family) return false;
  else if (o.Qrf1 != Qrf1) return false;
  else if (o.dQrf != dQrf) return false;
  else return true;
}

bool AccElement::operator!=(const AccElement &o) const
{
  if (operator==(o) == true) return false;
  else return true;
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
  case multipole:
    return "Multipole";
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
    <<std::setw(w)<< k0.x <<std::setw(w)<< k0.z <<std::setw(w)<< k0.s
    <<std::setw(w)<< k1 <<std::setw(w)<< k2 <<std::setw(w)<< dpsi
    <<std::setw(w)<< e1 <<std::setw(w)<< e2 << std::endl;

  return s.str();
}

// string output of header-line(s) for print()
string AccElement::printHeader() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< "Name" <<std::setw(w)<< "Type" <<std::setw(w)<< "Length/m"
    <<std::setw(w)<< "k0.x / 1/m" <<std::setw(w)<< "k0.z / 1/m" <<std::setw(w)<< "k0.s / 1/m" <<std::setw(w)<< "k1 / 1/m^2" <<std::setw(w)<< "k2 / 1/m^3" 
    <<std::setw(w)<< "Rotation(s)/rad"
    <<std::setw(w)<< "e1 / rad" <<std::setw(w)<< "e2 / rad" << std::endl;

  return s.str();
}





// ========= magnetic field calculation =================

// Magnet
AccTriple Magnet::B() const
{
  return k0.tilt(dpsi);
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
  tmp += k0;
  
  return tmp.tilt(this->dpsi);
}




// Multipole
AccTriple Multipole::B() const
{
  string msg="Multipole::B(): B-Field of "+type_string()+" depends on orbit! Please provide argument.";
  throw palatticeError(msg);
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
    throw palatticeError("AccElement::nameInTool(): unknown SimTool");
  }
  return "unknown SimTool";
}

// *************************sign of rotation angle dpsi:*********************************
// test with influence of dpsi on vertical closed orbit in madx show
// that dpsi is defined counter clockwise (dpsi>0 for dipole => kick to negative z)
// libpal and elegant (tilt) use clockwise definition, so sign is changed here
// to get the correct signs in madx (sign also changed during madximport, see AccLattice.cpp)
// *********************************************************************************
string AccElement::printTilt(SimTool t) const
{
  stringstream s;
  if (std::fabs(dpsi)>=MIN_EXPORT_TILT) {
    if (t == elegant)
      s <<", TILT="<< dpsi;
    else if (t == madx)
      s <<", TILT="<< - dpsi;
  }
  return s.str();
}

string AccElement::printEdges() const
{
  stringstream s;
  if (std::fabs(e1)>=MIN_EXPORT_TILT || std::fabs(e2)>=MIN_EXPORT_TILT)
    s << ", E1="<< e1 <<", E2="<< e2;
  return s.str();
}

string AccElement::printStrength() const
{
  stringstream s;
  if (family == F) {
    if (k1!=0.) s <<", K1="<< k1;
    if (k2!=0.) s <<", K2="<< k2;
  }
  else if (family == D) {
    if (k1!=0.) s <<", K1="<< -k1;
    if (k2!=0.) s <<", K2="<< -k2;
  }
  return s.str();
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
  if (k0.x!=0. || k0.s!=0.) 
    std::cout << "WARNING: " << name << " nonzero horizontal or longitudinal field is not exported!" << std::endl;

  s << name <<" : "<< nameInTool("SBEND","CSBEND",t) <<", "
    <<"L="<< length <<", "
    <<"ANGLE="<< k0.z*length;
  s << printStrength();
  s << printEdges() << printTilt(t) <<";"<< rfComment() << endl;
  return s.str();
}

string Corrector::printSimTool(SimTool t) const
{
  stringstream s;
  double kick=0.;
 
  s << name << " : ";
  if (plane == V) {
    if (k0.z!=0. || k0.s!=0.) 
      std::cout << "WARNING: " << name << " nonzero vertical or longitudinal field is not exported (plane=V)!" << std::endl;
    s << "V" << nameInTool("KICKER","KICK",t); 
    kick = k0.x; // plane==V => vertical kick => horizontal field!
  }
  else if (plane == H) {
    if (k0.x!=0. || k0.s!=0.) 
      std::cout << "WARNING: " << name << " nonzero horizontal or longitudinal field is not exported (plane=H)!" << std::endl;
    s << "H"<< nameInTool("KICKER","KICK",t);
    kick = k0.z; // plane==H => horizontal kick => vertical field!
  }
  else if (plane==noplane) {
    s << "KICKER";
  }
  else
    throw palatticeError("Export of Corrector with plane=L not implemented!");

  s << ", L="<< length <<", ";

  if (plane == noplane)
    s << "VKICK="<< asin(k0.x*length) <<", HKICK="<< asin(k0.z*length);
  else
    s << "KICK="<< asin(kick*length);

  
  s << printTilt(t) <<";"<< rfComment() << endl;
  return s.str();
}



string Quadrupole::printSimTool(SimTool t) const
{
  stringstream s;

  s << name <<" : "<< nameInTool("QUADRUPOLE","KQUAD",t) <<", "
    <<"L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) <<";"<< rfComment() << endl;
  return s.str();
}

string Sextupole::printSimTool(SimTool t) const
{
 stringstream s;

  s << name <<" : "<< nameInTool("SEXTUPOLE","KSEXT",t) <<", "
    <<"L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) <<";"<< rfComment() << endl;
  return s.str();
}


//elegants MULT has only 1 Order (e.g. not both k1 and k2)
//export only for madx. for elegant a drift is exported
string Multipole::printSimTool(SimTool t) const
{
 stringstream s;

 if (t == elegant) {
   cout << "Multipole::printSimTool: No Multipole (with k1 and k2) implemented for export to elegant! Export drift.";
   s << name <<" : "<< nameInTool("DRIFT","DRIF",t) <<", "
     <<"L="<< length <<"; ! Multipole in libpal (with k1 and k2)!"<<endl;
   return s.str();
 }

  s << name <<" : "<< nameInTool("MULTIPOLE","MULT",t) <<", "
    <<"L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) <<";"<< rfComment() << endl;
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
  if (k0.x!=0. || k0.s!=0.) 
    std::cout << "WARNING: " << name << "nonzero horizontal or longitudinal bending is not exported!" << std::endl;

  s << "\\dipole{"<< name <<"}{"<< length <<"}{"<< k0.z*length*180/M_PI <<"}" << endl;
  return s.str();
}

string Corrector::printLaTeX() const
{
  stringstream s;
  if (this->plane == noplane)
    s << "\\kicker{"<< name <<"}{"<< length <<"}" << endl;
  else
    s << "\\corrector{"<< name <<"}{"<< length <<"}" << endl;
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

string Multipole::printLaTeX() const //Multipole is exported as Sextupole
{
 stringstream s;
  s << "\\sextupole{"<< name <<"}{"<< length <<"} % multipole exported as sextupole" << endl;
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
