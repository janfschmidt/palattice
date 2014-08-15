/* Manage filenames for Bsupply input/output */
/* 21.11.2012 - J. Schmidt */

#include <string>
#include <stdio.h>
#include "filenames.hpp"


//constructor
FILENAMES::FILENAMES(string pathIn, simulationTool _simTool, bool elsa, bool diff, bool sgt_access, string spurenIn, string refIn)
  : simTool(_simTool)
{
  //======default filenames (private)=====
  if (simTool == madx) {
    file_lattice = "/madx/madx.twiss";
    file_orbit = "/madx/madx.twiss";
    file_misalign_dip = "/madx/dipols.ealign";
    file_misalign_dip_ref = "/madx/dipols_ref.ealign";
    file_lattice_ref = "/madx_ref.twiss";
    file_orbit_ref = "/madx_ref.twiss";
  }
  else {
    file_lattice = "/elegant/elegant.param";
    file_orbit = "/elegant/elegant.clo";
    file_misalign_dip = "/elegant/dipols.ealign";      //not needed for elegant
    file_misalign_dip_ref = "/madx/dipols_ref.ealign"; //not needed for elegant
    file_lattice_ref = "/elegant_ref.param";
    file_orbit_ref = "/elegant_ref.clo";
  }
  //======================================


  //tags (public):
  if (diff)
    difftag = "_diff";
  else
    difftag = "";


  //public filenames (including path):
  path = pathIn;  //project-path
  if (simTool == madx)
    path_simTool = path + "/madx/";
  else
    path_simTool = path + "/elegant/";
  lattice = path + file_lattice;
  orbit = path + file_orbit;
  misalign_dip = path + file_misalign_dip;
  misalign_dip_ref = path + file_misalign_dip_ref;

  if (sgt_access)
    spuren = "/sgt/elsa/data/bpm/"+spurenIn;
  else
    spuren = path+"/ELSA-Spuren/"+spurenIn;

  if (elsa) {
    if (sgt_access) {
      lattice_ref = "/sgt/elsa/data/bpm/"+refIn;
      orbit_ref = "/sgt/elsa/data/bpm/"+refIn;
    }
    else { // no sgt_access
      lattice_ref = path+"/ELSA-Spuren/"+refIn;
      orbit_ref = path+"/ELSA-Spuren/"+refIn;
    }
  }
  else { // not elsa
    if (simTool == madx) {
      orbit_ref = path+"/madx/"+refIn;
      lattice_ref = path+"/madx/"+refIn;
    }
    else {
      orbit_ref = path+"/elegant/"+refIn+".clo";
      lattice_ref = path+"/elegant/"+refIn+".param";
    }
  }
  
}


