/* create particle orbit from BPM data  */
/* 03.02.2012 - J.Schmidt */

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <vector>
#include "constants.hpp"
#include "types.hpp"


/* interpolate orbit with n_samp points from bpmorbit */
int getorbit (orbitvec &orbit, double circumference, orbitvec &bpmorbit, int n_samp)
{
  int i;
  int n_bpm = bpmorbit.size();
  double interval_samp = circumference/n_samp;              /* sampling interval in meter */
  const gsl_interp_type *type = gsl_interp_akima_periodic;  /* type of interpolation used (see GSL manual) */
  gsl_error_handler_t *old_error_handler;

  gsl_interp_accel *acc_x = gsl_interp_accel_alloc ();
  gsl_interp_accel *acc_z = gsl_interp_accel_alloc ();
  gsl_spline *spline_x = gsl_spline_alloc (type, n_bpm+2);
  gsl_spline *spline_z = gsl_spline_alloc (type, n_bpm+2);

  double tmp_pos[n_bpm+2], tmp_x[n_bpm+2], tmp_z[n_bpm+2];

  struct ORBIT otmp;

  // add initial entry to avoid extrapolation (Frank)
  tmp_pos[0] = bpmorbit[n_bpm-1].pos-circumference;
  tmp_x[0] = bpmorbit[n_bpm-1].x;
  tmp_z[0] = bpmorbit[n_bpm-1].z;
  for (i=0; i<n_bpm; i++) {
    tmp_pos[i+1] = bpmorbit[i].pos;
    tmp_x[i+1] = bpmorbit[i].x;
    tmp_z[i+1] = bpmorbit[i].z;
  }
  // copy first orbit entry for interpolation with periodic boundary condition
  tmp_pos[n_bpm+1] = bpmorbit[0].pos+circumference; 
  tmp_x[n_bpm+1] = bpmorbit[0].x;
  tmp_z[n_bpm+1] = bpmorbit[0].z;
  
  old_error_handler = gsl_set_error_handler_off();
  gsl_spline_init (spline_x, tmp_pos, tmp_x, n_bpm+2); // horizontal (x)
  gsl_spline_init (spline_z, tmp_pos, tmp_z, n_bpm+2); // vertical (z)


  for (i=0; i<n_samp; i++) {
    otmp.pos = i*interval_samp;
    otmp.x = gsl_spline_eval (spline_x, otmp.pos, acc_x);
    if (isnan(otmp.x))
      {
	cout << "ERROR: getorbit(): interpolation error in x orbit, otmp.pos="<<otmp.pos<< endl;
	otmp.x = 0.;
      }
    otmp.z = gsl_spline_eval (spline_z, otmp.pos, acc_z);
    if (isnan(otmp.z))
      {
	cout << "ERROR: getorbit(): interpolation error in z orbit, otmp.pos="<<otmp.pos<< endl;
	otmp.z = 0.;
      }
    #if DEBUG
      cout << "pos="<<otmp.pos<<"  x="<<otmp.x<<"  z="<<otmp.z << endl;
    #endif
    orbit.push_back(otmp);
  }

  gsl_spline_free (spline_x);
  gsl_interp_accel_free (acc_x);
  gsl_spline_free (spline_z);
  gsl_interp_accel_free (acc_z);
  gsl_set_error_handler(old_error_handler);

  return 0;
}



/* create output file with orbit data */
int orbit_out(orbitvec &orbit, char *filename)
{
 int i=0;
 int w=10;
 int n_samp = orbit.size();
 fstream file;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: Cannot open " << filename << "." << endl;
   return 1;
 }

file <<setw(w)<< "s [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
 for (i=0; i<n_samp; i++) {
   file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
   file <<setw(w)<< orbit[i].pos <<setw(w)<< orbit[i].x*1000 <<setw(w)<< orbit[i].z*1000 << endl;
 }
 file.close();
 cout << "Wrote " << filename  << endl;

 return 0;
}
