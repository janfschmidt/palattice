/* particle orbit (x&z) along accelerator */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include "types.hpp"
#include "orbit.hpp"

using namespace std;


// (observation point & turn) -> (array index)
unsigned int TRAJECTORY::Orbindex(unsigned int obs, unsigned int t) const
{
  if (obs==1)
    return (t-1);  //obs0001 with one more turn (see trajectoryimport() for details)
  else
    return (obs-2)*turns + (turns+1) + (t-1);
}



// add ORBITCOMP to ORBIT
void ORBIT::push_back(ORBITCOMP tmp)
{
  //ensure always turns = max turn (exeption pos=0.0, see comment in trajectoryimport() for details)
  if (tmp.turn > turns && tmp.pos>0.0) turns = tmp.turn;
  //count bpms during first turn (equal for every turn in madx)
  if (tmp.turn == 1) bpms++;
  Orb.push_back(tmp);
}



//subtract Ref orbit from this orbit.
//if pos or turn are unequal the number of the element with error is returned
int CLOSEDORBIT::diff(CLOSEDORBIT Ref)
{
  unsigned int i;
  for (i=0; i<this->size(); i++) {
    if (this->pos(i) == Ref.pos(i) && this->turn(i) == Ref.turn(i)) {
      Orb[i].x -= Ref.x(i);
      Orb[i].z -= Ref.z(i);
    }
    else {
      return i+1;
    }
  }
  
  return 0;
}




//output file
int CLOSEDORBIT::out(const char *filename) const
{
  unsigned int i;
  int w=10;
  fstream file;
  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: CLOSEDORBIT::out(): Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "s [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
  for (i=0; i<this->size(); i++) {
      file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
      file <<setw(w)<< pos(i) <<setw(w)<<  x(i)*1000 <<setw(w)<< z(i)*1000 << endl;
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}

int TRAJECTORY::out(const char *filename) const
{
  unsigned int t,obs;
  int w=10;
  fstream file;
  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: TRAJECTORY::out(): Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "turn" <<setw(w)<< "s [m]" <<setw(w)<< "s_tot [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
  for (t=1; t<=turns; t++) {
    for(obs=1; obs<=bpms; obs++) {
      file <<setiosflags(ios::fixed)<<noshowpoint<<setprecision(0);
      file <<setw(w)<< turn(obs,t);
      file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
      file <<setw(w)<< pos(obs,t) <<setw(w)<< (turn(obs,t)-1)*circumference+pos(obs,t) <<setw(w)<<  x(obs,t)*1000 <<setw(w)<< z(obs,t)*1000 << endl;
    }
  }
  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}




// interpolate closedorbit with n_samp points and write to other closedorbit object (interpOrbit)
int CLOSEDORBIT::interpol(CLOSEDORBIT &interpOrbit, unsigned int n_samp) const
{
  unsigned int i;
  double interval_samp = circumference/n_samp;              // sampling interval in meter
  const gsl_interp_type *type = gsl_interp_akima_periodic;  // type of interpolation used (see GSL manual)
  gsl_error_handler_t *old_error_handler;

  gsl_interp_accel *acc_x = gsl_interp_accel_alloc ();
  gsl_interp_accel *acc_z = gsl_interp_accel_alloc ();
  gsl_spline *spline_x = gsl_spline_alloc (type, bpms+2);
  gsl_spline *spline_z = gsl_spline_alloc (type, bpms+2);

  double tmp_pos[bpms+2], tmp_x[bpms+2], tmp_z[bpms+2];

  struct ORBITCOMP otmp;

  interpOrbit.clear(); //delete old orbit (from previous t)

  // add initial entry to avoid extrapolation (Frank)
  tmp_pos[0] = this->pos(bpms-1) - circumference;
  tmp_x[0] = this->x(bpms-1);
  tmp_z[0] = this->z(bpms-1);
  for (i=0; i<bpms; i++) {
    tmp_pos[i+1] = this->pos(i);
    tmp_x[i+1] = this->x(i);
    tmp_z[i+1] = this->z(i);
  }
  // copy first orbit entry for interpolation with periodic boundary condition
  tmp_pos[bpms+1] = this->pos(0) + circumference; 
  tmp_x[bpms+1] = this->x(0);
  tmp_z[bpms+1] = this->z(0);
  
  //interpolation
  old_error_handler = gsl_set_error_handler_off();
  gsl_spline_init (spline_x, tmp_pos, tmp_x, bpms+2); // horizontal (x)
  gsl_spline_init (spline_z, tmp_pos, tmp_z, bpms+2); // vertical (z)

  //write interpolated orbit data
  for (i=0; i<n_samp; i++) {
    otmp.pos = i*interval_samp;
    otmp.x = gsl_spline_eval (spline_x, otmp.pos, acc_x);
    if (isnan(otmp.x))
      {
	cout << "ERROR: CLOSEDORBIT::interpol(): interpolation error in x orbit, otmp.pos="<<otmp.pos<< endl;
	otmp.x = 0.;
      }
    otmp.z = gsl_spline_eval (spline_z, otmp.pos, acc_z);
    if (isnan(otmp.z))
      {
	cout << "ERROR: CLOSEDORBIT::interpol(): interpolation error in z orbit, otmp.pos="<<otmp.pos<< endl;
	otmp.z = 0.;
      }
    interpOrbit.push_back(otmp);
  }

  gsl_spline_free (spline_x);
  gsl_interp_accel_free (acc_x);
  gsl_spline_free (spline_z);
  gsl_interp_accel_free (acc_z);
  gsl_set_error_handler(old_error_handler);

  return 0;
}
