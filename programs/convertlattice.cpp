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

  cout << "usage:" << endl;
  cout << "[FILE] is:" << endl
       << "* a file that can be executed with madx or elegant" << endl
       << "* or a madx/elegant output file, if option -o is used (see below)" << endl << endl;
  cout << "options:" << endl
       << "* -m [FILE] input of a MadX file [FILE], Elegant output (default name: convert_[FILE].lte)" << endl
       << "* -e [FILE] input of an Elegant file [FILE], MadX output (default name: convert_[FILE].madx)" << endl
       << "* -l        additional output of a LaTeX format lattice file (default name: convert_[FILE].tex)" << endl
       << "* -o [name] specify output filenames ([name].lte/madx/tex). If [name]=stdout, output to terminal" << endl
       << "* -a        all 3 output formats"
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
  pal::simulationTool tool;
  pal::simToolMode mode = online;

  string file;

  //for getopt():
  int opt;
  extern char *optarg;
  extern int optopt, optind;

  while ((opt = getopt(argc, argv, "m:e:o:lna")) != -1) {
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
    case 'l':
      l=true;
      texout = optarg;
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
    default:
      usage();
      return 0;
    }

    if (e==false && m==false) {
      cerr << "No madx or elegant input specified." << endl;
      usage();
      return 1;
    }

    if (out == "-") out = "convert_"+file;
    else if (out == "stdout") out = "";

    pal::AccLattice lattice("convertlattice", tool, file, mode);


    if (m) {
      if (out != "") tmp = out+".madx";
      else tmp = out;
      lattice.madxexport(tmp);
    }
    if (e) {
      if (out != "") tmp = out+".lte";
      else tmp = out;
      lattice.elegantexport(tmp);
    }
    if (l) {
      if (out != "") tmp = out+".tex";
      else tmp = out;
      lattice.latexexport(tmp);
    }

  return 0;
}
