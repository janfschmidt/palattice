/* AccIterator Classes
 * iterators to access elements in an accelerator lattice and the position, where they are mounted.
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
 *
 *
 * AccIterator allows to iterate / loop through the lattice and perform calculations
 * regarding their position (distances etc.).
 * All these iterators are defined as inner classes within the "AccLattice" class and
 * are accessible via the AccLattice::begin() and AccLattice::end() methods.
 *
 * There are four iterator types intended for external usage:
 * AccLattice::iterator & AccLattice::const_iterator
 * -> iterate over all elements in lattice
 * AccLattice::type_iterator & AccLattice::const_type_iterator
 * -> iterate over all elements of one type in lattice
 */

#ifndef __LIBPALATTICE_ACCITERATOR_HXX_
#define __LIBPALATTICE_ACCITERATOR_HXX_


//
// template implementations
//


template<bool IS_CONST>
void AccLattice::AccIterator_Base<IS_CONST>::checkForEnd() const
{
  if (isEnd())
    throw palatticeError("Evaluation of lattice.end(), which is after last Element!");
}



// iteration helper functions
template<bool IS_CONST>
void AccLattice::AccIterator_Base<IS_CONST>::next_helper(element_type t, element_plane p, element_family f)
{
  ++it;
  for (; it!=latticeElements->end(); ++it) {
    if ( it->second->type==t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f) )
      break;
  }
  //throw noMatchingElement("type, plane, family");
}

template<bool IS_CONST>
void AccLattice::AccIterator_Base<IS_CONST>::prev_helper(element_type t, element_plane p, element_family f)
{
  --it;
  for (; it!=latticeElements->begin(); --it) {
    if ( it->second->type==t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f) )
      break;
  }
  //throw noMatchingElement("type, plane, family");
}

template<bool IS_CONST>
void AccLattice::AccIterator_Base<IS_CONST>::revolve_helper()
{
  if (it == latticeElements->end()) {
    setBegin();
  }
}




// position calculations
template<bool IS_CONST>
double AccLattice::AccIterator_Base<IS_CONST>::pos(Anchor anchor) const
{
  checkForEnd();

  if (anchor==*latticeRefPos)
    return it->first;

  double pos = it->first;
  double l = it->second->length;
  if (*latticeRefPos == Anchor::center) pos -= l/2;
  else if (*latticeRefPos == Anchor::end) pos -= l;

  switch(anchor) {
  case Anchor::begin:
    return pos;
  case Anchor::center:
    return pos + l/2;
  case Anchor::end:
    return pos + l;
  }

  return 0.;
}

template<bool IS_CONST>
bool AccLattice::AccIterator_Base<IS_CONST>::at(double pos) const
{
  if (pos>=begin() && pos<=end())
    return true;
  else
    return false;
}

// distance from anchor of element to pos (>0 if pos is after anchor)
template<bool IS_CONST>
double AccLattice::AccIterator_Base<IS_CONST>::distance(Anchor anchor, double pos) const
{
  return pos - this->pos(anchor);
}

// both directions are checked, shorter distance is returned.
template<bool IS_CONST>
double AccLattice::AccIterator_Base<IS_CONST>::distanceRing(Anchor anchor, double pos) const
{
  double d_normal = distance(anchor,pos);
  double d_other = *latticeCircumference - abs(d_normal);
  if ( d_other >= 0.5 * *latticeCircumference ) // regular direction shorter
    return d_normal;
  else {
    if (d_normal>0)
      d_other *= -1;
    return d_other;
  }
}



#endif /*__LIBPALATTICE_ACCITERATOR_HXX_*/
