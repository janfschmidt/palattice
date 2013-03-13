/* Manage filenames for Bsupply input/output */
/* 21.11.2012 - J. Schmidt */

#include <string>
#include <stdio.h>
#include "filenames.hpp"


//constructor
FILENAMES::FILENAMES(string pathIn, bool elsa, bool diff, bool sgt_access, string spurenIn, string refIn)
{
  //======default filenames (private)=====
  file_import = "/madx/madx.twiss";
  file_misalign_dip = "/madx/dipols.ealign";
  file_import_ref = "/madx_ref.twiss";
  //======================================


  //tags (public):
  if (diff)
    difftag = "_diff";
  else
    difftag = "";


  //public filenames (including path):
  path = pathIn;  //project-path
  import = path + file_import;
  misalign_dip = path + file_misalign_dip;
  if (sgt_access)
    spuren = "/sgt/elsa/data/bpm/"+spurenIn;
  else
    spuren = path+"/ELSA-Spuren/"+spurenIn;
  if (elsa)
    if (sgt_access)
      ref = "/sgt/elsa/data/bpm/"+refIn;
    else
      ref = path+"/ELSA-Spuren/"+refIn;
  else
    ref = path+"/madx/"+refIn;
  
}


string FILENAMES::tracking(unsigned int obs, unsigned int p) const
{
  char tmp[512];
  string out;
  sprintf(tmp, "%s/madx/madx.obs%04d.p%04d", path.c_str(), obs, p);
  out = tmp;

  return out;
}
