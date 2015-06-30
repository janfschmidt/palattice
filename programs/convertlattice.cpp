/* -----convertlattice-----
 * convert particle accelerator lattice definition files using libpalattice
 * - MadX <--> Elegant
 * - MadX, Elegant --> LaTeX (lattice package)
 * 
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */


#include <iostream>
#include <getopt.h>
#include <libpalattice/AccLattice.hpp>

using namespace std;


void usage()
{
  cout << "convert particle accelerator lattice definition files using libpalattice:" << endl
       << "- MadX <--> Elegant" << endl
       << "- MadX, Elegant --> LaTeX (tikz-palattice package)" << endl << endl
       << "usage:" << endl;
  cout << "convertlattice -m [FILE] [other options]" << endl
       << "convertlattice -e [FILE] [other options]" << endl;
  cout << "[FILE] is:" << endl
       << "* a lattice file for madx (including BEAM!) or elegant" << endl
       << "* or a madx/elegant output file, if option -n is used (see below)" << endl;
  cout << "options:" << endl
       << "* -m [FILE] input of a MadX file [FILE] => Elegant output (default: [FILE]_converted.lte)" << endl
       << "* -e [FILE] input of an Elegant file [FILE] => MadX output (default: [FILE]_converted.madx)" << endl
       << "            If both -m and -e are given, the last one is used." << endl
       << "* -l        additional output of a LaTeX format lattice file (default: [FILE].tex)" << endl
       << "* -L        MadX output as LINE (including drifts) instead of SEQUENCE" << endl
       << "* -o [name] specify output filenames ([name].lte/madx/tex). If [name]=stdout, output to terminal" << endl
       << "* -a        all 3 output formats" << endl
       << "* -n        offline mode. no madx or elegant execution." << endl
       << "            Thus, [FILE] must be a madx/elegant output filename:" << endl
       << "            - for MadX: a twiss file" << endl
       << "            - for Elegant: an ascii parameter file" << endl
       << "* -h        display this help." << endl << endl;
  cout << "examples:" << endl
       << "MadX-->Elegant: convertlattice -m [MADXFILE]" <<endl
       << "Elegant-->MadX & LaTeX: convertlattice -e [ELEGANTFILE] -l" <<endl << endl;
  cout << "For a beamline (no ring) MadX twiss module fails, because of missing start values (betx, bety)." << endl
       << "Add them to the twiss command in libpalattice.madx to run MadX successfully." << endl;
}

void showInfo()
{
  cout << "*************************** convertlattice ***************************" << endl
       << "This is unpublished testing software. Please do not copy/distribute it" << endl
       << "without prior agreement of the author." << endl
       << "Open Source publication of the full library coming soon :-)" << endl
       << "(c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015" << endl
       << "**********************************************************************" << endl;
}


int main (int argc, char *argv[])
{
  string out = "-";
  bool m,e,l;
  m=e=l=false;
  pal::SimTool tool;
  pal::SimToolMode mode = pal::online;
  pal::MadxLatticeType ltype = pal::sequence;

  string file;

  //for getopt():
  int opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "m:e:o:lLnah")) != -1) {
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
    case 'L':
      ltype = pal::line;
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
  

  // print info
  showInfo();

  try {
    // import lattice
    pal::SimToolInstance sim(tool, mode, file);
    pal::AccLattice lattice("convertlattice", sim);
  

    // remove path from input file name
    unsigned int found = file.find_last_of("/");
    file = file.substr(found+1);
    // set output file base name
    if (out == "-" || out == "") out = "converted_"+file;


    // export lattice
    if (m) {
      if (out == "stdout") lattice.madxexport("",ltype);
      else lattice.madxexport(out+".madx",ltype);
    }
    if (e) {
      if (out == "stdout") lattice.elegantexport("");
      else lattice.elegantexport(out+".lte");
    }
    if (l) {
      if (out == "stdout") lattice.latexexport("");
      else lattice.latexexport(out+".tex");
    }
  }
  catch (pal::palatticeError &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
