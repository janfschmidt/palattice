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
  unsigned int n_turns;                       //number of turns
  unsigned int n_bpms;                        //number of samples per turn
  gsl_interp_accel *acc_x;                    //gsl interpolation
  gsl_interp_accel *acc_z;
  gsl_spline *spline_x;
  gsl_spline *spline_z;
  bool interp_flag;
  virtual unsigned int Orbindex(unsigned int obs, unsigned int t) const =0;

public:
  const double circumference;   //circumference einlesen!!!!!!!!!!!!!!!!!!!!!!!!
  ORBIT() : n_turns(1), n_bpms(0), interp_flag(false), circumference(164.4) {}
  ~ORBIT() {}
  unsigned int size() const {return Orb.size();}
  unsigned int turns() const {return n_turns;}
  unsigned int bpms() const {return n_bpms;}

  // access to ORBITCOMPs by observation point (obs: 1 to bpms()) and turn (t: 1 to turns())
  double pos(unsigned int obs, unsigned int t=1) const {return Orb[Orbindex(obs,t)].pos;}
  double turn(unsigned int obs, unsigned int t=1) const {return Orb[Orbindex(obs,t)].turn;}
  double x(unsigned int obs, unsigned int t=1) const {return Orb[Orbindex(obs,t)].x;}
  double z(unsigned int obs, unsigned int t=1) const {return Orb[Orbindex(obs,t)].z;}

  virtual double pos_max() const =0;           //maximum pos (s in m) accessible by interpolation
  void push_back(ORBITCOMP tmp);
  void clear() {Orb.clear(); n_turns=1; n_bpms=0;}
  double interp_x(double any_pos);             // get orbit at any position (s in m) between 0 and pos_max()
  double interp_z(double any_pos);
  void interp_update();                        // deletes previous interpolation to consider data changes
  int interp_out(double stepwidth, const char *filename);
  virtual int out(const char *filename) const =0;
  virtual void interp_init() =0;
};




class CLOSEDORBIT: public ORBIT {

protected:
  unsigned int Orbindex(unsigned int obs, unsigned int t) const;
  void interp_init();

public:
  CLOSEDORBIT() {}
  ~CLOSEDORBIT();
  double pos_max() const {return pos(1) + circumference;}
  int out(const char *filename) const;
  int diff(CLOSEDORBIT Ref);
};




class TRAJECTORY: public ORBIT {

protected:
  unsigned int Orbindex(unsigned int obs, unsigned int t) const;
  void interp_init();

public:
  TRAJECTORY() {}
  ~TRAJECTORY() {}
  double pos_tot(unsigned int obs, unsigned int t) const {return (turn(obs,t)-1)*circumference+pos(obs,t);}
  double pos_max() const {if(n_bpms>0) return pos_tot(1, n_turns+1); else return 0.;}
  int out(const char *filename) const;
  int add_closedorbit(CLOSEDORBIT &co);
};




#endif

/*__BSUPPLY__ORBIT_HPP_*/
