#ifndef __BSUPPLY__ORBIT_HPP_
#define __BSUPPLY__ORBIT_HPP_

#include <vector>
#include "types.hpp"

using namespace std;


class ORBITCOMP {
public:
  double pos;
  double turn;
  double x;
  double z;

  ORBITCOMP() : pos(0), turn(0), x(0), z(0) {}
};




class ORBIT {

private:
  vector<ORBITCOMP> Orb;    

public:
  

  ORBIT() {}
  ~ORBIT() {}
  double pos(unsigned int i) const {return Orb[i].pos;}
  double turn(unsigned int i) const {return Orb[i].turn;}
  double x(unsigned int i) const {return Orb[i].x;}
  double z(unsigned int i) const {return Orb[i].z;}
  unsigned int size() const {return Orb.size();}
  void push_back(ORBITCOMP tmp) {Orb.push_back(tmp);}
  void clear() {Orb.clear();}
  unsigned int diff(ORBIT Ref);
  //int out(const char *filename) const;
 
  
 
};

#endif

/*__BSUPPLY__ORBIT_HPP_*/
