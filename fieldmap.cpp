/* magnetic field */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "types.hpp"
#include "fieldmap.hpp"

using namespace std;



//copy constructor
FIELDMAP::FIELDMAP(const FIELDMAP &other)
  : n_samp(other.n_samp), n_turns(other.n_turns), circumference(other.circumference)
{
  unsigned int i;

  B = new FIELD[this->size()];

  for (i=0; i<this->size(); i++) {
      B[i].turn = other.turn(i);
      B[i].x = other.x(i);
    B[i].z = other.z(i);
    }

  Grid = new ACCELERATOR_GRID[this->n_samp];

  for (i=0; i<this->n_samp; i++) {
    Grid[i].pos = other.pos(i);
    Grid[i].theta = other.theta(i);
    Grid[i].name = other.name(i);
  }
}


unsigned int FIELDMAP::Fieldindex(unsigned int samp, unsigned int t) const
{
//Catch calls for data out of range
  if (t==1 && samp >= size()) {
    cout << "ERROR: FIELDMAP::Fieldindex(): Field for index "<<samp<<" not known, only "
	 <<size()<<" points are imported from madx." << endl;
    return 0; // !! in this case the Field at begin of first turn is given
  }
  if ((t > n_turns) || (t>1 && t<=n_turns && samp> n_samp)) {
    cout << "ERROR: FIELDMAP::Fieldindex(): Field for turn "<<t<<" and sampling point "<<samp<<" not known, only "
	 <<n_turns<<" turns and "<<n_samp<<" samp are imported from madx." << endl;
    return 0; // !! in this case the Field at begin of first turn is given
  }
  if (t==0) {
    cout << "ERROR: FIELDMAP::Fieldindex(): Field turn must be > 0." << endl;
    return 0; // !! in this case the Field at begin of first turn is given
  }

  return samp + (t-1)*n_samp;  //default t=1 => samp is "usual" array index, if no t is given
}


unsigned int FIELDMAP::Gridindex(unsigned int samp) const
{
//Catch calls for data out of range
if (samp >= size()) {
    cout << "ERROR: FIELDMAP::Gridindex(): Field for index "<<samp<<" not known, only "
	 <<size()<<" points are imported from madx." << endl;
    return 0; // !! in this case the Field at begin of first turn is given
  }

  return samp%n_samp;
}



int FIELDMAP::set(unsigned int samp, unsigned int t, double pos, double x, double z, double theta, string name)
{

  if (t==1) {                   // pos,name,theta identical for all turns: set for turn 1 & checked else
    Grid[samp].name = name;
    Grid[samp].pos = pos;
    Grid[samp].theta = theta;
  }
  else {
    if (Grid[samp].name != name) {
      cout << "ERROR: FIELDMAP::set():" << endl;
      cout << "inconsistent lattice: element name changes with turn ("
	   <<Grid[samp].name<< " -> " <<name<< ")" << endl;
      return 1;
    } 
    if (Grid[samp].pos != pos) {
      cout << "ERROR: FIELDMAP::set():" << endl;
      cout << "inconsistent lattice: element position changes with turn ("
	   <<Grid[samp].pos<< " -> " <<pos<< ")" << endl;
      return 1;
    } 
    if (Grid[samp].theta != theta) {
      cout << "ERROR: FIELDMAP::set():" << endl;
      cout << "inconsistent lattice: element phaseadvance (theta) changes with turn ("
	   <<Grid[samp].theta<< " -> " <<theta<< ")" << endl;
      return 1;
    } 
  }

  unsigned int i = Fieldindex(samp,t);
  B[i].turn = t;
  B[i].x = x;
  B[i].z = z;

  return 0;
}



/* create output file with field data */
int FIELDMAP::out(const char *filename) const
{
 unsigned int i=0;
 int w=14;
 fstream file;
 double c = 299792458;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: FIELDMAP::out(): Cannot open " << filename << "." << endl;
   return 1;
 }

 file <<setw(w)<< "turn" <<setw(w)<< "s [m]" <<setw(w)<< "s_tot [m]" <<setw(w)<< "t [s]" <<setw(w)<< "phase [deg]" <<setw(w)<< "name" <<setw(w)<< "Bx [1/m]" <<setw(w)<< "Bz [1/m]" <<setw(w)<< "Bs [1/m]" << endl;
 for (i=0; i<size(); i++) {
   file <<setiosflags(ios::fixed)<<noshowpoint<<setprecision(0);
   file <<setw(w)<< turn(i);
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
   file <<setw(w)<< pos(i) <<setw(w)<< pos_tot(i) <<setw(w)<< pos_tot(i)/c <<setw(w)<< theta(i) <<setw(w)<< name(i);
     file <<setw(w)<< x(i) <<setw(w)<< z(i) <<setw(w)<< 0.0 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
