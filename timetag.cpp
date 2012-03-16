/* class TIMETAG: time(s) of ELSA-cycle to read orbit&fields */
/* 16.03.2012 - J. Schmidt */

#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "timetag.hpp"

using namespace std;

/* constructor for list of times */
TIMETAG::TIMETAG(char *tagfile)
{
  string tmp;
  fstream file;

  file.open(tagfile, ios::in);
  if (!file.is_open()) {
    //!!!!! return -1;
  }

  while(!file.eof()) {
    file >> tmp;
    if (tmp=="#") {
      getline(file, tmp);
    }
    else {
      Tvec.push_back(atoi(tmp.c_str()));
    }
  }

  multi = true;
  file.close();
}


/* set single time (deletes previous entries!) */
void TIMETAG::set(int t)
{
  Tvec.clear();
  Tvec.push_back(t);
  multi = false;
}


/* set list of times (deletes previous entries!) */
void TIMETAG::set(char *tagfile)
{
  string tmp;
  fstream file;

  Tvec.clear();

  file.open(tagfile, ios::in);
  if (!file.is_open()) {
    //!!!!! return -1;
  }

  file >> tmp;
  while(!file.eof()) {
    if (tmp=="#") {
      getline(file, tmp);
    }
    else {
      Tvec.push_back(atoi(tmp.c_str()));
    }
    file >> tmp;
  }
  
  multi = true;
  file.close();
}


/* get entry i, returns -1 if no valid entry */
int TIMETAG::get(unsigned int i) const
{
  int t = -1;
  if (i<Tvec.size())
    t = Tvec[i];

  return t;
}


/* get tag e.g. for output filenames */
string TIMETAG::tag(unsigned int i) const
{
  stringstream tag;
  if (multi) {
    if (i<Tvec.size())
      tag << "_" <<setw(4)<<setfill('0')<< Tvec[i] << "ms";
    else
      tag << "notValid";
  }
  else 
    tag << "";

  return tag.str();
}


/* get label e.g. for metadata */
string TIMETAG::label(unsigned int i) const
{
 stringstream tag;
  if (i<Tvec.size())
    tag << Tvec[i] << " ms";
  else
    tag << "notValid";

  return tag.str();
}


