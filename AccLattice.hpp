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

  enum class Anchor{begin,center,end};
  typedef std::map<double,AccElement*> AccMap;

  
  class AccLattice {
    public:
    #include "AccIterator.hpp"
    
protected:
  double circ;
  AccMap elements;  // first: position in lattice / m
  const Drift* empty_space;
  vector<string> ignoreList;              // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
  unsigned int ignoreCounter;
  string comment;

  double locate(double pos, const AccElement *obj, Anchor here) const;  // get here=begin/center/end (in meter) of obj at reference-position pos
  double slope(double pos, const_iterator it) const; // helper function for magnetic field edges
  void setCircumference(double c);


public:
  const Anchor refPos;
  Metadata info;

  AccLattice(double _circumference=0., Anchor _refPos=Anchor::begin);
  AccLattice(SimToolInstance &sim, Anchor _refPos=Anchor::end, string ignoreFile=""); //direct madx/elegant import
  AccLattice(const AccLattice &other);
  ~AccLattice();
  AccLattice& operator= (const AccLattice &other);

  double circumference() const {return circ;}

  void setComment(string _comment) {comment=_comment; info.add("Comment", comment);}
  string getComment() const {return comment;}

  double posMod(double posIn) const {return fmod(posIn,circ);}        // get position modulo circumference
  unsigned int turn(double posIn) const {return int(posIn/circ + ZERO_DISTANCE) + 1;} // get turn from position
  double theta(double posIn) const;                                   // get rotation angle [0,2pi]: increases lin. in bending dipoles, constant in-between. 

    // iterator
    iterator begin() {return iterator(elements.begin(),&elements,&refPos,&circ);}
    iterator end() {return iterator(elements.end(),&elements,&refPos,&circ);}
    iterator begin(element_type t, element_plane p=noplane, element_family f=nofamily);
    template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
    type_iterator<TYPE,PLANE,FAMILY> begin() {return type_iterator<TYPE,PLANE,FAMILY>(this->begin(TYPE,PLANE,FAMILY));}
    // const_iterator
    const_iterator begin() const {return const_iterator(elements.begin(),&elements,&refPos,&circ);}
    const_iterator end() const {return const_iterator(elements.end(),&elements,&refPos,&circ);}
    const_iterator begin(element_type t, element_plane p=noplane, element_family f=nofamily) const;
    template <element_type TYPE, element_plane PLANE=noplane, element_family FAMILY=nofamily>
    const_type_iterator<TYPE,PLANE,FAMILY> begin() const {return const_type_iterator<TYPE,PLANE,FAMILY>(this->begin(TYPE,PLANE,FAMILY));}

  const AccElement* operator[](double pos) const;  // get element (any position, Drift returned if not inside any element)
  iterator at(double pos);                         // get iterator by position (throws noMatchingElement, if pos is in Drift)
  iterator behind(double pos, Anchor anchor);      // get iterator to next element with "anchor" behind given position
  iterator operator[](string name);                // get iterator by name (first match in lattice, throws noMatchingElement otherwise)
  const_iterator at(double pos) const;
  const_iterator behind(double pos, Anchor anchor) const;
  const_iterator operator[](string name) const;

  void mount(double pos, const AccElement &obj, bool verbose=false); // mount an element (throws noFreeSpace if no free space for obj)
  void dismount(double pos);                                         // dismount element at Ref.position pos (if no element at pos: do nothing)
  void dismount(iterator it) {dismount(it.pos());}

  // import
  void setIgnoreList(string ignoreFile);                      // elements with a name in this list (can contain 1 wildcard * per entry) are not mounted in this lattice
  void simToolImport(SimToolInstance &sim) {if (sim.tool==madx) madximport(sim); else if (sim.tool==elegant) elegantimport(sim);}
  void madximport(SimToolInstance &madx);                      // mount elements from MAD-X Lattice (read from twiss-output, m=online: autom. madx run)
  void madximport(string madxFile, SimToolMode m=online) {SimToolInstance madx(pal::madx, m, madxFile); madximport(madx);}
  void madximportMisalignments(element_type t, string madxEalignFile); // set misalignments from MAD-X Lattice (read ealign-output)
                                                                       // !! currently only rotation (dpsi) around beam axis (s) is implemented!
  void elegantimport(SimToolInstance &elegant);                // mount elements from elegant Lattice (read from ascii parameter file ".param", m=online: autom. elegant run)
  void elegantimport(string elegantFile, SimToolMode m=online) {SimToolInstance e(pal::elegant, m, elegantFile); elegantimport(e);}

  void subtractCorrectorStrengths(AccLattice &other);    // subtract other corrector strengths from the ones of this lattice
  void subtractMisalignments(const AccLattice &other);   // subtract other misalignments from the ones of this lattice

  // ELSA specific import
  void setELSAoptics(string spurenFolder);                            // change quad&sext strengths to values from "ELSA-Spuren"
  unsigned int setELSACorrectors(ELSASpuren &spuren, unsigned int t); // change corrector pos&strength to values from "ELSA-Spuren" at time t


  // "information"
  unsigned int size(element_type _type, element_plane p=noplane, element_family f=nofamily) const;        // returns number of elements of a type in this lattice
  unsigned int size() const {return elements.size();} // returns total number of elements
  string sizeSummary() const; //formated "size" output for all element types

  vector<string> getIgnoreList() const {return ignoreList;}
  unsigned int ignoredElements() const {return ignoreCounter;}
  string refPos_string() const;
  string getElementDefs(SimTool tool,element_type _type) const; // return elegant or madx compliant element definitions for given type
  string getLine(SimTool tool) const; // return lattice in elegant or madx compliant "LINE=(..." format
  string getSequence(Anchor refer=Anchor::center) const;    // return lattice in madx compliant "SEQUENCE" format

  //magnetic field, including continuous slope at start/end
  // - "verlängern" um l_eff - l_metric = d
  // - l_metric = PHYSICAL_LENGTH_PERCENTAGE * l_eff wenn nicht gegeben ??
  // - slope: monotone funktion f mit f(l_metric) =: f(a) = 1 und f(l_eff + d) = f(l_metric + 2d) =: f(b) = 0.
  // - int_a^b f(x) = 1*d -> im vergleich mit rechteck bis l_eff soll sich die fläche NICHT ändern!
  // test mit Gauss: f(x) = exp(-0.5*(x*0.67449/d)^2), 50% quantil (median) bei 0.67449 sigma.
  // -> nähert sich 0, ist aber f(b) > 0.
  // -> problem: integral muss 1 sein (normierter Gauß) UND f(a) = 1 (nicht-normierter Gauß)
  AccTriple B(double pos, const AccPair &orbit) const;


  //additional Physical Quantities
  double Erev_keV_syli(const double& gamma) const;     // energy loss per turn in keV for electron beam with energy given by gamma
  double overvoltageFactor(const double& gamma) const; // overvoltage factor q, from total voltage of all cavities
  double integralDipoleRadius(int exponent=1) const;   // integral over bending radius around ring: R^exponent ds
  unsigned int harmonicNumber() const;                 // harmonic number h, from cavity frequency and circumference
  
  // output & export (stdout or file)
  // If no filename is given, print to stdout.
  void print(string filename="") const;                     // print lattice.
  void print(element_type _type, string filename="") const; // print all elements of one type.
  void simToolExport(SimTool tool, string filename="", MadxLatticeType ltype=sequence) const; // print lattice readable by elegant or madx
  void latexexport(string filename="") const;               // print lattice readable by LaTeX (using lattice package by Jan Schmidt <schmidt@physik.uni-bonn.de>)
  // "shortcuts:"
  void elegantexport(string file="") const {simToolExport(elegant,file);}
  void madxexport(string file="",MadxLatticeType t=sequence) const {simToolExport(madx,file,t);}

  private:
    iterator cast_helper(const const_iterator& it);
    
    // data format for elegant parameters file (".param")
    class paramRow {
    public:
      string name;
      string type;
      string param;
      double value;
      paramRow() : name(""), type(""), param(""), value(0.) {};
    };

    
  public:
    // exceptions
    class noMatchingElement : public pal::palatticeError {
    public:
      noMatchingElement(std::string elementDescription)
	: palatticeError("No matching element found (" + elementDescription + ")") {}
    };
    
    class noFreeSpace : public pal::palatticeError {
    public:
      noFreeSpace(string first, string second) : pal::palatticeError(first + " cannot be inserted --- overlap with " + second) {}
    };
};





  
string removeQuote(string s); //remove quotation marks ("" or '') from begin&end of string


  
#include "AccIterator.hxx"

} //namespace pal


#endif
/*__LIBPALATTICE_ACCLATTICE_HPP_*/
