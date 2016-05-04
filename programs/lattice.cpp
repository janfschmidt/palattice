/* libpalattice expamle program: Lattice access, editing & import
 * create a lattice and print some output, import a lattice from madx or elegant
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libpalattice/AccLattice.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  /*
   *--- manually filled lattice ---
   * if no circumference is given, it is always set to the end of the last mounted element
   * ! for rings with drift at the end circumference should be set in constructor (or drift mounted explicitly)
   * ! via madx/elegant import the circumference is set automatically
   * default element reference position (refPos) = pal::begin
   */
  pal::AccLattice beamline;

  double R = 11;
  pal::Dipole mb("MB1", 0.8, pal::H, 1/R);
  pal::Quadrupole quad("QD1", 0.5, pal::F, 4.2);

  // mount AccElements 
  bool verbose = true;
  beamline.mount(0.4, pal::Corrector("SSH1",0.2), verbose);
  beamline.mount(1.1, mb, verbose);
  beamline.mount(2.2, quad, verbose);
  mb.name = "MB2";
  beamline.mount(2.8, mb, verbose);
  mb.name = "MB3";
  mb.plane = pal::V;
  beamline.mount(4.1, mb, verbose);

  // number of elements in lattice
  cout << "* lattice \"beamline\" size: " << beamline.size() << endl;
  cout << beamline.sizeSummary() << endl << endl;

  // lattice range-loop
  std::cout << "elements mounted in beamline:" << std::endl;
  for (auto ele : beamline) {
    // ele is const pal::AccElement*
    std::cout << ele->name << std::endl;
  }

  // lattice iterators
  // access position via it.pos()
  // access AccElement* via it.element() or *it
  std::cout <<std::endl<< "types of elements mounted in beamline:" << std::endl;
  for (auto it=beamline.begin(); it!=beamline.end(); ++it) {
    // it is pal::AccLattice::iterator
    std::cout << it.pos() << "m: " << it.element()->type_string() << std::endl;
  }

  // lattice type iterators
  std::cout <<std::endl<< "dipoles mounted in beamline:" << std::endl;
  for (auto it=beamline.begin<pal::dipole>(); it!=beamline.end(); ++it) {
   // it is pal::AccLattice::type_iterator<pal::Dipole>
    std::cout << it.pos() << "m: " << it.element()->name << ", " << (*it)->type_string() << std::endl;
  }
  
   // lattice type iteration with usual lattice iterator (type can be chosen at runtime)
  std::cout <<std::endl<< "quadrupoles mounted in beamline:" << std::endl;
  for (auto it=beamline.begin(pal::quadrupole); it!=beamline.end(); it.next(pal::quadrupole)) {
    // it is pal::AccLattice::iterator
    std::cout << it.pos() << "m: " << it.element()->name << ", " << (*it)->type_string() << std::endl;
  }
  

  // access by position
  std::cout <<std::endl<< "access by position:" << std::endl;
  double pos = 0.4;
  const pal::AccElement* ele = beamline[pos];
  cout << pos << "m: " << ele->name <<", " << ele->type_string() << endl;
  for (double s : {2.1, 4.6}) {
    cout << s << "m: " << beamline[s]->name <<": Bz=" << beamline[s]->B().z << " 1/m" << endl;
  }

  // access by name
  string name = "MB2";
  pal::AccLattice::const_iterator it = beamline[name];
  cout <<std::endl<< "access by name:" << std::endl
       << name << " begins at " << it.begin() << "m and ends at " << it.end() << "m" << endl << endl;

  //print lattice to stdout
  cout <<std::endl<< "print lattice:" << std::endl;
  beamline.print();

  cout <<std::endl<< "export lattice as LaTeX:" << std::endl;
  beamline.latexexport("beamline.tex");

  cout <<std::endl<< "export lattice as Elegant:" << std::endl;
  beamline.elegantexport("beamline.lte");

  cout <<std::endl<< "export lattice as Mad-X:" << std::endl;
  beamline.madxexport("beamline.madx");



  // import lattice, here from Mad-X
  cout <<std::endl<< "import lattice from Mad-X:" << std::endl;
  if (argc<2) {
    cout << "please give madx lattice file as argument." << endl;
    return 1;
  }
  string latticefile = argv[1];
  
  pal::SimToolInstance sim(pal::madx, pal::online, latticefile);
  pal::AccLattice elsa(sim);
  elsa.setComment("I am a lattice from a file!");

  //print lattice to file
  elsa.print("elsa.lattice");

  return 0;
}
