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


Metadata::Metadata()
{
  add("created at", timestamp());
  add("by libAccLattice version", gitversion());
}

//add other Metadata, without first 2 entries ("default" metadata)
void Metadata::operator+=(Metadata &other)
{
  for (unsigned int i=2; i<other.size(); i++)
    this->add(other.getLabel(i), other.getEntry(i));
}


/* add metadata from madx-outputfile. returns number of read values */
int Metadata::madximport(string madxLabels, string madxfile)
{
  unsigned int i, n=0;
  unsigned int newStart;
  string tmp, tmpEntry;
  fstream madx;
  vector<string> tmpLabel;

  madx.open(madxfile.c_str(), ios::in);
  if (!madx.is_open()) {
    cout << "ERROR: Metadata::madximport(): Cannot open " << madxfile << endl;
    return -1;
  }

  add("Imported from", "MAD-X");
  add("Lattice Source file", madxfile);

  //read Labels from madxLabels
  istringstream s(madxLabels);
  while(  std::getline(s, tmp, ',') ) {
    tmpLabel.push_back(tmp);
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
	add(tmpLabel[i],tmpEntry);
	n++;
      }
    }
  }

  madx.close();
  return n;
}



//  add metadata from elegant .param ascii file (written with subprocess command).
// returns number of read values
int Metadata::elegantimport(string elegantLabels, string elegantfile)
{
  unsigned int i, n=0;
  string tmp, tmpEntry;
  fstream elegant;
  vector<string> tmpLabel;

  elegant.open(elegantfile.c_str(), ios::in);
  if (!elegant.is_open()) {
    cout << "ERROR: Metadata::elegantimport(): Cannot open " << elegantfile << endl;
    return -1;
  }

  add("Imported from", "Elegant");
  add("Lattice Source file", elegantfile);

 //read Labels from madxLabels
  istringstream s(elegantLabels);
  while(  std::getline(s, tmp, ',') ) {
    tmpLabel.push_back(tmp);
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
