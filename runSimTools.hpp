#ifndef __LIBPAL_RUNSIMTOOLS_HPP_
#define __LIBPAL_RUNSIMTOOLS_HPP_

#include <cstdlib>
#include <sstream>
#include <string>
#include "types.hpp"

using namespace std;

namespace pal
{
  string runSimTool(simulationTool t, string latticeFile);
  string runMadX(string latticeFile);
  string runElegant(string latticeFile);
}

#endif
/*__LIBPAL_RUNSIMTOOLS_HPP_*/
