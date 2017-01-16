/* Particle Accelerator Lattice library (libpalattice)
 *
 * Copyright (C) 2016 Jan Felix Schmidt <janschmidt@mailbox.org>
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __LIBPALATTICE_HPP_
#define __LIBPALATTICE_HPP_


// namespace pal

#include "AccLattice.hpp"    // particle accelerator lattice, including madx&elegant import/export

#include "AccElements.hpp"   // particle accelerator elements (e.g. dipole,quadrupole). used by AccLattice.

#include "AccIterator.hpp"   // special iterator to access and iterate AccLattice

#include "FunctionOfPos.hpp" // data of any type as a function of position (and turn) in a particle accelerator (e.g. orbit,trajectory,twiss). Interpolation and Spectrum (FFT) included.

#include "Spectrum.hpp"      // spectrum of any data, calculated by GSL FFT. used by FunctionOfPos.

#include "Field.hpp"         // magnetic field (3D) as a function of position (and turn). can be calculated from lattice and trajectory/orbit (FunctionOfPos).

#include "SimTools.hpp"      // run MadX or Elegant, read output files (SimToolTable) and manage their file names

#include "Interpolate.hpp"   // interpolateable data. wrapper for GSL interpolation. used by FunctionOfPos.

#include "Metadata.hpp"      // arbitrary meta-information (strings) as label/entry pairs with formatted output. used by AccLattice, FunctionOfPos and Spectrum.

#include "ELSASpuren.hpp"    // data structure for Closed Orbit and vert. Corrector Kicks measurement of one ELSA cycle from ELSA Control System. import included.

#include "types.hpp"         // type definitions. e.g. accelerator coordinates: 2D "Pair" (x,z) & 3D "Triple" (x,z,s)


#endif
/*__LIBPALATTICE_HPP_*/
