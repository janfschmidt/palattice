/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X or ELEGANT.
 * Uses the "AccElements" classes
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include "AccLattice.hpp"

using namespace pal;

//remove quotation marks ("" or '') from begin&end of string
string pal::removeQuote(string s)
{
  if ( (s.compare(0,1,"\"") && s.compare(s.length()-1,1,"\"")) || (s.compare(0,1,"\'") && s.compare(s.length()-1,1,"\'")) )
    return s.substr(1,s.length()-2);
  else
    return s;
}


//constructor
AccLattice::AccLattice(double _circumference, Anchor _refPos)
  : circ(0.), ignoreCounter(0), refPos(_refPos)
{
  empty_space = new Drift;

  info.add("Reference pos.", this->refPos_string());

  setCircumference(_circumference);
}

AccLattice::AccLattice(SimToolInstance &sim, Anchor _refPos, string ignoreFile)
  : circ(0.), ignoreCounter(0), refPos(_refPos)
{
  empty_space = new Drift;

  info.add("Reference pos.", this->refPos_string());

  if (ignoreFile != "") this->setIgnoreList(ignoreFile);
  this->simToolImport(sim);
}

//copy constructor
AccLattice::AccLattice(const AccLattice &other)
  : circ(other.circumference()), ignoreList(other.ignoreList), refPos(other.refPos), info(other.info)
{
  empty_space = new Drift;

  for (const_AccIterator it=other.getItBegin(); it!=other.getItEnd(); ++it) {
    this->mount(it->first, *(it->second));
  }
}


AccLattice::~AccLattice()
{
  for (AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    delete it->second;
  }
  delete empty_space;
}


