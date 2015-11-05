/* === Metadata Class ===
 * supplies arbitrary additional "meta"information for output files
 * as a nicely formatted file header
 *
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 *
 * This is unpublished software. Please do not copy/distribute it without
 * prior agreement of the author. Open Source publication coming soon :-)
 *
 * (c) Jan Schmidt <schmidt@physik.uni-bonn.de>, 2015
 */


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
  add("by libpalattice version", gitversion());
}

//add other Metadata, without first 2 entries ("default" metadata)
void Metadata::operator+=(const Metadata &other)
{
  for (unsigned int i=2; i<other.size(); i++)
    this->add(other.getLabel(i), other.getEntry(i));
}



//add metadata from parameters in madx/elegant file
void Metadata::simToolImport(SimToolInstance &sim, string file, string labels)
{
  add("Imported from", sim.tool_string());
  add("Lattice Source file", sim.lattice());

  if (file == "default")
    file = sim.twiss();

  if (labels == "default") {
    if (sim.tool==madx)
      labels = "TITLE,ORIGIN,GAMMA,Q1,Q2";
    else if (sim.tool==elegant && !sim.sddsMode())
      labels = "pCentral/m_e*c,tune:Qx,tune:Qz";
    else if (sim.sddsMode())
      labels = "pCentral,nux,nuy";
  }

  //parse labels from input string
  string tmp;
  vector<string> tmpLabel;
  istringstream s(labels);
  while(  std::getline(s, tmp, ',') ) {
    tmpLabel.push_back(tmp);
  }
  
  //read parameters from file
  for (unsigned int i=0; i<tmpLabel.size(); i++) {
    tmp = sim.readParameter<string>(file, tmpLabel[i]);
    add(tmpLabel[i], tmp);
  }
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

double Metadata::getGamma(SimTool t) const
{
  std::string label;
  if (t==madx)
    label = "GAMMA";
  else if (t==elegant)
    label = "pCentral/m_e*c";

  return std::stod( getbyLabel(label) );
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
