/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X.
 * Uses the "AccElements" classes
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#ifndef __ACCLATTICE_HPP_
#define __ACCLATTICE_HPP_

#include <map>
#include <stdexcept>
#include <iostream>
#include "AccElements.hpp"

typedef std::map<double,AccElement*>::iterator AccIterator;
typedef std::map<double,AccElement*>::const_iterator const_AccIterator;
enum Position{begin,center,end};


class AccLattice {

protected:
  std::map<double,AccElement*> elements;
  const Position refPos;
  const Drift* empty_space;
 

public:
  bool inside(double pos, const_AccIterator it) const;
  double locate(const_AccIterator it, Position here) const;
  double locate(const_AccIterator it) const {return locate(it, refPos);}

  AccLattice(Position _refPos=begin);
  ~AccLattice();

  //get const_iterator
  const_AccIterator getIt(double pos) const;

  //get element
  const AccElement* operator[](double pos) const;

  //set element
  void set(double pos, const AccElement& obj);

};


// exceptions
class eNoElement : public std::exception {
public:

  eNoElement() {}
};


#endif
/*__ACCLATTICE_HPP_*/
