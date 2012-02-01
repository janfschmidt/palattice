/* calculate the gamma-factor from beam energy for time t via energy ramp */
/* ! energy ramp starts at t=0s & ends at t=0.287s (manually from timing.dat) */


#include <stdio.h>
#include <cmath>


double gamma(double t)
{
  double E=0, gamma=0;
  double E_inj = 1.2;
  double E_ext = 2.35;
  double t_ext = 0.287;
  double rampspeed = 4.00;

  if (t<=t_ext)
    E = E_inj + rampspeed * t;
  else
    E = E_ext;

  gamma = E / (511e-6);

  return gamma;
}
