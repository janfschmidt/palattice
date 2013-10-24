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
#include "filenames.hpp"
#include "functionofpos.hpp"


using namespace std;



int madximport(const char *filename, FunctionOfPos<AccPair> &bpmorbit, magnetvec &dipols, magnetvec &quads, magnetvec &sexts, magnetvec &vcorrs)
{
  string tmp, name;
  double s, l, angle, k1l, k2l, vkick; // madx column variables
  string x, y;
  fstream madx;
  MAGNET mtmp;
  AccPair otmp;


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
      mtmp.strength = angle/l;   // 1/R (!!! assuming l is arclength (along ref. orbit) !!!)
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
	otmp.x = strtod(x.c_str(), NULL); // ! double x=0.0 if no valid format in string x
	otmp.z = strtod(y.c_str(), NULL);
	bpmorbit.set(otmp, s);
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
      //otmp.x = strtod(x.c_str(), NULL); // ! double x=0.0 if no valid format in string x
      //otmp.z = strtod(y.c_str(), NULL);
      //bpmorbit.set(otmp, s);
      //}
    }
    else if (tmp == "\"VKICKER\"") {
      madx >> name >> s >> x >> y >> l >> angle >> k1l >> k2l >> vkick;
      mtmp.name = name;
      mtmp.start = s-l;
      mtmp.end = s;
      mtmp.length = l;
      mtmp.strength = sin(vkick)/l;   // 1/R from kick-angle
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


/* import single particle trajectories from madx tracking data */
int trajectoryimport(const FILENAMES files, FunctionOfPos<AccPair> &trajectory, unsigned int particle)
{
  unsigned int obs=1;
  string tmp="init";
  unsigned int turn; // madx column variables
  double x, y, s;
  fstream madx;
  AccPair otmp;

  //for chosen particle read data from all observation points
  //-----------------------------------------------------------------------------------------------------
  //obs0001 is a special case: it corresponds to s=0.0m (START marker in MAD-X), but counts the turns different:
  //turn=0, s=0.0 are the initial conditions (beginning of turn 1)
  //turn=1, s=0.0 is the beginning of turn 2 or END of turn 1.
  //So the data of obs0001 is used with s=0.0 but one turn later than written in MAD-X to fit our notation
  //-----------------------------------------------------------------------------------------------------
  madx.open(files.tracking(obs,particle).c_str(), ios::in);
  while ( madx.is_open() ) {
    
    //go to end of "header" / first data line
    while (tmp != "$") {
      madx >> tmp;
      if (madx.eof()) {
	cout << "ERROR: madximport.cpp: Wrong data format in " << files.tracking(obs,particle) << endl;
	return 1;
      }
    }
    getline(madx, tmp);

    //read trajectory data
    while (!madx.eof()) {
      madx >> tmp >> turn >> x >> tmp >> y >> tmp >> tmp >> tmp >> s;
      getline(madx, tmp);
      if (madx.eof()) break;
      
      if (obs==1) { //see comment above
	turn += 1;
      }
      otmp.x = x;
      otmp.z = y;
      trajectory.set(otmp, s, turn);
    }

    madx.close();
    obs++;
    madx.open(files.tracking(obs,particle).c_str(), ios::in);
  }
  trajectory.pop_back_turn();

  return 0;
}
   
