#ifndef __BSUPPLY__FIELDMAP_HPP_
#define __BSUPPLY__FIELDMAP_HPP_

#include "types.hpp"


using namespace std;

class FIELD {
public:
  unsigned int turn;
  double x;     //[B.x]=1/m (missing factor gamma*m*c/e)
  double z;     //[B.z]=1/m (missing factor gamma*m*c/e)

  FIELD() : turn(1), x(0), z(0) {}
};


class ACCELERATOR_GRID {
public:
  double pos;   //position along ring in meter
  string name;
  double theta; //spin phaseadvance in degree

  ACCELERATOR_GRID() : pos(0), name("ndef"), theta(0) {}
};



class FIELDMAP {

private:
  ACCELERATOR_GRID *Grid;
  FIELD *B;
  unsigned int Fieldindex(unsigned int samp, unsigned int t=1) const;
  unsigned int Gridindex(unsigned int samp) const;

public:
  const unsigned int n_samp; //number of sampling points along ring
  const unsigned int n_turns;  //number of turns in ring
  const double circumference;   //circumference einlesen!!!!!!!!!!!!!!!!!!!!!!!!
  
  FIELDMAP(unsigned int n=1644, unsigned int t=1) : n_samp(n), n_turns(t), circumference(164.4) {B = new FIELD[size()]; Grid = new ACCELERATOR_GRID[n_samp];}
  ~FIELDMAP() {delete[] B; delete[] Grid;}
  FIELDMAP(const FIELDMAP &other);
  double pos(unsigned int samp, unsigned int t=1) const {return Grid[Gridindex(samp)].pos;}
  unsigned int turn(unsigned int samp, unsigned int t=1) const {return B[Fieldindex(samp,t)].turn;}
  string name(unsigned int samp, unsigned int t=1) const {return Grid[Gridindex(samp)].name;}
  double x(unsigned int samp, unsigned int t=1) const {return B[Fieldindex(samp,t)].x;}
  double z(unsigned int samp, unsigned int t=1) const {return B[Fieldindex(samp,t)].z;}
  double theta(unsigned int samp, unsigned int t=1) const {return Grid[Gridindex(samp)].theta;}
  unsigned int size() const {return n_samp*n_turns;}
  double pos_tot(unsigned int i) const {return pos(i) + (turn(i)-1)*circumference;}
  int set(unsigned int samp, unsigned int t, double pos, double x, double z, double theta, string name);
  int magnetlengths(magnetvec &dipols, const char *filename) const;
  int out(const char *filename) const;

};

#endif

/*__BSUPPLY__FIELDMAP_HPP_*/
