/* class METADATA: supply all additional "meta"information for exportfiles */
/* 15.03.2012 - J. Schmidt */

#include <ctime>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "metadata.hpp"

using namespace std;


/* constructor with entries for project, mode & spuren */
METADATA::METADATA(char *path, bool elsa, bool diff, char *spuren, char *Ref_spuren)
{
  METADATA::add("created at", timestamp());
  METADATA::add("Project path", path);

  if (elsa) {
    if (diff) {
      METADATA::add("Program Mode", "elsa + difference");
      METADATA::add("Referenz-Spuren", Ref_spuren);
    }
    else {
      METADATA::add("Program Mode", "elsa");
    }
    METADATA::add("Spuren", spuren);
  }
  else {
    METADATA::add("Program Mode", "madx");
  }  
}


/* add metadata from madx-outputfile. returns number of read values */
int METADATA::madximport(char *madxLabels, char *madxfile)
{
  unsigned int i, n=0;
  unsigned int newStart;
  string tmp, tmpEntry;
  fstream madx;
  char *p;
  vector<string> tmpLabel;

  madx.open(madxfile, ios::in);
  if (!madx.is_open()) {
    return -1;
  }

  //read Labels from madxLabels
  p = strtok(madxLabels, ", ");
  while(p != NULL) {
    tmp=p;
    tmpLabel.push_back(tmp);
    p = strtok(NULL, ", ");
  }

  while(!madx.eof()) {
    madx >> tmp;

    for (i=0; i<tmpLabel.size(); i++) {
      if(tmp==tmpLabel[i]) {
	madx >> tmp;
	getline(madx, tmpEntry);
	//remove leading space in tmpEntry
	newStart = tmpEntry.find_first_not_of(" ");
	if (newStart != string::npos)
	  tmpEntry = tmpEntry.substr(newStart);	
	//add to metadata
	label.push_back(tmpLabel[i]);
	entry.push_back(tmpEntry);
	n++;
      }
    }

  }

  madx.close();
  return n;
}


/* add one line manually */
void METADATA::add(string inLabel, string inEntry)
{
  label.push_back(inLabel);
  entry.push_back(inEntry);
}


/* get number of entries. (-1) if not consistent */
int METADATA::size() const
{
  int equalSize;
  if ((equalSize=label.size()) != entry.size()) {
    return -1;
  }

  return equalSize;
}

/* get i-th value */
string METADATA::getLabel(int i) const
{
  string tmpLabel="no valid label";
  if (i < METADATA::size())
    tmpLabel = label[i];

  return tmpLabel;
}

string METADATA::getEntry(int i) const
{
  string tmpEntry="no valid entry";
  if (i < METADATA::size())
    tmpEntry = entry[i];

  return tmpEntry;
}

/* get entry for input-label. returns "NA" if input-label is not found */
string METADATA::getbyLabel(string inLabel) const
{
  unsigned int i;
  for(i=0; i<label.size(); i++) {
    if (label[i] == inLabel)
      return entry[i];
  }
  return "NA";
}


/* re-set entry with input-label. add entry if label not found */
void METADATA::setbyLabel(string inLabel, string inEntry)
{
  unsigned int i;
  for(i=0; i<label.size(); i++) {
    if (label[i] == inLabel) {
      entry[i] = inEntry;
      return;
    }
  }
  METADATA::add(inLabel, inEntry);
}


/* get date&time */
string METADATA::timestamp() const
{
  time_t rawtime;
  struct tm *t;
  char timestamp[20];

  time(&rawtime);
  t = gmtime(&rawtime);
  if (t->tm_isdst < 0) t->tm_hour = +1; /* UTC -> ME(S)Z */
  else t->tm_hour += 1 + t->tm_isdst;
  snprintf(timestamp, 20, "%02d.%02d.%4d %02d:%02d:%02d", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec);

  return timestamp;
}
