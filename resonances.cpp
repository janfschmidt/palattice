/* horizontal magnetic field in spin-phaseadvance system (theta) as estimation for resonance strengths*/
/* 04.02.2013 - J. Schmidt */

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "types.hpp"
#include "resonances.hpp"


RESONANCES::RESONANCES(double thetastep, unsigned int n_dip)
: dtheta(thetastep), n(360/(n_dip*dtheta))
{
  if(thetastep == 0.0) {
    on = false;
  }
  else {
    on = true;
  }
  theta.push_back(0.0);
  kick.push_back(0.0);
  //cout << "init theta " << theta.size() << endl;
  //cout << "init kick " << kick.size() << endl;
}



// only dipoles all of same length!!! (equidistant global sampling)
int RESONANCES::adddip(MAGNET dip, double Bx)
{

  //interval_samp as approximation for sampling
  unsigned int i;
  //step in dipole
  double dkick = 1000 * Bx * dip.length/n;

  //------Welche Fehler abfangen ??--------

  for (i=0; i<n; i++) {
    theta.push_back(theta.back() + dtheta);
    kick.push_back(dkick);
  }

  return 0;
}



int RESONANCES::addother(MAGNET magnet, double Bx)
{
  //cout <<"kicksize "<< kick.size() << endl;
  //------Welche Fehler abfangen ??--------
  kick[kick.size()-1] += 1000 * Bx * magnet.length;

  return 0;
}



int RESONANCES::closering()
{
  if (theta[theta.size()-1] != 360) {
    cout << "ERROR: resonances.cpp: Phaseadvance theta no closed solution (360°)" << endl;
    return 1;
  }
  else {
    kick[0] += kick[kick.size()-1];  //add kick(360°) to kick(0°)
    kick.erase(kick.end()-1);
    theta.erase(theta.end()-1);
  }

  return 0;
}



unsigned int RESONANCES::size() const
{
  if (theta.size() == kick.size())
    return theta.size();
  else
    return 0;
}



void RESONANCES::clear()
{
  theta.clear();
  kick.clear();
  theta.push_back(0);
  kick.push_back(0);
}



int RESONANCES::out(const char *filename) const
{
 unsigned int i=0;
 int w=14;
 fstream file;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: Cannot open " << filename << "." << endl;
   return 1;
 }

 file <<setw(w)<< "theta [°]" <<setw(w)<<  "kick [mrad]" << endl;
 file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
 for (i=0; i<theta.size(); i++) {
   file <<setw(w)<< theta[i] <<setw(w)<< kick[i] << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;
  return 0;
}
