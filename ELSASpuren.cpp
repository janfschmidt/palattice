/* ELSASpuren class
 * read measured closed orbits and set vertical corrector magnet kicks
 * from files written by the ELSA control system.
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "types.hpp"
#include "ELSASpuren.hpp"

using namespace std;
using namespace pal;


ELSASpuren::ELSASpuren(string _spurenFolder)
{
  read(_spurenFolder);
}

void ELSASpuren::read(string _spurenFolder)
{
  spurenFolder = _spurenFolder;
  import_bpms();
  import_vcorrs();
}

// Write BPM data from files to bpms
void ELSASpuren::import_bpms()
{
  int i;
  char filename[1024];
  BPM_MS tmptime;
  double tmp;
  fstream file;
 
  // Read Monitor positions from "bpms_spos.dat"
  snprintf(filename, 1024, "%s/bpms_spos.dat", spurenFolder.c_str());
  file.open(filename, ios::in);
  if (!file.is_open()) {
    throw palatticeFileError(filename);
  }
  for (i=0; i<NBPMS; i++) {
    file >> bpms[i].pos;
  }
  file.close();
  
  // Read BPM Data from "bpmXX.dat" (all BPMs)
  for (i=0; i<NBPMS; i++) {
    snprintf(filename, 1024, "%s/bpm%02i.dat", spurenFolder.c_str(), i+1);
    file.open(filename, ios::in);
    if (!file.is_open()) {
      printf("! ELSASpuren::import_bpms(): Cannot open %s\n", filename);
      continue;
    }
    while(!file.eof()) {
      file >> tmptime.ms >> tmptime.x >> tmptime.z >> tmp;
      bpms[i].time.push_back(tmptime); 
    }
    file.close();
  }

}





// Write corrector data from files to vcorrs
void ELSASpuren::import_vcorrs()
{
  int i, j, tmp;
  char filename[1024];
  string str;
  CORR_MS tmpA, tmpB, tmpC;
  fstream file;
 
  // Read corrector positions from "VCORRS.SPOS"
  snprintf(filename, 1024, "%s/correctors/VCORRS.SPOS", spurenFolder.c_str());
  file.open(filename, ios::in);
  if (!file.is_open()) {
    throw palatticeFileError(filename);
  }
  for (i=0; i<NVCORRS; i++) {
    file >> tmp >> vcorrs[i].pos;
  }
  file.close();
  
  //Read corrector kickangles [mrad] from "VCxx.KICK"
for (i=0; i<NVCORRS; i++) {
    snprintf(filename, 1024, "%s/correctors/VC%02d.KICK", spurenFolder.c_str(), i+1);
    file.open(filename, ios::in);
    if (!file.is_open()) {
      printf("! ELSASpuren::import_vcorrs(): Cannot open %s\n", filename);
      continue;
    }
    //read Headline
    getline(file, str);
    //read t[ms] from first line to detect "Schleppfehler"
    file >> tmpA.ms >> tmpA.kick;
    getline(file, str); //additional columns
    for (j=0; j<tmpA.ms; j++) {
      tmpB.ms = j;
      tmpB.kick=0.0;
      vcorrs[i].time.push_back(tmpB);
    }
    vcorrs[i].time.push_back(tmpA);
    //now continue normally
    while (!file.eof()) {
      file >> tmpB.ms >> tmpB.kick;
      getline(file, str); //additional columns
      for(j=tmpA.ms+1; j<tmpB.ms; j++) {
	tmpC.ms = j;
	//linear interpolation of kicks
	tmpC.kick = tmpA.kick + (tmpB.kick-tmpA.kick)/(tmpB.ms-tmpA.ms)*(j-tmpA.ms);
	vcorrs[i].time.push_back(tmpC);
      }
      vcorrs[i].time.push_back(tmpB);
      tmpA.ms = tmpB.ms;
      tmpA.kick = tmpB.kick;
    }
    file.close();
  }
  
}

