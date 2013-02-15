/* Read data from madX output file: magnet positions, strengths, orbit, ...  */
/* 18.06.2012 - J.Schmidt */

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <vector>
#include "types.hpp"
#include "constants.hpp"
#include "orbit.hpp"


using namespace std;



int madximport(const char *filename, ORBIT &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs)
{
  string tmp, name;
  double s, l, angle, k1l, k2l, vkick; // madx column variables
  string x, y;
  fstream madx;
  MAGNET mtmp;
  ORBITCOMP otmp;


  madx.open(filename, ios::in);
  if (!madx.is_open()) {
    return 1;
  }

  while (!madx.eof()) {
    madx >> tmp;

    if (tmp == "\"SBEND\"" || tmp == "\"RBEND\"") {
      madx >> name >> s >> x >> y >> l >> angle;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.length = l;
      mtmp.strength = angle/l;   // 1/R
      dipols.push_back(mtmp);    // add new entry to dipols
    }
    else if (tmp == "\"QUADRUPOLE\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.length = l;
      mtmp.strength = k1l/l;   // k
      quads.push_back(mtmp);     // add new entries to quads & bpmorbit
      // BPMs at quads
      if (x!="na" && y!="na" && k1l!=0) { //k1l: no BPM for inactive quad (ELSA: SQ, LQ)
	otmp.pos = s;
	otmp.x = strtod(x.c_str(), NULL); // ! double x=0.0 if no valid format in string x
	otmp.z = strtod(y.c_str(), NULL);
	bpmorbit.push_back(otmp);
      }
    }
    else if (tmp == "\"SEXTUPOLE\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l >> k2l;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.length = l;
      mtmp.strength = k2l/l;
      sexts.push_back(mtmp);    // add new entries to sexts & bpmorbit      
      // BPMs at sexts
      //if (x!="na" && y!="na") {
      //otmp.pos = s;
      //otmp.x = strtod(x.c_str(), NULL); // ! double x=0.0 if no valid format in string x
      //otmp.z = strtod(y.c_str(), NULL);
      //bpmorbit.push_back(otmp);
      //}
    }
    else if (tmp == "\"VKICKER\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l >> k2l >> vkick;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.length = l;
      mtmp.strength = vkick/l;   // 1/R from kick-angle
      vcorrs.push_back(mtmp);    // add new entry to vcorrs
    }

  }
  madx.close();

  return 0;
}


/* import magnet misalignments from madx an change field accordingly */
int misalignments(const char *filename, magnetvec &dipols)
{
  string tmp;
  unsigned int j,d=0;
  fstream madx;
  unsigned int dmax=dipols.size();

  madx.open(filename, ios::in);
  if (!madx.is_open()) {
    return 1;
  }
  
  while (!madx.eof() && d<dmax) {
    madx >> tmp;

    if (tmp == dipols[d].name) {
      for (j=0; j<47; j++) madx >> tmp; //read stupid unnecessary columns
      madx >> dipols[d].dpsi;           //rotation around s-axis
      d++;
    }

  }


  return 0;
}
