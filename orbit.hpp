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



//abstract base class
class ORBIT {

protected:
  vector<ORBITCOMP> Orb;    
  unsigned int turns;                        //number of turns
  unsigned int bpms;                         //number of samples per turn
  const double circumference;

public:
 //circumference einlesen; s_tot als Funktion für interp?
  ORBIT() : turns(1), bpms(0), circumference(164.4) {}
  ~ORBIT() {}
  unsigned int size() const {return Orb.size();}
  unsigned int getturns() const {return turns;}
  unsigned int getbpms() const {return bpms;}
  void push_back(ORBITCOMP tmp);
  void clear() {Orb.clear();}
  // virtual double pos() const =0;
  // virtual double turn() const =0;
  // virtual double x() const =0;
  // virtual double z() const =0;
  virtual int out(const char *filename) const =0;
};




class CLOSEDORBIT: public ORBIT {

public:
  CLOSEDORBIT() {}
  ~CLOSEDORBIT() {}
  double pos(unsigned int i) const {return Orb[i].pos;}
  double turn(unsigned int i) const {return Orb[i].turn;}
  double x(unsigned int i) const {return Orb[i].x;}
  double z(unsigned int i) const {return Orb[i].z;}
  int out(const char *filename) const;
  int diff(CLOSEDORBIT Ref);
};




class TRAJECTORY: public ORBIT {

protected:
  unsigned int Orbindex(unsigned int obs, unsigned int t) const;

public:
  TRAJECTORY() {}
  ~TRAJECTORY() {}
  double pos(unsigned int obs, unsigned int t) const {return Orb[this->Orbindex(obs,t)].pos;}
  double turn(unsigned int obs, unsigned int t) const {return Orb[this->Orbindex(obs,t)].turn;}
  double x(unsigned int obs, unsigned int t) const {return Orb[this->Orbindex(obs,t)].x;}
  double z(unsigned int obs, unsigned int t) const {return Orb[this->Orbindex(obs,t)].z;}
  int out(const char *filename) const;
};




#endif

/*__BSUPPLY__ORBIT_HPP_*/
