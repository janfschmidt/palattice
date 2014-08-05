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
#include <vector>
#include "AccElements.hpp"

typedef std::map<double,AccElement*>::iterator AccIterator;
typedef std::map<double,AccElement*>::const_iterator const_AccIterator;

enum Anchor{begin,center,end};


class AccLattice {

protected:
  std::map<double,AccElement*> elements;                  // first: position in lattice / m
  const Drift* empty_space;
  vector<string> ignoreList;                              // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
  unsigned int ignoreCounter;

  AccIterator firstIt(element_type _type);                // get first element of given type
  AccIterator lastIt(element_type _type);                 // get last element of given type
  AccIterator nextIt(element_type _type, AccIterator it); // get next element of given type after it


public:
  const Anchor refPos;
  const double circumference;

  AccLattice(double _circumference, Anchor _refPos=begin);
  AccLattice(const AccLattice &other);
  ~AccLattice();
  AccLattice& operator= (const AccLattice other);

  const_AccIterator getIt(double pos) const;                           // get const_Iterator to element, if pos is inside it
  const_AccIterator getItBegin() const;                                // get iterator to begin (first Element)
  const_AccIterator getItEnd() const;                                  // get iterator to end (after last Element)
  const_AccIterator firstCIt(element_type _type) const;                 // get iterator to first element of given type
  const_AccIterator lastCIt(element_type _type) const;                  // get iterator to last element of given type
  const_AccIterator nextCIt(element_type _type, const_AccIterator it) const;  // get iterator to next element of given type after it

  double where(const_AccIterator it) const {return it->first;}          // get position of element with iterator it
  double locate(double pos, const AccElement *obj, Anchor here) const;  // get here=begin/center/end (in meter) of obj at reference-position pos
  bool inside(double pos, const AccElement *obj, double here) const;    // test if "here" is inside obj at position pos
  double locate(const_AccIterator it, Anchor here) const;        // get here=begin/center/end (in meter)  of lattice element "it"
  bool inside(const_AccIterator it, double here) const;                 // test if "here" is inside lattice element "it"
  const AccElement* first(element_type _type);                          // get first element of given type
  const AccElement* last(element_type _type);                           // get last element of given type
  const AccElement* next(element_type _type, double pos);               // get next element of given type after pos


  const AccElement* operator[](double pos) const;                  // get element (any position, Drift returned if not inside any element)
  void set(double pos, const AccElement &obj, bool verbose=false); // set element (throws XXX if no free space for obj)
  void erase(double pos);                                          // erase element at position pos

  void setIgnoreList(string ignoreFile);         // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
  void madximport(const char *madxTwissFile);              // set elements from MAD-X Lattice (read from twiss-output)
  void madximportMisalignments(const char *madxEalignFile);// set misalignments from MAD-X Lattice (read ealign-output)
                                                     // !! currently only rotation (dpsi) around beam axis (s) is implemented!
  void elegantimport(const char *elegantParamFile);        // set elements from elegant Lattice (read from ascii parameter file ".param")
  void setELSAoptics(const char *spurenFolder);                    // change quad&sext strengths to values from "ELSA-Spuren"
  unsigned int setELSACorrectors(CORR *ELSAvcorrs, unsigned int t);// change corrector pos&strength to values from "ELSA-Spuren" at time t
  void subtractCorrectorStrengths(const AccLattice &other);    // subtract other corrector strengths from the ones of this lattice
  void subtractMisalignments(const AccLattice &other);         // subtract other misalignments from the ones of this lattice

  // "information"
  unsigned int size(element_type _type) const;        // returns number of elements of a type in this lattice
  unsigned int size() const {return elements.size();} // returns total number of elements
  vector<string> getIgnoreList() const {return ignoreList;}
  unsigned int ignoredElements() const {return ignoreCounter;}
  string refPos_string() const;
  string getElegantDef(element_type _type) const; // return elegant compliant element definitions for given type


  // output (stdout or file)
  // If no filename is given, print to stdout.
  void print(const char *filename="") const;                     // print lattice.
  void print(element_type _type, const char *filename="") const; // print all elements of one type.
  void elegantexport(const char *filename="") const;             // print lattice readable by elegant.
  void latexexport(const char *filename="") const;             // print lattice readable by LaTeX (using lattice package by Jan Schmidt <schmidt@physik.uni-bonn.de>)

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


// data format for elegant parameters file (".param")
class paramRow {
public:
  string name;
  string type;
  string param;
  double value;
  paramRow() : name(""), type(""), param(""), value(0.) {};
};


string removeQuote(string s); //remove quotation marks ("" or '') from begin&end of string

#endif
/*__ACCLATTICE_HPP_*/
