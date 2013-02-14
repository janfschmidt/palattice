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
#include "resonances.hpp"
#include "constants.hpp"
#include "types.hpp"
#include "fieldmap.hpp"

using namespace std;

/* create magn. field with n_samp points along ring */
/* UNIT: [B] = 1/m (factor gamma*m*c/e for [B]=T) */
int getfields (FIELDMAP &B, double circumference, orbitvec &orbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs, RESONANCES &Res)
{
  
 unsigned int i;
 unsigned int d=0, q=0, s=0, v=0;
 bool dSwitch=false,  qSwitch=false, sSwitch=false, vSwitch=false;
 double interval_samp = circumference/B.n_samp; // sampling interval of magn. field values along ring in meter
 double phase_perdip = 360 / dipols.size();   //spin-phaseadvance per dipole
 //-> phase_perdip variabel machen mit dipollänge/gesamtbogenlänge? ... 
 FIELD Btmp;

 Res.clear(); //delete data from previous t

 for (i=0; i<B.n_samp; i++) {
   Btmp.pos = i*interval_samp;

   /* dipoles */
   if (d<dipols.size() && Btmp.pos >= dipols[d].start && Btmp.pos <= dipols[d].end) {
     Btmp.name = dipols[d].name;
     Btmp.x = - dipols[d].strength * sin(dipols[d].dpsi);
     Btmp.z = + dipols[d].strength * cos(dipols[d].dpsi);
     Btmp.theta = phase_perdip * (d + (Btmp.pos-dipols[d].start)/dipols[d].length); //linear in dipole
     B.set(i, Btmp);
     dSwitch=true;
   }
   /* quadrupoles */
   else if (q<quads.size() && Btmp.pos >= quads[q].start && Btmp.pos <= quads[q].end) {
     Btmp.name = quads[q].name;
     Btmp.x = quads[q].strength * orbit[i].z;
     Btmp.z = 0; // neglect
     Btmp.theta = d * phase_perdip;
     B.set(i, Btmp);
     qSwitch=true;
   }
   /* sextupoles */
   else if (s<sexts.size() && Btmp.pos >= sexts[s].start && Btmp.pos <= sexts[s].end) {
     Btmp.name = sexts[s].name;
     Btmp.x = 0.5 * sexts[s].strength * orbit[i].x * orbit[i].z;
     Btmp.z = 0; // neglect
     Btmp.theta = d * phase_perdip;
     B.set(i, Btmp);
     sSwitch=true;
   }
   /* vertical correctors */
   else if (v<vcorrs.size() && Btmp.pos >= vcorrs[v].start && Btmp.pos <= vcorrs[v].end) {
     Btmp.name = vcorrs[v].name;
     Btmp.x = vcorrs[v].strength;
     Btmp.z = 0;
     Btmp.theta = d * phase_perdip;
     B.set(i, Btmp);
     vSwitch=true;
   }
   /* --add more magnet types here-- */
   // drift
   // & increase magnet-index if end of a magnet was reached
   // & add previous magnet to Res
   else {
     if (dSwitch) {        
       if (Res.on) {Res.adddip(dipols[d], B.x(i-1));}
       d++;
       dSwitch=false;
     }
     else if (qSwitch) {
       if (Res.on) {Res.addother(quads[q], B.x(i-1));}
       q++;
       qSwitch=false;
     }
     else if (sSwitch) {
       if (Res.on) {Res.addother(sexts[s], B.x(i-1));}
       s++;
       sSwitch=false;
     }
     else if (vSwitch) {
       if (Res.on) {Res.addother(vcorrs[v], B.x(i-1));}
       v++;
       vSwitch=false;
     }
     Btmp.name = "\"DRIFT\"";
     Btmp.x = 0;
     Btmp.z = 0;
     Btmp.theta = d * phase_perdip;
     B.set(i, Btmp);
   }
   
 }

 if (Res.on) {Res.closering();}

 return 0;
}




/* create output file with field data */
int fields_out(FIELDMAP B, const char *filename)
{
 unsigned int i=0;
 int w=14;
 fstream file;
 double c = 299792458;

 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: Cannot open " << filename << "." << endl;
   return 1;
 }

 file <<setw(w)<< "s [m]" <<setw(w)<< "t [s]" <<setw(w)<< "phase [deg]" <<setw(w)<< "name" <<setw(w)<< "Bx [1/m]" <<setw(w)<< "Bz [1/m]" <<setw(w)<< "Bs [1/m]" << endl;
 for (i=0; i<B.n_samp; i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(4);
   file <<setw(w)<< B.pos(i) <<setw(w)<< B.pos(i)/c <<setw(w)<< B.theta(i) <<setw(w)<< B.name(i) <<setw(w)<< B.x(i) <<setw(w)<< B.z(i) <<setw(w)<< 0.0 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
