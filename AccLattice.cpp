/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X or ELEGANT.
 * Uses the "AccElements" classes
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "AccLattice.hpp"
#include "constants.hpp"


//remove quotation marks ("" or '') from begin&end of string
string removeQuote(string s)
{
  if ( (s.compare(0,1,"\"") && s.compare(s.length()-1,1,"\"")) || (s.compare(0,1,"\'") && s.compare(s.length()-1,1,"\'")) )
    return s.substr(1,s.length()-2);
  else
    return s;
}


//constructor
AccLattice::AccLattice(double _circumference, Anchor _refPos)
  : ignoreCounter(0), refPos(_refPos), circumference(_circumference)
{
  empty_space = new Drift;
}

//copy constructor
AccLattice::AccLattice(const AccLattice &other)
  : refPos(other.refPos), circumference(other.circumference)
{
  empty_space = new Drift;

  for (const_AccIterator it=other.getItBegin(); it!=other.getItEnd(); ++it) {
    this->set(it->first, *(it->second));
  }
}


AccLattice::~AccLattice()
{
  for (AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    delete it->second;
  }
  delete empty_space;
}


AccLattice& AccLattice::operator= (const AccLattice other)
{
  stringstream msg;

  if (refPos != other.refPos) {
    msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different refPos ("
	<< refPos_string() <<"/"<< other.refPos_string() <<")";
    throw logic_error(msg.str());
  }
  if (circumference != other.circumference) {
    msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different circumferences ("
	<< circumference <<"/"<< other.circumference <<")";
    throw logic_error(msg.str());
  }

  for (const_AccIterator it=other.getItBegin(); it!=other.getItEnd(); ++it) {
    this->set(it->first, *(it->second));
  }

  return *this;
}






// get here=begin/center/end (in meter) of obj at reference-position pos
// works for all reference Anchors (refPos)
double AccLattice::locate(double pos, const AccElement *obj, Anchor here) const
{
  double l = obj->length;        //element length

  if (refPos == center) pos -= l/2;
  else if (refPos == end) pos -= l;

  switch(here) {
  case begin:
    return pos;
  case center:
    return pos + l/2;
  case end:
    return pos + l;
  }

  return 0.;
}

// get here=begin/center/end (in meter)  of lattice element "it"
// works for all Reference Anchors (refPos, it->first)
double AccLattice::locate(const_AccIterator it, Anchor here) const
{
  return locate(it->first, it->second, here);
}




// test if "here" is inside obj at position pos
bool AccLattice::inside(double pos, const AccElement *obj, double here) const
{
  if (here >= locate(pos,obj,begin) && here <= locate(pos,obj,end))
    return true;
  else
    return false;
}

// test if "here" is inside lattice element "it"
bool AccLattice::inside(const_AccIterator it, double here) const
{
  if (here >= locate(it,begin) && here <= locate(it,end))
    return true;
  else
    return false;
}



// get first element of given type
// returns iterator to end if there is none
AccIterator AccLattice::firstIt(element_type _type)
{
  for (AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();
}


const_AccIterator AccLattice::firstCIt(element_type _type) const
{
  for (const_AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();
}



// get last element of given type
// returns iterator to end if there is none
AccIterator AccLattice::lastIt(element_type _type) 
{
  AccIterator it = elements.end();
  it--;
  for (; it!=elements.begin(); it--) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();  
}

const_AccIterator AccLattice::lastCIt(element_type _type)  const
{
  const_AccIterator it = elements.end();
  it--;
  for (; it!=elements.begin(); it--) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();  
}

// get next element of given type after it
// returns iterator to end if there is none
AccIterator AccLattice::nextIt(element_type _type, AccIterator it)
{
  it++; // check only elements AFTER it
 for (; it!=elements.end(); ++it) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();  
}
const_AccIterator AccLattice::nextCIt(element_type _type, const_AccIterator it) const
{
  it++; // check only elements AFTER it
 for (; it!=elements.end(); ++it) {
    if (it->second->type == _type)
      return it;
  }

  return elements.end();  
}


// get first element of given type
// returns Drift if there is none
const AccElement* AccLattice::first(element_type _type)
{
  AccIterator it = this->firstIt(_type);
  if (it == elements.end()) return empty_space;
  else return it->second;
}

// get last element of given type
// returns Drift if there is none
const AccElement* AccLattice::last(element_type _type)
{
  AccIterator it = lastIt(_type);
  if (it == elements.end()) return empty_space;
  else return it->second;
}

// get next element of given type after pos
// returns Drift if there is none
const AccElement* AccLattice::next(element_type _type, double pos)
{
  for (const_AccIterator it=getIt(pos); it!=elements.end(); ++it) {
    if (it->second->type == _type)
      return it->second;
  }

  return empty_space;  
}



// get const_Iterator to element, if pos is inside it
const_AccIterator AccLattice::getIt(double pos) const
{
  const_AccIterator candidate_next = elements.upper_bound(pos); //"first element whose key goes after pos"
  const_AccIterator candidate_previous = candidate_next;
  
  if (candidate_next != elements.begin()) {
    candidate_previous--;
    if (refPos == begin || refPos == center) {
      if (inside(candidate_previous, pos))
	return candidate_previous;
    }
  }
  
  if (candidate_next != elements.end()) {
    if (refPos == end || refPos == center) {
      if (inside(candidate_next, pos))
	return candidate_next;
    }
  }
  
  throw eNoElement();
}


// get iterator to begin (first Element)
const_AccIterator AccLattice::getItBegin() const
{
  return elements.begin();
}

// get iterator to end (after last Element)
const_AccIterator AccLattice::getItEnd() const
{
  return elements.end();
}







// get element
const AccElement* AccLattice::operator[](double pos) const
{
  if (pos > circumference) {
    stringstream msg;
    msg << pos << " m is larger than lattice circumference " << circumference << " m.";
    throw std::out_of_range(msg.str());
  }

  try {
    return getIt(pos)->second;
  }
  // otherwise pos not inside any element:
  catch (eNoElement &e) {
    return empty_space;
  }
}



// set element (replace if key (pos) already used; check for "free space" to insert element)
// if name is in ignoreList, element is not set and ignoreCounter is increased
void AccLattice::set(double pos, const AccElement& obj, bool verbose)
{
  //ignoreList
  if ( obj.nameMatch(ignoreList) ) {
    ignoreCounter++;
    return;
  }

 if (pos > circumference) {
    stringstream msg;
    msg << pos << " m is larger than lattice circumference " << circumference << " m.";
    throw std::out_of_range(msg.str());
  }

  bool first_element = false;
  bool last_element = false;
  const AccElement *objPtr = &obj;
  double newBegin = locate(pos, objPtr, begin);
  double newEnd = locate(pos, objPtr, end);
  stringstream msg;
  
  if (pos < 0.) {
    cout << "ERROR: AccLattice::set(): Position of Lattice elements must be > 0. " << pos << " is not." <<endl;
    exit(1);
  }

  // empty map (set first element)
  if (elements.size() == 0) {
    elements[pos] = objPtr->clone();
    if (verbose) cout << objPtr->name << " inserted." << endl;
    return;
  }


  const_AccIterator next = elements.upper_bound(pos); //"first element whose key goes after pos"
  const_AccIterator previous = next;
  if (next == elements.begin())
    first_element = true;
  else
    previous--;
  if (next == elements.end()) //"past-the-end element"
    last_element = true;

 
  // try deleting possibly existing element at pos
  try{
    delete elements.at(pos);    
  }
  catch(std::out_of_range &e){ }

  // avoid numerical problems when checking for "free space"
  if (abs(newBegin - locate(previous,end)) < ZERO_DISTANCE) {
    newBegin += ZERO_DISTANCE;
  }
  if (abs(newEnd - locate(next,begin)) < ZERO_DISTANCE)
      newEnd -= ZERO_DISTANCE;

  // check for "free space" to insert obj
  if (newBegin < 0.) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m) cannot be inserted --- overlap with lattice begin at 0.0m";
    throw eNoFreeSpace(msg.str());
  }
  else if (!first_element &&  newBegin < locate(previous,end)) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m) cannot be inserted --- overlap with "
	<< previous->second->name << " ("<< locate(previous,begin) <<" - "<<locate(previous,end) << "m)";
    throw eNoFreeSpace(msg.str());
  }
  else if (newEnd > circumference) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd <<  "m) cannot be inserted --- overlap with lattice end at " << circumference << "m";
    throw eNoFreeSpace(msg.str());
  }
  else if (!last_element && newEnd > locate(next,begin)) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m) cannot be inserted --- overlap with "
	<< next->second->name << " ("<< locate(next,begin) <<" - " << locate(next,end) << "m)";
    throw eNoFreeSpace(msg.str());
  }
  //if there is free space:
  else elements[pos] = objPtr->clone();

  if (verbose) cout << objPtr->name << " inserted." << endl;

}


