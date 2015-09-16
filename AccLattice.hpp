/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X.
 * Uses the "AccElements" classes
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#ifndef __LIBPALATTICE_ACCLATTICE_HPP_
#define __LIBPALATTICE_ACCLATTICE_HPP_

#include <map>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "AccElements.hpp"
#include "ELSASpuren.hpp"
#include "Metadata.hpp"
#include "config.hpp"
#include "types.hpp"
#include "SimTools.hpp"

namespace pal
{


typedef std::map<double,AccElement*>::iterator AccIterator;
typedef std::map<double,AccElement*>::const_iterator const_AccIterator;

enum Anchor{begin,center,end};


class AccLattice {

protected:
  double circ;
  std::map<double,AccElement*> elements;  // first: position in lattice / m
  const Drift* empty_space;
  vector<string> ignoreList;              // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
  unsigned int ignoreCounter;

  AccIterator firstIt(element_type _type, element_plane p=noplane, element_family f=nofamily); // get iterator to first element of given type
  AccIterator lastIt(element_type _type, element_plane p=noplane, element_family f=nofamily);  // get iterator to last element of given type
  AccIterator nextIt(double pos, element_plane p=noplane, element_family f=nofamily);                         // get iterator to next element after pos
  AccIterator nextIt(double pos, element_type _type, element_plane p=noplane, element_family f=nofamily);     // get iterator to next element of given type after pos
  AccIterator nextIt(AccIterator it, element_type _type, element_plane p=noplane, element_family f=nofamily); // get iterator to next element of given type after it (for any type just use it++ ;) )
  AccIterator nextIt(double pos, Anchor anchor);       // get iterator to first element, whose begin/center/end is > pos. circulating.
  AccIterator revolve(AccIterator it);                 // like it++, but starts at begin() after last element (never reaches end()!)

  double slope(double pos, const_AccIterator it) const; // helper function for magnetic field edges


public:
  const Anchor refPos;
  Metadata info;

  AccLattice(string _name, double _circumference, Anchor _refPos=begin);
  AccLattice(string _name, SimToolInstance &sim, Anchor _refPos=end, string ignoreFile="");    //direct madx/elegant import
  AccLattice(AccLattice &other);
  ~AccLattice();
  AccLattice& operator= (AccLattice &other);

  double circumference() const {return circ;}

  // ------- these functions may be replaced soon by a new API for lattice iteration -------------------------
  const_AccIterator getIt(double pos) const;      // get const_Iterator to element, if pos is inside it
  const_AccIterator getItBegin() const;           // get iterator to begin (first Element)
  const_AccIterator getItEnd() const;             // get iterator to end (after last Element)
  const_AccIterator firstCIt(element_type _type, element_plane p=noplane, element_family f=nofamily) const; // get iterator to first element of given type
  const_AccIterator lastCIt(element_type _type, element_plane p=noplane, element_family f=nofamily) const;  // get iterator to last element of given type
  const_AccIterator nextCIt(double pos, element_plane p=noplane, element_family f=nofamily) const;                               // get iterator to next element after pos
  const_AccIterator nextCIt(double pos, element_type _type, element_plane p=noplane, element_family f=nofamily) const;           // get iterator to next element of given type after pos
  const_AccIterator nextCIt(const_AccIterator it, element_type _type, element_plane p=noplane, element_family f=nofamily) const; // get iterator to next element of given type after it (for any type just use it++ ;) )
  const_AccIterator nextCIt(double pos, Anchor anchor) const;       // get iterator to first element, whose begin/center/end is > pos. circulating.
  const_AccIterator revolve(const_AccIterator it) const;            // like it++, but starts at begin() after last element (never reaches end()!)


  double pos(const_AccIterator it) const {return it->first;}          // get position of element with iterator it
  double posMod(double posIn) const {return fmod(posIn,circ);}        // get position modulo circumference
  unsigned int turn(double posIn) const {return int(posIn/circ + ZERO_DISTANCE) + 1;} // get turn from position
  double theta(double posIn) const;                                     // get rotation angle [0,2pi]: increases lin. in bending dipoles, constant in-between. 
  double locate(double pos, const AccElement *obj, Anchor here) const;  // get here=begin/center/end (in meter) of obj at reference-position pos
  double locate(const_AccIterator it, Anchor here) const;               // get here=begin/center/end (in meter) of lattice element "it"
  double locate(string name, Anchor here) const {return locate(operator[](name),here);} // get here=begin/center/end (in meter) of lattice element by name
  bool inside(double pos, const AccElement *obj, double here) const;    // test if "here" is inside obj at position pos
  bool inside(const_AccIterator it, double here) const;                 // test if "here" is inside lattice element "it"  
  bool inside(string name, double here) const {return inside(operator[](name),here);}   // test if "here" is inside lattice element with name
  double distance(double pos, const_AccIterator it, Anchor itRef) const;// distance from itRef of element it to pos (>0 if pos is after itRef)
  double distanceRing(double pos, const_AccIterator it, Anchor itRef) const;// distance in a ring (both directions, shorter distance returned)
  double distanceNext(const_AccIterator it) const;                      // |distance| from it to next element (circulating)

