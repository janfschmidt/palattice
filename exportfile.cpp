/* Create export file of magnetic field spectrum as readin for Polarization simulation (Olli) */
/* 29.03.2012 - J.Schmidt */

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <ctime>
#include "types.hpp"
#include "constants.hpp"
#include "exportfile.hpp"
#include "metadata.hpp"
#include "spectrum.hpp"

using namespace std;



int exportfile(SPECTRUM &bx, METADATA metadata, string tag, const char *filename)
{
  unsigned int i;
  fstream file;
  const int w = 15;     /* column width in spectrum data  */
  char tmp[10];

  // add tag to metadata
  metadata.add("Field Component", tag);
  // add fmax to metadata
  snprintf(tmp, 10, "%d", bx.fmax);
  metadata.add("max. frequency", tmp);
  // add phase-warning for harmcorr
  if (tag=="harmcorr" || tag=="resonances") {
    metadata.add("WARNING:", "Phase NOT equal harmcorr in ELSA-CCS! (sign in cos)");
  }


  // set column width for metadata
  int metaw = columnwidth(metadata);

  
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: Cannot open " << filename << "." << endl;
    return 1;
  }
  
  // write metadata
  for (i=0; i<metadata.size(); i++) {
    file << setiosflags(ios::left);
    file <<setw(2)<<"#";
    file <<setw(metaw)<< metadata.getLabel(i) << metadata.getEntry(i) <<endl;
  }
  file << endl;

  // write table with spectrum data
  file << resetiosflags(ios::left);
  if (tag=="harmcorr" || tag=="resonances")
    file <<"#"<<setw(w+1)<<"Freq[rev.harm.]"<<setw(w)<<"Amp[mrad]"<<setw(w)<<"Phase[deg]" << endl;
  else
    file <<"#"<<setw(w+1)<<"Freq[Hz]"<<setw(w)<<"Amp[1/m]"<<setw(w)<<"Phase[deg]" << endl; 
  for (i=0; i<=bx.fmax; i++) {
    file <<resetiosflags(ios::fixed)<<setiosflags(ios::scientific)<<showpoint<<setprecision(6);
    if (tag=="harmcorr")
      file <<setw(2+w)<< i;
    else
      file <<setw(2+w)<< bx.freq(i);
    file <<setw(w)<< bx.amp(i);
    file <<resetiosflags(ios::scientific)<<setiosflags(ios::fixed)<<setprecision(1);
    file <<setw(w)  << bx.phase(i) * 360/(2*M_PI) << endl;
  }

  file.close();
  cout << "* Wrote " << filename  << endl;

  return 0;
}




/* set column width to maximum label-length + 2 */
int columnwidth(METADATA metadata)
{
 unsigned  int i;
  unsigned int width=0;

  for (i=0; i<metadata.size(); i++) {
    if (metadata.getLabel(i).length() > width) {
      width = metadata.getLabel(i).length();
    }
  }

  return width + 2;
}



/* get date&time, can be used for filenames (set in main.cpp) */
string timestamp()
{
  time_t rawtime;
  struct tm *t;
  char timestamp[20];

  time(&rawtime);
  t = localtime(&rawtime); //Sommerzeit geht, Winterzeit?
  //if (t->tm_isdst < 0) t->tm_hour = +1; /* UTC -> ME(S)Z */
  //else t->tm_hour += 1 + t->tm_isdst;
  snprintf(timestamp, 20, "%4d-%02d-%02d-%02d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return timestamp;
}
