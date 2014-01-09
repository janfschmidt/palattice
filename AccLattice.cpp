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

// get last element of given type
// returns iterator to end if there is none
AccIterator AccLattice::lastIt(element_type _type) 
{
  for (AccIterator it=elements.end(); it!=elements.begin(); it--) {
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


// erase element at position pos
void AccLattice::erase(double pos)
{
  try {
    const_AccIterator it = getIt(pos);
  }
  catch (eNoElement &e) {
    cout << "WARNING: AccLattice::erase(): There is no element at position "<<pos<< " m. Nothing is erased." << endl;
    return;
  }
  elements.erase(it);
}



// set elements from MAD-X Lattice (read from twiss-output)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// MAD-X uses different signs of strengths (k,m)
// ===================================================
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


// set misalignments from MAD-X Lattice (read ealign-output)
// ! currently only rotation (dpsi) around beam axis (s) is implemented   !
// ! to add others: define member in class AccElements & implement import !
void AccLattice::madximportMisalignments(char *madxEalignFile)
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
	  it->second->dpsi = _dpsi; 
	  getline(madxEalign, tmp);
	  break;
	}
      }
    }

  }

}




// change quad&sext strengths to values from "ELSA-Spuren"
void AccLattice::setELSAoptics(char *spurenFolder)
{
  char filename[1024];
  string tmp;
  double kf=0, kd=0, mf=0, md=0;
  fstream f_magnets;
  AccIterator it;

 // Read "optics.dat"
  snprintf(filename, 1024, "%s/optics.dat", spurenFolder);
  f_magnets.open(filename, ios::in);
  if (!f_magnets.is_open()) {
    throw std::runtime_error("ERROR: AccLattice::setELSAoptics(): Cannot open %s\n", filename);
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
    else
      throw std::runtime_error("ERROR: AccLattice::setELSAoptics(): Unexpected entry in %s\n", filename);
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



// change corrector pos&strength to values from "ELSA-Spuren" at time t(ms
void AccLattice::setELSACorrectors(CORR *ELSAvcorrs, unsigned int t)
{
 unsigned int i, j=0;
 Corrector corrTmp("default name", 0.);
 char name[20];
 char msg[1024];
 stringstream strMsg;
 double diff=0;
 AccIterator it = firstIt(corrector);

 
 for (i=0; i<NVCORRS; i++) {
   if (ELSAvcorrs[i].pos==0.0) {   //inactive correctors have pos=0 in VCORRS.SPOS
     continue;
   }
   else if (t > ELSAvcorrs[i].time.size()) {
     snprintf(msg, 1024, "ERROR: AccLattice::setELSACorrectors(): No ELSA VC%02d corrector data available for %d ms.\n", i+1, t);
     throw std::invalid_argument(msg);
   }
   
   //same corrector in Mad-X and ELSA-Spuren?
   //...check by name
   snprintf(name, 20, "\"KV%02i\"", i+1);
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
   
   // corrTmp.name = name;
   // corrTmp.length = it->second->length;
   // corrTmp.dpsi = it->second->dpsi;
   corrTmp = *(it->second);

   corrTmp.strength = ELSAvcorrs[i].time[t].kick/1000.0/corrTmp.length;   //unit 1/m
   this->set(ELSAvcorrs[i].pos, corrTmp);

   it = nextIt(corrector,it);
   j++;
   if (it == elements.end())
     break;
 }
 
 if (j != this->size(correctors))
   cout << "WARNING: Not all correctors overwritten by ELSA-Spuren" << endl;
 
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

