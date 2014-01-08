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
enum Anchor{begin,center,end};


class AccLattice {

protected:
  std::map<double,AccElement*> elements;
  const Drift* empty_space;

  const_AccIterator getIt(double pos) const;   //get iterator 

public:
  const Anchor refPos;
  const double circumference;

  AccLattice(double _circumference, Anchor _refPos=begin);
  ~AccLattice();

  double locate(double pos, const AccElement *obj, Anchor here) const;  // get here=begin/center/end (in meter) of obj at reference-position pos
  double locate(const_AccIterator it, Anchor here) const;               // get here=begin/center/end (in meter)  of lattice element "it"
  bool inside(double pos, const AccElement *obj, double here) const;    // test if "here" is inside obj at position pos
  bool inside(const_AccIterator it, double here) const;                 // test if "here" is inside lattice element "it"

  const AccElement* operator[](double pos) const;    // get element (any position, Drift returned if not inside any element)
  void set(double pos, const AccElement &obj);       // set element (throws XXX if no free space for obj)

  void madximport(char *madxTwissFile);              // set elements from MAD-X Lattice (read from twiss-output)

  string refPos_string() const;
  void print() const;                                // print lattice to stdout

};


// exceptions
class eNoElement : public std::exception {
public:

  eNoElement() {}
};

class eNoFreeSpace : public std::invalid_argument {
public:
  eNoFreeSpace(string msg) : std::invalid_argument(msg) {}
};


#endif
/*__ACCLATTICE_HPP_*/
