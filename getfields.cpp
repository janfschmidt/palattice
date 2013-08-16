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
#include "orbit.hpp"
#include "functionofpos.hpp"


using namespace std;

/* create magn. field with n_samp points along ring */
/* UNIT: [B] = 1/m (factor gamma*m*c/e for [B]=T) */
int getfields (FIELDMAP &B, FunctionOfPos<AccPair> &orbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs, RESONANCES &Res)
{
  
 unsigned int i, t;
 unsigned int d=0, q=0, s=0, v=0;
 bool dSwitch=false,  qSwitch=false, sSwitch=false, vSwitch=false;
 double interval_samp = B.circumference/B.n_samp; // sampling interval of magn. field values along ring in meter
 double phase_perdip = 360 / dipols.size();   //spin-phaseadvance per dipole
 //-> phase_perdip variabel machen mit dipollänge/gesamtbogenlänge? ... 
 double pos_tot;
 
 //tmp. FIELDMAP elements:
 double pos, x, z, theta;
 string name;


 if (B.circumference != orbit.circ) {
   cout << "ERROR: getfields(): FIELDMAP and orbit have different circumferences ("
	<<B.circumference<<", "<<orbit.circ<<")." << endl;
   return 1;
 }
 if (B.n_turns != orbit.turns()) {
   cout << "ERROR: getfields(): FIELDMAP and orbit have different number of turns ("
	<<B.n_turns<<", "<<orbit.turns()<<")." << endl;
   return 1;
 }

 Res.clear(); //delete data from previous t

 for(t=1; t<=B.n_turns; t++) {
   d=0; q=0; s=0; v=0;
   for (i=0; i<B.n_samp; i++) {
     pos = i*interval_samp;
     pos_tot = orbit.posTotal(pos, t); // ALT::::: pos + (t-1)*B.circumference; //equivalent to FIELDMAP::pos_tot()
     
     /* dipoles */
     if (d<dipols.size() && pos >= dipols[d].start && pos <= dipols[d].end) {
       name = dipols[d].name;
       x = - dipols[d].strength * sin(dipols[d].dpsi);
       z = + dipols[d].strength * cos(dipols[d].dpsi);
       theta = phase_perdip * (d + (pos-dipols[d].start)/dipols[d].length); //+ (t-1)*360; //linear in dipole
       dSwitch=true;
     }
     /* quadrupoles */
     else if (q<quads.size() && pos >= quads[q].start && pos <= quads[q].end) {
       name = quads[q].name;
       x = quads[q].strength * orbit.interp(pos_tot).z;
       z = 0; // neglect
       theta = d * phase_perdip; //+ (t-1)*360;
       qSwitch=true;
     }
     /* sextupoles */
     else if (s<sexts.size() && pos >= sexts[s].start && pos <= sexts[s].end) {
       name = sexts[s].name;
       x = 0.5 * sexts[s].strength * orbit.interp(pos_tot).x * orbit.interp(pos_tot).z;
       z = 0; // neglect
       theta = d * phase_perdip; //+ (t-1)*360;
       sSwitch=true;
     }
     /* vertical correctors */
     else if (v<vcorrs.size() && pos >= vcorrs[v].start && pos <= vcorrs[v].end) {
       name = vcorrs[v].name;
       x = vcorrs[v].strength;
       z = 0;
       theta = d * phase_perdip; //+ (t-1)*360;
       vSwitch=true;
     }
     /* --add more magnet types here-- */
     // drift
     // & increase magnet-index if end of a magnet was reached
     // & add previous magnet to Res
     else {
       if (dSwitch) {        
	 if (Res.on) {Res.adddip(dipols[d], B.x(i-1,t));}
	 d++;
	 dSwitch=false;
       }
       else if (qSwitch) {
	 if (Res.on) {Res.addother(quads[q], B.x(i-1,t));}
	 q++;
	 qSwitch=false;
       }
       else if (sSwitch) {
	 if (Res.on) {Res.addother(sexts[s], B.x(i-1,t));}
	 s++;
	 sSwitch=false;
       }
       else if (vSwitch) {
	 if (Res.on) {Res.addother(vcorrs[v], B.x(i-1,t));}
	 v++;
	 vSwitch=false;
       }
       name = "\"DRIFT\"";
       x = 0;
       z = 0;
       theta = d * phase_perdip; //+ (t-1)*360;
     }
     B.set(i, t, pos, x, z, theta, name);
   }
 }

 if (Res.on) {Res.closering();}

 return 0;
}





