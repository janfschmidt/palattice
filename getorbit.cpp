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

  gsl_interp_accel *acc_x = gsl_interp_accel_alloc ();
  gsl_interp_accel *acc_z = gsl_interp_accel_alloc ();
  gsl_spline *spline_x = gsl_spline_alloc (type, n_bpm+1);
  gsl_spline *spline_z = gsl_spline_alloc (type, n_bpm+1);

  double tmp_pos[n_bpm+1], tmp_x[n_bpm+1], tmp_z[n_bpm+1];

  struct ORBIT otmp;

  for (i=0; i<n_bpm; i++) {
    tmp_pos[i] = bpmorbit[i].pos;
    tmp_x[i] = bpmorbit[i].x;
    tmp_z[i] = bpmorbit[i].z;
  }
  /* copy first orbit entry for interpolation with periodic boundary condition */
  tmp_pos[n_bpm] = tmp_pos[0]+circumference; 
  tmp_x[n_bpm] = tmp_x[0];
  tmp_z[n_bpm] = tmp_z[0];
  
  gsl_spline_init (spline_x, tmp_pos, tmp_x, n_bpm+1); /* horizontal (x) */
  gsl_spline_init (spline_z, tmp_pos, tmp_z, n_bpm+1); /* vertical (z) */

  for (i=0; i<n_samp; i++) {
    otmp.pos = i*interval_samp;
    otmp.x = gsl_spline_eval (spline_x, otmp.pos, acc_x);
    otmp.z = gsl_spline_eval (spline_z, otmp.pos, acc_z);
    orbit.push_back(otmp);
  }
  gsl_spline_free (spline_x);
  gsl_interp_accel_free (acc_x);
  gsl_spline_free (spline_z);
  gsl_interp_accel_free (acc_z);

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
