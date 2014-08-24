/* -----convertlattice-----
 * convert particle accelerator lattice definition files using libpal
 * - MadX <--> Elegant
 * - MadX, Elegant --> LaTeX (lattice package)
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <libpal/AccLattice.hpp>

using namespace std;


void usage()
{
  cout << "---convertlattice---" << endl
       << "convert particle accelerator lattice definition files using libpal" << endl
       << "- MadX <--> Elegant" << endl
       << "- MadX, Elegant --> LaTeX (lattice package)" << endl << endl;
  cout << "usage: convertlattice [OPTION] [FILE]" << endl;
  cout << "[FILE] is:" << endl
       << "* a file that can be executed with madx or elegant" << endl
       << "* or madx/elegant output files without extension, if option -o is used (see below)" << endl << endl;
  cout << "options:" << endl
       << "* -m [name] output of a MadX format lattice file [name] (default name: convert_[FILE].madx)" << endl
       << "* -e [name] output of an Elegant format lattice file [name] (default name: convert_[FILE].lte)" << endl
       << "* -l [name] output of a LaTeX format lattice file [name] (default name: convert_[FILE].tex)" << endl
       << "* -o        offline mode. no madx or elegant execution." << endl
       << "            Thus, [FILE] must be  madx/elegant output filename without extension. needed files:" << endl
       << "            - for MadX: [FILE].twiss (twiss file)" << endl
       << "            - for Elegant: [FILE].param (ascii parameter file)" << endl
       << "            See libpal documentation for details... " << endl;
}


int main (int argc, char *argv[])
{



  return 0;
}
