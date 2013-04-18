#ifndef __POLSIM__EXPORTFILE_HPP_
#define __POLSIM__EXPORTFILE_HPP_

#include "metadata.hpp"
#include "spectrum.hpp"

int exportfile(SPECTRUM &bx, METADATA metadata, string tag, const char *filename);

int columnwidth(METADATA metadata);

string timestamp();

#endif

/*__POLSIM__EXPORTFILE_HPP_*/