// erase element at position pos
void AccLattice::erase(double pos)
{
  AccIterator it =  elements.find(pos);

  if (it == elements.end()) {
    cout << "WARNING: AccLattice::erase(): There is no element at position "<<pos<< " m. Nothing is erased." << endl;
    return;
  }
  elements.erase(it);
}




// elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
void AccLattice::setIgnoreList(string ignoreFile)
{
  ifstream f;
  string tmp;

  f.open(ignoreFile.c_str());
  if (!f.is_open()) {
    cout << "ERROR: AccLattice::setIgnoreList(): Cannot open " << ignoreFile << endl;
    exit(1);
  }

  while (!f.eof()) {
    f >> tmp;
    ignoreList.push_back(tmp);
    // cout << "* ignore magnets named " << tmp << endl;
  }
}




// set elements from MAD-X Lattice (read from twiss-output)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// MAD-X uses different signs of strengths (k,m)
// ===================================================
void AccLattice::madximport(const char *madxTwissFile)
{
  // madx column variables
  string tmp, tmpName;
  string expectedColumns[11] = {"KEYWORD","NAME","S","X","Y","L","ANGLE","K1L","K2L","VKICK","HKICK"};
  unsigned int j;
  double s, angle, k1l, k2l, vkick; 
  string x, y;

  fstream madxTwiss;

  //AccElements
  Dipole vDip("defaultName", 0., 0., V);
  Dipole hDip("defaultName", 0., 0., H);
  Corrector vCorr("defaultName", 0., 0., H); // vertical kicker has HORIZONTAL field
  Corrector hCorr("defaultName", 0., 0., V);
  RFdipole vRFdip("defaultName", 0., 0., H); // vertical kicker has HORIZONTAL field
  Quadrupole Quad("defaultName", 0., 0., F); // madx uses negative sign "strength" for D magnets,
  Sextupole Sext("defaultName", 0., 0., F);  // so here all Quads/Sexts are defined as family F (also see AccElements.hpp)

  if (refPos != end)
    cout << "WARNING: AccLattice::madximport(): The input file (MAD-X twiss) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;

  madxTwiss.open(madxTwissFile, ios::in);
  if (!madxTwiss.is_open()) {
    cout << "ERROR: AccLattice::madximport(): Cannot open " << madxTwissFile << endl;
    exit(1);
  }

  while (!madxTwiss.eof()) {
    madxTwiss >> tmp;

    // --- check correct column order by comparing headline ---------------------
    if (tmp == "*") {
      for (j=0; j<11; j++) {
	madxTwiss >> tmp;
	//cout << "debug: madx=>" <<tmp<< " expected=>" <<expectedColumns[j] << endl; //debug
	if (tmp != expectedColumns[j]) {
	  cout << "ERROR: AccLattice::madximport(): Unexpected columns (or column order) in " << madxTwissFile << endl;
	  exit(1);
	}
      }
    }
    // --------------------------------------------------------------------------
  
    if (tmp == "\"SBEND\"" || tmp == "\"RBEND\"") {  //vertical Dipole (assume all bends have vertical field)
      madxTwiss >> tmpName >> s >> x >> y >> vDip.length >> angle;
      vDip.name = removeQuote(tmpName);
      vDip.strength = angle/vDip.length;   // 1/R (!!! assuming l is arclength (along ref. orbit) !!!)
      if (refPos == begin) s -= vDip.length;
      else if (refPos == center) s -= vDip.length/2;
      this->set(s, vDip);
    }
    else if (tmp == "\"QUADRUPOLE\"") {
      madxTwiss >> tmpName >> s >> x >> y >> Quad.length >> angle >> k1l;
      Quad.name = removeQuote(tmpName);
      Quad.strength = k1l/Quad.length;   // k
      if (refPos == begin) s -= Quad.length;
      else if (refPos == center) s -= Quad.length/2;
      this->set(s, Quad);
      // BPMs at quads - Closed orbit not included in lattice, read in separate function of Orbit-class
    }
    else if (tmp == "\"SEXTUPOLE\"") {
      madxTwiss >> tmpName >> s >> x >> y >> Sext.length >> angle >> k1l >> k2l;
      Sext.name = removeQuote(tmpName);
      Sext.strength = k2l/Sext.length;
      if (refPos == begin) s -= Sext.length;
      else if (refPos == center) s -= Sext.length/2;
      this->set(s, Sext);
    }
    else if (tmp == "\"VKICKER\"") {
      madxTwiss >> tmpName >> s >> x >> y >> vCorr.length >> angle >> k1l >> k2l >> vkick;
      vCorr.name = removeQuote(tmpName);
      vCorr.strength = sin(vkick)/vCorr.length;   // 1/R from kick-angle
      if (refPos == begin) s -= vCorr.length;
      else if (refPos == center) s -= vCorr.length/2;
      //RF dipole:
      if (vCorr.name.substr(0,6) == "RFDIP.") {
	vRFdip.name = vCorr.name;
	vRFdip.length = vCorr.length;
	vRFdip.strength = vCorr.strength;
	vRFdip.Qrf0 = 0.625;              // !! hardcoded RF-tune values !!
	vRFdip.dQrf = 5.402e-6;
	this->set(s, vRFdip);
      }
      //Corrector:
      else {
	this->set(s, vCorr);
      }
    }

  }
  madxTwiss.close();
}


