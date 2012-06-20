/* Manage filenames for Bsupply input/output */
/* 19.06.2012 - J. Schmidt */

#include <string>
#include "filenames.hpp"

//constructor
FILENAMES::FILENAMES(string path, bool elsa, bool diff, string spurenIn, string refIn)
{
  //===========default filenames==========
  s_import = "/madx/madx.twiss";
  s_misalign_dip = "/madx/dipols.ealign";
  s_import_ref = "/madx_ref.twiss";
  //======================================
  project = path;
  spurenpath = path+"/ELSA-Spuren/"+spurenIn;

  if (elsa)
    referencepath = project+"/ELSA-Spuren/"+refIn;
  else
    referencepath = project+"/madx/"+refIn;

  if (diff)
    difftag = "_diff";
  else
    difftag = "";
}

