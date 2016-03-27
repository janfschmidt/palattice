#include "AccIterator.hpp"

using namespace pal;

AccLatticeIterator& AccLatticeIterator::next(element_type t)
{
  ++it;
  while (it->second->type != t)
    ++it;
  return *this;
}

AccLatticeIterator& AccLatticeIterator::previous(element_type t)
{
  --it;
  while (it->second->type != t)
    --it;
  return *this;
}

AccLatticeIterator& AccLatticeIterator::revolve()
{
  ++it;
  if (it == lattice.end())
    it = lattice.begin();
  return *this;
}
