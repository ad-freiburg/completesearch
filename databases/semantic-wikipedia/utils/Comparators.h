// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_UTILS_COMPARATORS_H_
#define SEMANTIC_WIKIPEDIA_UTILS_COMPARATORS_H_

#include <string>
#include <vector>
#include "./Globals.h"
#include "./StringUtils.h"

using std::string;
using std::vector;

// General purpose comparators. Only comparators that are
// used or suited for use in more than one subfolder / topic
// are moved into this file.
namespace ad_semsearch
{
//! Comparator that is used to distribute word IDs.
//! Works just like the normal less than (<) operator
//! on std::string with the exception that words
//! starting with the ENTITY_PREFIX specified in Globals.h
//! are always considered greater than "normal" words.
class EntitiesLastLessThanStringComparator
{
  public:
    bool operator()(const string& first, const string& second) const
    {
      bool firstIsEntity = ad_utility::startsWith(first, ENTITY_PREFIX);
      bool secondIsEntity = ad_utility::startsWith(second, ENTITY_PREFIX);
      if (firstIsEntity == secondIsEntity)
      {
        // Either both are entities or none of them is.
        // Use normal comparison.
        return first < second;
      }
      else
      {
        // Otherwise the first is "less than" the second iff the
        // second is an entity, because we can be sure exactly of of them
        // is an entity.
        return secondIsEntity;
      }
    }
};
}
namespace ad_utility
{
class StringLengthIsLessComparator
{
  public:
    bool operator()(const string& first, const string& second) const
    {
      return first.size() < second.size();
    }
};

class StringLengthIsGreaterComparator
{
  public:
    bool operator()(const string& first, const string& second) const
    {
      return first.size() > second.size();
    }
};

class VectorSizeIsLessComparator
{
  public:
    template<class T>
    bool operator()(const vector<T>& first, const vector<T>& second) const
    {
      return first.size() < second.size();
    }
};

class VectorSizeIsGreaterComparator
{
  public:
    template<class T>
    bool operator()(const vector<T>& first, const vector<T>& second) const
    {
      return first.size() > second.size();
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_UTILS_COMPARATORS_H_
