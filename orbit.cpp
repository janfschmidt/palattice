/* particle orbit (x&z) along accelerator */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "types.hpp"
#include "orbit.hpp"

using namespace std;


// (observation point & turn) -> (array index)
unsigned int TRAJECTORY::Orbindex(unsigned int obs, unsigned int t) const
{
  if (obs==1)
    return (t-1);  //obs0001 with one more turn (see trajectoryimport() for details)
  else
    return (obs-2)*turns + (turns+1) + (t-1);
}



// add ORBITCOMP to ORBIT
void ORBIT::push_back(ORBITCOMP tmp)
{
  //ensure always turns = max turn (exeption pos=0.0, see comment in trajectoryimport() for details)
  if (tmp.turn > turns && tmp.pos>0.0) turns = tmp.turn;
  //count bpms during first turn (equal for every turn in madx)
  if (tmp.turn == 1) bpms++;
  Orb.push_back(tmp);
}



//subtract Ref orbit from this orbit.
//if pos or turn are unequal the number of the element with error is returned
int CLOSEDORBIT::diff(CLOSEDORBIT Ref)
{
  unsigned int i;
  for (i=0; i<this->size(); i++) {
    if (this->pos(i) == Ref.pos(i) && this->turn(i) == Ref.turn(i)) {
      Orb[i].x -= Ref.x(i);
      Orb[i].z -= Ref.z(i);
    }
    else {
      return i+1;
    }
  }
  
  return 0;
}




//output file
int CLOSEDORBIT::out(const char *filename) const
{
  unsigned int i;
  int w=10;
  fstream file;
  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: CLOSEDORBIT::out(): Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "s [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
  for (i=0; i<this->size(); i++) {
      file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
      file <<setw(w)<< pos(i) <<setw(w)<<  x(i)*1000 <<setw(w)<< z(i)*1000 << endl;
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}

int TRAJECTORY::out(const char *filename) const
{
  unsigned int t,obs;
  int w=10;
  fstream file;
  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: TRAJECTORY::out(): Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "turn" <<setw(w)<< "s [m]" <<setw(w)<< "s_tot [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
  for (t=1; t<=turns; t++) {
    for(obs=1; obs<=bpms; obs++) {
      file <<setiosflags(ios::fixed)<<noshowpoint<<setprecision(0);
      file <<setw(w)<< turn(obs,t);
      file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
      file <<setw(w)<< pos(obs,t) <<setw(w)<< (turn(obs,t)-1)*circumference+pos(obs,t) <<setw(w)<<  x(obs,t)*1000 <<setw(w)<< z(obs,t)*1000 << endl;
    }
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}
