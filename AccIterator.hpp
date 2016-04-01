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

  
  template <typename VAL_T, typename MAP_T, typename IT_T>
  class AccLatticeIterator {
    friend class AccLattice;
    
  protected:
    IT_T it;
    MAP_T* latticeElements;
    const Anchor* latticeRefPos;
    const double* latticeCircumference;
    AccLatticeIterator(IT_T in, MAP_T* e, const Anchor* rP, const double* circ) : it(in), latticeElements(e), latticeRefPos(rP), latticeCircumference(circ) {}
    void checkForEnd() const;

  public:
    // accessors
    double pos() const;                                      // position in Lattice in meter
    VAL_T element() const;                                  // pointer to Element
    VAL_T operator*() {return element();}
    
    // iteration
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& operator++() {it++; return *this;}   //prefix
    AccLatticeIterator<VAL_T,MAP_T,IT_T> operator++(int) {it++; return *this;} //postfix
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& operator--() {it--; return *this;}   //prefix
    AccLatticeIterator<VAL_T,MAP_T,IT_T> operator--(int) {it--; return *this;} //postfix
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& revolve();                           // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& next() {return operator++();}
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& next(element_type t, element_plane p=noplane, element_family f=nofamily);
    bool isEnd() const {return (it==latticeElements->end());}
    
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


  // ---------- TypeIterator class to iterate only a specific element_type (and plane and family) ----------------
  template <typename VAL_T, typename MAP_T, typename IT_T, element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  class AccLatticeTypeIterator : public AccLatticeIterator<VAL_T,MAP_T,IT_T> {
    friend class AccLattice;

  protected:
    AccLatticeTypeIterator(IT_T in, MAP_T* e, const Anchor* rP, const double* circ) : AccLatticeIterator<VAL_T,MAP_T,IT_T>(in,e,rP,circ) {}

  public:
    // iteration
    AccLatticeIterator<VAL_T,MAP_T,IT_T>& operator++() {return this->next(TYPE, PLANE, FAMILY);}   //prefix
    AccLatticeIterator<VAL_T,MAP_T,IT_T> operator++(int) {return this->next(TYPE, PLANE, FAMILY);} //postfix
  };



  // ---------- typedefs for the user -------------
  typedef AccLatticeIterator<AccElement*,AccMap,AccMap::iterator> AccIterator;
  typedef AccLatticeIterator<const AccElement*,const AccMap,AccMap::const_iterator> const_AccIterator;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using AccTypeIterator = AccLatticeTypeIterator<AccElement*,AccMap,AccMap::iterator,TYPE,PLANE,FAMILY>;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using const_AccTypeIterator = AccLatticeTypeIterator<const AccElement*,const AccMap,AccMap::const_iterator,TYPE,PLANE,FAMILY>;



  // ------------ exceptions ----------------------
  class noMatchingElement : public pal::palatticeError {
  public:
    noMatchingElement(std::string elementDescription)
      : palatticeError("No matching element found (" + elementDescription + ")") {}
  };



  //-------------- template implementations -----------------------
  template<typename VAL_T, typename MAP_T, typename IT_T>
  VAL_T AccLatticeIterator<VAL_T,MAP_T,IT_T>::element() const {
    checkForEnd();
    return it->second;
  }
  template<typename VAL_T, typename MAP_T, typename IT_T>
  VAL_T AccLatticeIterator<VAL_T,MAP_T,IT_T>::elementModifier() const {
    checkForEnd();
    return it->second;
  } 
  template<typename VAL_T, typename MAP_T, typename IT_T>
  AccLatticeIterator<VAL_T,MAP_T,IT_T>& AccLatticeIterator<VAL_T,MAP_T,IT_T>::next(element_type t, element_plane p, element_family f)
  {
    ++it;
    for (; it!=latticeElements->end(); ++it) {
      if ( it->second->type==t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f) )
	break;
  }
    return *this;
    //throw noMatchingElement("type, plane, family");
  }
  template<typename VAL_T, typename MAP_T, typename IT_T>
  AccLatticeIterator<VAL_T,MAP_T,IT_T>& AccLatticeIterator<VAL_T,MAP_T,IT_T>::revolve()
  {
    ++it;
    if (it == latticeElements->end()) {
      it = latticeElements->begin();
    }
    return *this;
  }

  

} //namespace pal

#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
