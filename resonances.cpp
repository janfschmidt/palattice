/* horizontal magnetic field in spin-phaseadvance system (theta) as estimation for resonance strengths*/
/* 04.02.2013 - J. Schmidt */

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include "resonances.hpp"


RESONANCES::RESONANCES(double thetastep, unsigned int n_dip, unsigned int turns)
  : dtheta(thetastep), n(360/(n_dip*dtheta)), ndip(n_dip), n_turns(turns)
{
  if(thetastep == -1.0) {
    on = false;
  }
  else {
    on = true;
  }
  theta.push_back(0.0);
  kick.push_back(0.0);
  vCorrKick.push_back(0.);
  quadKick.push_back(0.);
}



// only dipoles all of same length!!! (equidistant global sampling)
void RESONANCES::addDip(const pal::AccElement* dip)
{
  //interval_samp as approximation for sampling
  unsigned int i;
  //step in dipole
  double dkick = dip->hKick_mrad() / n; //(assume length as arclength along orbit, see AccLattice::madximport)

  //------Fehler abfangen ??--------

  for (i=0; i<n; i++) {
    theta.push_back(theta.back() + dtheta);
    kick.push_back(dkick);
    vCorrKick.push_back(0.);
    quadKick.push_back(0.);
  }

}



void RESONANCES::addOther(const pal::AccElement* magnet, AccPair orbit)
{
  //------Fehler abfangen ??--------
  double tmpKick = magnet->hKick_mrad(orbit);
  kick.back() +=tmpKick;
  if (magnet->type == corrector)  vCorrKick.back() += tmpKick;
  else if (magnet->type == quadrupole)  quadKick.back() += tmpKick;

}



int RESONANCES::closering()
{
  if (theta.back() != theta_max()) {
    cout << "===============================================================================" << endl;
    cout << "ERROR: resonances.cpp: Phaseadvance theta no closed solution ("<<theta.back()<<"°)" << endl;
    cout << "                       resonance-strength calculation switched off!" << endl;
    cout << "===============================================================================" << endl;
    on = false;
    return 1;
  }
  else {
    kick[0] += kick.back();  //add kick(360°) to kick(0°) (or equivalent for n_turns>1)
    vCorrKick[0] += vCorrKick.back();
    quadKick[0] += quadKick.back();
    kick.erase(kick.end()-1);
    vCorrKick.erase(vCorrKick.end()-1);
    quadKick.erase(quadKick.end()-1);
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
  vCorrKick.clear();
  quadKick.clear();
  theta.push_back(0.);
  kick.push_back(0.);
  vCorrKick.push_back(0.);
  quadKick.push_back(0.);
}







void RESONANCES::set(pal::AccLattice &lattice, pal::FunctionOfPos<pal::AccPair> &orbit)
{
  unsigned int t;
  double pos_tot;
  pal::const_AccIterator it;

  if (!on) return; // calculate only if Resonance calculation is switched on

  clear(); //delete data from previous time t

 // copy metadata from lattice and orbit to RESONANCES
  this->info += lattice.info;
  this->info += orbit.info;
 
  for (t=1; t<=orbit.turns(); t++) {
    for(it=lattice.getItBegin(); it !=lattice.getItEnd(); ++it) {

      if (it->second->type == dipole) { // Dipole field independent of orbit (turn)
	addDip(it->second);
      }
      else {
	pos_tot = orbit.posTotal(it->first, t);
	addOther(it->second, orbit.interp(pos_tot));
      }

    }
  }

  closering();
}




pal::Spectrum RESONANCES::getSpectrum(unsigned int fmaxrevIn, double ampcutIn) const
{
  // construct spectrum from kick-vector
  // (unit=degree, circumference=360)
  // Resonances spectrum is normalized to number of bending dipoles ndip
  Spectrum s("resonances", this->getkickVector(), 360, n_turns, ndip*n_turns, fmaxrevIn, ampcutIn, degree);

  return s;
}



//output of kicks as function of spin phaseadvance theta (total, vcorr, quad)
void RESONANCES::out(string filename) const
{
 unsigned int i=0;
 int w=14;
 fstream file;
 stringstream msg;

 if (!on) return;  //write output only, if Resonances are used

 file.open(filename.c_str(), ios::out);
 if (!file.is_open()) {
   msg << "ERROR: RESONANCES::out: Cannot open " << filename << ".";
   throw std::runtime_error(msg.str());
 }

 file << info.out("#");

 file <<setw(w)<< "theta [°]" <<setw(w)<<  "tot.kick [mrad]" <<setw(w)<< "corrs [mrad]"
      <<setw(w)<< "quads [mrad]"<<setw(w)<< "corrs+quads" << endl;
 file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
 for (i=0; i<theta.size(); i++) {
   file <<setw(w)<< theta[i] <<setw(w)<< kick[i] <<setw(w)<< vCorrKick[i]
	<<setw(w)<< quadKick[i] <<setw(w)<< vCorrKick[i]+quadKick[i] << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;
}
