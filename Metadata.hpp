/* Metadata Class
 * supplies arbitrary additional "meta"information for output files as a nicely formatted file header
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

#ifndef __LIBPALATTICE__METADATA_HPP_
#define __LIBPALATTICE__METADATA_HPP_

#include <vector>
#include <string>
#include "SimTools.hpp"
#include "gitversion.hpp"

using namespace std;

namespace pal
{

class Metadata {
private:
  vector<string> label;
  vector<string> entry;

public:
  Metadata();
  ~Metadata() {}

  template <class T> void add(string inLabel, T inEntry);  //add an entry. if label already exists, update entry
  template <class T> void addStatistics(T mean, T rms);
  void simToolImport(SimToolInstance &sim, string file="default", string labels="default");
  void madximport(string madxFile, string labels, SimToolMode m=online) {SimToolInstance mad(pal::madx,m,madxFile); simToolImport(mad,madxFile,labels);}
  void elegantimport(string eleFile, string labels, SimToolMode m=online) {SimToolInstance ele(pal::elegant,m,eleFile); simToolImport(ele,eleFile,labels);}

  void operator+=(const Metadata &other);       //add other Metadata, without first 2 entries ("default" metadata)

  unsigned int size() const {return label.size();}
  string getLabel(unsigned int i) const;
  string getEntry(unsigned int i) const;
  string getbyLabel(string inLabel) const;

  string out(string delimiter) const; //formatted output, each line starting with delimiter
  unsigned int columnwidth() const;
  string timestamp() const;
};

} //namespace pal



template <class T>
void pal::Metadata::add(string inLabel, T inEntry)
{
  //get string from T
  std::stringstream sEntry;
  sEntry << inEntry;
  //change existing entry
  for(unsigned int i=0; i<label.size(); i++) {
    if (label[i] == inLabel) {
      entry[i] = sEntry.str();
      return;
    }
  }
  //add new entry
  label.push_back(inLabel);
  entry.push_back(sEntry.str());
}

template <class T>
void pal::Metadata::addStatistics(T mean, T stddev)
{
  add("mean", mean);
  add("standard deviation", stddev);
}


#endif
/*__LIBPALATTICE__METADATA_HPP_*/