// set misalignments from MAD-X Lattice (read ealign-output)
// ! currently only rotation (dpsi) around beam axis (s) is implemented   !
// ! to add others: define member in class AccElements & implement import !
// *************************sign of rotation angle:*********************************
// test with influence of dpsi on vertical closed orbit in madx show
// that dpsi is defined counter clockwise (dpsi>0 for dipole => kick to negative z)
// Bsupply and elegant use clockwise definition, so sign is changed here
// to get the correct signs for the magnetic fields calculated from dpsi
// *********************************************************************************
void AccLattice::madximportMisalignments(const char *madxEalignFile)
{
  string tmp;
  unsigned int j;
  double _dpsi=0.;
  fstream madxEalign;
  AccIterator it=elements.begin();

  madxEalign.open(madxEalignFile, ios::in);
  if (!madxEalign.is_open()) {
    cout << "ERROR: AccLattice::madximportMisalignments(): Cannot open " << madxEalignFile << endl;
    exit(1);
  }
  
  while (!madxEalign.eof()) {
    madxEalign >> tmp;

    if (tmp == "@" || tmp == "*" || tmp == "$") { // header lines
      getline(madxEalign, tmp);
    }
    else {
      for (; it!=elements.end(); ++it) {          //madxEalign is sorted by position => loop continued
	if (it->second->name == tmp) {
	  for (j=0; j<47; j++) madxEalign >> tmp; //read stupid unnecessary columns
	  madxEalign >> _dpsi;                    //read & write rotation around s-axis
	  it->second->dpsi = - _dpsi;    // <<<<<<!!! sign of rotation angle (see comment above)
	  getline(madxEalign, tmp);
	  break;
	}
      }
    }

  }

}





