#ifndef __POLSIM__EXPORTFILE_HPP_
#define __POLSIM__EXPORTFILE_HPP_

#include "metadata.hpp"

int exportfile(SPECTRUM *bx, int fmax, METADATA metadata, char *outputFolder, string tag, bool timetaged);

int columnwidth(METADATA metadata);

string timestamp();

#endif

/*__POLSIM__EXPORTFILE_HPP_*/
