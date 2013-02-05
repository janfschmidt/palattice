#ifndef __BSUPPLY__RESONANCES_HPP_
#define __BSUPPLY__RESONANCES_HPP_

#include <vector>
#include "types.hpp"

using namespace std;

class RESONANCES {

private:
  vector<double> theta; // in degree [0,360]
  vector<double> kick;  // in mrad
  const double dtheta;  // stepwidth phaseadvance
  const unsigned int n; // sample points per dipole


public:
  bool on;

  RESONANCES() :  dtheta(0), n(0), on(false) {}
  RESONANCES(double thetastep, unsigned int n_dip); //: dtheta(thetastep), n(360/(n_dip*dtheta)) {if(dtheta==0) on=false; else on=true;}
  ~RESONANCES() {}
  double gettheta(unsigned int i) const {return theta[i];}
  double lasttheta() const {return theta[theta.size()-1];}
  double getkick(unsigned int i) const {return kick[i];}
  int out(const char *filename) const;
  int adddip(MAGNET dip, double Bx);
  int addother(MAGNET magnet, double Bx);
  int closering();
  unsigned int size() const;
  void clear();
 
};

#endif

/*__BSUPPLY__RESONANCES_HPP_*/
