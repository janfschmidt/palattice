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
    double pos() const {return it->first;}
    const AccElement* element() const {return it->second;}
    AccElement* elementModifier() const {return it->second;}
    
    // iteration
    AccLatticeIterator& operator++() {it++; return *this;}   //prefix
    AccLatticeIterator operator++(int) {it++; return *this;} //postfix
    AccLatticeIterator& next() {return operator++();}
    AccLatticeIterator& next(element_type t);
    AccLatticeIterator& operator--() {it--; return *this;}   //prefix
    AccLatticeIterator operator--(int) {it--; return *this;} //postfix
    AccLatticeIterator& previous() {return operator--();}
    AccLatticeIterator& previous(element_type t);
    AccLatticeIterator& revolve();
    
    // comparison
    bool operator==(const AccLatticeIterator& o) const {if(o.it==it) return true; else return false;}
    bool operator!=(const AccLatticeIterator& o) const {return !operator==(o);}

    // position calculations
    double pos(Anchor anchor) const;
    double begin() const {return pos(Anchor::begin);}
    double center() const {return pos(Anchor::center);}
    double end() const {return pos(Anchor::end);}
    bool at(double pos) const;
    double distance(Anchor anchor, double pos) const;
    double distanceRing(Anchor anchor, double pos) const;
    double distanceNext(Anchor anchor) const;
  };
  
} //namespace pal

#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
