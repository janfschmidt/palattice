#ifndef __BSUPPLY__RESONANCES_HPP_
#define __BSUPPLY__RESONANCES_HPP_

#include <vector>
#include "types.hpp"

using namespace std;

class RESONANCES {

private:
  vector<double> theta;    // in degree [0,n_turns*360]
  vector<double> kick;     // in mrad
  const double dtheta;     // stepwidth phaseadvance
  const unsigned int n;    // sample points per dipole
  const unsigned int ndip; // number of dipoles (per turn)

public:
  bool on;
  const unsigned int n_turns;

  RESONANCES() :  dtheta(-1), n(0), ndip(0), on(false), n_turns(1) {}
  RESONANCES(double thetastep, unsigned int n_dip, unsigned int turns);
  ~RESONANCES() {}
  double gettheta(unsigned int i) const {return theta[i];}
  double lasttheta() const {return theta.back();}
  double getkick(unsigned int i) const {return kick[i];}
  vector<double> getkickVector() const {return kick;}
  unsigned int ndipols() const {return ndip;}
  int out(const char *filename) const;
  int adddip(MAGNET dip, double Bx);
  int addother(MAGNET magnet, double Bx);
  int closering();
  unsigned int size() const;
  double theta_max() const {return 360*n_turns;}
  void clear();
 
};

#endif

/*__BSUPPLY__RESONANCES_HPP_*/
