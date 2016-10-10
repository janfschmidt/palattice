/* "Function of Position" Class
 * contain any value(pos) for an accelerator ring, e.g. Twiss-, orbit- or field-data.
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
 *
 *
 * !!! Convention:
 * !!! first "turn"   = 1
 * !!! first "sample" = 0
 */

#include <string>
#include <sstream>
#include "FunctionOfPos.hpp"

using namespace std;
using namespace pal;

//AccAxis string output
string pal::axis_string(AccAxis a)
{
  switch (a) {
  case x:
    return "horizontal";
  case z:
    return "vertical";
  case s:
    return "longitudinal";
  }
  return "Please implement this AccAxis in axis_string() in FunctionOfPos.cpp.";
}


// =========== template specialization ============

//double
template <>
vector<double> FunctionOfPos<double>::getVector(double stepwidth,AccAxis) const
{
  // axis not needed, if only 1D values exist.
  vector<double> out;
  
  //data values, no interpolation
  if (stepwidth == 0.) {
    for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
      out.push_back( it->second );
  }
  //interpolation: equidistant data values
  else {
    for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
      out.push_back( this->interp(pos) );
  }
   return out;
}

//int
template <>
vector<double> FunctionOfPos<int>::getVector(double stepwidth,AccAxis) const
{ 
  // axis not needed, if only 1D values exist.
  vector<double> out;
  
  //data values, no interpolation
  if (stepwidth == 0.) {
    for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
      out.push_back( double(it->second) );
  }
  //interpolation: equidistant data values
  else {
    for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
      out.push_back( double(this->interp(pos)) );
  }
   return out;
 
}

//AccPair
template <>
vector<double> FunctionOfPos<AccPair>::getVector(double stepwidth,AccAxis axis) const
{
  vector<double> out;

  switch(axis) {
  case s:
    throw invalid_argument("FunctionOfPos<AccPair>::getVector(): s coordinate is not defined for AccPair. Use AccTriple instead.");
    break;
  case x:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.x );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
	out.push_back( this->interp(pos).x );
    }
    break;
  case z:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.z );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
	out.push_back( this->interp(pos).z );
    }
    break;
  }

  return out;
}

//AccTriple
template <>
vector<double> FunctionOfPos<AccTriple>::getVector(double stepwidth,AccAxis axis) const
{
  vector<double> out;

  switch(axis) {
  case s:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.s );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
	out.push_back( this->interp(pos).s );
    }
    break;
  case x:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.x );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
	out.push_back( this->interp(pos).x );
    }
    break;
  case z:
    if (stepwidth == 0.) {     //data values, no interpolation
      for (const_FoPiterator it=data.begin(); it!=data.end(); it++)
	out.push_back( it->second.z );
    }
    else {                     //interpolation: equidistant data values
      for (double pos=0; pos<turns()*circumference(); pos+=stepwidth)
	out.push_back( this->interp(pos).z );
    }
    break;
  }
  
  return out;
}







//import closed orbit from ELSA BPM-measurement at time t/ms
template <>
void FunctionOfPos<AccPair>::elsaClosedOrbit(ELSASpuren &spuren, unsigned int t)
{
  int i;
  char msg[1024];
  AccPair otmp;

  this->clear(); //delete old-BPM-data
  
  for (i=0; i<NBPMS; i++) {
    if (t > spuren.bpms[i].time.size()) {
      snprintf(msg, 1024, "ERROR: FunctionOfPos::elsaClosedOrbit: No ELSA BPM%02d data available for %d ms.\n", i+1, t);
      throw palatticeError(msg);
    }

    otmp.x = spuren.bpms[i].time[t].x / 1000; // unit mm -> m
    otmp.z = spuren.bpms[i].time[t].z / 1000;
    this->set(otmp, spuren.bpms[i].pos);
  }

 //metadata
 stringstream spureninfo;
 spureninfo << t << "ms in "<<spuren.spurenFolder;
 info.add("ELSA Closed Orbit from", spureninfo.str());
}



//import closed orbit from madx twiss file or elegant .clo file
template<>
void FunctionOfPos<AccPair>::simToolClosedOrbit(SimToolInstance &s)
{
  if (s.tool==pal::madx)
    readSimToolColumn(s,s.orbit(),"S","X","Y");
  else if (s.tool==pal::elegant)
    readSimToolColumn(s,s.orbit(),"s","x","y");
  
  //metadata
  // info.add("Closed Orbit from", s.tool_string());
  // info.add("Orbit Source file", orbitFile);
  
  //init interpolation (needed here for objects passed as const)
  if (!this->ready) {
    this->init();
  }

  //stdout info
    cout  << "* "<<size()<<" orbit sampling points read"<<endl
	  <<"  from "<<s.orbit() << endl;
}



template<>
void FunctionOfPos<AccPair>::simToolTrajectory(SimToolInstance &s, unsigned int particle)
{
  cout << "Initializing trajectory... " << endl;
  
  if (s.tool==pal::madx)
    readSimToolParticleColumn(s,particle,"X","Y");
  else if (s.tool==pal::elegant)
    readSimToolParticleColumn(s,particle,"x","y");

  cout << "* Trajectory of particle "<<particle<<" read at "<<samplesInTurn(1)
       <<" observation points for "<<turns()<<" turns"<<endl;
}

