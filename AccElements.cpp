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

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
#include "AccElements.hpp"


using namespace pal;

// static member definition
AccPair AccElement::zeroPair;
AccTriple AccElement::zeroTriple;


string pal::filterCharactersForLaTeX(string in)
{
  std::map<string,string> list = {{"_","\\_"}, {"$", "\\$"}};
  for(auto& it : list) {
    size_t start_pos = 0;
    while((start_pos = in.find(it.first, start_pos)) != std::string::npos) {
      in.replace(start_pos, it.first.length(), it.second);
      start_pos += it.second.length();
    }
  }
  return in;
}

string pal::type_string(pal::element_type type)
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
  case marker:
    return "Marker";
  case monitor:
    return "Monitor";
  case rcollimator:
    return "Rcollimator";
  case solenoid:
    return "Slenoid";
  case drift:
    return "Drift";
  }
  return "Please implement this type in pal::type_string()!";
}

string pal::type_string(element_type type, SimTool t)
{
  switch (type) {
  case dipole:
    return pal::simToolConditional("SBEND","CSBEND",t);
  case quadrupole:
    return pal::simToolConditional("QUADRUPOLE","KQUAD",t);
  case corrector:
    return pal::simToolConditional("KICKER","KICK",t);
  case sextupole:
    return pal::simToolConditional("SEXTUPOLE","KSEXT",t);
  case multipole:
    return pal::simToolConditional("MULTIPOLE","MULT",t);
  case cavity:
    return pal::simToolConditional("RFCAVITY","RFCA",t);
  case marker:
    return pal::simToolConditional("MARKER","MARK",t);
  case monitor:
    return pal::simToolConditional("MONITOR","MONI",t);
  case rcollimator:
    return pal::simToolConditional("RCOLLIMATOR","RCOL",t);
  case solenoid:
    return pal::simToolConditional("SOLENOID","",t);
  case drift:
    return pal::simToolConditional("DRIFT","DRIF",t);
  }
  return "Please implement this type in pal::type_string(simTool)!";
}





AccElement::AccElement(element_type _type, string _name, double _length)
  : type(_type),name(_name),length(_length),plane(noplane),family(nofamily)
{
  physLength = k1 = k2 = Qrf1 = dQrf = tilt = e1 = e2 = 0.;

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
  family = F;
  plane = p;
  switch(plane) {
  case V:
    k0.x = _k0;
    break;
  case H:
    k0.z = _k0;
    break;
  case noplane:
    k0.x = k0.z = _k0;
    break;
  }
}

Dipole::Dipole(string _name, double _length, AccTriple _k0)
  : Magnet(dipole,_name,_length) {family = F; k0 = _k0;}

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
      // cout <<"WARNING: DEFAULT_LENGTH_DIFFERENCE (config.hpp) > length for AccElement "<< this->name
      // 	   << ". physical length set to zero." << endl;
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
   this->tilt = other->tilt;
   this->displacement = other->displacement;
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
bool AccElement::nameMatch(const vector<string> &nameList) const
{
  for (unsigned int i=0; i<nameList.size(); i++) {
    if (nameMatch(nameList[i]))
      return true;
  }
  return false;
}


// true if element name matches pattern (can include 1 wildcard *)
bool AccElement::nameMatch(const string &pattern) const
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
  else if (std::fabs(o.tilt-tilt) > COMPARE_DOUBLE_EQUAL) return false;
  else if ((displacement-o.displacement).abs() > COMPARE_DOUBLE_EQUAL) return false;
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






// string output of (some) element properties
// ! if you change this function, !
// ! also modify printHeader()    !
string AccElement::print() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< name <<std::setw(w)<< type_string() <<std::setw(w) << length
    <<std::setw(w)<< k0.x <<std::setw(w)<< k0.z <<std::setw(w)<< k0.s
    <<std::setw(w)<< k1 <<std::setw(w)<< k2
    <<std::setw(w)<< tilt <<std::setw(w)<< displacement.x <<std::setw(w)<< displacement.z
    <<std::setw(w)<< e1 <<std::setw(w)<< e2
    <<std::setw(w)<< halfWidth.x <<std::setw(w)<< halfWidth.z << std::endl;

  return s.str();
}

