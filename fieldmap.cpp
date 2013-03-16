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
    B[i].pos = other.pos(i);
    B[i].turn = other.turn(i);
    B[i].name = other.name(i);
    B[i].x = other.x(i);
    B[i].z = other.z(i);
    B[i].theta = other.theta(i);
  }
}



int FIELDMAP::set(unsigned int samp, unsigned int t, FIELD tmp)
{
  unsigned int i = samp + (t-1)*n_samp;

  B[i].pos = tmp.pos;
  B[i].turn = tmp.turn;
  B[i].name = tmp.name;
  B[i].x = tmp.x;
  B[i].z = tmp.z;
  B[i].theta = tmp.theta;

  return 0;
}
