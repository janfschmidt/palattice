#ifndef __LIBPAL_RUNSIMTOOLS_HPP_
#define __LIBPAL_RUNSIMTOOLS_HPP_

#include <cstdlib>
#include <sstream>
#include <string>
#include "types.hpp"

using namespace std;

namespace pal
{
  string runMadX(string latticeFile);
  void runElegant(string latticeFile);
  void runSimTool(simulationTool t, string file);
}

#endif
/*__LIBPAL_RUNSIMTOOLS_HPP_*/