// set elements from elegant Lattice (read from ascii parameter file ".param")
// misalignments included (no separate function as for MADX)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// elegant uses different signs of strengths (k,m)
// ===================================================
void AccLattice::elegantimport(const char *elegantParamFile)
{
  double s, pos;
  double l, k1, k2, angle, kick, tilt; //parameter values
  paramRow row, row_old;
  AccElement *element;
  fstream elegantParam;
  bool firstElement = true;
  string tmp;

  pos=l=k1=k2=angle=kick=tilt=0.;   // initialize param. values

  //AccElements
  Dipole vDip("defaultName", 0., 0., V);
  Dipole hDip("defaultName", 0., 0., H);
  Corrector vCorr("defaultName", 0., 0., H); // vertical kicker has HORIZONTAL field
  Corrector hCorr("defaultName", 0., 0., V);
  Quadrupole Quad("defaultName", 0., 0., F); // elegant uses negative sign "strength" for D magnets,
  Sextupole Sext("defaultName", 0., 0., F);  // so here all Quads/Sexts are defined as family F (also see AccElements.hpp)
  Drift empty_space("defaultName", 0.);

  if (refPos != end)
    cout << "WARNING: AccLattice::elegantimport(): The input file (elegant parameter) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;

  elegantParam.open(elegantParamFile, ios::in);
  if (!elegantParam.is_open()) {
    cout << "ERROR: AccLattice::elegantimport(): Cannot open " << elegantParamFile << endl;
    exit(1);
  }

  
  // goto first line with data (*** marks end of parameters, two header lines below it)
  elegantParam >> tmp;
  while (tmp != "***") elegantParam >> tmp;
  for (int i=0; i<3; i++)   getline(elegantParam, tmp);

  //read each row of .param file data
  while (!elegantParam.eof()) {
    elegantParam >> row.name >> row.param >> row.value >> row.type;
    getline(elegantParam, tmp); //ignore additional columns
    if (firstElement) {
      row_old = row;
      firstElement = false;
    }

    //mount element if next element reached (=all parameters read)
    if (row.name != row_old.name) {

     if (row_old.type=="CSBEND" || row_old.type=="CSRCSBEND" || row_old.type=="KSBEND" || row_old.type=="NIBEND" || row_old.type=="TUBEND") {
       element = &vDip;
       element->strength = angle / l;
     }
     else if (row_old.type=="QUAD") {
       element = &Quad;
       element->strength = k1;
     }
     else if (row_old.type=="SEXT") {
       element = &Sext;
       element->strength = k2;
     }
     else if (row_old.type=="VKICK") {
       element = &vCorr;
       element->strength = sin(kick) / l;  // 1/R from kick-angle
     }
     else if (row_old.type=="HKICK") {
       element = &hCorr;
       element->strength = sin(kick) / l;  // 1/R from kick-angle
     }
     else
       element = &empty_space; //Drift

     if (element->type != drift) {
       element->length = l;
       element->name = row_old.name;
       element->dpsi = tilt;
       if (refPos == begin) pos = s-l;
       else if (refPos == center) pos = s-l/2;
       else pos = s; 
       this->set(pos, *element); // mount element
       pos=l=k1=k2=angle=kick=tilt=0.;   // clear param. values to avoid reuse of an old value
     }
    }


    //read parameter in row (if needed)
    if (row.param == "L") {    //element length L used to get position (s)
      l = row.value;
      s += l;
    }
    else if (row.param == "K1") k1 = row.value;
    else if (row.param == "K2") k2 = row.value;
    else if (row.param == "ANGLE") angle = row.value;
    else if (row.param == "KICK") kick = row.value;
    else if (row.param == "ETILT") tilt = row.value;
    else if (row.param == "TILT" && row.type!="CSBEND" && row.type!="CSRCSBEND" && row.type!="KSBEND" && row.type!="NIBEND") tilt = row.value;
    //... add more parameters here

 
   row_old = row;
  }
  

  elegantParam.close();
}







