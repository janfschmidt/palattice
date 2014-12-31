/* -----convertlattice-----
 * convert particle accelerator lattice definition files using libpal
 * - MadX <--> Elegant
 * - MadX, Elegant --> LaTeX (lattice package)
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <getopt.h>
#include <libpal/AccLattice.hpp>

using namespace std;


void usage()
{
  cout << "convert particle accelerator lattice definition files using libpal:" << endl
       << "- MadX <--> Elegant" << endl
       << "- MadX, Elegant --> LaTeX (lattice package)" << endl << "---usage:---" << endl;
  cout << "convertlattice -m [FILE] [other options]" << endl
       << "convertlattice -e [FILE] [other options]" << endl;
  cout << "[FILE] is:" << endl
       << "* a lattice file for madx (including BEAM!) or elegant" << endl
       << "* or a madx/elegant output file, if option -o is used (see below)" << endl;
  cout << "options:" << endl
       << "* -m [FILE] input of a MadX file [FILE] => Elegant output (default: convert_[FILE].lte)" << endl
       << "* -e [FILE] input of an Elegant file [FILE] => MadX output (default: convert_[FILE].madx)" << endl
       << "     if both -m and -e are given, the last one is used." << endl
       << "* -l        additional output of a LaTeX format lattice file (default: convert_[FILE].tex)" << endl
       << "* -o [name] specify output filenames ([name].lte/madx/tex). If [name]=stdout, output to terminal" << endl
       << "* -a        all 3 output formats" << endl
       << "* -n        offline mode. no madx or elegant execution." << endl
       << "            Thus, [FILE] must be a madx/elegant output filename:" << endl
       << "            - for MadX: a twiss file" << endl
       << "            - for Elegant: an ascii parameter file" << endl
       << "            See libpal documentation for details... " << endl;
}


int main (int argc, char *argv[])
{
  string out = "-";
  bool m,e,l;
  m=e=l=false;
  pal::SimTool tool;
  pal::SimToolMode mode = pal::online;

  string file;

  //for getopt():
  int opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "m:e:o:lnah")) != -1) {
    switch(opt) {
    case 'm':
      tool = pal::madx;
      e=true;
      file = optarg;
      break;
    case 'e':
      tool = pal::elegant;
      m=true;
      file = optarg;
      break;
    case 'o':
      out = optarg;
      break;
    case 'n':
      mode = pal::offline;
      break;
    case 'l':
      l=true;
      break;
    case 'a':
      m=e=l=true;
      break;
    case 'h':
      usage();
      return 0;
    default:
      usage();
      return 0;
    }
  }

  if (e==false && m==false) {
    cerr << "No madx or elegant input specified." << endl << endl;
    usage();
    return 1;
  }
  
  if (out == "-" || out == "") out = "convert_"+file;
  

  // import lattice
  pal::SimToolInstance sim(tool, mode, file);
  pal::AccLattice lattice("convertlattice", sim);
  

  // export lattice
  if (m) {
    if (out == "stdout") lattice.madxexport("");
    else lattice.madxexport(out+".madx");
  }
  if (e) {
    if (out == "stdout") lattice.elegantexport("");
    else lattice.elegantexport(out+".lte");
  }
  if (l) {
    if (out == "stdout") lattice.latexexport("");
    else lattice.latexexport(out+".tex");
  }
  
  return 0;
}
