/* Read data from ELSA CCS, /sgt and ELSA MadX Lattice: element positions, bpm- & magnet-data, ... */
/* 13.03.2012 - J.Schmidt */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "constants.hpp"
#include "types.hpp"
#include "ELSAimport.hpp"

using namespace std;


/* read ELSA data */
int ELSAimport(BPM *ELSAbpms, CORR *ELSAvcorrs, char *spurenFolder)
{
  
  ELSAimport_bpms(ELSAbpms, spurenFolder);
  ELSAimport_vcorrs(ELSAvcorrs, spurenFolder);

  return 0;
}



/* Write quad- & sext-strengths */
int ELSAimport_magnetstrengths(magnetvec &quads, magnetvec &sexts, char *spurenFolder)
{
  int i, n;
  char filename[1024];
  string tmp;
  double kf=0, kd=0, mf=0, md=0;
  fstream f_magnets;

 // Read "magnets.dat"
  snprintf(filename, 1024, "%s/magnets.dat", spurenFolder);
  f_magnets.open(filename, ios::in);
  if (!f_magnets.is_open()) {
    printf("ERROR: ELSAimport.cpp: Cannot open %s\n", filename);
    return 1;
  }
  while(!f_magnets.eof()) {
    f_magnets >> tmp;
    if (tmp=="ELS_MAGNETE_QUADF.KF_AC")
      f_magnets >> kf;
    else if (tmp=="ELS_MAGNETE_QUADD.KD_AC")
      f_magnets >> kd;
    else if (tmp=="ELS_MAGNETE_SEXTF.MF_AC")
      f_magnets >> mf;
    else if (tmp=="ELS_MAGNETE_SEXTD.MD_AC")
      f_magnets >> md;
    else if (tmp=="#")         //jump over comment lines
      getline(f_magnets, tmp);
    else
      printf("ERROR: ELSAimport.cpp: Unexpected entry in %s\n", filename);
  }
  f_magnets.close();

  //Write strengths to quads & sexts
  n = quads.size();
  for (i=0; i<n; i++) {
    if (quads[i].name.compare(1,2,"QF") == 0) {
      quads[i].strength = kf;
    }
    else if (quads[i].name.compare(1,2,"QD") == 0) {
      quads[i].strength = kd;
    }
  }
  n = sexts.size();
  for (i=0; i<n; i++) {
    if (sexts[i].name.compare(1,2,"SF") == 0) {
      sexts[i].strength = mf;
    }
    else if (sexts[i].name.compare(1,2,"SD") == 0) {
      sexts[i].strength = md;
    }
  }
  
  return 0;
}



/* Write BPM data from files to bpms */
int ELSAimport_bpms(BPM *ELSAbpms, char *spurenFolder)
{
  int i;
  char filename[1024];
  BPM_MS tmptime;
  double tmp;
  fstream file;
 
  // Read Monitor positions from "bpms_spos.dat"
  snprintf(filename, 1024, "%s/bpms_spos.dat", spurenFolder);
  file.open(filename, ios::in);
  if (!file.is_open()) {
    printf("ERROR: ELSAimport.cpp: Cannot open %s\n", filename);
    return 1;
  }
  for (i=0; i<NBPMS; i++) {
    file >> ELSAbpms[i].pos;
  }
  file.close();
  
  // Read BPM Data from "bpmXX.dat" (all BPMs)
  for (i=0; i<NBPMS; i++) {
    snprintf(filename, 1024, "%s/bpm%02i.dat", spurenFolder, i+1);
    file.open(filename, ios::in);
    if (!file.is_open()) {
      printf("ERROR: ELSAimport.cpp: Cannot open %s\n", filename);
      return 1;
    }
    while(!file.eof()) {
      file >> tmptime.ms >> tmptime.x >> tmptime.z >> tmp;
      ELSAbpms[i].time.push_back(tmptime); 
    }
    file.close();
  }

  return 0;
}



/* write BPM data for time t to bpmorbit */
int ELSAimport_getbpmorbit(BPM *ELSAbpms, orbitvec &bpmorbit, unsigned int t)
{
  int i;
  ORBIT otmp;

  bpmorbit.clear(); //delete old-BPM-data (from madx or previous t)

  if (t > ELSAbpms[1].time.size()) {  //ELSAbpms[1] chosen, all have same .size()
    printf("ERROR: ELSAimport.cpp: No ELSA BPM data available for %d ms.\n", t);
    return 1;
  }
  
  for (i=0; i<NBPMS; i++) {
    otmp.pos = ELSAbpms[i].pos;
    otmp.x = ELSAbpms[i].time[t].x / 1000; // unit mm -> m
    otmp.z = ELSAbpms[i].time[t].z / 1000;
    bpmorbit.push_back(otmp);
  }
  
  return 0;
}




