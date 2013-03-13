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

  ORBITCOMP() : pos(0), turn(1), x(0), z(0) {}
};




class ORBIT {

private:
  vector<ORBITCOMP> Orb;    
  unsigned int turns; //number of turns
  unsigned int bpms;  //number of samples per turn
  unsigned int orbitindex(unsigned int obs, unsigned int t) const;
  const double circumference;
  bool CO; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

public:
  ORBIT() : turns(1), bpms(0) circumference(164.4) CO(false) {}  //circumference einlesen; s_tot als Funktion für interp?
  ~ORBIT() {}
  double pos(unsigned int i) const {return Orb[i].pos;}
  double pos(unsigned int obs, unsigned int t) const {return Orb[this->orbitindex(obs,t)].pos;}
  double turn(unsigned int i) const {return Orb[i].turn;}
  double turn(unsigned int obs, unsigned int t) const {return Orb[this->orbitindex(obs,t)].turn;}
  double x(unsigned int i) const {return Orb[i].x;}
  double x(unsigned int obs, unsigned int t) const {return Orb[this->orbitindex(obs,t)].x;}
  double z(unsigned int i) const {return Orb[i].z;}
  double z(unsigned int obs, unsigned int t) const {return Orb[this->orbitindex(obs,t)].z;}
  unsigned int size() const {return Orb.size();}
  unsigned int getturns() const {return turns;}
  unsigned int getbpms() const {return bpms;}
  void push_back(ORBITCOMP tmp);
  void clear() {Orb.clear();}
  unsigned int diff(ORBIT Ref);
  int out(const char *filename) const;
 
  
 
};

#endif

/*__BSUPPLY__ORBIT_HPP_*/
