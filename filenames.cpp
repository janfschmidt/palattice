/* Manage filenames for Bsupply input/output */
/* 21.11.2012 - J. Schmidt */

#include <string>
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
    ref = path+"/ELSA-Spuren/"+refIn;
  else
    ref = path+"/madx/"+refIn;
  
}

