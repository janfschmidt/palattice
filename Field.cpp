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
#include "Field.hpp"

using namespace pal;


// set all magnetic field values from lattice and orbit
void Field::set(AccLattice &lattice, FunctionOfPos<AccPair>& orbit, unsigned int n_samples, bool edgefields)
{
  //metadata
  stringstream stmp;
  stmp << n_samples << " points per turn";
  this->info.add("Field sampling", stmp.str());
  this->info += lattice.info;
  this->info += orbit.info;

  unsigned int i, t;
  double _pos, _pos_tot;
  stringstream msg;
  AccPair otmp;
  AccTriple Btmp;
  bool noorbit = false;

 double interval_samp = this->circ / n_samples; // sampling interval of magn. field values along ring in meter

  if (this->circ != orbit.circ) {
    msg << "ERROR: Field::set(): Field and orbit have different circumferences ("
	 <<this->circ <<", "<<orbit.circ<<").";
    throw libpalError(msg.str());
  }

   for (t=1; t<=orbit.turns(); t++) {
     for (i=0; i<n_samples; i++) {
       _pos = i*interval_samp;
       _pos_tot = orbit.posTotal(_pos, t);
       try{
       otmp = orbit.interp(_pos_tot);
       }
       catch (std::runtime_error &e) { //no orbit available: use field without orbit
	 if (!noorbit) {
	   cout << e.what() << endl;
	   noorbit = true;
	 }
	 Btmp = lattice[_pos]->B_rf(t); //field without orbit not implemented with edgefields (AccLattice::B())
	 this->FunctionOfPos<AccTriple>::set(Btmp, _pos, t);
       }

       if (edgefields)
	 Btmp = lattice.B(_pos_tot,otmp);
       else
	 Btmp = lattice[_pos]->B_rf(t,otmp);

       this->FunctionOfPos<AccTriple>::set(Btmp, _pos, t);
     }
   }
}


//compare magnet lengths in FIELDMAP with exact lengths from lattice
//to analyse influence of sampling
int Field::magnetlengths(AccLattice &lattice, const char *filename) const
{
  unsigned int w=10;
  double tmp_start, tmp_end;
  fstream file;
  const_AccIterator dipIt=lattice.firstCIt(dipole);

  file.open(filename, ios::out);
  if (!file.is_open()) {
    throw libpalFileError(filename);
  }
  file << "deviation of start/end/length of stepwise field from lattice (field - lattice)" << endl;
  file << "n_samp = " << this->samplesInTurn(1) << " sampling points per turn" << endl;
  file << "lattice metadata:" << endl;
  file << lattice.info.out("") << endl;
  file <<setw(w)<< "Name" <<setw(w)<< "start/mm" <<setw(w) << "end/mm" <<setw(w)<< "length/mm" << endl;


  for (std::map<double,AccTriple>::const_iterator it=data.begin(); it!=data.end() && turn(it->first)<2; it++) {

    if ( lattice.inside(dipIt, it->first) ) {
      tmp_start = it->first;
      while ( it!=data.end()&& turn(it->first)<2 && lattice.inside(dipIt, it->first) ) it++;
      tmp_end = (--it)->first;

      //write deviations from exact values to file
      file <<setw(w)<< dipIt->second->name;
      file <<setw(w)<< (tmp_start - lattice.locate(dipIt,begin))*1000 <<setw(w)<< (tmp_end - lattice.locate(dipIt,end))*1000;
      file <<setw(w)<< ((tmp_end-tmp_start) - dipIt->second->length)*1000 << endl; 
      
      if (dipIt == lattice.lastCIt(dipole)) break;
      dipIt=lattice.nextCIt(dipIt, dipole);
    }
  }

  file.close();
  cout << "* Wrote " << filename  << endl;

  return 0;
}


