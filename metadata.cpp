/* class METADATA: supply all additional "meta"information for exportfiles */
/* 29.03.2012 - J. Schmidt */

#include <ctime>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "metadata.hpp"

using namespace std;


/* constructor with entries for project, mode & spuren */
METADATA::METADATA(string path, bool elsa, bool diff, char *spuren, char *Reference)
{
  METADATA::add("created at", timestamp());
  METADATA::add("Project path", path.c_str());

  if (elsa) {
    if (diff) {
      METADATA::add("Program Mode", "elsa + difference");
      METADATA::add("Referenz-Spuren", Reference);
    }
    else {
      METADATA::add("Program Mode", "elsa");
    }
    METADATA::add("Spuren", spuren);
  }
  else {
    if (diff) {
      METADATA::add("Program Mode", "madx + difference");
      METADATA::add("Referenz-MadXfile", Reference);
    }
    else {
      METADATA::add("Program Mode", "madx");
    }
  }  
}


/* add metadata from madx-outputfile. returns number of read values */
int METADATA::madximport(char *madxLabels, const char *madxfile)
{
  unsigned int i, n=0;
  unsigned int newStart;
  string tmp, tmpEntry;
  fstream madx;
  char *p;
  vector<string> tmpLabel;

  madx.open(madxfile, ios::in);
  if (!madx.is_open()) {
    cout << "ERROR: metadata.madximport(): Cannot open " << madxfile << endl;
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



//  add metadata from elegant .param ascii file (written with subprocess command).
// returns number of read values
int METADATA::elegantimport(char *elegantLabels, const char *elegantfile)
{
  unsigned int i, n=0;
  string tmp, tmpEntry;
  fstream elegant;
  char *p;
  vector<string> tmpLabel;

  elegant.open(elegantfile, ios::in);
  if (!elegant.is_open()) {
    cout << "ERROR: metadata.elegantimport(): Cannot open " << elegantfile << endl;
    return -1;
  }

  //read Labels from elegantLabels
  p = strtok(elegantLabels, ", ");
  while(p != NULL) {
    tmp=p;
    tmpLabel.push_back(tmp);
    p = strtok(NULL, ", ");
  }

  //read corresponding entries 
  while(!elegant.eof()) {
    elegant >> tmp;
    for (i=0; i<tmpLabel.size(); i++) {
      if(tmp==tmpLabel[i]) {
	elegant >> tmpEntry;
	//add to metadata
	label.push_back(tmpLabel[i]);
	entry.push_back(tmpEntry);
	n++;
      }
    }
    if (tmp == "***") break; //end of header
  }

  elegant.close();
  return n;
}


/* add one line manually */
void METADATA::add(string inLabel, string inEntry)
{
  label.push_back(inLabel);
  entry.push_back(inEntry);
}


/* get number of entries. (-1) if not consistent */
unsigned int METADATA::size() const
{
  unsigned int equalSize;
  if ((equalSize=label.size()) != entry.size()) {
    return 1;
  }

  return equalSize;
}

/* get i-th value */
string METADATA::getLabel(unsigned int i) const
{
  string tmpLabel="no valid label";
  if (i < METADATA::size())
    tmpLabel = label[i];

  return tmpLabel;
}

string METADATA::getEntry(unsigned int i) const
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
  t = localtime(&rawtime); //Sommerzeit geht, Winterzeit?
  //if (t->tm_isdst < 0) t->tm_hour = +1; /* UTC -> ME(S)Z */
  //else t->tm_hour += 1 + t->tm_isdst;
  snprintf(timestamp, 20, "%02d.%02d.%4d %02d:%02d:%02d", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec);

  return timestamp;
}



// formated output of all metadata to be written to a file.
// Additional metadata for Spectrum spec are added to output.
string METADATA::get(Spectrum spec, string tag) const
{
  METADATA tmpMeta = *this; // Spectrum metadata should not be added permanently
  char tmp[30];

  // add tag to metadata
  if (!tag.empty())
    tmpMeta.add("Field Component", tag);
  // add fmax to metadata
  snprintf(tmp, 30, "%d", spec.fMax());
  tmpMeta.add("max. frequency", tmp);
  // add ampcut to metadata
  snprintf(tmp, 30, "%.2e (%d cutted)", spec.getAmpcut(), spec.fMax()+1-spec.size());
  tmpMeta.add("cutted Amp <", tmp);
  // add phase-warning for harmcorr
  if (tag=="harmcorr" || tag=="resonances") {
    tmpMeta.add("WARNING:", "Phase NOT equal harmcorr in ELSA-CCS! (sign in cos)");
  }

  unsigned int w = tmpMeta.columnwidth();

  //write metadata to string
  ostringstream out;
  for (unsigned int i=0; i<tmpMeta.size(); i++) {
    out << setiosflags(ios::left);
    out <<setw(2)<<"#";
    out <<setw(w)<< tmpMeta.getLabel(i) << tmpMeta.getEntry(i) <<endl;
  }

  return out.str();
}



// set column width to maximum label-length + 2
unsigned int METADATA::columnwidth() const
{
  unsigned  int i;
  unsigned int width=0;

  for (i=0; i<size(); i++) {
    if (getLabel(i).length() > width) {
      width = getLabel(i).length();
    }
  }

  return width + 2;
}
