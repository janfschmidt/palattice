#ifndef __BSUPPLY__ORBIT_HPP_
#define __BSUPPLY__ORBIT_HPP_

#include <vector>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
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
  unsigned int n_turns;                        //number of turns
  unsigned int n_bpms;                         //number of samples per turn
  double pos_maxvalue;                        //maximum pos (s in m) accessible by interpolation
  const double circumference;

public:
 //circumference einlesen; s_tot als Funktion für interp?
  ORBIT() : n_turns(1), n_bpms(0), pos_maxvalue(0), circumference(164.4) {}
  ~ORBIT() {}
  unsigned int size() const {return Orb.size();}
  unsigned int turns() const {return n_turns;}
  unsigned int bpms() const {return n_bpms;}
  double pos_max() const {return pos_maxvalue;}
  void push_back(ORBITCOMP tmp);
  void clear() {Orb.clear();}
  virtual int out(const char *filename) const =0;
};




class CLOSEDORBIT: public ORBIT {

protected:
  gsl_interp_accel *acc_x;  //gsl interpolation
  gsl_interp_accel *acc_z;
  gsl_spline *spline_x;
  gsl_spline *spline_z;
  bool interp_flag;
  void interp_init();

public:
  CLOSEDORBIT() : interp_flag(false) {}
  ~CLOSEDORBIT();
  double pos(unsigned int i) const {return Orb[i].pos;}
  double turn(unsigned int i) const {return Orb[i].turn;}
  double x(unsigned int i) const {return Orb[i].x;}
  double z(unsigned int i) const {return Orb[i].z;}
  int out(const char *filename) const;
  int diff(CLOSEDORBIT Ref);
  double interp_x(double any_pos);        // get orbit at any position (s in m) between 0 and pos_max()
  double interp_z(double any_pos);
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