// change quad&sext strengths to values from "ELSA-Spuren"
void AccLattice::setELSAoptics(const char *spurenFolder)
{
  char filename[1024];
  string tmp;
  stringstream msg;
  double kf=0, kd=0, mf=0, md=0;
  fstream f_magnets;
  AccIterator it;

 // Read "optics.dat"
  snprintf(filename, 1024, "%s/optics.dat", spurenFolder);
  f_magnets.open(filename, ios::in);
  if (!f_magnets.is_open()) {
    msg << "ERROR: AccLattice::setELSAoptics(): Cannot open " << filename;
    throw std::runtime_error(msg.str());
  }
  while(!f_magnets.eof()) {
    f_magnets >> tmp;
    if (tmp=="ELS_MAGNETE_QUADF.KF_AC")
      f_magnets >> kf;
    else if (tmp=="ELS_MAGNETE_QUADD.KD_AC")
      f_magnets >> kd;
    else if (tmp=="ELS_MAGNETE_SEXTF.MF_AC")
      f_magnets >> mf;
    else if (tmp=="ELS_MAGNETE_SEXTD.MD_AC")
      f_magnets >> md;
    else if (tmp=="#")         //ignore comment lines
      getline(f_magnets, tmp);
    else {
      msg << "ERROR: AccLattice::setELSAoptics(): Unexpected entry in " << filename;
      throw std::runtime_error(msg.str());
    }
  }
  f_magnets.close();

  //Write strengths to quads
  for (it=firstIt(quadrupole); it!=elements.end(); it=nextIt(quadrupole,it)) {
    if (it->second->name.compare(1,2,"QF") == 0) {
      it->second->strength = kf;
    }
    else if (it->second->name.compare(1,2,"QD") == 0) {
      it->second->strength = -kd;
    }
  }

  //Write strengths to sexts
 for (it=firstIt(sextupole); it!=elements.end(); it=nextIt(sextupole,it)) {
    if (it->second->name.compare(1,2,"SF") == 0) {
      it->second->strength = mf;
    }
    else if (it->second->name.compare(1,2,"SD") == 0) {
      it->second->strength = -md;
    }
  }
  
}



