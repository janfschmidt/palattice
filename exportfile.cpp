/* Create export file of magnetic field spectrum as readin for Polarization simulation (Olli) */
/* 27.01.2012 - J.Schmidt */

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

using namespace std;



int exportfile(SPECTRUM *bx, int fmax, METADATA metadata, char *outputFolder, string tag, bool timetaged)
{
  int i;
  char filename[1024];
  fstream file;
  const int w = 15;     /* column width in spectrum data  */
  char tmp[10];

  // add tag to metadata
  metadata.add("Field Component", tag);
  // add fmax to metadata
  snprintf(tmp, 10, "%d", fmax);
  metadata.add("max. frequency", tmp);

  /* set column width for metadata  */
  int metaw = columnwidth(metadata);

  

  /* create&open file */
  if (timetaged) {
    snprintf(filename, 1024, "%s/%s_%s.spectrum", outputFolder, timestamp().c_str(), tag.c_str());
  }
  else {
    snprintf(filename, 1024, "%s/%s.spectrum", outputFolder, tag.c_str());
  }
  file.open(filename, ios::out);
  if (!file.is_open()) {
    cout << "ERROR: Cannot open " << filename << "." << endl;
    return 1;
  }
  
  /* write metadata */ 
  for (i=0; i<metadata.size(); i++) {
    file << setiosflags(ios::left);
    file <<setw(2)<<"#";
    file <<setw(metaw)<< metadata.getLabel(i) << metadata.getEntry(i) <<endl;
  }
  file << endl;

  /* write table with spectrum data */
  file << resetiosflags(ios::left);
  file <<"#"<<setw(w+1)<<"Freq[Hz]"<<setw(w)<<"Amp[1/m]"<<setw(w)<<"Phase[deg]" << endl; 
  for (i=0; i<=fmax; i++) {
    file <<resetiosflags(ios::fixed)<<setiosflags(ios::scientific)<<showpoint<<setprecision(6);
    file <<setw(2+w)<< bx[i].omega/(2*M_PI);
    file <<setw(w)<< bx[i].amp;
    file <<resetiosflags(ios::scientific)<<setiosflags(ios::fixed)<<setprecision(1);
    file <<setw(w)  << bx[i].phase * 360/(2*M_PI) << endl;
  }

  file.close();
  cout << "Wrote " << filename  << endl;

  return 0;
}




/* set column width to maximum label-length + 2 */
int columnwidth(METADATA metadata)
{
  int i;
  unsigned int width=0;

  for (i=0; i<metadata.size(); i++) {
    if (metadata.getLabel(i).length() > width) {
      width = metadata.getLabel(i).length();
    }
  }

  return width + 2;
}



/* get date&time */
string timestamp()
{
  time_t rawtime;
  struct tm *t;
  char timestamp[20];

  time(&rawtime);
  t = gmtime(&rawtime);
  if (t->tm_isdst < 0) t->tm_hour = +1; /* UTC -> ME(S)Z */
  else t->tm_hour += 1 + t->tm_isdst;
  snprintf(timestamp, 20, "%4d-%02d-%02d-%02d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

  return timestamp;
}
