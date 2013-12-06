/* == AccElements Classes ===
 * Elements of an Accelerator (like magnets).
 * used by the "AccLattice" Container
 * by Jan Schmidt <schmidt@physik.uni-bonn.de>
 */

#include "AccElements.hpp"

// static member definition
AccPair AccElement::zeroPair;
AccTriple AccElement::zeroTriple;



string AccElement::type_string() const
{
  switch (type) {
  case dipole:
    return "Dipole";
  case quadrupole:
    return "Quadrupole";
  case corrector:
    return "Corrector";
  case sextupole:
    return "Sextupole";
  case drift:
    return "Drift";
  }

  return "Please implement this type in AccElement::type_string()!";
}
