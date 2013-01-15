/* create magnetic field distributions Bx & Bz along ring from magnet-position-data and orbit */
/* 15.01.2013 - J.Schmidt */

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <vector>
#include "constants.hpp"
#include "types.hpp"

using namespace std;

/* create magn. field with n_samp points along ring */
/* UNIT: [B] = 1/m (factor gamma*m*c/e for [B]=T) */
int getfields (FIELD *B, double circumference, orbitvec &orbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs)
{
  
 int i;
 unsigned int d=0, q=0, s=0, v=0;
 bool dSwitch=false,  qSwitch=false, sSwitch=false, vSwitch=false;
 int n_samp = orbit.size();
 double interval_samp = circumference/n_samp; // sampling interval of magn. field values along ring in meter
 double phase_perdip = 360 / dipols.size();   //spin-phaseadvance per dipole
 

 for (i=0; i<n_samp; i++) {
   B[i].pos = i*interval_samp;

   /* dipoles */
   if (d<dipols.size() && B[i].pos >= dipols[d].start && B[i].pos <= dipols[d].end) {
     B[i].name = dipols[d].name;
     B[i].x = - dipols[d].strength * sin(dipols[d].dpsi);
     B[i].z = + dipols[d].strength * cos(dipols[d].dpsi);
     B[i].spinphase = phase_perdip * (d + (B[i].pos-dipols[d].start)/dipols[d].length); //linear in dipole
     dSwitch=true;
   }
   /* quadrupoles */
   else if (q<quads.size() && B[i].pos >= quads[q].start && B[i].pos <= quads[q].end) {
     B[i].name = quads[q].name;
     B[i].x = quads[q].strength * orbit[i].z;
     B[i].z = 0; // neglect
     B[i].spinphase = d * phase_perdip;
     qSwitch=true;
   }
   /* sextupoles */
   else if (s<sexts.size() && B[i].pos >= sexts[s].start && B[i].pos <= sexts[s].end) {
     B[i].name = sexts[s].name;
     B[i].x = 0.5 * sexts[s].strength * pow(orbit[i].z, 2);
     B[i].z = 0; // neglect
     B[i].spinphase = d * phase_perdip;
     sSwitch=true;
   }
   /* vertical correctors */
   else if (v<vcorrs.size() && B[i].pos >= vcorrs[v].start && B[i].pos <= vcorrs[v].end) {
     B[i].name = vcorrs[v].name;
     B[i].x = vcorrs[v].strength;
     B[i].z = 0;
     B[i].spinphase = d * phase_perdip;
     vSwitch=true;
   }
   /* --add more magnet types here-- */
   /* drift */
   else {
     if (dSwitch) {        // increase magnet-index if end of a magnet was reached
       d++;
       dSwitch=false;
     }
     else if (qSwitch) {
       q++;
       qSwitch=false;
     }
     else if (sSwitch) {
       s++;
       sSwitch=false;
     }
     else if (vSwitch) {
       v++;
       vSwitch=false;
     }
     B[i].name = "\"DRIFT\"";
     B[i].x = 0;
     B[i].z = 0;
     B[i].spinphase = d * phase_perdip;
   }
   
 }
 

 return 0;
}




/* create output file with field data */
int fields_out(FIELD *B, int n_samp, const char *filename)
{
 int i=0;
 int w=14;
 fstream file;
 double c = 299792458;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: Cannot open " << filename << "." << endl;
   return 1;
 }

 file <<setw(w)<< "s [m]" <<setw(w)<< "t [s]" <<setw(w)<< "phase [deg]" <<setw(w)<< "name" <<setw(w)<< "Bx [1/m]" <<setw(w)<< "Bz [1/m]" <<setw(w)<< "Bs [1/m]" << endl;
 for (i=0; i<n_samp; i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
   file <<setw(w)<< B[i].pos <<setw(w)<< B[i].pos/c <<setw(w)<< B[i].spinphase <<setw(w)<< B[i].name <<setw(w)<< B[i].x <<setw(w)<< B[i].z <<setw(w)<< 0.0 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
