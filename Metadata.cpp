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
#include "Metadata.hpp"

using namespace std;
using namespace pal;


Metadata::Metadata()
{
  add("created at", timestamp());
  add("by libpal version", libpalGitversion());
}

//add other Metadata, without first 2 entries ("default" metadata)
void Metadata::operator+=(Metadata &other)
{
  for (unsigned int i=2; i<other.size(); i++)
    this->add(other.getLabel(i), other.getEntry(i));
}



//add metadata from parameters in madx/elegant file
void Metadata::simToolImport(SimToolInstance &sim, string file, string labels)
{
  add("Imported from", sim.tool_string());
  add("Lattice Source file", sim.lattice());

  //read labels
  string tmp;
  vector<string> tmpLabel;
  istringstream s(labels);
  while(  std::getline(s, tmp, ',') ) {
    tmpLabel.push_back(tmp);
  }

  for (unsigned int i=0; i<tmpLabel.size(); i++) {
    tmp = sim.readParameter<string>(file, tmpLabel[i]);
    add(tmpLabel[i], tmp);
  }
}




// add an entry. if label already exists, update entry
void Metadata::add(string inLabel, string inEntry)
{
  //change existing entry
  for(unsigned int i=0; i<label.size(); i++) {
    if (label[i] == inLabel) {
      entry[i] = inEntry;
      return;
    }
  }
  //add new entry
  label.push_back(inLabel);
  entry.push_back(inEntry);
}



/* get i-th value */
string Metadata::getLabel(unsigned int i) const
{
  string tmpLabel="no valid label";
  if (i < Metadata::size())
    tmpLabel = label[i];

  return tmpLabel;
}

string Metadata::getEntry(unsigned int i) const
{
  string tmpEntry="no valid entry";
  if (i < Metadata::size())
    tmpEntry = entry[i];

  return tmpEntry;
}

/* get entry for input-label. returns "NA" if input-label is not found */
string Metadata::getbyLabel(string inLabel) const
{
  unsigned int i;
  for(i=0; i<label.size(); i++) {
    if (label[i] == inLabel)
      return entry[i];
  }
  return "NA";
}




// formated output of all metadata to be written to a file.
string Metadata::out(string delimiter) const
{
  unsigned int w = this->columnwidth();
  unsigned int w_del = delimiter.length();

  //write metadata to string
  ostringstream out;
  for (unsigned int i=0; i<this->size(); i++) {
    out << setiosflags(ios::left);
    out <<setw(w_del+1)<< delimiter;
    out <<setw(w)<< this->getLabel(i) << this->getEntry(i) <<endl;
  }

  return out.str();
}



// set column width to maximum label-length + 2
unsigned int Metadata::columnwidth() const
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




/* get date&time */
string Metadata::timestamp() const
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
