/* === Magnetic Field Class ===
 * magnetic field distribution (3D) along an accelerator ring
 * implemented as FunctionOfPos<AccTriple> with additional function(s)
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "field.hpp"



// set all magnetic field values from lattice and orbit
void Field::set(AccLattice &lattice, FunctionOfPos<AccPair>& orbit, double n_samples)
{
  unsigned int i, t;
  double _pos, _pos_tot;
  stringstream msg;
  AccPair otmp;
  AccTriple Btmp;

 double interval_samp = this->circ / n_samples; // sampling interval of magn. field values along ring in meter

  if (this->circ != orbit.circ) {
    msg << "ERROR: Field::set(): Field and orbit have different circumferences ("
	 <<this->circ <<", "<<orbit.circ<<").";
    throw runtime_error(msg.str());
  }

   for (t=1; t<=orbit.turns(); t++) {
     for (i=0; i<n_samples; i++) {
       _pos = i*interval_samp;
       _pos_tot = orbit.posTotal(_pos, t);
       otmp = orbit.interp(_pos_tot);

       Btmp = lattice[_pos]->B(otmp,t);
       this->modify(Btmp, i, t);
     }
   }
}


//compare magnet lengths in FIELDMAP with exact lengths from lattice
//to analyse influence of sampling
int Field::magnetlengths(AccLattice &lattice, const char *filename) const
{
  unsigned int w=10;
  unsigned int i;
  double tmp_start, tmp_end;
  fstream file;
  const_AccIterator it=lattice.firstCIt(dipole);

  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: Field::magnetlengths(): Cannot open " << filename << "." << endl;
    return 1;
  }
  file << "deviation of start/end/length of stepwise field from lattice (field - lattice)" << endl;
  file << "n_samp = " << this->samples() << " sampling points per turn" << endl << endl;
  file <<setw(w)<< "Name" <<setw(w)<< "start/mm" <<setw(w) << "end/mm" <<setw(w)<< "length/mm" << endl;


  for (i=0; i<this->samples(); i++) {

    if ( lattice.inside(it, getPos(i)) ) {
      tmp_start = this->getPos(i);
      while ( lattice.inside(it, getPos(i)) ) i++;
      tmp_end = getPos(i-1);

      //write deviations from exact values to file
      file <<setw(w)<< it->second->name;
      file <<setw(w)<< (tmp_start - lattice.locate(it,begin))*1000 <<setw(w)<< (tmp_end - lattice.locate(it,end))*1000;
      file <<setw(w)<< ((tmp_end-tmp_start) - it->second->length)*1000 << endl; 
      
      if (it == lattice.lastCIt(dipole)) break;
      it=lattice.nextCIt(dipole,it);
    }
  }

  file.close();
  cout << "* Wrote " << filename  << endl;

  return 0;
}