// string output of header-line(s) for print()
string AccElement::printHeader() const
{
  int w=16;
  std::stringstream s;

  s <<std::setw(w)<< "Name" <<std::setw(w)<< "Type" <<std::setw(w)<< "Length/m"
    <<std::setw(w)<< "k0.x / 1/m" <<std::setw(w)<< "k0.z / 1/m" <<std::setw(w)<< "k0.s / 1/m" <<std::setw(w)<< "k1 / 1/m^2" <<std::setw(w)<< "k2 / 1/m^3" 
    <<std::setw(w)<< "tilt(s)/rad" <<std::setw(w)<< "dx/m" <<std::setw(w)<< "dz/m"
    <<std::setw(w)<< "e1 / rad" <<std::setw(w)<< "e2 / rad"
    <<std::setw(w)<< "halfWidth.x / m" <<std::setw(w)<< "halfWidth.z / m" << std::endl;

  return s.str();
}



// ====== synchrotron radiation ======
//critical photon energy in Joule at electron beam with reference energy given as gamma0
//according to Matthew Sands "Physics of Electron Storage Rings" Eq. (5.9)
double Magnet::syli_Ecrit_Joule(const double& gamma0) const
{
  double E = 3./2. * GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR * GSL_CONST_MKSA_SPEED_OF_LIGHT * std::pow(gamma0,3) * k0.abs(); //k0 = 1/R
  return E;
}
//... in keV
double Magnet::syli_Ecrit_keV(const double& gamma0) const
{
  return syli_Ecrit_Joule(gamma0) / GSL_CONST_MKSA_ELECTRON_VOLT / 1000.;
}
//... in units of gamma
double Magnet::syli_Ecrit_gamma(const double& gamma0) const
{
  return syli_Ecrit_Joule(gamma0) / (GSL_CONST_MKSA_MASS_ELECTRON * std::pow(GSL_CONST_MKSA_SPEED_OF_LIGHT,2));
}


//mean number of photons emmited in this magnet by electron with reference energy given as gamma0
//'photons per radian' according to Matthew Sands "Physics of Electron Storage Rings" Eq. (5.15)
//'photons in bending magnet' by multiplication with bending angle l/R = l*|k0|
double Magnet::syli_meanPhotons(const double& gamma0) const
{
  return 5./(2*std::sqrt(3)) * gamma0/137. * length*k0.abs();
} 



// ========= magnetic field calculation =================

// Magnet
AccTriple Magnet::B() const
{
  return k0.tilt(tilt);
}

AccTriple Magnet::B(const AccPair &orbit) const
{
  AccTriple tmp;
  //misalignment: displacement in x and z:
  // shift orbit to sytem of magnet
  AccPair oPrime = orbit - displacement;
  //misalignment: tilt around s-Axis:
  // if B depends on orbit, two steps are required:
  // 1. rotate orbit to system of magnet (-tilt)
  // 2. rotate calculated field back to lab frame (+tilt)
  oPrime = oPrime.tilt(- tilt);
  
  //quadrupole & sextupole field (k1 & k2)
  tmp.x= k1*oPrime.z + k2*oPrime.x*oPrime.z;
  tmp.z= k1*oPrime.x + 0.5*k2*(oPrime.x*oPrime.x-oPrime.z*oPrime.z);
  tmp.s=0;
  
  if(family==D) tmp*=(-1.); //sign of k1,k2

  //dipole field (k0)
  tmp += k0;
  
  return tmp.tilt(this->tilt);
}




// Multipole
AccTriple Multipole::B() const
{
  string msg="Multipole::B(): B-Field of "+type_string()+" depends on orbit! Please provide argument.";
  throw palatticeError(msg);
}


 




// ============ printSimTool (elegant or madx) ======================


// *************************sign of rotation angle tilt:*********************************
// test with influence of tilt on vertical closed orbit in madx show
// that tilt is defined counter clockwise (tilt>0 for dipole => kick to negative z)
// libpal and elegant (tilt) use clockwise definition, so sign is changed here
// to get the correct signs in madx (sign also changed during madximport, see AccLattice.cpp)
// *********************************************************************************
string AccElement::printTilt(SimTool t) const
{
  stringstream s;
  if (std::fabs(tilt)>=MIN_EXPORT_TILT) {
    if (t == elegant)
      s <<", TILT="<< tilt;
    else if (t == madx)
      s <<", TILT="<< - tilt;
  }
  return s.str();
}

