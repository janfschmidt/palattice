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



  // abstract base class:
  template <bool IS_CONST>
  class AccLatticeIterator_Base {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;
    
  protected:
    IteratorType it;
    MapType* latticeElements;
    const Anchor* latticeRefPos;
    const double* latticeCircumference;
    AccLatticeIterator_Base(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : it(in), latticeElements(e), latticeRefPos(rP), latticeCircumference(circ) {}
    void checkForEnd() const;

  public:
    friend class AccLatticeIterator_Base<true>;
    friend class AccLatticeIterator_Base<false>;
    AccLatticeIterator_Base(const AccLatticeIterator_Base<false>& o) : it(o.it), latticeElements(o.latticeElements), latticeRefPos(o.latticeRefPos), latticeCircumference(o.latticeCircumference) {}
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

// forward declaration of  AccLatticeTypeIterator
template <bool IS_CONST, element_type TYPE, element_plane PLANE, element_family FAMILY>
class AccLatticeTypeIterator;




  // Iterator class:
  template <bool IS_CONST>
  class AccLatticeIterator : public AccLatticeIterator_Base<IS_CONST> {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;
    
  protected:
    AccLatticeIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : AccLatticeIterator_Base<IS_CONST>(in,e,rP,circ) {}
    void setBegin() override {this->it = this->latticeElements->begin();}
    
  public:
    friend class AccLatticeIterator<true>;
    friend class AccLatticeIterator<false>;
    AccLatticeIterator(const AccLatticeIterator<false>& other) : AccLatticeIterator_Base<IS_CONST>(other) {}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    AccLatticeIterator(const AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& other) : AccLatticeIterator_Base<IS_CONST>(other) {}

    // iteration
    AccLatticeIterator<IS_CONST>& next() {++(this->it); return *this;}
    AccLatticeIterator<IS_CONST>& prev() {--(this->it); return *this;}
    AccLatticeIterator<IS_CONST>& revolve() {next(); this->revolve_helper(); return *this;}       // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccLatticeIterator<IS_CONST>& next(element_type t, element_plane p=noplane, element_family f=nofamily) {this->next_helper(t,p,f); return *this;}
    AccLatticeIterator<IS_CONST>& prev(element_type t, element_plane p=noplane, element_family f=nofamily) {this->prev_helper(t,p,f); return *this;}
    AccLatticeIterator<IS_CONST>& operator++() {return next();}   //prefix
    AccLatticeIterator<IS_CONST>& operator--()  {return prev();}   //prefix
    AccLatticeIterator<IS_CONST> operator++(int) {auto old=*this; next(); return old;} //postfix
    AccLatticeIterator<IS_CONST> operator--(int) {auto old=*this; prev(); return old;} //postfix

    // comparison
    bool operator==(const AccLatticeIterator<true>& o) const {return o.equal(this->it);}
    bool operator!=(const AccLatticeIterator<true>& o) const {return !operator==(o);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator==(const AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator!=(const AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator==(const AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    template <element_type TYPE,element_plane PLANE,element_family FAMILY>
    bool operator!=(const AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}


    // position calculations
    double distanceNext(Anchor anchor) const override {auto n=*this; n.revolve(); return std::abs(this->distanceRing(anchor,n.pos(anchor)));} 
  };


  
  // Type-Iterator class: iterate only a specific element_type (and plane and family)
  template <bool IS_CONST, element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  class AccLatticeTypeIterator : public AccLatticeIterator_Base<IS_CONST> {
    friend class AccLattice;
    typedef typename std::conditional<IS_CONST, const AccElement*, AccElement*>::type ValueType;
    typedef typename std::conditional<IS_CONST, const AccMap, AccMap>::type MapType;
    typedef typename std::conditional<IS_CONST, AccMap::const_iterator, AccMap::iterator>::type IteratorType;

  protected:
    AccLatticeTypeIterator(IteratorType in, MapType* e, const Anchor* rP, const double* circ) : AccLatticeIterator_Base<IS_CONST>(in,e,rP,circ) {}
    void setBegin() override {this->it=this->latticeElements->begin(); if(this->it->second->type!=TYPE) next();}

  public:
    friend class AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>;
    friend class AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>;
    AccLatticeTypeIterator(const AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>& other) : AccLatticeIterator_Base<IS_CONST>(other) {}
    AccLatticeTypeIterator(const AccLatticeIterator<IS_CONST>& other) : AccLatticeIterator_Base<IS_CONST>(other) {}
    
    // iteration
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& next() {this->next_helper(TYPE,PLANE,FAMILY); return *this;}
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& prev() {this->prev_helper(TYPE,PLANE,FAMILY); return *this;}
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& revolve() {next(); this->revolve_helper(); return *this;}       // a circular ++: apply to last element to get lattice.begin() (NOT lattice.end())
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& operator++() {return next();}   //prefix
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY>& operator--()  {return prev();}   //prefix
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY> operator++(int) {auto old=*this; next(); return old;} //postfix
    AccLatticeTypeIterator<IS_CONST,TYPE,PLANE,FAMILY> operator--(int) {auto old=*this; prev(); return old;} //postfix
    
    // comparison
    bool operator==(const AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return o.equal(this->it);}
    bool operator!=(const AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>& o) const {return !operator==(o);}
    bool operator==(const AccLatticeIterator<true>& o) const {return o.equal(this->it);}
    bool operator!=(const AccLatticeIterator<true>& o) const {return !operator==(o);}

    // position calculations
    double distanceNext(Anchor anchor) const override {auto n=*this; n.revolve(); return std::abs(this->distanceRing(anchor,n.pos(anchor)));} 
  };




  // typedefs: AccLattice::iterator and AccLattice::type_iterator
  typedef AccLatticeIterator<false> iterator;
  typedef AccLatticeIterator<true> const_iterator;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using type_iterator = AccLatticeTypeIterator<false,TYPE,PLANE,FAMILY>;
  template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
  using const_type_iterator = AccLatticeTypeIterator<true,TYPE,PLANE,FAMILY>;





#endif /*__LIBPALATTICE_ACCITERATOR_HPP_*/
