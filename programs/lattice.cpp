/* -----lattice-----
 * libpalattice example program
 * create a lattice and print some output,
 * import a lattice from madx or elegant
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include "libpalattice/AccLattice.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{

  // manuell angelegtes Lattice
  pal::AccLattice beamline("test", 5.0); // default refPos = begin

  double R = 11;
  pal::Corrector ssh1("SSH1", 0.15);
  pal::Dipole mb("MB1", 0.8, pal::H, 1/R);
  pal::Quadrupole quad("QD1", 0.5, pal::F, 4.2);

  beamline.mount(0.4, ssh1);
  beamline.mount(1.0, mb);
  beamline.mount(2.0, quad);
  mb.name = "MB2";
  beamline.mount(2.6, mb);
  mb.name = "MB3";
  mb.plane = pal::V;
  beamline.mount(3.5, mb);

  double pos = 1.5;
  cout << "Zugriff über Position " << pos << "m: "
       << beamline[pos]->name <<", " << beamline[pos]->type_string() << endl;

  string name = "MB2";
  cout << "Zugriff über Name " << name
       << ": Ende bei " << beamline.locate(name, pal::end) << "m" << endl << endl;

  cout << "Magnetfeld " << beamline[2.6]->name <<": " << beamline[2.6]->B() << endl;
  cout << "Magnetfeld " << beamline[3.5]->name <<": " << beamline[3.5]->B() << endl << endl;

  beamline.print();



  //importiertes Lattice
  string latticefile = argv[1];
  pal::SimToolInstance sim(pal::madx, pal::online, latticefile);
  //pal::SimToolInstance sim(pal::elegant, pal::online, "../lattice/ELSA/elsa.lte");
  pal::AccLattice elsa("ELSA", sim);

  elsa.print("elsa.lattice");

  return 0;
}