string AccElement::printDisplace() const
{
  stringstream s;
  if (std::fabs(displacement.x)>=MIN_EXPORT_DISPLACEMENT) {
    s <<", DX="<< displacement.x;
  }
  if (std::fabs(displacement.z)>=MIN_EXPORT_DISPLACEMENT) {
    s <<", DY="<< displacement.z;
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

string AccElement::printAperture(SimTool t) const
{
  stringstream s;
  if (t == madx)
    s << ", APERTYPE=RECTANGLE, APERTURE={" <<halfWidth.x<<","<<halfWidth.z<<"}";
  else if (t == elegant)
    s << ", X_MAX="<<halfWidth.x<<", "<<"Y_MAX="<<halfWidth.z;
  return s.str();
}

string AccElement::printRF(SimTool t) const
{
  stringstream s;
  s << "VOLT=";
  if(t == madx)
    s << volt / 1e6;
  else if (t == elegant)
    s << volt;
  s << ", FREQ=";
  if(t == madx)
    s << freq / 1e6;
  else if (t == elegant)
    s << freq;
  return s.str();
}

string AccElement::printSyli(SimTool t) const
{
  stringstream s;

  // there are two different synchrotron-radiation models in elegant:
  // "use_synch_rad" uses a realistic photon energy spectrum
  // "synch_rad" uses a gaussian distribution for better performance

  if(t==elegant)
    s << ", use_rad_dist="<<ELEGANT_SYNCH_RAD_SETTING;
    // s << ", synch_rad="<<ELEGANT_SYNCH_RAD_SETTING<<", isr="<<ELEGANT_SYNCH_RAD_SETTING;

  return s.str();
}

string AccElement::printNKicks(SimTool t) const
{
  stringstream s;

  if(t==elegant) {
    if (type==dipole)
      s << ", N_KICKS=" << ELEGANT_DIPOLE_NKICKS;
    else if (type==quadrupole)
      s << ", N_KICKS=" << ELEGANT_QUADRUPOLE_NKICKS;
    else if (type==sextupole)
      s << ", N_KICKS=" << ELEGANT_SEXTUPOLE_NKICKS;
  }
  
  return s.str();
}

string AccElement::rfMagComment() const
{
  stringstream s;
  if (Qrf1!=0. || dQrf!=0.)
    s << "! RF magnet in libpal (Qrf1=" << Qrf1 << ", dQrf=" << dQrf << ")";

  return s.str();
}



string NoMagnet::printSimTool(SimTool t) const
{
  stringstream s;

  s << printNameType(t) <<", "
    <<"L="<< length <<";"<<endl;
  return s.str();
}

string Cavity::printSimTool(SimTool t) const
{
  stringstream s;

  s << printNameType(t) <<", "
    <<"L="<< length <<", "
    << printRF(t) << printDisplace() <<";"<< endl;
  return s.str();
}


string Dipole::printSimTool(SimTool t) const
{
  stringstream s;
  if (k0.x!=0. || k0.s!=0.) 
    std::cout << "WARNING: " << name << " nonzero horizontal or longitudinal field is not exported!" << std::endl;

  s << printNameType(t) << printNKicks(t)
    <<", L="<< length <<", ANGLE="<< k0.z*length;
  s << printStrength();
  s << printEdges() << printTilt(t) << printDisplace() << printSyli(t) << ";"<< rfMagComment() << endl;
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
    s << "V" << type_string(t); 
    kick = k0.x; // plane==V => vertical kick => horizontal field!
  }
  else if (plane == H) {
    if (k0.x!=0. || k0.s!=0.) 
      std::cout << "WARNING: " << name << " nonzero horizontal or longitudinal field is not exported (plane=H)!" << std::endl;
    s << "H"<< type_string(t);
    kick = k0.z; // plane==H => horizontal kick => vertical field!
  }
  else { //noplane
    s << "KICKER";
  }

  s << ", L="<< length <<", ";

  if (plane == noplane)
    s << "VKICK="<< asin(k0.x*length) <<", HKICK="<< asin(k0.z*length);
  else
    s << "KICK="<< asin(kick*length);

  
  s << printTilt(t) <<";"<< rfMagComment() << endl;
  return s.str();
}



string Quadrupole::printSimTool(SimTool t) const
{
  stringstream s;

  s << printNameType(t) << printNKicks(t)
    <<", L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) << printDisplace()<<";"<< rfMagComment() << endl;
  return s.str();
}

string Sextupole::printSimTool(SimTool t) const
{
 stringstream s;

  s << printNameType(t) << printNKicks(t)
    <<", L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) << printDisplace() <<";"<< rfMagComment() << endl;
  return s.str();
}

string Solenoid::printSimTool(SimTool t) const
{
 stringstream s;

  s << printNameType(t)
    <<", L="<< length<<", KS="<< k0.s;
  s << printStrength();
  s << printEdges() << printTilt(t) << printDisplace() <<";"<< rfMagComment() << endl;
  return s.str();
}

//elegants MULT has only 1 Order (e.g. not both k1 and k2)
//export only for madx. for elegant a drift is exported
string Multipole::printSimTool(SimTool t) const
{
 stringstream s;

 if (t == elegant) {
   cout << "Multipole::printSimTool: No Multipole (with k1 and k2) implemented for export to elegant! Export drift.";
   s << pal::type_string(drift, t) <<", "
     <<"L="<< length <<"; ! Multipole in libpalattice (with k1 and k2)!"<<endl;
   return s.str();
 }

  s << printNameType(t) <<", "
    <<"L="<< length;
  s << printStrength();
  s << printEdges() << printTilt(t) << printDisplace() <<";"<< rfMagComment() << endl;
  return s.str();
}

string Marker::printSimTool(SimTool t) const
{
  stringstream s;
  s << printNameType(t) <<";"<<endl;
  return s.str();
}

string Monitor::printSimTool(SimTool t) const
{
  stringstream s;
  s << printNameType(t) <<", "
    <<"L="<< length;
  s << printTilt(t) << printDisplace() <<";"<<endl;
  return s.str();
}

string Rcollimator::printSimTool(SimTool t) const
{
  stringstream s;
  s << printNameType(t) <<", "
    <<"L="<< length;
  s << printAperture(t) << printDisplace() <<";"<<endl;
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
  s << "\\cavity{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Dipole::printLaTeX() const
{
  stringstream s;
  if (k0.x!=0. || k0.s!=0.) 
    std::cout << "WARNING: " << name << "nonzero horizontal or longitudinal bending is not exported!" << std::endl;

  s << "\\dipole{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}{"<< k0.z*length*180/M_PI <<"}" << endl;
  return s.str();
}

string Corrector::printLaTeX() const
{
  stringstream s;
  if (this->plane == noplane)
    s << "\\kicker{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  else
    s << "\\corrector{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Solenoid::printLaTeX() const
{
  stringstream s;
  s << "\\solenoid{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Quadrupole::printLaTeX() const
{
  stringstream s;
  s << "\\quadrupole{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Sextupole::printLaTeX() const
{
 stringstream s;
  s << "\\sextupole{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
  return s.str();
}

string Multipole::printLaTeX() const //Multipole is exported as Sextupole
{
 stringstream s;
  s << "\\sextupole{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"} % multipole exported as sextupole" << endl;
  return s.str();
}

string Marker::printLaTeX() const
{
  stringstream s;
  s << "\\marker{"<< filterCharactersForLaTeX(name) <<"}" << endl;
  return s.str();
}

string Monitor::printLaTeX() const
{
  stringstream s;
  s << "\\screen{"<< filterCharactersForLaTeX(name) <<"}["<< length <<"]" << endl;
  return s.str();
}

string Rcollimator::printLaTeX() const //Rcollimator is exported as Kicker
{
  stringstream s;
  s << "\\kicker{"<< filterCharactersForLaTeX(name) <<"}{"<< length <<"}" << endl;
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
