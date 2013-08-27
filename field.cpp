/* === Magnetic Field Class ===
 * magnetic field distribution (3D) along an accelerator ring
 * implemented as FunctionOfPos<AccTriple> with additional function(s)
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 */


#include <iostream>
#include <fstream>
#include <iomanip>
#include "field.hpp"



//compare magnet lengths in FIELDMAP with exact lengths from lattice
//to analyse influence of sampling
int Field::magnetlengths(magnetvec &dipols, const char *filename) const
{
  unsigned int w=10;
  unsigned int i, j=0;
  double tmp_start, tmp_end;
  fstream file;

  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: FIELDMAP::magnetlengths(): Cannot open " << filename << "." << endl;
    return 1;
  }
  file << "deviation of magnet start/end/length from lattice (fieldmap - lattice)" << endl;
  file << "n_samp = " << this->samples() << " sampling points per turn" << endl << endl;
  file <<setw(w)<< "Name" <<setw(w)<< "start/mm" <<setw(w) << "end/mm" <<setw(w)<< "length/mm" << endl;


  for (i=0; i<this->samples(); i++) {

    if (getPos(i) >= dipols[j].start) {
      tmp_start = this->getPos(i);
      while (this->getPos(i) <= dipols[j].end) i++;
      tmp_end = getPos(i-1);

      //write deviations from exact values to file
      file <<setw(w)<< dipols[j].name;
      file <<setw(w)<< (tmp_start - dipols[j].start)*1000 <<setw(w)<< (tmp_end - dipols[j].end)*1000;
      file <<setw(w)<< ((tmp_end-tmp_start) - dipols[j].length)*1000 << endl; 

      j++;
    }
    if (j >= dipols.size()) break;
  }

  file.close();
  cout << "* Wrote " << filename  << endl;

  return 0;
}


