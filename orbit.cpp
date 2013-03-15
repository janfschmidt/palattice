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




CLOSEDORBIT::~CLOSEDORBIT()
{
  if (interp_flag) {
    gsl_spline_free (spline_x);
    gsl_interp_accel_free (acc_x);
    gsl_spline_free (spline_z);
    gsl_interp_accel_free (acc_z);
  }
}



// (observation point & turn) -> (array index)
unsigned int TRAJECTORY::Orbindex(unsigned int obs, unsigned int t) const
{
  //Catch calls for data out of range
  if ((t > n_turns+1) || ((t>n_turns) && (obs!=1)) || (obs > n_bpms)) {
    cout << "ERROR: TRAJECTORY::Orbindex(): Orbit for turn "<<t<<" and obs "<<obs<<" not known, only "
	 <<n_turns<<" turns and "<<n_bpms<<" obs are imported from madx." << endl;
    return 0; // !! in this case the initial value orbit is given
  }
  if (t==0 || obs==0) {
    cout << "ERROR: TRAJECTORY::Orbindex(): Orbit turn and obs must be > 0." << endl;
    return 0; // !! in this case the initial value orbit is given
  }


  if (obs==1)
    return (t-1);  //obs0001 with one more turn (see trajectoryimport() for details)
  else
    return (obs-2)*n_turns + (n_turns+1) + (t-1);
}



// add ORBITCOMP to ORBIT
void ORBIT::push_back(ORBITCOMP tmp)
{
  //ensure always n_turns = max turn (exeption pos=0.0, see comment in trajectoryimport() for details)
  if (tmp.turn > n_turns && tmp.pos>0.0) n_turns = tmp.turn;
  //count n_bpms during first turn (equal for every turn in madx)
  if (tmp.turn == 1) n_bpms++;
  Orb.push_back(tmp);
}



//subtract Ref orbit from this orbit.
//if pos or turn are unequal the number of the element with error is returned
int CLOSEDORBIT::diff(CLOSEDORBIT Ref)
{
  unsigned int i;
  for (i=0; i<size(); i++) {
    if (pos(i) == Ref.pos(i) && turn(i) == Ref.turn(i)) {
      Orb[i].x -= Ref.x(i);
      Orb[i].z -= Ref.z(i);
    }
    else {
      return i+1;
    }
  }
  
  return 0;
}




//add a CLOSEDORBIT to a TRAJECTORY to get absolute x & z,
//if madx trajectory data is relative to closed orbit
int TRAJECTORY::add_closedorbit(CLOSEDORBIT co)
{
  unsigned int i;

  if (circumference != co.circumference) {
    cout << "ERROR: TRAJECTORY::add_closedorbit(): trajectory circumference is "<<circumference
	 << " m and closedorbit circumference is "<<co.circumference<< " m." <<endl;
    cout << " So I guess it is not the same accelerator and therefore adding the orbits is not recommended." << endl;
    return 1;
  }

  interp_update();

  for (i=0; i<size(); i++) {
    Orb[i].x += co.interp_x(Orb[i].pos);
    Orb[i].z += co.interp_z(Orb[i].pos);
  }

  return 0;
}





// --------------------------------- output files ---------------------------------------------------

//output file with interpolated orbit from pos 0 to pos_max()
int ORBIT::interp_out(double stepwidth, const char *filename)
{
  double s=0.0;
  int w=10;
  fstream file;
  
  if (stepwidth <= 0.) {
    cout << "ERROR: ORBIT::interp_out(): output stepwidth must be > 0.0 (" <<stepwidth<< " is not)" <<endl;
    return 1;
  }
 
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: ORBIT::interp_out(): Cannot open " << filename << "." << endl;
    return 1;
  }
  
  file <<setw(w)<< "s [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
  file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
  if (pos_max() > 0) {
    for (s=0.0; s<=pos_max(); s+=stepwidth) {
      file <<setw(w)<< s <<setw(w)<< interp_x(s)*1000 <<setw(w)<< interp_z(s)*1000 << endl;
    }
  }
  file.close();
  cout << "* Wrote " << filename  << endl;

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
  file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
  for (i=0; i<size(); i++) {
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
  for (t=1; t<=n_turns; t++) {
    for(obs=1; obs<=n_bpms; obs++) {
      file <<setiosflags(ios::fixed)<<noshowpoint<<setprecision(0);
      file <<setw(w)<< turn(obs,t);
      file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
      file <<setw(w)<< pos(obs,t) <<setw(w)<< pos_tot(obs,t) <<setw(w)<<  x(obs,t)*1000 <<setw(w)<< z(obs,t)*1000 << endl;
    }
  }
  // add end of last turn (=begin of next, see trajectoryimport() for details)
  if (n_bpms>0) {
    obs=1;
    file <<setiosflags(ios::fixed)<<noshowpoint<<setprecision(0);
    file <<setw(w)<< turn(obs,t);
    file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
    file <<setw(w)<< pos(obs,t) <<setw(w)<< pos_tot(obs,t) <<setw(w)<<  x(obs,t)*1000 <<setw(w)<< z(obs,t)*1000 << endl;
  }

  file.close();
  cout << "* Wrote " << filename  << endl;
  
  return 0;
}






// --------------------------------- interpolation ---------------------------------------------------

