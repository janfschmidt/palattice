/* === AccIterator Classes ===
 * iterators to access elements in an accelerator lattice and the position, where they are mounted.
 * It allows to iterate / loop through the lattice and perform calculations regarding their position (distances etc.).
 * All these iterators are defined as inner classes within the "AccLattice" class and
 * are accessible via the AccLattice::begin() and AccLattice::end() methods.
 * There are four iterator types intended for external usage:
 * AccLattice::iterator & AccLattice::const_iterator           -> iterate over all elements in lattice
 * AccLattice::type_iterator & AccLattice::const_type_iterator -> iterate over all elements of one type in lattice
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



// !!! The following classes are inner classes of pal::AccLattice
// !!! This file is included in AccLattice.hpp WITHIN AccLattice class


// forward declaration of  AccTypeIterator
template <bool IS_CONST, element_type TYPE, element_plane PLANE, element_family FAMILY>
class AccTypeIterator;


  // abstract base class:
  template <bool IS_CONST>
  class AccIterator_Base {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;
    
  protected:
    IteratorType it;
    MapType* latticeElements;
    const Anchor* latticeRefPos;
    const double* latticeCircumference;
    AccIterator_Base(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : it(in), latticeElements(e), latticeRefPos(rP), latticeCircumference(circ) {}
    void checkForEnd() const;

  public:
    AccIterator_Base(const AccIterator_Base<false>& o) : it(o.it), latticeElements(o.latticeElements), latticeRefPos(o.latticeRefPos), latticeCircumference(o.latticeCircumference) {}
    // accessors
    double pos() const {checkForEnd(); return it->first;}         // position in Lattice in meter
    ValueType element() const {checkForEnd(); return it->second;}  // pointer to Element
    ValueType operator*() {return element();}
    
    // iteration
    bool isEnd() const {return (it==latticeElements->end());}

    // comparison
    bool equal(const AccMap::const_iterator& other) const {return (other==it);}
    
    // position calculations
    double pos(Anchor anchor) const;                         // get position of "anchor" of this element in meter
    double begin() const {return pos(Anchor::begin);}        // get begin of this element in meter
    double center() const {return pos(Anchor::center);}      // get center of this element in meter
    double end() const {return pos(Anchor::end);}            // get end of this element in meter
    bool at(double pos) const;                               // test, if position "pos" is within this element
    double distance(Anchor anchor, double pos) const;        // get distance from "anchor" of this element to position "pos" in meter (>0 if pos is after/upstream anchor)
    double distanceRing(Anchor anchor, double pos) const;    // distance() for rings: considers both directions and returns shortest distance
    virtual double distanceNext(Anchor anchor) const =0;     // absolute value of distance from "anchor" of this element to "anchor" of next element. For last element: distance to first element (in a ring)

  protected:
    virtual void setBegin() =0;
    void next_helper(element_type t, element_plane p, element_family f);
    void prev_helper(element_type t, element_plane p, element_family f);
    void revolve_helper();
  };





  // Iterator class:
  template <bool IS_CONST>
  class AccIterator : public AccIterator_Base<IS_CONST> {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;
    
  protected:
    AccIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : AccIterator_Base<IS_CONST>(in,e,rP,circ) {}
    void setBegin() override {this->it = this->latticeElements->begin();}
    
  public:
    AccIterator(const AccIterator<false>& other) : AccIterator_Base<IS_CONST>(other) {}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    AccIterator(const AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& other) : AccIterator_Base<IS_CONST>(other) {}

    // iteration
    AccIterator<IS_CONST>& next() {++(this->it); return *this;}
    AccIterator<IS_CONST>& prev() {--(this->it); return *this;}
    AccIterator<IS_CONST>& revolve() {next(); this->revolve_helper(); return *this;}       // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccIterator<IS_CONST>& next(element_type t, element_plane p=noplane, element_family f=nofamily) {this->next_helper(t,p,f); return *this;}
    AccIterator<IS_CONST>& prev(element_type t, element_plane p=noplane, element_family f=nofamily) {this->prev_helper(t,p,f); return *this;}
    AccIterator<IS_CONST>& operator++() {return next();}   //prefix
    AccIterator<IS_CONST>& operator--()  {return prev();}   //prefix
    AccIterator<IS_CONST> operator++(int) {auto old=*this; next(); return old;} //postfix
    AccIterator<IS_CONST> operator--(int) {auto old=*this; prev(); return old;} //postfix

    // comparison
    bool operator==(const AccIterator<true>& o) const {return o.equal(this->it);}
    bool operator!=(const AccIterator<true>& o) const {return !operator==(o);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator==(const AccTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator!=(const AccTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator==(const AccTypeIterator<false,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator!=(const AccTypeIterator<false,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}


    // position calculations
    double distanceNext(Anchor anchor) const override {auto n=*this; n.revolve(); return std::abs(this->distanceRing(anchor,n.pos(anchor)));} 
  };


  
  // Type-Iterator class: iterate only a specific element_type (and plane and family)
  template <bool IS_CONST, element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  class AccTypeIterator : public AccIterator_Base<IS_CONST> {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;

  protected:
    AccTypeIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : AccIterator_Base<IS_CONST>(in,e,rP,circ) {}
    void setBegin() override {this->it=this->latticeElements->begin(); if(this->it->second->type!=TYPE) next();}

  public:
    AccTypeIterator(const AccTypeIterator<false,TYPE,PLANE,FAMILY>& other) : AccIterator_Base<IS_CONST>(other) {}
    AccTypeIterator(const AccIterator<IS_CONST>& other) : AccIterator_Base<IS_CONST>(other) {}
    
    // iteration
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& next() {this->next_helper(TYPE,PLANE,FAMILY); return *this;}
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& prev() {this->prev_helper(TYPE,PLANE,FAMILY); return *this;}
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& revolve() {next(); this->revolve_helper(); return *this;}       // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& operator++() {return next();}   //prefix
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& operator--()  {return prev();}   //prefix
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY> operator++(int) {auto old=*this; next(); return old;} //postfix
    AccTypeIterator<IS_CONST,TYPE,PLANE,FAMILY> operator--(int) {auto old=*this; prev(); return old;} //postfix
    
    // comparison
    bool operator==(const AccTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    bool operator!=(const AccTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}
    bool operator==(const AccIterator<true>& o) const {return o.equal(this->it);}
    bool operator!=(const AccIterator<true>& o) const {return !operator==(o);}

    // position calculations
    double distanceNext(Anchor anchor) const override {auto n=*this; n.revolve(); return std::abs(this->distanceRing(anchor,n.pos(anchor)));} 
  };




  // typedefs: AccLattice::iterator and AccLattice::type_iterator
  typedef AccIterator<false> iterator;
  typedef AccIterator<true> const_iterator;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using type_iterator = AccTypeIterator<false,TYPE,PLANE,FAMILY>;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using const_type_iterator = AccTypeIterator<true,TYPE,PLANE,FAMILY>;





#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
