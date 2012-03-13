#ifndef __POLSIM__EXPORTFILE_HPP_
#define __POLSIM__EXPORTFILE_HPP_

#include "metadata.hpp"

int exportfile(SPECTRUM *bx, int fmax, METADATA metadata, string tag, char *filename);

int columnwidth(METADATA metadata);

string timestamp();

#endif

/*__POLSIM__EXPORTFILE_HPP_*/