  const AccElement* operator[](double pos) const;                    // get element (any position, Drift returned if not inside any element)
  const_AccIterator operator[](string name) const;                   // get iterator by name (first match in lattice, Drift returned otherwise)
  void mount(double pos, const AccElement &obj, bool verbose=false); // mount an element (throws eNoFreeSpace if no free space for obj)
  void dismount(double pos);                                         // dismount element at Ref.position pos (if no element at pos: do nothing)

  void setIgnoreList(string ignoreFile);                      // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted in this lattice
  void simToolImport(SimToolInstance &sim) {if (sim.tool==madx) madximport(sim); else if (sim.tool==elegant) elegantimport(sim);}
  void madximport(SimToolInstance &madx);                      // mount elements from MAD-X Lattice (read from twiss-output, m=online: autom. madx run)
  void madximport(string madxFile, SimToolMode m=online) {SimToolInstance madx(pal::madx, m, madxFile); madximport(madx);}
  void madximportMisalignments(element_type t, string madxEalignFile); // set misalignments from MAD-X Lattice (read ealign-output)
                                                                       // !! currently only rotation (dpsi) around beam axis (s) is implemented!
  void elegantimport(SimToolInstance &elegant);                // mount elements from elegant Lattice (read from ascii parameter file ".param", m=online: autom. elegant run)
  void elegantimport(string elegantFile, SimToolMode m=online) {SimToolInstance e(pal::elegant, m, elegantFile); elegantimport(e);}
  void setELSAoptics(string spurenFolder);                    // change quad&sext strengths to values from "ELSA-Spuren"
  unsigned int setELSACorrectors(ELSASpuren &spuren, unsigned int t); // change corrector pos&strength to values from "ELSA-Spuren" at time t
  void subtractCorrectorStrengths(AccLattice &other);    // subtract other corrector strengths from the ones of this lattice
  void subtractMisalignments(const AccLattice &other);         // subtract other misalignments from the ones of this lattice

  // "information"
  unsigned int size(element_type _type, element_plane p=noplane, element_family f=nofamily) const;        // returns number of elements of a type in this lattice
  unsigned int size() const {return elements.size();} // returns total number of elements
  string sizeSummary() const; //formated "size" output for all element types

  vector<string> getIgnoreList() const {return ignoreList;}
  unsigned int ignoredElements() const {return ignoreCounter;}
  string refPos_string() const;
  string getElementDefs(SimTool tool,element_type _type) const; // return elegant or madx compliant element definitions for given type
  string getLine(SimTool tool) const; // return lattice in elegant or madx compliant "LINE=(..." format
  string getSequence(Anchor refer=center) const;    // return lattice in madx compliant "SEQUENCE" format

  //magnetic field, including continuous slope at start/end
  // - "verlängern" um l_eff - l_metric = d
  // - l_metric = PHYSICAL_LENGTH_PERCENTAGE * l_eff wenn nicht gegeben ??
  // - slope: monotone funktion f mit f(l_metric) =: f(a) = 1 und f(l_eff + d) = f(l_metric + 2d) =: f(b) = 0.
  // - int_a^b f(x) = 1*d -> im vergleich mit rechteck bis l_eff soll sich die fläche NICHT ändern!
  // test mit Gauss: f(x) = exp(-0.5*(x*0.67449/d)^2), 50% quantil (median) bei 0.67449 sigma.
  // -> nähert sich 0, ist aber f(b) > 0.
  // -> problem: integral muss 1 sein (normierter Gauß) UND f(a) = 1 (nicht-normierter Gauß)
  AccTriple B(double pos, const AccPair &orbit) const;


  // output (stdout or file)
  // If no filename is given, print to stdout.
  void print(string filename="") const;                     // print lattice.
  void print(element_type _type, string filename="") const; // print all elements of one type.
  void simToolExport(SimTool tool, string filename="", MadxLatticeType ltype=sequence) const; // print lattice readable by elegant or madx
  void latexexport(string filename="") const;               // print lattice readable by LaTeX (using lattice package by Jan Schmidt <schmidt@physik.uni-bonn.de>)
  // "shortcuts:"
  void elegantexport(string file="") const {simToolExport(elegant,file);}
  void madxexport(string file="",MadxLatticeType t=sequence) const {simToolExport(madx,file,t);}

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

} //namespace pal

#endif
/*__LIBPALATTICE_ACCLATTICE_HPP_*/