// change corrector pos&strength to values from "ELSA-Spuren" at time t/ms
// return number of correctors, where set
unsigned int AccLattice::setELSACorrectors(CORR *ELSAvcorrs, unsigned int t)
{
  unsigned int i, n=0;
 AccElement* corrTmp;
 char name[20];
 char msg[1024];
 stringstream strMsg;
 double diff=0;
 double endPos;
 AccIterator it = firstIt(corrector);
 AccIterator it_next;

 
 for (i=0; i<NVCORRS; i++) {
   if (ELSAvcorrs[i].pos==0.0) {   //inactive correctors have pos=0 in VCORRS.SPOS
     continue;
   }
   else if (t > ELSAvcorrs[i].time.size()) {
     cout << ELSAvcorrs[i].time[ELSAvcorrs[i].time.size()-1].ms << endl;
     snprintf(msg, 1024, "ERROR: AccLattice::setELSACorrectors(): No ELSA VC%02d corrector data available for %d ms.\n", i+1, t);
     throw std::invalid_argument(msg);
   }
   
   //same corrector in Mad-X and ELSA-Spuren?
   //...check by name
   snprintf(name, 20, "KV%02i", i+1);
   if (it->second->name != name) {
     strMsg << "ERROR: AccLattice::setELSACorrectors(): Unexpected corrector name. Mad-X lattice does not fit to ELSA." << endl;
     strMsg << "       Mad-X: " <<it->second->name<< " -- expected: " <<name;
     throw std::runtime_error(strMsg.str());
   }
   //...check by position
   diff = fabs(ELSAvcorrs[i].pos - locate(it,center));
   if (diff > VCPOS_WARNDIFF) {
     cout << "! Position of " <<name<< " differs by " <<diff<< "m in Mad-X and ELSA-Spuren. Use ELSA-Spuren." << endl;
   }
   
   corrTmp = it->second->clone();

   corrTmp->strength = ELSAvcorrs[i].time[t].kick/1000.0/corrTmp->length;   //unit 1/m
   it_next = nextIt(corrector,it);
   elements.erase(it);   // erase "old" corrector (madx) to be able to mount new one
   endPos = ELSAvcorrs[i].pos + corrTmp->length/2;
   this->set(endPos, *(corrTmp));
   delete corrTmp;

   //it = nextIt(corrector,it);
   it = it_next;
   n++;
   if (it == elements.end())
     break;
 }
 
 if (n != this->size(corrector))
   cout << "WARNING: Not all correctors overwritten by ELSA-Spuren" << endl;
 
 return n;
}