// initialize interpolation of closedorbit (spline_x & spline_z)
void CLOSEDORBIT::interp_init()
{
  if (interp_flag) {
    cout << "ERROR: CLOSEDORBIT::interp_init: Interpolation initialisation called twice. Skip." << endl;
    return;
  }

  unsigned int i;
  const gsl_interp_type *type = gsl_interp_akima_periodic;  // type of interpolation used (see GSL manual)
  gsl_error_handler_t *old_error_handler;
  struct ORBITCOMP otmp;
  double tmp_pos[n_bpms+2], tmp_x[n_bpms+2], tmp_z[n_bpms+2];

  acc_x = gsl_interp_accel_alloc ();
  acc_z = gsl_interp_accel_alloc ();
  spline_x = gsl_spline_alloc (type, n_bpms+2);
  spline_z = gsl_spline_alloc (type, n_bpms+2);

  
  // add initial entry to avoid extrapolation (Frank)
  tmp_pos[0] = pos(n_bpms-1) - circumference;
  tmp_x[0] = x(n_bpms-1);
  tmp_z[0] = z(n_bpms-1);
  for (i=0; i<n_bpms; i++) {
    tmp_pos[i+1] = pos(i);
    tmp_x[i+1] = x(i);
    tmp_z[i+1] = z(i);
  }
  // copy first orbit entry for interpolation with periodic boundary condition
  tmp_pos[n_bpms+1] = pos_max(); 
  tmp_x[n_bpms+1] = x(0);
  tmp_z[n_bpms+1] = z(0);
  // set pos_max to pos of last (largest) data point
  
  //interpolation
  old_error_handler = gsl_set_error_handler_off();
  gsl_spline_init (spline_x, tmp_pos, tmp_x, n_bpms+2); // horizontal (x)
  gsl_spline_init (spline_z, tmp_pos, tmp_z, n_bpms+2); // vertical (z)

  gsl_set_error_handler(old_error_handler);

  interp_flag = true;
}



// initialize interpolation of trajectory (spline_x & spline_z)
void TRAJECTORY::interp_init()
{
  if (interp_flag) {
    cout << "ERROR: TRAJECTORY::interp_init: Interpolation initialisation called twice. Skip." << endl;
    return;
  }
  
  unsigned int t, obs, i=0;
  unsigned int n = size();
  const gsl_interp_type *type = gsl_interp_akima;  // type of interpolation used (see GSL manual)
  gsl_error_handler_t *old_error_handler;
  struct ORBITCOMP otmp;
  double tmp_pos[n], tmp_x[n], tmp_z[n];
  
  acc_x = gsl_interp_accel_alloc ();
  acc_z = gsl_interp_accel_alloc ();
  spline_x = gsl_spline_alloc (type, n);
  spline_z = gsl_spline_alloc (type, n);

  for (t=1; t<=n_turns; t++) {
    for(obs=1; obs<=n_bpms; obs++) {
      tmp_pos[i] = pos_tot(obs,t);
      tmp_x[i] = x(obs,t);
      tmp_z[i] = z(obs,t);
      i++;
    }
  }
  // add end of last turn (=begin of next, see trajectoryimport() for details)
  obs=1;
  tmp_pos[i] = pos_tot(obs,t);
  tmp_x[i] = x(obs,t);
  tmp_z[i] = z(obs,t);
  
  //interpolation
  old_error_handler = gsl_set_error_handler_off();
  gsl_spline_init (spline_x, tmp_pos, tmp_x, n); // horizontal (x)
  gsl_spline_init (spline_z, tmp_pos, tmp_z, n); // vertical (z)
  
  gsl_set_error_handler(old_error_handler);
  
  interp_flag = true;
}




// return interpolated value of x (in mm) at position any_pos (s in m)
double ORBIT::interp_x(double any_pos)
{
  double tmp;
  if (!interp_flag) {
    interp_init();
  }

  if (any_pos > pos_max()) {
    cout << "ERROR: ORBIT::interp_x(): pos=" <<any_pos<< " m is to large for interpolation." <<endl;
    return 0.;
  }

  tmp = gsl_spline_eval (spline_x, any_pos, acc_x);

  if (isnan(tmp)) {
    cout << "ERROR: ORBIT::interp_x(): orbit interpolation error at pos="<<any_pos<< endl;
    return 0.;
  }
  else
    return tmp;
}

// return interpolated value of z (in mm) at position any_pos (s in m)
double ORBIT::interp_z(double any_pos)
{
  double tmp;
  if (!interp_flag) {
    interp_init();
  }

  if (any_pos > pos_max()) {
    cout << "ERROR: ORBIT::interp_z(): pos=" <<any_pos<< " m is to large for interpolation." <<endl;
    return 0.;
  }

  tmp = gsl_spline_eval (spline_z, any_pos, acc_z);

  if (isnan(tmp)) {
    cout << "ERROR: ORBIT::interp_z(): orbit interpolation error at pos="<<any_pos<< endl;
    return 0.;
  }
  else
    return tmp;
}




// deletes previous interpolation to consider data changes (-> new init() at next inperp. call)
void ORBIT::interp_update()
{
  if (interp_flag) {
    gsl_spline_free (spline_x);
    gsl_interp_accel_free (acc_x);
    gsl_spline_free (spline_z);
    gsl_interp_accel_free (acc_z);
  }
  interp_flag = false;
}
