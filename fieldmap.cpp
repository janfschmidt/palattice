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
  : n_samp(other.n_samp), turns(other.turns)
{
  unsigned int i;

  B = new FIELD[this->size()];

  for (i=0; i<this->size(); i++) {
    B[i].pos = other.pos(i);
    B[i].name = other.name(i);
    B[i].x = other.x(i);
    B[i].z = other.z(i);
    B[i].theta = other.theta(i);
  }
}



int FIELDMAP::set(unsigned int i, FIELD tmp)
{
  B[i].pos = tmp.pos;
  B[i].name = tmp.name;
  B[i].x = tmp.x;
  B[i].z = tmp.z;
  B[i].theta = tmp.theta;

  return 0;
}
