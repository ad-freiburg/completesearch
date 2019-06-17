// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <vector>
#include <iostream>
#include <map>
#include "./FeatureVector.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::map;
using std::make_pair;
using std::cerr;

namespace ad_decompose
{
// _____________________________________________________________________________
void FeatureVector::addFeature(int fIndex, double value)
{
  static std::pair<map<int, double>::iterator, bool> ret;

  if (value != 0)
  {
    ret = _fv.insert(make_pair(fIndex, value));
    if (ret.second == false)
    {
      LOG(WARN) << "Feature vector already has an entry: " << fIndex
          << ", " << value << ".\n";
    }
  }
}

// _____________________________________________________________________________
map<int, double> const & FeatureVector::getFeatureVector() const
{
  return _fv;
}

// _____________________________________________________________________________
size_t FeatureVector::getSize() const
{
  return _fv.size();
}
}