/* Write corrector data from files to ELSAvcorrs */
int ELSAimport_vcorrs(CORR *ELSAvcorrs, char *spurenFolder)
{
  int i, j, tmp;
  char filename[1024];
  string str;
  CORR_MS tmptime, tmpzero;
  fstream file;
 
  // Read corrector positions from "VCORRS.SPOS"
  snprintf(filename, 1024, "%s/correctors/VCORRS.SPOS", spurenFolder);
  file.open(filename, ios::in);
  if (!file.is_open()) {
    printf("ERROR: ELSAimport.cpp: Cannot open %s\n", filename);
    return 1;
  }
  for (i=0; i<NVCORRS; i++) {
    file >> tmp >> ELSAvcorrs[i].pos;
  }
  file.close();
  
  //Read corrector kickangles from "VCxx.KICK"
  for (i=0; i<NVCORRS; i++) {
    snprintf(filename, 1024, "%s/correctors/VC%02d.KICK", spurenFolder, i+1);
    file.open(filename, ios::in);
    if (!file.is_open()) {
      printf("ERROR: ELSAimport.cpp: Cannot open %s\n", filename);
      //return 1;
    }
    //read Headline
    file >> str >> str;
    //read t[ms] from first line to detect "Schleppfehler"
    file >> tmptime.ms >> tmptime.kick;
    for (j=0; j<tmptime.ms; j++) {
      tmpzero.ms = j;
      tmpzero.kick=0.0;
      ELSAvcorrs[i].time.push_back(tmpzero);
    }
    ELSAvcorrs[i].time.push_back(tmptime);
    //now continue normally
    while (!file.eof()) {
      file >> tmptime.ms >> tmptime.kick;  //kickangle [mrad]  
      ELSAvcorrs[i].time.push_back(tmptime);
    }
    file.close();
  }

  return 0;
}




/* write corrector data for time t to vcorrs
  !vcorrs must be read from madx before to get length! */
int ELSAimport_getvcorrs(CORR *ELSAvcorrs, magnetvec &vcorrs, double corrlength, unsigned int t)
{
  int i;
  MAGNET mtmp;
  char name[20];

  vcorrs.clear(); //delete old corrector-data (from madx or previous t)

  if (t > ELSAvcorrs[1].time.size()) {  //ELSAvcorrs[1] chosen, all have same .size()
    printf("ERROR: ELSAimport.cpp: No ELSA vertical corrector data available for %d ms.\n", t);
    return 1;
  }

  for (i=0; i<NVCORRS; i++) {
    if (ELSAvcorrs[i].pos==0.0) {
      continue;
    }
    snprintf(name, 20, "\"KV%02i\"", i+1);
    mtmp.name = name;
    mtmp.start = ELSAvcorrs[i].pos - corrlength/2.0;
    mtmp.end = ELSAvcorrs[i].pos + corrlength/2.0; 
    mtmp.strength = ELSAvcorrs[i].time[t].kick/1000.0/corrlength;   //unit 1/m
    vcorrs.push_back(mtmp);
  }
  
  return 0;
}




/* create output file with BPM data */
int bpms_out(orbitvec bpmorbit, char *filename)
{
 unsigned int i=0;
 int w=10;
 fstream file;
 
 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: ELSAimport.cpp: Cannot open " << filename << "." << endl;
   return 1;
 }
 
 file <<setw(w)<< "s [m]" <<setw(w)<< "x [mm]" <<setw(w)<< "z [mm]" << endl;
 for (i=0; i<bpmorbit.size(); i++) {
   file <<setiosflags(ios::fixed)<<showpoint<<setprecision(3);
   file <<setw(w)<< bpmorbit[i].pos <<setw(w)<< bpmorbit[i].x*1000 <<setw(w)<< bpmorbit[i].z*1000 << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}




/* create output file with corrector data */
int corrs_out(magnetvec vcorrs, char *filename)
{
 unsigned int i=0;
 int w=14;
 fstream file;
 
 file.open(filename, ios::out);
 if (!file.is_open()) {
   cout << "ERROR: ELSAimport.cpp: Cannot open " << filename << "." << endl;
   return 1;
 }
 
 file <<setw(w)<< "s[m](start)" <<setw(w)<< "strength[1/m]" << endl;
 for (i=0; i<vcorrs.size(); i++) {
   file <<setiosflags(ios::scientific)<<showpoint<<setprecision(3);
   file <<setw(w)<< vcorrs[i].start <<setw(w)<< vcorrs[i].strength << endl;
 }
 file.close();
 cout << "* Wrote " << filename  << endl;

 return 0;
}
