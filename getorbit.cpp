/* create particle orbit from BPM data  */
/* 14.03.2012 - J.Schmidt */

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
#include "orbit.hpp"


/* interpolate orbit with n_samp points from bpmorbit */
int getorbit (CLOSEDORBIT &orbit, double circumference, CLOSEDORBIT &bpmorbit, unsigned int n_samp)
{
  unsigned int i;
  unsigned int n_bpm = bpmorbit.size();
  double interval_samp = circumference/n_samp;              /* sampling interval in meter */
  const gsl_interp_type *type = gsl_interp_akima_periodic;  /* type of interpolation used (see GSL manual) */
  gsl_error_handler_t *old_error_handler;

  gsl_interp_accel *acc_x = gsl_interp_accel_alloc ();
  gsl_interp_accel *acc_z = gsl_interp_accel_alloc ();
  gsl_spline *spline_x = gsl_spline_alloc (type, n_bpm+2);
  gsl_spline *spline_z = gsl_spline_alloc (type, n_bpm+2);

  double tmp_pos[n_bpm+2], tmp_x[n_bpm+2], tmp_z[n_bpm+2];

  struct ORBITCOMP otmp;

  orbit.clear(); //delete old orbit (from previous t)

  // add initial entry to avoid extrapolation (Frank)
  tmp_pos[0] = bpmorbit.pos(n_bpm-1) - circumference;
  tmp_x[0] = bpmorbit.x(n_bpm-1);
  tmp_z[0] = bpmorbit.z(n_bpm-1);
  for (i=0; i<n_bpm; i++) {
    tmp_pos[i+1] = bpmorbit.pos(i);
    tmp_x[i+1] = bpmorbit.x(i);
    tmp_z[i+1] = bpmorbit.z(i);
  }
  // copy first orbit entry for interpolation with periodic boundary condition
  tmp_pos[n_bpm+1] = bpmorbit.pos(0) + circumference; 
  tmp_x[n_bpm+1] = bpmorbit.x(0);
  tmp_z[n_bpm+1] = bpmorbit.z(0);
  
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

