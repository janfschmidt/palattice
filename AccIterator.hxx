template<bool IS_CONST>
void AccLatticeIterator_Base<IS_CONST>::checkForEnd() const
{
  if (isEnd())
    throw palatticeError("Evaluation of lattice.end(), which is after last Element!");
}



// iteration helper functions
template<bool IS_CONST>
void AccLatticeIterator_Base<IS_CONST>::next_helper(element_type t, element_plane p, element_family f)
{
  ++it;
  for (; it!=latticeElements->end(); ++it) {
    if ( it->second->type==t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f) )
      break;
  }
  //throw noMatchingElement("type, plane, family");
}

template<bool IS_CONST>
void AccLatticeIterator_Base<IS_CONST>::prev_helper(element_type t, element_plane p, element_family f)
{
  --it;
  for (; it!=latticeElements->begin(); --it) {
    if ( it->second->type==t && (p==noplane || it->second->plane==p) && (f==nofamily || it->second->family==f) )
      break;
  }
  //throw noMatchingElement("type, plane, family");
}

template<bool IS_CONST>
void AccLatticeIterator_Base<IS_CONST>::revolve_helper()
{
  if (it == latticeElements->end()) {
    setBegin();
  }
}




// position calculations
template<bool IS_CONST>
double AccLatticeIterator_Base<IS_CONST>::pos(Anchor anchor) const
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

template<bool IS_CONST>
bool AccLatticeIterator_Base<IS_CONST>::at(double pos) const
{
  if (pos>=begin() && pos<=end())
    return true;
  else
    return false;
}

// distance from anchor of element to pos (>0 if pos is after anchor)
template<bool IS_CONST>
double AccLatticeIterator_Base<IS_CONST>::distance(Anchor anchor, double pos) const
{
  return pos - this->pos(anchor);
}

// both directions are checked, shorter distance is returned.
template<bool IS_CONST>
double AccLatticeIterator_Base<IS_CONST>::distanceRing(Anchor anchor, double pos) const
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

// // |distance| to next element (anchor to anchor, circulating)
// template<bool IS_CONST>
// double AccLatticeIterator_Base<IS_CONST>::distanceNext(Anchor anchor) const
// {
//   auto nextIt = *this;
//   nextIt.revolve();
//   double d = distance(anchor,nextIt.pos(anchor));
//   if (d < 0)
//     d += *latticeCircumference;
//   return d;
// }


