#include "AccIterator.hpp"

using namespace pal;

void AccLatticeIterator::checkForEnd() const
{
  if (isEnd())
    throw palatticeError("Evaluation of lattice.end(), which is after last Element!");
}


// accessors

// position in Lattice in meter
double AccLatticeIterator::pos() const {
  checkForEnd(); 
  return it->first;
}




// position calculations

double AccLatticeIterator::pos(Anchor anchor) const
{
  checkForEnd();

  if (anchor==*latticeRefPos)
    return it->first;

  double pos = it->first;
  double l = it->second->length;
  if (*latticeRefPos == Anchor::center) pos -= l/2;
  else if (*latticeRefPos == Anchor::end) pos -= l;

  switch(anchor) {
  case Anchor::begin:
    return pos;
  case Anchor::center:
    return pos + l/2;
  case Anchor::end:
    return pos + l;
  }

  return 0.;
}

bool AccLatticeIterator::at(double pos) const
{
  if (pos>=begin() && pos<=end())
    return true;
  else
    return false;
}

// distance from anchor of element to pos (>0 if pos is after anchor)
double AccLatticeIterator::distance(Anchor anchor, double pos) const
{
  return pos - this->pos(anchor);
}

// both directions are checked, shorter distance is returned.
double AccLatticeIterator::distanceRing(Anchor anchor, double pos) const
{
  double d_normal = distance(anchor,pos);
  double d_other = *latticeCircumference - abs(d_normal);
  if ( d_other >= 0.5 * *latticeCircumference ) // regular direction shorter
    return d_normal;
  else {
    if (d_normal>0)
      d_other *= -1;
    return d_other;
  }
}

// |distance| to next element (anchor to anchor, circulating)
double AccLatticeIterator::distanceNext(Anchor anchor) const
{
  auto nextIt = *this;
  nextIt.revolve();
  double d = distance(anchor,nextIt.pos(anchor));
  if (d < 0)
    d += *latticeCircumference;
  return d;
}


