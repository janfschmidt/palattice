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
  typedef std::map<double,AccElement*> AccMap;

  
  template <bool IS_CONST>
  class AccLatticeIterator {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;
    
  protected:
    IteratorType it;
    MapType* latticeElements;
    const Anchor* latticeRefPos;
    const double* latticeCircumference;
    AccLatticeIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : it(in), latticeElements(e), latticeRefPos(rP), latticeCircumference(circ) {}
    void checkForEnd() const;

  public:
    friend class AccLatticeIterator<true>;
    friend class AccLatticeIterator<false>;
    AccLatticeIterator(const AccLatticeIterator<false>& o) : it(o.it), latticeElements(o.latticeElements), latticeRefPos(o.latticeRefPos), latticeCircumference(o.latticeCircumference) {}
    // accessors
    double pos() const {checkForEnd(); return it->first;}         // position in Lattice in meter
    ValueType element() const {checkForEnd(); return it->second;}  // pointer to Element
    ValueType operator*() {return element();}
    
    // iteration
    AccLatticeIterator<IS_CONST>& operator++() {++it; return *this;}   //prefix
    AccLatticeIterator<IS_CONST> operator++(int) {auto old=*this; ++it; return old;} //postfix
    AccLatticeIterator<IS_CONST>& operator--() {--it; return *this;}   //prefix
    AccLatticeIterator<IS_CONST> operator--(int) {auto old=*this; --it; return old;} //postfix
    AccLatticeIterator<IS_CONST>& revolve();                           // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccLatticeIterator<IS_CONST>& next() {return operator++();}
    AccLatticeIterator<IS_CONST>& next(element_type t, element_plane p=noplane, element_family f=nofamily);
    bool isEnd() const {return (it==latticeElements->end());}
    
    // comparison
    bool operator==(const AccLatticeIterator<true>& o) const {if(o.it==it) return true; else return false;}
    bool operator!=(const AccLatticeIterator<true>& o) const {return !operator==(o);}

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


  // ---------- TypeIterator class to iterate only a specific element_type (and plane and family) ----------------
  template <bool IS_CONST, element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  class AccLatticeTypeIterator : public AccLatticeIterator<IS_CONST> {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;


  protected:
    AccLatticeTypeIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : AccLatticeIterator<IS_CONST>(in,e,rP,circ) {}

  public:
    AccLatticeTypeIterator(const AccLatticeIterator<IS_CONST>& other) : AccLatticeIterator<IS_CONST>(other) {std::cout << "möp\n"; if(this->element()->type!=TYPE) operator++();}
    // iteration
    AccLatticeIterator<IS_CONST>& operator++() {return this->next(TYPE, PLANE, FAMILY);}   //prefix
    AccLatticeIterator<IS_CONST> operator++(int) {auto old= *this; this->next(TYPE, PLANE, FAMILY); return old;} //postfix
  };



  // ---------- typedefs for the user -------------
  typedef AccLatticeIterator<false> AccIterator;
  typedef AccLatticeIterator<true> const_AccIterator;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using AccTypeIterator = AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using const_AccTypeIterator = AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>;



  // ------------ exceptions ----------------------
  class noMatchingElement : public pal::palatticeError {
  public:
    noMatchingElement(std::string elementDescription)
      : palatticeError("No matching element found (" + elementDescription + ")") {}
  };



  #include "AccIterator.hxx"

} //namespace pal

#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