AccLattice& AccLattice::operator= (const AccLattice &other)
{
  stringstream msg;

  if (refPos != other.refPos) {
    msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different refPos ("
  	<< refPos_string() <<"/"<< other.refPos_string() <<")";
    throw palatticeError(msg.str());
  }
  // if (circumference() != other.circumference()) {
  //   msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different circumferences ("
  // 	<< circumference() <<"/"<< other.circumference() <<")";
  //   throw palatticeError(msg.str());
  // }
  circ = other.circ;
  ignoreList = other.ignoreList;
  info = other.info;
  
  for (const_AccIterator it=other.getItBegin(); it!=other.getItEnd(); ++it) {
    this->mount(it->first, *(it->second));
  }

  return *this;
}




void AccLattice::setCircumference(double c)
{
  if (c < circ)
    throw std::logic_error("DEBUG: This should not have happened! AccLattice circumference was decreased.");

  circ = c;
  info.add("AccLattice circumference / m", c);
}



// get rotation angle [0,2pi]: increases lin. in bending dipoles, constant in-between. 
double AccLattice::theta(double posIn) const
{
  double theta = 0.;
  // sum theta of all bending dipoles that end is at a pos < posIn
  for (const_AccIterator it=firstCIt(dipole); locate(it,end) < posIn; it=nextCIt(it,dipole)) {
    theta += it->second->length * it->second->k0.z; // theta= l/R = l*k0.z
  }
  // if posIn is inside a dipole, add theta of this magnet up to posIn
  try {
    const_AccIterator atPosIn = getIt(posIn); // throws if there is no element
    if (atPosIn->second->type == dipole) {
      theta += (posIn - locate(atPosIn,begin)) * atPosIn->second->k0.z;
    }
  }
  catch (eNoElement) {}
  return theta;
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
  if (it == elements.end())
    return this->circumference();
  else
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





// get first element of given type (returns iterator to end if there is none)
AccIterator AccLattice::firstIt(element_type _type, element_plane p, element_family f)
{
  for (AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == _type && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
// get last element of given type (returns iterator to end if there is none)
AccIterator AccLattice::lastIt(element_type _type, element_plane p, element_family f)
{
  AccIterator it = elements.end();
  it--;
  for (; it!=elements.begin(); it--) {
    if (it->second->type == _type && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();  
}
// get iterator to next element after pos (returns iterator to end if there is none)
AccIterator AccLattice::nextIt(double posIn, element_plane p, element_family f)
{
  double pos = posMod(posIn);
  for (AccIterator it=elements.upper_bound(pos); it!=elements.end(); ++it) {
    if ((p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
// get iterator to next element of given type after pos (returns iterator to end if there is none)
AccIterator AccLattice::nextIt(double posIn, element_type _type, element_plane p, element_family f)
{
  double pos = posMod(posIn);
  for (AccIterator it=elements.upper_bound(pos); it!=elements.end(); ++it) {
    if (it->second->type == _type && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
// get iterator to next element of given type after it (returns iterator to end if there is none)
AccIterator AccLattice::nextIt(AccIterator it, element_type _type, element_plane p, element_family f)
{
  it++; // check only elements AFTER it
 for (; it!=elements.end(); ++it) {
   if (it->second->type == _type && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();  
}
// get iterator to first element, whose begin/center/end is > pos
// circulating: "next" after "last" is "first", pos > circumference allowed
AccIterator AccLattice::nextIt(double posIn, Anchor anchor)
{
  double pos = posMod(posIn);
  AccIterator b = nextIt(pos);
  AccIterator a = b; a--;
  if (b == elements.end()) b = elements.begin(); // circulating (a="end--" is last element -> correct)
  if (distanceRing(pos,a,anchor) < 0.) return a;
  else if (distanceRing(pos,b,anchor) < 0.) return b;
  else {
    AccIterator c = b; c++;
    if (c == elements.end()) c = elements.begin(); // circulating
    return c;
  }
}
AccIterator AccLattice::revolve(AccIterator it)
{
  it++;
  if (it == elements.end())
    return elements.begin();
  else
    return it;
}


// public const_AccIterator versions of first/last/nextIt
// implemented AGAIN, because:
// - they are const members (can be used in other const members like print())
// - the above versions cannot be const (should allow modification)
// - conversion from const_iterator to iterator is only possible via advance() & distance(), which would be slower
const_AccIterator AccLattice::firstCIt(element_type t, element_plane p, element_family f) const
{
  for (const_AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
const_AccIterator AccLattice::lastCIt(element_type t, element_plane p, element_family f) const
{
 const_AccIterator it = elements.end();
  it--;
  for (; it!=elements.begin(); it--) {
    if (it->second->type == t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();  
}
const_AccIterator AccLattice::nextCIt(double posIn, element_plane p, element_family f) const
{
  double pos = posMod(posIn);
  for (const_AccIterator it=elements.upper_bound(pos); it!=elements.end(); ++it) {
    if ((p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
const_AccIterator AccLattice::nextCIt(double posIn, element_type t, element_plane p, element_family f) const
{
  double pos = posMod(posIn);
 for (const_AccIterator it=elements.upper_bound(pos); it!=elements.end(); ++it) {
    if (it->second->type == t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();
}
const_AccIterator AccLattice::nextCIt(const_AccIterator it, element_type t, element_plane p, element_family f) const
{
  it++; // check only elements AFTER it
 for (; it!=elements.end(); ++it) {
   if (it->second->type == t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      return it;
  }
  return elements.end();  
}
const_AccIterator AccLattice::nextCIt(double posIn, Anchor anchor) const
{
  double pos = posMod(posIn);
  const_AccIterator b = nextCIt(pos);
  const_AccIterator a = b; a--;
  if (b == elements.end()) b = elements.begin(); // circulating (a="end--" is last element -> correct)
  if (distanceRing(pos,a,anchor) < 0.) return a;
  else if (distanceRing(pos,b,anchor) < 0.) return b;
  else {
    const_AccIterator c = b; c++;
    if (c == elements.end()) c = elements.begin(); // circulating
    return c;
  }
}
const_AccIterator AccLattice::revolve(const_AccIterator it) const
{
  it++;
  if (it == elements.end())
    return elements.begin();
  else
    return it;
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



// distance from itRef of element it to pos (>0 if pos is after itRef)
double AccLattice::distance(double pos, const_AccIterator it, Anchor itRef) const
{
  return (pos - locate(it,itRef));
}
// distance from itRef of element it to pos (>0 if pos is after itRef) for a ring.
// both directions are checked, shorter distance is returned.
double AccLattice::distanceRing(double pos, const_AccIterator it, Anchor itRef) const
{
  double d_normal = distance(pos,it,itRef);
  double d_other = circumference() - abs(d_normal);
  if ( d_other >= 0.5*circumference() ) // regular direction shorter
    return d_normal;
  else {
    if (d_normal>0) return - d_other;
    else return d_other;
  }
}
// |distance| from it to next element (circulating)
double AccLattice::distanceNext(const_AccIterator it) const
{
  const_AccIterator next = revolve(it);
  return distanceRing(next->first, it, refPos);
}




// get element
const AccElement* AccLattice::operator[](double pos) const
{
  if (pos > circumference()) {
    stringstream msg;
    msg << pos << " m is larger than lattice circumference " << circumference() << " m.";
    throw palatticeError(msg.str());
  }

  try {
    return getIt(pos)->second;
  }
  // otherwise pos not inside any element:
  catch (eNoElement &e) {
    return empty_space;
  }
}

// get iterator by name, returns lattice end, if name is not found
// ! name can be ambiguous! always returns it. to first matching element
const_AccIterator AccLattice::operator[](string _name) const
{
  for (const_AccIterator it=elements.begin(); it!=elements.end(); it++) {
    if (it->second->name == _name )
      return it;
  }
  // otherwise name does not match any element:
  //return elements.end();
  throw eNoElement("No element "+_name+" found");
}


// mount element (replace if key (pos) already used; check for "free space" to insert element)
// if name is in ignoreList, element is not set and ignoreCounter is increased
void AccLattice::mount(double pos, const AccElement& obj, bool verbose)
{
  //ignoreList
  if ( obj.nameMatch(ignoreList) ) {
    ignoreCounter++;
    //metadata
    stringstream ignore;
    ignore << ignoreCounter;
    info.add("ignored elements", ignore.str());
    return;
  }

  // if (pos > circumference()) {
  //   stringstream msg;
  //   msg << pos << " m is larger than lattice circumference " << circumference() << " m.";
  //   throw palatticeError(msg.str());
  // }

  bool first_element = false;
  bool last_element = false;
  const AccElement *objPtr = &obj;
  double newBegin = locate(pos, objPtr, begin);
  double newEnd = locate(pos, objPtr, end);
  stringstream msg;
  
  if (pos < 0.) {
    stringstream msg;
    msg << "ERROR: AccLattice::mount(): Position of Lattice elements must be > 0. " << pos << " is not." <<endl;
    throw palatticeError(msg.str());
  }

  // empty map (mount first element)
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
  //else if (newEnd > circumference()) {
    // msg << objPtr->name << " (" << newBegin <<" - "<< newEnd <<  "m) cannot be inserted --- overlap with lattice end at " << circumference() << "m";
    // throw eNoFreeSpace(msg.str());
  //}
  else if (!last_element && newEnd > locate(next,begin)) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m) cannot be inserted --- overlap with "
	<< next->second->name << " ("<< locate(next,begin) <<" - " << locate(next,end) << "m)";
    throw eNoFreeSpace(msg.str());
  }
  //if there is free space:
  else elements[pos] = objPtr->clone();

  // update circumference
  if (newEnd > circumference())
    setCircumference(newEnd);

  if (verbose) cout << objPtr->name << " inserted." << endl;

}


// dismount element at position pos
void AccLattice::dismount(double pos)
{
  AccIterator it =  elements.find(pos);

  if (it == elements.end()) {
    cout << "WARNING: AccLattice::dismount(): There is no element at position "<<pos<< " m. Nothing is dismounted." << endl;
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
    throw palatticeFileError(ignoreFile);
  }

  //metadata
  info.add("ignore list", ignoreFile);

  while (!f.eof()) {
    f >> tmp;
    ignoreList.push_back(tmp);
  }

  //metadata
  info.add("ignore list", ignoreFile);
}




// mount elements from MAD-X Lattice (read from twiss-output)
// - arbitrary column order
// - error if needed column(s) missing
// madxFile is:
// * a file that can be executed with madx (mode=online)
// * a madx twiss file (mode=offline)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// MAD-X uses different signs of strengths (k,m)
// ===================================================
void AccLattice::madximport(SimToolInstance &madx)
{
  if (madx.tool != pal::madx)
    throw palatticeError("AccLattice::madximport() is only allowed for SimToolInstance::tool=madx");

  // run MadX
  if (madx.mode == pal::online)
    madx.run();

  string madxTwissFile=madx.lattice();
  
  //get metadata and set circumference
    try {
      info.simToolImport(madx);
      setCircumference(madx.readCircumference());
    } catch(palatticeFileError &e) {
      std::cout << "WARNING: Cannot read circumference & SimTool metadata from " <<std::endl
		<<e.filename << std::endl;
    }

  if (refPos != end)
    cout << "WARNING: AccLattice::madximport(): The input file (MAD-X twiss) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;


  //read columns from file (execute madx if mode=online)
  SimToolTable twi;
  twi = madx.readTable(madxTwissFile, {
      "KEYWORD",
	"NAME",
	"S",
	"L",
	"ANGLE",
	"K1L",
	"K2L",
	"HKICK",
	"VKICK",
	"E1",
	"E2",
	"TILT",
	"APERTYPE",
	"APER_1",
	"APER_2",
	"VOLT",
	"FREQ"
	}
    );


  //mount elements
  AccElement *element;
  double s=0.;
  for (unsigned int i=0; i<twi.rows(); i++) {
    string key = twi.gets(i,"KEYWORD");
    string name = removeQuote(twi.gets(i,"NAME"));
    double l = twi.getd(i,"L");
    double angle = twi.getd(i,"ANGLE");
    double hkick = twi.getd(i,"HKICK");
    double vkick = twi.getd(i,"VKICK");

    if (key == "\"SBEND\"" || key == "\"RBEND\"") {  //horizontal bending Dipole (assume all bends have vertical field)
      element = new Dipole(name, l, H);
    }
    else if (key == "\"QUADRUPOLE\"") {
      element = new Quadrupole(name, l, F);
    }
    else if (key == "\"SEXTUPOLE\"") {
      element = new Sextupole(name, l, F);
    }
    else if (key == "\"VKICKER\"") {
      element = new Corrector(name, l,V);
    }
    else if (key == "\"HKICKER\"") {
      element = new Corrector(name, l,H);
    }
    else if (key == "\"KICKER\"") {
      element = new Corrector(name, l);
    }
    else if (key == "\"RFCAVITY\"") {
      element = new Cavity(name, l);
    }
    else if (key == "\"MULTIPOLE\"") {
      element = new Multipole(name, l);
    }
    else if (key == "\"MARKER\"") {
      element = new Marker(name);
    }
    else if (key == "\"MONITOR\"") {
      element = new Monitor(name, l);
    }
    else if (key == "\"RCOLLIMATOR\"") {
      element = new Rcollimator(name, l);
    }
    else continue; //Drifts are not mounted explicitly
    
    // rf magnet
    // if (twi.gets("NAME",i).substr(0,6) == "RFDIP.") {
    //   element->Qrf0 = 0.625;          // !! hardcoded RF-tune values !!
    //   element->dQrf = 5.402e-6;
    // }

    if (angle!=0.) element->k0.z += angle / l; // 1/R from bending angle, curved length l
    if (hkick!=0. && element->plane!=V)  element->k0.z += sin(hkick) / l; // 1/R from kick angle, straight length l
    if (vkick!=0. && element->plane!=H)  element->k0.x += sin(vkick) / l;
    element->e1 = twi.getd(i,"E1");
    element->e2 = twi.getd(i,"E2");
    if (twi.gets(i,"APERTYPE")=="\"RECTANGLE\"") {
      element->halfWidth.x = twi.getd(i,"APER_1");
      element->halfWidth.z = twi.getd(i,"APER_2");
    }
    if (element->type == cavity) {
      element->volt = twi.getd(i,"VOLT") * 1e6;
      element->freq = twi.getd(i,"FREQ") * 1e6;
    }
    element->k1 = twi.getd(i,"K1L")/l;
    element->k2 = twi.getd(i,"K2L")/l;
    element->dpsi += - twi.getd(i,"TILT"); // non-error tilt (e.g. skew magnets), sign see misalignments
    //misalignments in AccLattice::madximportMisalignments()
    s = twi.getd(i,"S");
    if (refPos == begin) s -= l;
    else if (refPos == center) s -= l/2;
    this->mount(s, *element);
    delete element;
  }

  // import misalignments
  this->madximportMisalignments(dipole, madx.outFile("dipealign"));
  this->madximportMisalignments(quadrupole, madx.outFile("quadealign"));


  //info stdout
  cout << this->sizeSummary() << " read" << endl
       <<"  as " <<this->circumference() << "m lattice from " <<madxTwissFile<<endl;
  if (this->ignoreList.size() > 0) {
    cout <<"* "<<this->ignoredElements()<<" elements ignored due to match with ignore list."<<endl;
  }
}




// set misalignments from MAD-X Lattice (read ealign-output)
// ! currently only rotation (dpsi) around beam axis (s) is implemented   !
// ! to add others: define member in class AccElements & implement import !
// *************************sign of rotation angle:*********************************
// test with influence of dpsi on vertical closed orbit in madx show
// that dpsi is defined counter clockwise (dpsi>0 for dipole => kick to negative z)
// libpalattice and elegant (tilt) use clockwise definition, so sign is changed here
// to get the correct signs for the magnetic fields calculated from dpsi
// *********************************************************************************
void AccLattice::madximportMisalignments(element_type t, string madxEalignFile)
{
  SimToolInstance madx(pal::madx, pal::offline, madxEalignFile);

  //read columns from file (execute madx if mode=online)
  SimToolTable ealign;
  ealign = madx.readTable(madxEalignFile, {"NAME", "DPSI"});

  //set misalignments to AccLattice elements
  AccIterator it=firstIt(t);
  string type=it->second->type_string();
  for (; it!=elements.end(); it=nextIt(it,t)) {
    for (unsigned int i=0; i<ealign.rows(); i++) {
      if (it->second->name == removeQuote(ealign.gets(i,"NAME"))) {
	it->second->dpsi += - ealign.getd(i,"DPSI");    // <<<<<<!!! sign of rotation angle (see comment above)
      }
    }
  }

  //metadata
  info.add("MAD-X " + type + " misalignments from", madxEalignFile);
}





// mount elements from elegant Lattice (read from ascii parameter file ".param")
// misalignments included (no separate function as for MADX)
// >> here the file is not read by SimToolInstance::readTable,
// >> because of different layout of .param file (no column based table!)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// elegant uses different signs of strengths (k,m)
// ===================================================
void AccLattice::elegantimport(SimToolInstance &elegant)
{
  if (elegant.tool != pal::elegant)
    throw palatticeError("AccLattice::elegantimport() is only allowed for SimToolInstance::tool=elegant");

  string elegantParamFile = elegant.lattice();

  // run Elegant
  if (elegant.mode == pal::online)
    elegant.run();

  double s, pos;
  double l, k1, k2, angle, kick, hkick, vkick, tilt, e1,e2,volt,freq; //parameter values
  AccPair halfWidth;
  paramRow row, row_old;
  AccElement *element;
  fstream elegantParam;
  bool firstElement = true;
  string tmp;

  pos=l=k1=k2=angle=kick=tilt=e1=e2=volt=freq=0.;   // initialize param. values
  s = 0.;

  //get metadata and set circumference
  try {
    info.simToolImport(elegant);
    setCircumference(elegant.readCircumference());
  } catch(palatticeFileError &e) {
    std::cout << "WARNING: Cannot read circumference & SimTool metadata from " <<std::endl
	      << e.filename << std::endl;
  }

  if (refPos != end)
    cout << "WARNING: AccLattice::elegantimport(): The input file (elegant parameter) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;

  SimToolTable tab = elegant.readTable(elegant.lattice(), {"ElementName", "ElementParameter", "ParameterValue", "ElementType"});
  
  for (auto i=0u; i<tab.rows(); i++) {
    row.name = tab.gets(i, "ElementName");
    row.type = tab.gets(i, "ElementType");
    row.param = tab.gets(i, "ElementParameter");
    row.value = tab.getd(i, "ParameterValue");

    if (firstElement) {
      row_old = row;
      firstElement = false;
    }

    //mount element if next element reached (=all parameters read)
    if (row.name != row_old.name) {

     if (row_old.type=="CSBEND" || row_old.type=="CSRCSBEND" || row_old.type=="KSBEND" || row_old.type=="NIBEND" || row_old.type=="TUBEND" || row_old.type=="SBEN") {
       element = new Dipole(row_old.name, l, H);
     }
     else if (row_old.type=="QUAD" || row_old.type=="KQUAD") {
       element = new Quadrupole(row_old.name, l, F);
     }
     else if (row_old.type=="SEXT" || row_old.type=="KSEXT") {
       element = new Sextupole(row_old.name, l, F);
     }
     else if (row_old.type=="VKICK") {
       element = new Corrector(row_old.name, l, V);
       if (kick!=0.)  element->k0.x += sin(kick) / l; // 1/R from kick angle, straight length l
     }
     else if (row_old.type=="HKICK") {
       element = new Corrector(row_old.name, l, H);
       if (kick!=0.)  element->k0.z += sin(kick) / l;
     }
     else if (row_old.type=="KICKER") {
       element = new Corrector(row_old.name, l);
       element->k0.x += sin(vkick) / l;
       element->k0.z += sin(hkick) / l;
     }
     else if (row_old.type=="RFCA") {
       element = new Cavity(row_old.name, l);
     }
     else if (row_old.type=="MARK") {
       element = new Marker(row_old.name);
     }
     else if (row_old.type=="MONI") {
       element = new Monitor(row_old.name, l);
     }
     else if (row_old.type=="RCOL") {
       element = new Rcollimator(row_old.name, l);
     }
     else
       element = new Drift(); //Drifts are not mounted explicitly
     
     // rf magnet
     // if (row_old.name.substr(0,6) == "RFDIP.") {
     //   element->Qrf0 = 0.625;          // !! hardcoded RF-tune values !!
     //   element->dQrf = 5.402e-6;
     // }

     if (element->type != drift) {
       if (angle!=0.) element->k0.z += angle / l; // 1/R from bending angle, curved length l
       element->k1 = k1;
       element->k2 = k2;
       element->dpsi = tilt;
       element->e1 = e1;
       element->e2 = e2;
       element->halfWidth = halfWidth;
       if (element->type == cavity) {
	 element->volt = volt;
	 element->freq = freq;
       }
	      
       if (refPos == begin) pos = s-l;
       else if (refPos == center) pos = s-l/2;
       else pos = s; 
       this->mount(pos, *element); // mount element
     }
     delete element;
     // clear param. values to avoid reuse of an old value
     pos=l=k1=k2=angle=kick=hkick=vkick=tilt=e1=e2=volt=freq=0.;
     halfWidth = AccPair();
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
    else if (row.param == "HKICK") hkick = row.value;
    else if (row.param == "VKICK") vkick = row.value;
    else if (row.param == "ETILT") tilt += row.value;
    else if (row.param == "TILT") tilt += row.value;
    else if (row.param == "E1") e1 = row.value;
    else if (row.param == "E2") e2 = row.value;
    else if (row.param == "X_MAX") halfWidth.x = row.value;
    else if (row.param == "Y_MAX") halfWidth.z = row.value;
    else if (row.param == "VOLT") volt = row.value;
    else if (row.param == "FREQ") freq = row.value;
    //... add more parameters here

   row_old = row;
  }
  
  //info stdout
  cout << this->sizeSummary() << " read" << endl
       <<"  as " <<this->circumference() << "m lattice from " <<elegantParamFile<<endl;
  if (this->ignoreList.size() > 0) {
    cout <<"* "<<this->ignoredElements()<<" elements ignored due to match with ignore list."<<endl;
  }
}







// change quad&sext strengths to values from "ELSA-Spuren"
void AccLattice::setELSAoptics(string spurenFolder)
{
  char filename[1024];
  string tmp;
  stringstream msg;
  double kf=0, kd=0, mf=0, md=0;
  fstream f_magnets;
  AccIterator it;

 // Read "optics.dat"
  snprintf(filename, 1024, "%s/optics.dat", spurenFolder.c_str());
  f_magnets.open(filename, ios::in);
  if (!f_magnets.is_open()) {
    msg << "ERROR: AccLattice::setELSAoptics(): Cannot open " << filename;
    throw palatticeError(msg.str());
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
      throw palatticeError(msg.str());
    }
  }
  f_magnets.close();

  //Write strengths to quads
  for (it=firstIt(quadrupole); it!=elements.end(); it=nextIt(it,quadrupole)) {
    if (it->second->name.compare(1,2,"QF") == 0) {
      it->second->k1 = kf;
    }
    else if (it->second->name.compare(1,2,"QD") == 0) {
      it->second->k1 = -kd;
    }
  }

  //Write strengths to sexts
  for (it=firstIt(sextupole); it!=elements.end(); it=nextIt(it,sextupole)) {
    if (it->second->name.compare(1,2,"SF") == 0) {
      it->second->k2 = mf;
    }
    else if (it->second->name.compare(1,2,"SD") == 0) {
      it->second->k2 = -md;
    }
  }

 //metadata
 info.add("ELSA optics from", spurenFolder);
}



// change corrector pos&strength to values from "ELSA-Spuren" at time t/ms
// return number of correctors, where set
unsigned int AccLattice::setELSACorrectors(ELSASpuren &spuren, unsigned int t)
{
  unsigned int i, n=0;
 AccElement* corrTmp;
 char name1[20], name2[20];
 char msg[1024];
 stringstream strMsg;
 double diff=0;
 double endPos;
 AccIterator it = firstIt(corrector,V); // only vertical correctors!
 AccIterator it_next;

 
 for (i=0; i<NVCORRS; i++) {
   if (spuren.vcorrs[i].pos==0.0) {   //inactive correctors have pos=0 in VCORRS.SPOS
     continue;
   }
   else if (t > spuren.vcorrs[i].time.size()) {
     snprintf(msg, 1024, "ERROR: AccLattice::setELSACorrectors(): No ELSA VC%02d corrector data available for %d ms.\n", i+1, t);
     throw palatticeError(msg);
   }
   
   //same corrector in Mad-X and ELSA-Spuren?
   //...check by name
   snprintf(name1, 20, "KV%02i", i+1); // old madx-lattice: KVxx
   snprintf(name2, 20, "VC%02i", i+1); // new madx-lattice (2014): VCxx
   if (it->second->name != name1 && it->second->name != name2) {
     strMsg << "ERROR: AccLattice::setELSACorrectors(): Unexpected corrector name. Mad-X lattice does not fit to ELSA." << endl;
     strMsg << "       Mad-X: " <<it->second->name<< " -- expected: " <<name2<< " (" <<name1<< ")" << endl;
     throw palatticeError(strMsg.str());
   }
   //...check by position
   diff = spuren.vcorrs[i].pos - locate(it,center);
   if (fabs(diff) > VCPOS_WARNDIFF) {
     cout << "! Position of " <<name2<< " differs by " <<diff<< "m in Mad-X and ELSA-Spuren. Use ELSA-Spuren." << endl;
   }
   
   corrTmp = it->second->clone();
   corrTmp->k0.x = spuren.vcorrs[i].time[t].kick/1000.0/corrTmp->length;   //unit 1/m
   it_next = nextIt(it,corrector,V);  // only vertical correctors!
   elements.erase(it);   // erase "old" corrector (madx) to be able to mount new one
   endPos = spuren.vcorrs[i].pos + corrTmp->length/2;
   this->mount(endPos, *(corrTmp));
   delete corrTmp;

   it = it_next;
   n++;
   if (it == elements.end())
     break;
 }
 
 if (n != this->size(corrector,V)) // only vertical correctors!
   cout << "WARNING: Not all correctors overwritten by ELSA-Spuren" << endl;

 //metadata
 stringstream spureninfo;
 spureninfo << t << "ms in "<<spuren.spurenFolder;
 info.add("ELSA VCs from", spureninfo.str());
 
 return n;
}




// subtract other corrector strengths from the ones of this lattice
void AccLattice::subtractCorrectorStrengths(AccLattice &other)
{
  stringstream msg;
  AccIterator it;
  const_AccIterator otherIt;
  
  if (this->size(corrector) != other.size(corrector)) {
    msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal number of correctors to subtract.";
    throw palatticeError(msg.str());
  }
  
  otherIt = other.firstCIt(corrector);

  for  (it=firstIt(corrector); it!=lastIt(corrector); it=nextIt(it,corrector)) {

    // check by name
    if (otherIt->second->name != it->second->name) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal names of correctors to subtract. ("
	  << it->second->name <<"/"<< otherIt->second->name << ").";
      throw palatticeError(msg.str());
    }
    // check by position
    if (otherIt->first != it->first) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal positions of correctors to subtract. ("
	  << it->first <<"/"<< otherIt->first << ").";
      throw palatticeError(msg.str());
    }
    // check plane
    if (otherIt->second->plane != it->second->plane) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal planes of correctors to subtract.";
      throw palatticeError(msg.str());
    }

    // subtract
    it->second->k0 -= otherIt->second->k0;
    // set otherIt to next corrector
    otherIt=nextCIt(otherIt,corrector);
  }

  //metadata
  string tmp = other.info.getbyLabel("Lattice Source file");
  if (tmp != "NA")
    info.add("subtracted corrector strengths", tmp);
  else
    info.add("subtracted corrector strengths", "no lattice source file");
}


// subtract other misalignments from the ones of this lattice
void AccLattice::subtractMisalignments(const AccLattice &other)
{
  stringstream msg;
  AccIterator it;
  const_AccIterator otherIt;
  
  if (this->size() != other.size()) {
    msg << "ERROR: AccLattice::subtractMissalignments(): Unequal number of elements to subtract.";
    throw palatticeError(msg.str());
  }
  
  otherIt = other.getItBegin();

  for  (it=elements.begin(); it!=elements.end(); ++it) {

    // check by name
    if (otherIt->second->name != it->second->name) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal names of elements to subtract. ("
	  << it->second->name <<"/"<< otherIt->second->name << ").";
      throw palatticeError(msg.str());
    }
    // check by position
    if (otherIt->first != it->first) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal positions of elements to subtract. ("
	  << it->first <<"/"<< otherIt->first << ").";
      throw palatticeError(msg.str());
    }

    // subtract
    it->second->dpsi -= otherIt->second->dpsi;
    // set otherIt to next corrector
    otherIt++;
  }

  //metadata
  string tmp = other.info.getbyLabel("Lattice Source file");
  if (tmp != "NA")
    info.add("subtracted misalignments", tmp);
  else
    info.add("subtracted misalignments", "no lattice source file");
}



// ------------------ "information" -----------------------
// returns number of elements of a type in this lattice
unsigned int AccLattice::size(element_type _type, element_plane p, element_family f) const
{
  unsigned int n=0;
  for (const_AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    if (it->second->type == _type && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f))
      n++;
  }

  return n;
}


string AccLattice::sizeSummary() const
{
  stringstream s;
  s << "* "<<this->size(dipole)<<" dipoles, "<<this->size(quadrupole)<<" quadrupoles, "
    <<this->size(sextupole)<<" sextupoles, "<<this->size(corrector)<<" correctors, "
    <<this->size(cavity)<<" cavities, "<<this->size(multipole)<<" multipoles";
  return s.str();
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


double AccLattice::slope(double pos, const_AccIterator it) const
{
  double x=1.;
  double dl = it->second->dl()/2; // half of dl() at each magnet end
  double distBegin = distanceRing(pos,it,begin) - dl; // distance to phys. begin of it
  double distEnd = distanceRing(pos,it,end) + dl;     // distance to phys. end of it
  if (distBegin < 0) x = distBegin;
  else if (distEnd > 0) x = distEnd;
  else return 1.;

  double sigma = dl * sqrt(2./M_PI); // dl * 0.797884561
  return exp(-0.5*pow(x/sigma,2));
}

// magnetic field including edge field (with slope)
AccTriple AccLattice::B(double posIn, const AccPair &orbit) const
{
  double pos = posMod(posIn);
  unsigned int t = turn(posIn);
  // next magnet with center > pos:
  const_AccIterator it = nextCIt(pos,center);
  AccTriple field;
  field = it->second->B_rf(t,orbit) * slope(pos, it); //B_rf includes rf magnets
  // previous magnet (center <= pos):
  if (it == elements.begin()) it = elements.end();
  it--;                                       
  field += it->second->B_rf(t,orbit) * slope(pos, it);

  return field;
}


// ----------- output (stdout or file) ----------------------

// print lattice. If no filename is given, print to stdout
void AccLattice::print(string filename) const
{
  const_AccIterator it=elements.begin();
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << info.out("#");
  // NO! s <<"# (* unit of Strength depends on magnet type! e.g. Quad: k1, Sext: k2, Dipole: 1/R)" <<endl;
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it->second->printHeader();

  for (; it!=elements.end(); ++it) {
    s <<std::setw(w+1)<< it->first << it->second->print();
  }

  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
      throw palatticeFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}


// print all elements of one type If no filename is given, print to stdout
void AccLattice::print(element_type _type, string filename) const
{
  const_AccIterator it=firstCIt(_type);
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << info.out("#");
  s << "# List of " << it->second->type_string() << "s only!" << endl;
  // NO! s <<"# (* unit of Strength depends on magnet type! e.g. Quad: k1, Sext: k2, Dipole: 1/R)" <<endl;
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it->second->printHeader();

  for (; it!=lastCIt(_type); it=nextCIt(it,_type)) {
    s <<std::setw(w+1)<< it->first << it->second->print();
  }

  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
     throw palatticeFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}



// return list of elegant or madx compliant element definitions for given type
string AccLattice::getElementDefs(SimTool tool, element_type _type) const
{
  stringstream s;
  const_AccIterator it=firstCIt(_type);
  const_AccIterator firstOccur;

  if (it == elements.end())
    return "";
  s << "! " << it->second->type_string() << "s";
  if (_type==dipole && tool==elegant)
    s <<" (synch_rad & isr for synchrotron radiation)";
  s << endl;
  for (; it!=elements.end(); it=nextCIt(it, _type)) {
    //only list identical elements once
    firstOccur = operator[](it->second->name); //first element in lattice with this name
    if ((it->first-firstOccur->first) > ZERO_DISTANCE && *(firstOccur->second) == *(it->second)) {
      //debug
      // std::cout << "EXPORT-DEBUG: ignored element " << it->second->name << " @ " << it->first << "m," << std::endl
      // 		<< "because it's not the first occurrence (" << firstOccur->first << "m)" << std::endl; 
    }
    else { 
      s << it->second->printSimTool(tool);
    } 
  }
  s << endl;

  return s.str();
}


// return lattice in elegant or madx compliant "LINE=(..." format
// including definition of drifts in-between elements
string AccLattice::getLine(SimTool tool) const
{
  std::stringstream s;    //drift definitions
  std::stringstream line; //lattice line 
  double driftlength, lastend;
  const_AccIterator it=elements.begin();

  line << "LIBPALATTICE : LINE=(";

  //marker at pos=0.0 if lattice starts with drift
  if (it->first > ZERO_DISTANCE || it->second->type != marker) {
    s << "BEGIN : ";
    if (tool==elegant)
      s << "MARK";
    else
      s << "MARKER;";
    s << endl << endl;
    line << "BEGIN, ";
  }
  s << "! Drifts (caculated from element positions)" << endl; //Drifts


  unsigned int n=0u, nInRow=1u;
  for (; it!=elements.end(); ++it) {
    //comma
    if (it != elements.begin()) line << ", ";
    
    //line break
    if (nInRow >= EXPORT_LINE_ELEMENTS_PER_ROW) {
      if (tool==elegant)
	line <<"&";
      line << std::endl << "                    ";
      nInRow = 0;
    }

    //calc, define and insert Drift
    if (it == elements.begin()) {
      driftlength = locate(it, begin);
    }
    else {
      driftlength = locate(it, begin) - lastend;
    }
    lastend = locate(it, end);
    if (driftlength > ZERO_DISTANCE) { //ignore very short drifts
      s << "DRIFT_" << n << " : ";
      if (tool==elegant)
	s << "DRIF, l=";
      else
	s << "DRIFT, L=";
      s << driftlength <<";"<< endl;  //drift definition
      line << "DRIFT_" << n << ", ";  //insert drift in line
      nInRow++;
      n++;
    }
    
    //insert element
    line << it->second->name;
    nInRow++;
  }//loop elements in lattice
  
  //final drift to end
  if ( (circumference() - lastend) > ZERO_DISTANCE ) { //ignore very short drift
    s << "DRIFT_" << n << " : ";
    if (tool==elegant)
      s << "DRIF, l=";
    else
      s << "DRIFT, L=";
    s << this->circumference() - lastend <<";"<<endl;
    line << "DRIFT_" << n;   // drift to end
  }

  s << endl<<endl << line.str() << ")";
  if (tool==madx)
    s << ";";

  return s.str();
}


// return lattice in madx compliant "SEQUENCE" format
string AccLattice::getSequence(Anchor refer) const
{
  std::stringstream s;

  s << "LIBPALATTICE : SEQUENCE, REFER=";
  if (refer==center)
    s << "CENTRE, ";
  else if (refer==begin)
    s << "ENTRY, ";
  else if (refer==end)
    s << "EXIT, ";
  s << "L=" << this->circumference() <<";"<< endl;

    const_AccIterator it=elements.begin();
    
    //marker at pos=0.0 if lattice starts with drift
    if (it->first > ZERO_DISTANCE || it->second->type != marker)
      s << "BEGIN : MARKER, AT=0.0;" << endl;

  for (; it!=elements.end(); ++it) {
    s << it->second->name << ", AT=" << this->locate(it, refer) << ";" << endl;
  }
  s << "END : MARKER, AT=" <<this->circumference() <<";"<< endl;
  s << "ENDSEQUENCE;";

  return s.str();
}


// print lattice readable by elegant or madx. If no filename is given, print to stdout
void AccLattice::simToolExport(SimTool tool, string filename, MadxLatticeType ltype) const
{
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  if (tool==elegant)
    s << "! Lattice for ELEGANT" << endl;
  else
    s << "! Lattice for MAD-X" << endl;
  s << info.out("!") << endl;


  //write element definitions
  for (element_type type=dipole; type<drift; type=element_type(type+1)) {
    s << this->getElementDefs(tool,type);
  }
  s << endl;

  //write lattice
  if (tool == madx && ltype == sequence) {
    s << getSequence();
  }
  else {
    s << getLine(tool);
  }
  


  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open()) {
      msg << "ERROR: AccLattice::elegantexport(): Cannot open " << filename << ".";
      throw palatticeError(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }
}




// print lattice readable by LaTeX. If no filename is given, print to stdout
// (using package tikz-palattice)
void AccLattice::latexexport(string filename) const
{
  const_AccIterator it=elements.begin();
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << "% Lattice for LaTeX" << endl
    << "%" << endl;
  s << info.out("%")
    << endl;

  //preamble
  s << "\\documentclass[]{standalone}" <<endl
    << "\\usepackage[ngerman]{babel}" <<endl
    << "\\usepackage[utf8]{inputenc}" <<endl
    << "\\usepackage{tikz-palattice} % available at CTAN, included in TeXlive and MikTeX" <<endl<<endl;

  //lattice
  s << "\\begin{document}" << endl << "\\begin{lattice}" << endl;
  double driftlength, lastend;
  it=elements.begin();
  for (; it!=elements.end(); ++it) {
    if (it == elements.begin()) {
      driftlength = locate(it, begin);
    }
    else {
      driftlength = locate(it, begin) - lastend;
    }
    lastend = locate(it, end);
    s << getLaTeXDrift(driftlength);    //drift
    s << it->second->printLaTeX(); //element
  }

  //drift to end
  driftlength = circumference() - lastend;
  s << getLaTeXDrift(driftlength);

  s << "\\end{lattice}" << endl << "\\end{document}" << endl;


  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open()) {
      msg << "ERROR: AccLattice::latexexport(): Cannot open " << filename << ".";
      throw palatticeError(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }
}



//energy loss per turn in keV for electron beam with energy given by gamma
double AccLattice::Erev_keV_syli(const double& gamma) const
{
  double dE = 0;
  for(auto it=firstCIt(dipole); it!=getItEnd(); it=nextCIt(it, dipole)) {
    dE += std::pow(it->second->k0.abs(), 2) * it->second->length;
  }
  dE *= std::pow(gamma, 4) * GSL_CONST_MKSA_ELECTRON_CHARGE / (6*M_PI*GSL_CONST_MKSA_VACUUM_PERMITTIVITY); // in eV
  return std::move(dE/1000.); //in keV
}

// overvoltage factor q, from total voltage of all cavities
double AccLattice::overvoltageFactor(const double& gamma) const {
  double U = 0;
  for(auto it=firstCIt(cavity); it!=getItEnd(); it=nextCIt(it, cavity)) {
    U += it->second->volt;
  }
  return U / (Erev_keV_syli(gamma) * 1000.);
}


// integral over bending radius around ring: R^exponent ds
double AccLattice::integralDipoleRadius(int exponent) const
{
  double sum = 0;
  double totalLength = 0;
  for(auto it=firstCIt(dipole); it!=getItEnd(); it=nextCIt(it, dipole)) {
    sum += std::pow(it->second->k0.abs(), -1*exponent) * it->second->length;
    totalLength += it->second->length;
  }
  return sum / totalLength;
}

// harmonic number h, from cavity frequency and circumference
unsigned int AccLattice::harmonicNumber() const {
  auto it=firstCIt(cavity);
  double freq = it->second->freq;
  
  for(; it!=getItEnd(); it=nextCIt(it, cavity)) {
    if (it->second->freq != freq)
      throw palatticeError("harmonicNumber(): Cavities with different frequencies in Lattice. What is the definition of h in this case??");
  }

  return (unsigned int) std::round( freq / (GSL_CONST_MKSA_SPEED_OF_LIGHT/circumference()) );
}
