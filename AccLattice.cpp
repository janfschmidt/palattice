/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X.
 * Uses the "AccElements" classes
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "AccLattice.hpp"


AccLattice::AccLattice(double _circumference, Anchor _refPos)
  : refPos(_refPos), circumference(_circumference)
{
  empty_space = new Drift;
}

AccLattice::~AccLattice()
{
  for (AccIterator it=elements.begin(); it!=elements.end(); ++it) {
    delete it->second;
  }
  delete empty_space;
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




// get const_Iterator
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

  if (refPos == end || refPos == center) {
    if (inside(candidate_next, pos))
      return candidate_next;
  }

  throw eNoElement();
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
void AccLattice::set(double pos, const AccElement& obj)
{
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
    cout << objPtr->name << " inserted." << endl;
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
  cout << objPtr->name << " inserted." << endl;

}




// set elements from MAD-X Lattice (read from twiss-output)
void AccLattice::madximport(char *madxTwissFile)
{
  // madx column variables
  string tmp;
  double s, angle, k1l, k2l, vkick; 
  string x, y;

  fstream madxTwiss;

  //AccElements
  Dipole vDip("defaultName", 0., 0., V);
  Dipole hDip("defaultName", 0., 0., H);
  Quadrupole Quad("defaultName", 0.);    // madx uses negative sign "strength" for D magnets,
  Sextupole Sext("defaultName", 0.);     // so here all Quads/Sexts are defined as family F (also see AccElements.hpp)

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
  
    if (tmp == "\"SBEND\"" || tmp == "\"RBEND\"") {  //vertical Dipole (assume all bends have vertical field)
      madxTwiss >> vDip.name >> s >> x >> y >> vDip.length >> angle;
      vDip.strength = angle/vDip.length;   // 1/R (!!! assuming l is arclength (along ref. orbit) !!!)
      if (refPos == begin) s -= vDip.length;
      else if (refPos == center) s -= vDip.length/2;
      this->set(s, vDip);
    }
    else if (tmp == "\"QUADRUPOLE\"") {
      madxTwiss >> Quad.name >> s >> x >> y >> Quad.length >> angle >> k1l;
      Quad.strength = k1l/Quad.length;   // k
      if (refPos == begin) s -= Quad.length;
      else if (refPos == center) s -= Quad.length/2;
      this->set(s, Quad);
      // BPMs at quads - Closed orbit not included in lattice, read in separate function of Orbit-class
    }
    else if (tmp == "\"SEXTUPOLE\"") {
      madxTwiss >> Sext.name >> s >> x >> y >> Sext.length >> angle >> k1l >> k2l;
      Sext.strength = k2l/Sext.length;
      if (refPos == begin) s -= Sext.length;
      else if (refPos == center) s -= Sext.length/2;
      this->set(s, Sext);
    }
    else if (tmp == "\"VKICKER\"") {
      madxTwiss >> vDip.name >> s >> x >> y >> vDip.length >> angle >> k1l >> k2l >> vkick;
      vDip.strength = sin(vkick)/vDip.length;   // 1/R from kick-angle
      if (refPos == begin) s -= vDip.length;
      else if (refPos == center) s -= vDip.length/2;
      this->set(s, vDip);
    }

  }
  madxTwiss.close();
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


// print lattice to stdout
void AccLattice::print() const
{
  const_AccIterator it;
  AccElement* obj;
  const int w = 12;

  cout << "# Reference Position: " << refPos_string() << endl;
  cout <<"#"<< std::setw(w) << "Name" <<std::setw(w)<< "Ref.Pos/m" <<std::setw(w)<< "Begin/m" <<std::setw(w)<< "End/m" << endl;

  for (it=elements.begin(); it!=elements.end(); ++it) {
    obj = it->second;
    cout <<std::setw(w+1)<< obj->name <<std::setw(w)<< it->first <<std::setw(w)<< locate(it, begin) <<std::setw(w)<< locate(it, end) << endl;
  }

}