// subtract other corrector strengths from the ones of this lattice
void AccLattice::subtractCorrectorStrengths(const AccLattice &other)
{
  stringstream msg;
  AccIterator it;
  const_AccIterator otherIt;
  
  if (this->size(corrector) != other.size(corrector)) {
    msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal number of correctors to subtract.";
    throw std::invalid_argument(msg.str());
  }
  
  otherIt = other.firstCIt(corrector);

  for  (it=firstIt(corrector); it!=lastIt(corrector); it=nextIt(corrector,it)) {

    // check by name
    if (otherIt->second->name != it->second->name) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal names of correctors to subtract. ("
	  << it->second->name <<"/"<< otherIt->second->name << ").";
      throw std::runtime_error(msg.str());
    }
    // check by position
    if (otherIt->first != it->first) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal positions of correctors to subtract. ("
	  << it->first <<"/"<< otherIt->first << ").";
      throw std::runtime_error(msg.str());
    }

    // subtract
    it->second->strength -= otherIt->second->strength;
    // set otherIt to next corrector
    otherIt=nextCIt(corrector,otherIt);
  }

}


// subtract other misalignments from the ones of this lattice
void AccLattice::subtractMisalignments(const AccLattice &other)
{
  stringstream msg;
  AccIterator it;
  const_AccIterator otherIt;
  
  if (this->size() != other.size()) {
    msg << "ERROR: AccLattice::subtractMissalignments(): Unequal number of elements to subtract.";
    throw std::invalid_argument(msg.str());
  }
  
  otherIt = other.getItBegin();

  for  (it=elements.begin(); it!=elements.end(); ++it) {

    // check by name
    if (otherIt->second->name != it->second->name) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal names of elements to subtract. ("
	  << it->second->name <<"/"<< otherIt->second->name << ").";
      throw std::runtime_error(msg.str());
    }
    // check by position
    if (otherIt->first != it->first) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal positions of elements to subtract. ("
	  << it->first <<"/"<< otherIt->first << ").";
      throw std::runtime_error(msg.str());
    }

    // subtract
    it->second->dpsi -= otherIt->second->dpsi;
    // set otherIt to next corrector
    otherIt++;
  }

}



// ------------------ "information" -----------------------
// returns number of elements of a type in this lattice
unsigned int AccLattice::size(element_type _type) const
{
  unsigned int n=0;
  for (const_AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == _type)
      n++;
  }

  return n;
}


string AccLattice::refPos_string() const
{
  switch (refPos) {
  case begin:
    return "begin";
  case center:
    return "center";
  case end:
    return "end";
  }
  
  return "Please implement this type in AccLattice::refPos_string()!";
}



// ----------- output (stdout or file) ----------------------

// print lattice. If no filename is given, print to stdout
void AccLattice::print(const char *filename) const
{
  const_AccIterator it=elements.begin();
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << "# Reference Position: " << refPos_string() << endl;
  s <<"# (* unit of Strength depends on magnet type! e.g. Quad: k1, Sext: k2, Dipole: 1/R)" <<endl;
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it->second->printHeader();

  for (; it!=elements.end(); ++it) {
    s <<std::setw(w+1)<< it->first << it->second->print();
  }

  // output of s
  if (string(filename) == "") 
    cout << s.str();
  else {
    file.open(filename, ios::out);
    if (!file.is_open()) {
      msg << "ERROR: corrs_out(): Cannot open " << filename << ".";
      throw std::runtime_error(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}


// print all elements of one type If no filename is given, print to stdout
void AccLattice::print(element_type _type, const char *filename) const
{
  const_AccIterator it=firstCIt(_type);
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << "# Reference Position: " << refPos_string() << endl;
  s << "# List of " << it->second->type_string() << "s only!" << endl;
  s <<"# (* unit of Strength depends on magnet type! e.g. Quad: k1, Sext: k2, Dipole: 1/R)" <<endl;
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it->second->printHeader();

  for (; it!=lastCIt(_type); it=nextCIt(_type, it)) {
    s <<std::setw(w+1)<< it->first << it->second->print();
  }

  // output of s
  if (string(filename) == "") 
    cout << s.str();
  else {
    file.open(filename, ios::out);
    if (!file.is_open()) {
      msg << "ERROR: corrs_out(): Cannot open " << filename << ".";
      throw std::runtime_error(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}
