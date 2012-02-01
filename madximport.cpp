/* Read data from madX output file: magnet positions, strengths, orbit, ...  */
/* 18.01.2012 - J.Schmidt */

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

using namespace std;



int madximport(char *filename, orbitvec &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs)
{
  string tmp, name;
  double s, l, angle, k1l, k2l, vkick; // madx column variables
  string x, y;
  fstream madx;
  MAGNET mtmp;
  ORBIT otmp;


  madx.open(filename, ios::in);
  if (!madx.is_open()) {
    return 1;
  }

  while (!madx.eof()) {
    madx >> tmp;

    if (tmp == "\"SBEND\"") {
      madx >> name >> s >> x >> y >> l >> angle;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.strength = angle/l;   // 1/R
      dipols.push_back(mtmp);    // add new entry to dipols
    }
    else if (tmp == "\"QUADRUPOLE\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.strength = k1l/l;   // k
      quads.push_back(mtmp);     // add new entries to quads & bpmorbit
      // BPMs at quads
      if (x!="na" && y!="na") {
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
      mtmp.strength = k2l/l;
      sexts.push_back(mtmp);    // add new entries to sexts & bpmorbit      
      // BPMs at sexts
      if (x!="na" && y!="na") {
	otmp.pos = s;
	otmp.x = strtod(x.c_str(), NULL); // ! double x=0.0 if no valid format in string x
	otmp.z = strtod(y.c_str(), NULL);
	bpmorbit.push_back(otmp);
      }
    }
    else if (tmp == "\"VKICKER\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l >> k2l >> vkick;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.strength = vkick/l;   // 1/R from kick-angle
      vcorrs.push_back(mtmp);    // add new entry to vcorrs
    }

  }
  madx.close();

  return 0;
}

