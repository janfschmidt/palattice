/* === AccLattice Class ===
 * a container to store elements of an accelerator (ring) by position
 * They can be imported from MAD-X or ELEGANT.
 * Uses the "AccElements" classes
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include "AccLattice.hpp"

using namespace pal;

//remove quotation marks ("" or '') from begin&end of string
string pal::removeQuote(string s)
{
  if ( (s.compare(0,1,"\"") && s.compare(s.length()-1,1,"\"")) || (s.compare(0,1,"\'") && s.compare(s.length()-1,1,"\'")) )
    return s.substr(1,s.length()-2);
  else
    return s;
}


//constructor
AccLattice::AccLattice(double _circumference, Anchor _refPos)
  : circ(0.), ignoreCounter(0), refPos(_refPos)
{
  empty_space = new Drift;

  info.add("Reference pos.", this->refPos_string());

  setCircumference(_circumference);
}

AccLattice::AccLattice(SimToolInstance &sim, Anchor _refPos, string ignoreFile)
  : circ(0.), ignoreCounter(0), refPos(_refPos)
{
  empty_space = new Drift;

  info.add("Reference pos.", this->refPos_string());

  if (ignoreFile != "") this->setIgnoreList(ignoreFile);
  this->simToolImport(sim);
}

//copy constructor
AccLattice::AccLattice(const AccLattice &other)
  : circ(other.circumference()), ignoreList(other.ignoreList), refPos(other.refPos), info(other.info)
{
  empty_space = new Drift;

  for (auto it=other.begin(); it!=other.end(); ++it) {
    this->mount(it.pos(), *(it.element()));
  }
}


AccLattice::~AccLattice()
{
  for (auto it=begin(); it!=end(); ++it) {
    delete it.element();
  }
  delete empty_space;
}


AccLattice& AccLattice::operator= (const AccLattice &other)
{
  stringstream msg;

  if (refPos != other.refPos) {
    msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different refPos ("
  	<< refPos_string() <<"/"<< other.refPos_string() <<")";
    throw palatticeError(msg.str());
  }
  // if (circumference() != other.circumference()) {
  //   msg << "ERROR: AccLattice::operator=(): Cannot assign Lattice - different circumferences ("
  // 	<< circumference() <<"/"<< other.circumference() <<")";
  //   throw palatticeError(msg.str());
  // }
  circ = other.circ;
  ignoreList = other.ignoreList;
  info = other.info;
  
  for (auto it=other.begin(); it!=other.end(); ++it) {
    this->mount(it.pos(), *(it.element()));
  }

  return *this;
}




void AccLattice::setCircumference(double c)
{
  if (c < circ)
    throw std::logic_error("DEBUG: This should not have happened! AccLattice circumference was decreased.");

  circ = c;
  info.add("AccLattice circumference / m", c);
}



// get rotation angle [0,2pi]: increases lin. in bending dipoles, constant in-between. 
double AccLattice::theta(double posIn) const
{
  double theta = 0.;
  // sum theta of all bending dipoles that end is at a pos < posIn
  for (auto it=begin<dipole>(); it.pos(Anchor::end) < posIn; ++it) {
    theta += it.element()->length * it.element()->k0.z; // theta= l/R = l*k0.z
  }
  // if posIn is inside a dipole, add theta of this magnet up to posIn
  try {
    auto atPosIn = at(posIn); // throws if there is no element
    if (atPosIn.element()->type == dipole) {
      theta += (posIn - atPosIn.pos(Anchor::begin)) * atPosIn.element()->k0.z;
    }
  }
  catch (noMatchingElement) {}
  return theta;
}



// get here=begin/center/end (in meter) of obj at reference-position pos
// works for all reference Anchors (refPos)
double AccLattice::locate(double pos, const AccElement *obj, Anchor here) const
{
  double l = obj->length;        //element length

  if (refPos == Anchor::center) pos -= l/2;
  else if (refPos == Anchor::end) pos -= l;

  switch(here) {
  case Anchor::begin:
    return pos;
  case Anchor::center:
    return pos + l/2;
  case Anchor::end:
    return pos + l;
  }

  return 0.;
}



// get element
const AccElement* AccLattice::operator[](double pos) const
{
  if (pos > circumference()) {
    stringstream msg;
    msg << pos << " m is larger than lattice circumference " << circumference() << " m.";
    throw palatticeError(msg.str());
  }

  try {
    return at(pos).element();
  }
  // otherwise pos not inside any element:
  catch (noMatchingElement &e) {
    return empty_space;
  }
}



AccLattice::const_iterator AccLattice::begin(element_type t, element_plane p, element_family f) const
{
  auto it=begin();
  for (; it!=end(); ++it) {
    if ( it.element()->type==t && (p==noplane || it.element()->plane==p) && (f==nofamily || it.element()->family==f) )
      break;
  }
  return it;
}

// get iterator by name, throws noMatchingElement if name is not found
// ! name can be ambiguous! always returns first match
AccLattice::const_iterator AccLattice::operator[](string _name) const
{
  for (auto it=begin(); it!=end(); ++it) {
    if (it.element()->name == _name)
      return it;
  }
  // otherwise name does not match any element:
  throw noMatchingElement("No element "+_name+" found");
}

// get iterator by position, throws noMatchingElement if pos is in Drift
AccLattice::const_iterator AccLattice::at(double pos) const
{
  // pos can be within the following elements:
  // | refPos | --upper_bound(pos) | upper_bound(pos) |
  // |--------+--------------------+------------------|
  // | begin  |          X         |                  |
  // | center |          X         |        X         |
  // | end    |                    |        X         |
   auto it = const_iterator(elements.upper_bound(pos),&elements,&refPos,&circ);
   if (it!=end() && it.at(pos))
     return it;
   --it;
   if (it.at(pos))
     return it;
   
  std::stringstream s;
  s << "no element at " << pos << " m";
  throw noMatchingElement(s.str());
}

// get iterator to next element with "anchor" behind given position
AccLattice::const_iterator AccLattice::behind(double pos, Anchor anchor) const
{
  try {
    auto it = at(pos);
    if (it.pos(anchor) > pos)
      return it;
    else
      return ++it;
  }
  catch (noMatchingElement) {
    return const_iterator(elements.upper_bound(pos),&elements,&refPos,&circ);
  }
}


// non-const implementations:
// no duplication: call const_iterator implementation, loop iterator and 
// compare to result of const_iterator implementation (uses cast iterator -> const_iterator)
// [no dirty trick to cast from const_iterator to iterator, but slower]
AccLattice::iterator AccLattice::cast_helper(const const_iterator& result)
{
  iterator it=begin();
  std::advance( it, std::distance(const_iterator(it), result) );
  return it;
}
AccLattice::iterator AccLattice::begin(element_type t, element_plane p, element_family f) {
  return cast_helper(const_cast<const AccLattice*>(this)->begin(t,p,f));
}
AccLattice::iterator AccLattice::operator[](string _name) {
  return cast_helper(const_cast<const AccLattice*>(this)->operator[](_name));
}
AccLattice::iterator AccLattice::at(double pos) {
  return cast_helper(const_cast<const AccLattice*>(this)->at(pos));
}
AccLattice::iterator AccLattice::behind(double pos, Anchor anchor) {
  return cast_helper(const_cast<const AccLattice*>(this)->behind(pos,anchor));
}





// mount element (replace if key (pos) already used; check for "free space" to insert element)
// if name is in ignoreList, element is not set and ignoreCounter is increased
void AccLattice::mount(double pos, const AccElement& obj, bool verbose)
{
  //ignoreList
  if ( obj.nameMatch(ignoreList) ) {
    ignoreCounter++;
    //metadata
    stringstream ignore;
    ignore << ignoreCounter;
    info.add("ignored elements", ignore.str());
    return;
  }

  bool first_element = false;
  bool last_element = false;
  const AccElement *objPtr = &obj;
  double newBegin = locate(pos, objPtr, Anchor::begin);
  double newEnd = locate(pos, objPtr, Anchor::end);
  stringstream msg, msg2;
  
  if (pos < 0.) {
    stringstream msg;
    msg << "ERROR: AccLattice::mount(): Position of Lattice elements must be > 0. " << pos << " is not." <<endl;
    throw palatticeError(msg.str());
  }

  // empty map (mount first element)
  if (elements.size() == 0) {
    elements[pos] = objPtr->clone();
    if (verbose) cout << objPtr->name << " inserted." << endl;
    return;
  }

  //"first element whose key goes after pos"
  auto next = iterator(elements.upper_bound(pos),&elements,&refPos,&circ);
  auto previous = next;
  if (next == begin())
    first_element = true;
  else
    previous--;
  if (next == end()) //"past-the-end element"
    last_element = true;

 
  // try deleting possibly existing element at pos
  try{
    delete elements.at(pos);
  }
  catch(std::out_of_range &e){ }

  // avoid numerical problems when checking for "free space"
  if (!first_element && abs(newBegin - previous.end()) < ZERO_DISTANCE) {
    newBegin += ZERO_DISTANCE;
  }
  if (!last_element && abs(newEnd - next.begin()) < ZERO_DISTANCE)
      newEnd -= ZERO_DISTANCE;

  // check for "free space" to insert obj
  if (newBegin < 0.) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m)";
    throw noFreeSpace(msg.str(), "lattice begin at 0.0m");
  }
  else if (!first_element &&  newBegin < previous.end()) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m)";
    msg2 << previous.element()->name << " ("<< previous.begin() <<" - "<< previous.end() << "m)";
    throw noFreeSpace(msg.str(), msg2.str());
  }
  else if (!last_element && newEnd > next.begin()) {
    msg << objPtr->name << " (" << newBegin <<" - "<< newEnd << "m)";
    msg2 << next.element()->name << " ("<< next.begin() <<" - " << next.end() << "m)";
    throw noFreeSpace(msg.str(), msg2.str());
  }
  //if there is free space:
  else elements[pos] = objPtr->clone();

  // update circumference
  if (newEnd > circumference())
    setCircumference(newEnd);

  if (verbose) cout << objPtr->name << " inserted." << endl;

}


// dismount element at position pos
void AccLattice::dismount(double pos)
{
  auto it =  elements.find(pos);

  if (it == elements.end()) {
    cout << "WARNING: AccLattice::dismount(): There is no element at position "<<pos<< " m. Nothing is dismounted." << endl;
    return;
  }
  elements.erase(it);
}




// elements with a name in this list (can contain 1 wildcard * per entry) are not mounted (set) in this lattice
void AccLattice::setIgnoreList(string ignoreFile)
{
  ifstream f;
  string tmp;

  f.open(ignoreFile.c_str());
  if (!f.is_open()) {
    throw palatticeFileError(ignoreFile);
  }

  //metadata
  info.add("ignore list", ignoreFile);

  while (!f.eof()) {
    f >> tmp;
    ignoreList.push_back(tmp);
  }

  //metadata
  info.add("ignore list", ignoreFile);
}




// mount elements from MAD-X Lattice (read from twiss-output)
// - arbitrary column order
// - error if needed column(s) missing
// madxFile is:
// * a file that can be executed with madx (mode=online)
// * a madx twiss file (mode=offline)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// MAD-X uses different signs of strengths (k,m)
// ===================================================
void AccLattice::madximport(SimToolInstance &madx)
{
  if (madx.tool != pal::madx)
    throw palatticeError("AccLattice::madximport() is only allowed for SimToolInstance::tool=madx");

  // run MadX
  if (madx.mode == pal::online)
    madx.run();

  string madxTwissFile=madx.lattice();
  
  //get metadata and set circumference
    try {
      info.simToolImport(madx);
      setCircumference(madx.readCircumference());
    } catch(palatticeFileError &e) {
      std::cout << "WARNING: Cannot read circumference & SimTool metadata from " <<std::endl
		<<e.filename << std::endl;
    }

  if (refPos != Anchor::end)
    cout << "WARNING: AccLattice::madximport(): The input file (MAD-X twiss) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;


  //read columns from file (execute madx if mode=online)
  SimToolTable twi;
  twi = madx.readTable(madxTwissFile, {
      "KEYWORD",
	"NAME",
	"S",
	"L",
	"ANGLE",
	"K1L",
	"K2L",
	"HKICK",
	"VKICK",
	"E1",
	"E2",
	"TILT",
	"APERTYPE",
	"APER_1",
	"APER_2",
	"VOLT",
	"FREQ"
	}
    );


  //mount elements
  AccElement *element;
  double s=0.;
  for (unsigned int i=0; i<twi.rows(); i++) {
    string key = twi.gets(i,"KEYWORD");
    string name = removeQuote(twi.gets(i,"NAME"));
    double l = twi.getd(i,"L");
    double angle = twi.getd(i,"ANGLE");
    double hkick = twi.getd(i,"HKICK");
    double vkick = twi.getd(i,"VKICK");

    if (key == "\"SBEND\"" || key == "\"RBEND\"") {  //horizontal bending Dipole (assume all bends have vertical field)
      element = new Dipole(name, l, H);
    }
    else if (key == "\"QUADRUPOLE\"") {
      element = new Quadrupole(name, l, F);
    }
    else if (key == "\"SEXTUPOLE\"") {
      element = new Sextupole(name, l, F);
    }
    else if (key == "\"VKICKER\"") {
      element = new Corrector(name, l,V);
    }
    else if (key == "\"HKICKER\"") {
      element = new Corrector(name, l,H);
    }
    else if (key == "\"KICKER\"") {
      element = new Corrector(name, l);
    }
    else if (key == "\"RFCAVITY\"") {
      element = new Cavity(name, l);
    }
    else if (key == "\"MULTIPOLE\"") {
      element = new Multipole(name, l);
    }
    else if (key == "\"MARKER\"") {
      element = new Marker(name);
    }
    else if (key == "\"MONITOR\"") {
      element = new Monitor(name, l);
    }
    else if (key == "\"RCOLLIMATOR\"") {
      element = new Rcollimator(name, l);
    }
    else continue; //Drifts are not mounted explicitly
    
    // rf magnet
    // if (twi.gets("NAME",i).substr(0,6) == "RFDIP.") {
    //   element->Qrf0 = 0.625;          // !! hardcoded RF-tune values !!
    //   element->dQrf = 5.402e-6;
    // }

    if (angle!=0.) element->k0.z += angle / l; // 1/R from bending angle, curved length l
    if (hkick!=0. && element->plane!=V)  element->k0.z += sin(hkick) / l; // 1/R from kick angle, straight length l
    if (vkick!=0. && element->plane!=H)  element->k0.x += sin(vkick) / l;
    element->e1 = twi.getd(i,"E1");
    element->e2 = twi.getd(i,"E2");
    if (twi.gets(i,"APERTYPE")=="\"RECTANGLE\"") {
      element->halfWidth.x = twi.getd(i,"APER_1");
      element->halfWidth.z = twi.getd(i,"APER_2");
    }
    if (element->type == cavity) {
      element->volt = twi.getd(i,"VOLT") * 1e6;
      element->freq = twi.getd(i,"FREQ") * 1e6;
    }
    element->k1 = twi.getd(i,"K1L")/l;
    element->k2 = twi.getd(i,"K2L")/l;
    element->dpsi += - twi.getd(i,"TILT"); // non-error tilt (e.g. skew magnets), sign see misalignments
    //misalignments in AccLattice::madximportMisalignments()
    s = twi.getd(i,"S");
    if (refPos == Anchor::begin) s -= l;
    else if (refPos == Anchor::center) s -= l/2;
    this->mount(s, *element);
    delete element;
  }

  // import misalignments
  this->madximportMisalignments(dipole, madx.outFile("dipealign"));
  this->madximportMisalignments(quadrupole, madx.outFile("quadealign"));


  //info stdout
  cout << this->sizeSummary() << " read" << endl
       <<"  as " <<this->circumference() << "m lattice from " <<madxTwissFile<<endl;
  if (this->ignoreList.size() > 0) {
    cout <<"* "<<this->ignoredElements()<<" elements ignored due to match with ignore list."<<endl;
  }
}




// set misalignments from MAD-X Lattice (read ealign-output)
// ! currently only rotation (dpsi) around beam axis (s) is implemented   !
// ! to add others: define member in class AccElements & implement import !
// *************************sign of rotation angle:*********************************
// test with influence of dpsi on vertical closed orbit in madx show
// that dpsi is defined counter clockwise (dpsi>0 for dipole => kick to negative z)
// libpalattice and elegant (tilt) use clockwise definition, so sign is changed here
// to get the correct signs for the magnetic fields calculated from dpsi
// *********************************************************************************
void AccLattice::madximportMisalignments(element_type t, string madxEalignFile)
{
  SimToolInstance madx(pal::madx, pal::offline, madxEalignFile);

  //read columns from file (execute madx if mode=online)
  SimToolTable ealign;
  ealign = madx.readTable(madxEalignFile, {"NAME", "DPSI"});

  //set misalignments to AccLattice elements
  string type = "-";
  for (auto ele : *this) {
    for (unsigned int i=0; i<ealign.rows(); i++) {
      if (ele->type==t && ele->name==removeQuote(ealign.gets(i,"NAME"))) {
	ele->dpsi += - ealign.getd(i,"DPSI");    // <<<<<<!!! sign of rotation angle (see comment above)
	type = ele->type_string();
      }
    }
  }

  //metadata
  info.add("MAD-X " + type + " misalignments from", madxEalignFile);
}





// mount elements from elegant Lattice (read from ascii parameter file ".param")
// misalignments included (no separate function as for MADX)
// >> here the file is not read by SimToolInstance::readTable,
// >> because of different layout of .param file (no column based table!)
// ====== ATTENTION ==================================
// "FamilyMagnets" (Quad,Sext) all of type F, because
// elegant uses different signs of strengths (k,m)
// ===================================================
void AccLattice::elegantimport(SimToolInstance &elegant)
{
  if (elegant.tool != pal::elegant)
    throw palatticeError("AccLattice::elegantimport() is only allowed for SimToolInstance::tool=elegant");

  string elegantParamFile = elegant.lattice();

  // run Elegant
  if (elegant.mode == pal::online)
    elegant.run();

  double s, pos;
  double l, k1, k2, angle, kick, hkick, vkick, tilt, e1,e2,volt,freq; //parameter values
  AccPair halfWidth;
  paramRow row, row_old;
  AccElement *element;
  fstream elegantParam;
  bool firstElement = true;
  string tmp;

  pos=l=k1=k2=angle=kick=tilt=e1=e2=volt=freq=0.;   // initialize param. values
  s = 0.;

  //get metadata and set circumference
  try {
    info.simToolImport(elegant);
    setCircumference(elegant.readCircumference());
  } catch(palatticeFileError &e) {
    std::cout << "WARNING: Cannot read circumference & SimTool metadata from " <<std::endl
	      << e.filename << std::endl;
  }

  if (refPos != Anchor::end)
    cout << "WARNING: AccLattice::elegantimport(): The input file (elegant parameter) uses element end as positions." << endl
	 << "They are transformed to the current Anchor set for this lattice: " << refPos_string() << endl;

  SimToolTable tab = elegant.readTable(elegant.lattice(), {"ElementName", "ElementParameter", "ParameterValue", "ElementType"});
  
  for (auto i=0u; i<tab.rows(); i++) {
    row.name = tab.gets(i, "ElementName");
    row.type = tab.gets(i, "ElementType");
    row.param = tab.gets(i, "ElementParameter");
    row.value = tab.getd(i, "ParameterValue");

    if (firstElement) {
      row_old = row;
      firstElement = false;
    }

    //mount element if next element reached (=all parameters read)
    if (row.name != row_old.name) {

     if (row_old.type=="CSBEND" || row_old.type=="CSRCSBEND" || row_old.type=="KSBEND" || row_old.type=="NIBEND" || row_old.type=="TUBEND" || row_old.type=="SBEN") {
       element = new Dipole(row_old.name, l, H);
     }
     else if (row_old.type=="QUAD" || row_old.type=="KQUAD") {
       element = new Quadrupole(row_old.name, l, F);
     }
     else if (row_old.type=="SEXT" || row_old.type=="KSEXT") {
       element = new Sextupole(row_old.name, l, F);
     }
     else if (row_old.type=="VKICK") {
       element = new Corrector(row_old.name, l, V);
       if (kick!=0.)  element->k0.x += sin(kick) / l; // 1/R from kick angle, straight length l
     }
     else if (row_old.type=="HKICK") {
       element = new Corrector(row_old.name, l, H);
       if (kick!=0.)  element->k0.z += sin(kick) / l;
     }
     else if (row_old.type=="KICKER") {
       element = new Corrector(row_old.name, l);
       element->k0.x += sin(vkick) / l;
       element->k0.z += sin(hkick) / l;
     }
     else if (row_old.type=="RFCA") {
       element = new Cavity(row_old.name, l);
     }
     else if (row_old.type=="MARK") {
       element = new Marker(row_old.name);
     }
     else if (row_old.type=="MONI") {
       element = new Monitor(row_old.name, l);
     }
     else if (row_old.type=="RCOL") {
       element = new Rcollimator(row_old.name, l);
     }
     else
       element = new Drift(); //Drifts are not mounted explicitly
     
     // rf magnet
     // if (row_old.name.substr(0,6) == "RFDIP.") {
     //   element->Qrf0 = 0.625;          // !! hardcoded RF-tune values !!
     //   element->dQrf = 5.402e-6;
     // }

     if (element->type != drift) {
       if (angle!=0.) element->k0.z += angle / l; // 1/R from bending angle, curved length l
       element->k1 = k1;
       element->k2 = k2;
       element->dpsi = tilt;
       element->e1 = e1;
       element->e2 = e2;
       element->halfWidth = halfWidth;
       if (element->type == cavity) {
	 element->volt = volt;
	 element->freq = freq;
       }
	      
       if (refPos == Anchor::begin) pos = s-l;
       else if (refPos == Anchor::center) pos = s-l/2;
       else pos = s; 
       this->mount(pos, *element); // mount element
     }
     delete element;
     // clear param. values to avoid reuse of an old value
     pos=l=k1=k2=angle=kick=hkick=vkick=tilt=e1=e2=volt=freq=0.;
     halfWidth = AccPair();
    }

    //read parameter in row (if needed)
    if (row.param == "L") {    //element length L used to get position (s)
      l = row.value;
      s += l;
    }
    else if (row.param == "K1") k1 = row.value;
    else if (row.param == "K2") k2 = row.value;
    else if (row.param == "ANGLE") angle = row.value;
    else if (row.param == "KICK") kick = row.value;
    else if (row.param == "HKICK") hkick = row.value;
    else if (row.param == "VKICK") vkick = row.value;
    else if (row.param == "ETILT") tilt += row.value;
    else if (row.param == "TILT") tilt += row.value;
    else if (row.param == "E1") e1 = row.value;
    else if (row.param == "E2") e2 = row.value;
    else if (row.param == "X_MAX") halfWidth.x = row.value;
    else if (row.param == "Y_MAX") halfWidth.z = row.value;
    else if (row.param == "VOLT") volt = row.value;
    else if (row.param == "FREQ") freq = row.value;
    //... add more parameters here

   row_old = row;
  }
  
  //info stdout
  cout << this->sizeSummary() << " read" << endl
       <<"  as " <<this->circumference() << "m lattice from " <<elegantParamFile<<endl;
  if (this->ignoreList.size() > 0) {
    cout <<"* "<<this->ignoredElements()<<" elements ignored due to match with ignore list."<<endl;
  }
}







// change quad&sext strengths to values from "ELSA-Spuren"
void AccLattice::setELSAoptics(string spurenFolder)
{
  char filename[1024];
  string tmp;
  stringstream msg;
  double kf=0, kd=0, mf=0, md=0;
  fstream f_magnets;

 // Read "optics.dat"
  snprintf(filename, 1024, "%s/optics.dat", spurenFolder.c_str());
  f_magnets.open(filename, ios::in);
  if (!f_magnets.is_open()) {
    msg << "ERROR: AccLattice::setELSAoptics(): Cannot open " << filename;
    throw palatticeError(msg.str());
  }
  while(!f_magnets.eof()) {
    f_magnets >> tmp;
    if (tmp=="ELS_MAGNETE_QUADF.KF_AC")
      f_magnets >> kf;
    else if (tmp=="ELS_MAGNETE_QUADD.KD_AC")
      f_magnets >> kd;
    else if (tmp=="ELS_MAGNETE_SEXTF.MF_AC")
      f_magnets >> mf;
    else if (tmp=="ELS_MAGNETE_SEXTD.MD_AC")
      f_magnets >> md;
    else if (tmp=="#")         //ignore comment lines
      getline(f_magnets, tmp);
    else {
      msg << "ERROR: AccLattice::setELSAoptics(): Unexpected entry in " << filename;
      throw palatticeError(msg.str());
    }
  }
  f_magnets.close();

  //Write strengths to quads
  for (auto it=begin<quadrupole>(); it!=end(); ++it) {
    if (it.element()->name.compare(1,2,"QF") == 0) {
      it.element()->k1 = kf;
    }
    else if (it.element()->name.compare(1,2,"QD") == 0) {
      it.element()->k1 = -kd;
    }
  }

  //Write strengths to sexts
  for (auto it=begin<sextupole>(); it!=end(); ++it) {
    if (it.element()->name.compare(1,2,"SF") == 0) {
      it.element()->k2 = mf;
    }
    else if (it.element()->name.compare(1,2,"SD") == 0) {
      it.element()->k2 = -md;
    }
  }

 //metadata
 info.add("ELSA optics from", spurenFolder);
}



// change corrector pos&strength to values from "ELSA-Spuren" at time t/ms
// return number of correctors, where set
unsigned int AccLattice::setELSACorrectors(ELSASpuren &spuren, unsigned int t)
{
  unsigned int i, n=0;
 AccElement* corrTmp;
 char name1[20], name2[20];
 char msg[1024];
 stringstream strMsg;
 double diff=0;
 double endPos;
 type_iterator<pal::corrector,pal::V> it = begin<pal::corrector,pal::V>(); // only vertical correctors!
 type_iterator<pal::corrector,pal::V> it_next = it;

 
 for (i=0; i<NVCORRS; i++) {
   if (spuren.vcorrs[i].pos==0.0) {   //inactive correctors have pos=0 in VCORRS.SPOS
     continue;
   }
   else if (t > spuren.vcorrs[i].time.size()) {
     snprintf(msg, 1024, "ERROR: AccLattice::setELSACorrectors(): No ELSA VC%02d corrector data available for %d ms.\n", i+1, t);
     throw palatticeError(msg);
   }
   
   //same corrector in Mad-X and ELSA-Spuren?
   //...check by name
   snprintf(name1, 20, "KV%02i", i+1); // old madx-lattice: KVxx
   snprintf(name2, 20, "VC%02i", i+1); // new madx-lattice (2014): VCxx
   if (it.element()->name != name1 && it.element()->name != name2) {
     strMsg << "ERROR: AccLattice::setELSACorrectors(): Unexpected corrector name. Mad-X lattice does not fit to ELSA." << endl;
     strMsg << "       Mad-X: " <<it.element()->name<< " -- expected: " <<name2<< " (" <<name1<< ")" << endl;
     throw palatticeError(strMsg.str());
   }
   //...check by position
   diff = spuren.vcorrs[i].pos - it.center();
   if (fabs(diff) > VCPOS_WARNDIFF) {
     cout << "! Position of " <<name2<< " differs by " <<diff<< "m in Mad-X and ELSA-Spuren. Use ELSA-Spuren." << endl;
   }
   
   corrTmp = it.element()->clone();
   corrTmp->k0.x = spuren.vcorrs[i].time[t].kick/1000.0/corrTmp->length;   //unit 1/m
   it_next = it;
   ++it_next;
   dismount(it);   // erase "old" corrector (madx) to be able to mount new one
   endPos = spuren.vcorrs[i].pos + corrTmp->length/2;
   this->mount(endPos, *(corrTmp));
   delete corrTmp;

   it = it_next;
   n++;
   if (it == end())
     break;
 }
 
 if (n != this->size(corrector,V)) // only vertical correctors!
   cout << "WARNING: Not all correctors overwritten by ELSA-Spuren" << endl;

 //metadata
 stringstream spureninfo;
 spureninfo << t << "ms in "<<spuren.spurenFolder;
 info.add("ELSA VCs from", spureninfo.str());
 
 return n;
}




// subtract other corrector strengths from the ones of this lattice
void AccLattice::subtractCorrectorStrengths(AccLattice &other)
{
  stringstream msg;
  
  if (this->size(corrector) != other.size(corrector)) {
    msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal number of correctors to subtract.";
    throw palatticeError(msg.str());
  }
  
  auto otherIt = other.begin<pal::corrector>();

  for  (auto it=begin<pal::corrector>(); it!=end(); ++it) {

    // check by name
    if (otherIt.element()->name != it.element()->name) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal names of correctors to subtract. ("
	  << it.element()->name <<"/"<< otherIt.element()->name << ").";
      throw palatticeError(msg.str());
    }
    // check by position
    if (otherIt.pos() != it.pos()) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal positions of correctors to subtract. ("
	  << it.pos() <<"/"<< otherIt.pos() << ").";
      throw palatticeError(msg.str());
    }
    // check plane
    if (otherIt.element()->plane != it.element()->plane) {
      msg << "ERROR: AccLattice::subtractCorrectorStrengths(): Unequal planes of correctors to subtract.";
      throw palatticeError(msg.str());
    }

    // subtract
    it.element()->k0 -= otherIt.element()->k0;
    // set otherIt to next corrector
    ++otherIt;
  }

  //metadata
  string tmp = other.info.getbyLabel("Lattice Source file");
  if (tmp != "NA")
    info.add("subtracted corrector strengths", tmp);
  else
    info.add("subtracted corrector strengths", "no lattice source file");
}


// subtract other misalignments from the ones of this lattice
void AccLattice::subtractMisalignments(const AccLattice &other)
{
  stringstream msg;
  
  if (this->size() != other.size()) {
    msg << "ERROR: AccLattice::subtractMissalignments(): Unequal number of elements to subtract.";
    throw palatticeError(msg.str());
  }
  
  auto otherIt = other.begin();

  for  (auto it=begin(); it!=end(); ++it) {

    // check by name
    if (otherIt.element()->name != it.element()->name) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal names of elements to subtract. ("
	  << it.element()->name <<"/"<< otherIt.element()->name << ").";
      throw palatticeError(msg.str());
    }
    // check by position
    if (otherIt.pos() != it.pos()) {
      msg << "ERROR: AccLattice::subtractMisalignments(): Unequal positions of elements to subtract. ("
	  << it.pos() <<"/"<< otherIt.pos() << ").";
      throw palatticeError(msg.str());
    }

    // subtract
    it.element()->dpsi -= otherIt.element()->dpsi;
    // set otherIt to next corrector
    ++otherIt;
  }

  //metadata
  string tmp = other.info.getbyLabel("Lattice Source file");
  if (tmp != "NA")
    info.add("subtracted misalignments", tmp);
  else
    info.add("subtracted misalignments", "no lattice source file");
}



// ------------------ "information" -----------------------
// returns number of elements of a type in this lattice
unsigned int AccLattice::size(element_type _type, element_plane p, element_family f) const
{
  unsigned int n=0;
  for (auto it=begin(); it!=end(); ++it) {
    if (it.element()->type == _type && (p==noplane || it.element()->plane==p) && (f==nofamily || it.element()->family==f))
      n++;
  }

  return n;
}


string AccLattice::sizeSummary() const
{
  stringstream s;
  s << "* "<<this->size(dipole)<<" dipoles, "<<this->size(quadrupole)<<" quadrupoles, "
    <<this->size(sextupole)<<" sextupoles, "<<this->size(corrector)<<" correctors, "
    <<this->size(cavity)<<" cavities, "<<this->size(multipole)<<" multipoles";
  return s.str();
}

string AccLattice::refPos_string() const
{
  switch (refPos) {
  case Anchor::begin:
    return "begin";
  case Anchor::center:
    return "center";
  case Anchor::end:
    return "end";
  }
  
  return "Please implement this type in AccLattice::refPos_string()!";
}


double AccLattice::slope(double pos, const_iterator it) const
{
  double x=1.;
  double dl = it.element()->dl()/2; // half of dl() at each magnet end
  double distBegin = it.distanceRing(Anchor::begin,pos) - dl; // distance to phys. begin of it
  double distEnd = it.distanceRing(Anchor::end,pos) + dl;     // distance to phys. end of it
  if (distBegin < 0) x = distBegin;
  else if (distEnd > 0) x = distEnd;
  else return 1.;

  double sigma = dl * sqrt(2./M_PI); // dl * 0.797884561
  return exp(-0.5*pow(x/sigma,2));
}

// magnetic field including edge field (with slope)
AccTriple AccLattice::B(double posIn, const AccPair &orbit) const
{
  double pos = posMod(posIn);
  unsigned int t = turn(posIn);
  // next magnet with center > pos:
  const_iterator it = behind(pos,Anchor::center);
  AccTriple field;
  field = it.element()->B_rf(t,orbit) * slope(pos, it); //B_rf includes rf magnets
  // previous magnet (center <= pos):
  if (it == begin()) it = end();
  --it;                                       
  field += it.element()->B_rf(t,orbit) * slope(pos, it);

  return field;
}







// ----------- output & export (stdout or file) ----------------------

// print lattice. If no filename is given, print to stdout
void AccLattice::print(string filename) const
{
  const_iterator it=begin();
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << info.out("#");
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it.element()->printHeader();

  for (; it!=end(); ++it) {
    s <<std::setw(w+1)<< it.pos() << it.element()->print();
  }

  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
      throw palatticeFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}


// print all elements of one type If no filename is given, print to stdout
void AccLattice::print(element_type _type, string filename) const
{
  const_iterator it=begin(_type);
  const int w = 12;
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << info.out("#");
  s << "# List of " << it.element()->type_string() << "s only!" << endl;
  s <<"#"<< std::setw(w) << "Ref.Pos/m" << it.element()->printHeader();

  for (; it!=end(); it.next(_type)) {
    s <<std::setw(w+1)<< it.pos() << it.element()->print();
  }

  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open())
     throw palatticeFileError(filename);
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }

}



// return list of elegant or madx compliant element definitions for given type
string AccLattice::getElementDefs(SimTool tool, element_type _type) const
{
  stringstream s;
  const_iterator it=begin(_type);

  if (it == end())
    return "";
  s << "! " << it.element()->type_string() << "s";
  if (_type==dipole && tool==elegant)
    s <<" (synch_rad & isr for synchrotron radiation)";
  s << endl;
  for (; it!=end(); it.next(_type)) {
    //only list identical elements once
    auto firstOccur = operator[](it.element()->name); //first element in lattice with this name
    if ((it.pos()-firstOccur.pos()) < ZERO_DISTANCE || *(firstOccur.element()) != *(it.element())) {
      s << it.element()->printSimTool(tool);
    }
  }
  s << endl;

  return s.str();
}


// return lattice in elegant or madx compliant "LINE=(..." format
// including definition of drifts in-between elements
string AccLattice::getLine(SimTool tool) const
{
  std::stringstream s;    //drift definitions
  std::stringstream line; //lattice line 
  double driftlength, lastend;
  const_iterator it=begin();

  line << "LIBPALATTICE : LINE=(";

  //marker at pos=0.0 if lattice starts with drift
  if (it.pos() > ZERO_DISTANCE || it.element()->type != marker) {
    s << "BEGIN : ";
    if (tool==elegant)
      s << "MARK";
    else
      s << "MARKER;";
    s << endl << endl;
    line << "BEGIN, ";
  }
  s << "! Drifts (caculated from element positions)" << endl; //Drifts


  unsigned int n=0u, nInRow=1u;
  for (; it!=end(); ++it) {
    //comma
    if (it!=begin()) line << ", ";
    
    //line break
    if (nInRow >= EXPORT_LINE_ELEMENTS_PER_ROW) {
      if (tool==elegant)
	line <<"&";
      line << std::endl << "                    ";
      nInRow = 0;
    }

    //calc, define and insert Drift
    if (it==begin()) {
      driftlength = it.begin();
    }
    else {
      driftlength = it.begin() - lastend;
    }
    lastend = it.end();
    if (driftlength > ZERO_DISTANCE) { //ignore very short drifts
      s << "DRIFT_" << n << " : ";
      if (tool==elegant)
	s << "DRIF, l=";
      else
	s << "DRIFT, L=";
      s << driftlength <<";"<< endl;  //drift definition
      line << "DRIFT_" << n << ", ";  //insert drift in line
      nInRow++;
      n++;
    }
    
    //insert element
    line << it.element()->name;
    nInRow++;
  }//loop elements in lattice
  
  //final drift to end
  if ( (circumference() - lastend) > ZERO_DISTANCE ) { //ignore very short drift
    s << "DRIFT_" << n << " : ";
    if (tool==elegant)
      s << "DRIF, l=";
    else
      s << "DRIFT, L=";
    s << this->circumference() - lastend <<";"<<endl;
    line << "DRIFT_" << n;   // drift to end
  }

  s << endl<<endl << line.str() << ")";
  if (tool==madx)
    s << ";";

  return s.str();
}


// return lattice in madx compliant "SEQUENCE" format
string AccLattice::getSequence(Anchor refer) const
{
  std::stringstream s;

  s << "LIBPALATTICE : SEQUENCE, REFER=";
  if (refer==Anchor::center)
    s << "CENTRE, ";
  else if (refer==Anchor::begin)
    s << "ENTRY, ";
  else if (refer==Anchor::end)
    s << "EXIT, ";
  s << "L=" << this->circumference() <<";"<< endl;

    const_iterator it=begin();
    
    //marker at pos=0.0 if lattice starts with drift
    if (it.pos() > ZERO_DISTANCE || it.element()->type != marker)
      s << "BEGIN : MARKER, AT=0.0;" << endl;

  for (; it!=end(); ++it) {
    s << it.element()->name << ", AT=" << it.pos(refer) << ";" << endl;
  }
  s << "END : MARKER, AT=" <<this->circumference() <<";"<< endl;
  s << "ENDSEQUENCE;";

  return s.str();
}


// print lattice readable by elegant or madx. If no filename is given, print to stdout
void AccLattice::simToolExport(SimTool tool, string filename, MadxLatticeType ltype) const
{
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  if (tool==elegant)
    s << "! Lattice for ELEGANT" << endl;
  else
    s << "! Lattice for MAD-X" << endl;
  s << info.out("!") << endl;


  //write element definitions
  for (element_type type=dipole; type<drift; type=element_type(type+1)) {
    s << this->getElementDefs(tool,type);
  }
  s << endl;

  //write lattice
  if (tool == madx && ltype == sequence) {
    s << getSequence();
  }
  else {
    s << getLine(tool);
  }
  


  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open()) {
      msg << "ERROR: AccLattice::elegantexport(): Cannot open " << filename << ".";
      throw palatticeError(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }
}




// print lattice readable by LaTeX. If no filename is given, print to stdout
// (using package tikz-palattice)
void AccLattice::latexexport(string filename) const
{
  std::stringstream s;
  std::stringstream msg;
  fstream file;

  //write text to s
  s << "% Lattice for LaTeX" << endl
    << "%" << endl;
  s << info.out("%")
    << endl;

  //preamble
  s << "\\documentclass[]{standalone}" <<endl
    << "\\usepackage[ngerman]{babel}" <<endl
    << "\\usepackage[utf8]{inputenc}" <<endl
    << "\\usepackage{tikz-palattice} % available at CTAN, included in TeXlive and MikTeX" <<endl<<endl;

  //lattice
  s << "\\begin{document}" << endl << "\\begin{lattice}" << endl;
  double driftlength, lastend;
  for (auto it=begin(); it!=end(); ++it) {
    if (it == begin()) {
      driftlength = it.begin();
    }
    else {
      driftlength = it.begin() - lastend;
    }
    lastend = it.end();
    s << getLaTeXDrift(driftlength);    //drift
    s << it.element()->printLaTeX(); //element
  }

  //drift to end
  driftlength = circumference() - lastend;
  s << getLaTeXDrift(driftlength);

  s << "\\end{lattice}" << endl << "\\end{document}" << endl;


  // output of s
  if (filename == "") 
    cout << s.str();
  else {
    file.open(filename.c_str(), ios::out);
    if (!file.is_open()) {
      msg << "ERROR: AccLattice::latexexport(): Cannot open " << filename << ".";
      throw palatticeError(msg.str());
    }
    file << s.str();
    file.close();
    cout << "* Wrote " << filename  << endl;
  }
}



//energy loss per turn in keV for electron beam with energy given by gamma
double AccLattice::Erev_keV_syli(const double& gamma) const
{
  double dE = 0;
  for(auto it=begin<pal::dipole>(); it!=end(); ++it) {
    dE += std::pow(it.element()->k0.abs(), 2) * it.element()->length;
  }
  dE *= std::pow(gamma, 4) * GSL_CONST_MKSA_ELECTRON_CHARGE / (6*M_PI*GSL_CONST_MKSA_VACUUM_PERMITTIVITY); // in eV
  return std::move(dE/1000.); //in keV
}

// overvoltage factor q, from total voltage of all cavities
double AccLattice::overvoltageFactor(const double& gamma) const {
  double U = 0;
  for(auto it=begin<pal::cavity>(); it!=end(); ++it) {
    U += it.element()->volt;
  }
  return U / (Erev_keV_syli(gamma) * 1000.);
}


// integral over bending radius around ring: R^exponent ds
double AccLattice::integralDipoleRadius(int exponent) const
{
  double sum = 0;
  double totalLength = 0;
  for(auto it=begin<pal::dipole>(); it!=end(); ++it) {
    sum += std::pow(it.element()->k0.abs(), -1*exponent) * it.element()->length;
    totalLength += it.element()->length;
  }
  return sum / totalLength;
}

// harmonic number h, from cavity frequency and circumference
unsigned int AccLattice::harmonicNumber() const {
  auto it = begin<pal::cavity>();
  double freq = it.element()->freq;
  
  for(; it!=end(); ++it) {
    if (it.element()->freq != freq)
      throw palatticeError("harmonicNumber(): Cavities with different frequencies in Lattice. What is the definition of h in this case??");
  }

  return (unsigned int) std::round( freq / (GSL_CONST_MKSA_SPEED_OF_LIGHT/circumference()) );
}
