/* === AccIterator Class ===
 * a special iterator to access elements in an accelerator lattice and the position, where they are mounted.
 * It also allows to iterate / loop through the lattice.
 * Used by the "AccLattice" class
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2016
 */


#ifndef __LIBPALATTICE_ACCITERATOR_HPP_
#define __LIBPALATTICE_ACCITERATOR_HPP_

#include <map>
#include "AccElements.hpp"

namespace pal {

  enum class Anchor{begin,center,end};
  

  class AccLatticeIterator {
    friend class AccLattice;
    
  protected:
    std::map<double, AccElement*>::iterator it;
    std::map<double, AccElement*>* latticeElements;
    const Anchor* latticeRefPos;
    const double* latticeCircumference;
    AccLatticeIterator(std::map<double, AccElement*>::iterator in, std::map<double, AccElement*>* e, const Anchor* rP, const double* circ) : it(in), latticeElements(e), latticeRefPos(rP), latticeCircumference(circ) {}
    void checkForEnd() const;

  public:
    // accessors
    double pos() const;                                      // position in Lattice in meter
    const AccElement* element() const;                       // pointer to Element
    AccElement* elementModifier() const;                     // pointer to Element, edit allowed
    AccLatticeIterator& operator*() {return *this;}          // get AccLatticeIterator in range for-loop over AccLattice
    
    // iteration
    AccLatticeIterator& operator++() {it++; return *this;}   //prefix
    AccLatticeIterator operator++(int) {it++; return *this;} //postfix
    AccLatticeIterator& operator--() {it--; return *this;}   //prefix
    AccLatticeIterator operator--(int) {it--; return *this;} //postfix
    AccLatticeIterator& revolve();                           // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccLatticeIterator& next() {return operator++();}
    AccLatticeIterator& previous() {return operator--();}
    AccLatticeIterator& next(element_type t, element_plane p=noplane, element_family f=nofamily);
    AccLatticeIterator& previous(element_type t, element_plane p=noplane, element_family f=nofamily);
    
    // comparison
    bool operator==(const AccLatticeIterator& o) const {if(o.it==it) return true; else return false;}
    bool operator!=(const AccLatticeIterator& o) const {return !operator==(o);}

    // position calculations
    double pos(Anchor anchor) const;                         // get position of "anchor" of this element in meter
    double begin() const {return pos(Anchor::begin);}        // get begin of this element in meter
    double center() const {return pos(Anchor::center);}      // get center of this element in meter
    double end() const {return pos(Anchor::end);}            // get end of this element in meter
    bool at(double pos) const;                               // test, if position "pos" is within this element
    double distance(Anchor anchor, double pos) const;        // get distance from "anchor" of this element to position "pos" in meter (>0 if pos is after/upstream anchor)
    double distanceRing(Anchor anchor, double pos) const;    // distance() for rings: considers both directions and returns shortest distance
    double distanceNext(Anchor anchor) const;                // absolute value of distance from "anchor" of this element to "anchor" of next element. For last element: distance to first element (in a ring)
  };

  
// exceptions
  class noMatchingElement : public pal::palatticeError {
  public:
    noMatchingElement(std::string elementDescription)
      : palatticeError("No matching element found (" + elementDescription + ")") {}
  };


} //namespace pal

#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
