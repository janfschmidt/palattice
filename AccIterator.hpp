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

  class AccLatticeIterator {
    friend class AccLattice;

  protected:
    std::map<double, AccElement*>::iterator it;
    AccLatticeIterator(std::map<double, AccElement*>::iterator in) : it(in) {}

  public:
    AccLatticeIterator(const AccLatticeIterator& other) {it = other.it;}
    ~AccLatticeIterator() {}
    
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
   bool operator==(const AccLatticeIterator& other) const {if(other.it==it) return true; else return false;}
   bool operator!=(const AccLatticeIterator& other) const {return !operator==(other);}
  };
  
} //namespace pal

#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
