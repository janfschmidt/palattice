/* particle orbit (x&z) along accelerator */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "types.hpp"
#include "orbit.hpp"

using namespace std;


//subtract Ref orbit from this orbit.
//if pos or turn are unequal the number of the element with error is returned
unsigned int ORBIT::diff(ORBIT Ref)
{
  unsigned int i;
  for (i=0; i<this->size(); i++) {
    if (this->pos(i) == Ref.pos(i) && this->turn(i) == Ref.turn(i)) {
      Orb[i].x -= Ref.x(i);
      Orb[i].z -= Ref.z(i);
    }
    else return i+1;
  }
  
  return 0;
}
