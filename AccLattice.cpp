/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X.
 * Uses the "AccElements" classes
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include "AccLattice.hpp"


AccLattice::AccLattice(Position _refPos)
  : refPos(_refPos)
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



// get position (in meter) of here=begin/center/end of lattice element "it"
// works for all Reference Positions (refPos, it->first)
double AccLattice::locate(const_AccIterator it, Position here) const
{
  double pos = it->first;        //reference position
  double l = it->second->length; //element length

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



// test if position pos is inside lattice element "it"
  bool AccLattice::inside(double pos, const_AccIterator it) const
{
  if (pos >= locate(it,begin) && pos <= locate(it,end))
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
      if (inside(pos, candidate_previous))
	return candidate_previous;
    }
  }

  if (refPos == end || refPos == center) {
    if (inside(pos, candidate_next))
      return candidate_next;
  }

  throw eNoElement();
}



// get element
const AccElement* AccLattice::operator[](double pos) const
{
  try {
    return getIt(pos)->second;
  }
  // otherwise pos not inside any element:
  catch (eNoElement &e) {
    return empty_space;
  }
}



// set element (replace if key (pos) already used)
void AccLattice::set(double pos, const AccElement& obj)
{
  /*
  try{
    getIt(pos);
    getIt(pos - obj.length/2); // je nach refPos: auch begin/end of obj not within existing element....
    // .....................
  }
  catch (eNoElement &e) { // pos not within an existing element
    
  }
  */
  try{
    delete elements.at(pos);
  }
  catch(std::out_of_range &e){ }

  elements[pos] = obj.clone();
}



