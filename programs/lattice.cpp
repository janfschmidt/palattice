/* ===libpalattice expamle program: lattice===
 * create a lattice and print some output,
 * import a lattice from madx or elegant
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


#include "libpalattice/AccLattice.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{

  //--- manually filled lattice ---
  // if no circumference is given, it is always set to the end of the last mounted element
  // ! for rings with drift at the end circumference should be set in constructor (or drift mounted explicitly)
  // ! via madx/elegant import the circumference is set automatically
  // default element reference position (refPos) = pal::begin
  pal::AccLattice beamline("test");       //all arguments: pal::AccLattice beamline("test", 5.0, pal::end);

  double R = 11;
  pal::Corrector ssh1("SSH1", 0.15);
  pal::Dipole mb("MB1", 0.8, pal::H, 1/R);
  pal::Quadrupole quad("QD1", 0.5, pal::F, 4.2);

  bool verbose = true;
  beamline.mount(0.4, ssh1, verbose);
  beamline.mount(1.0, mb, verbose);
  beamline.mount(2.0, quad, verbose);
  mb.name = "MB2";
  beamline.mount(2.6, mb, verbose);
  mb.name = "MB3";
  mb.plane = pal::V;
  beamline.mount(3.5, mb, verbose);

  cout << "* lattice \"beamline\" size: " << beamline.size() << endl;
  cout << beamline.sizeSummary() << endl << endl;

  double pos = 1.5;
  cout << "Access by position " << pos << "m: "
       << beamline[pos]->name <<", " << beamline[pos]->type_string() << endl;

  string name = "MB2";
  cout << "Access by name " << name
       << ": end at " << beamline.locate(name, pal::end) << "m" << endl << endl;

  pos = 0.5;
  cout << "magnetic field of " << beamline[pos]->name <<": " << beamline[pos]->B() << endl;
  pos = 3.51;
  cout << "magnetic field of " << beamline[pos]->name <<": " << beamline[pos]->B() << endl << endl;

  //print lattice to stdout
  beamline.print();



  //--- import lattice, here from Mad-X ---
  if (argc<2) {
    cout << "please give madx lattice file as argument." << endl;
    return 1;
  }
  string latticefile = argv[1];
  
  pal::SimToolInstance sim(pal::madx, pal::online, latticefile);
  pal::AccLattice elsa("ELSA", sim);

  //print lattice to file
  elsa.print("elsa.lattice");

  return 0;
}
