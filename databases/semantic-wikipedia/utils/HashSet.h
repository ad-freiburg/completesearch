// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_UTILS_HASHSET_H_
#define SEMANTIC_WIKIPEDIA_UTILS_HASHSET_H_

#include <google/dense_hash_set>
#include <string>
#include "./DefaultKeyProvider.h"

using std::string;

namespace ad_utility
{
//! Wrapper for HashSets to be used everywhere throughout code for
//! the semantic search. This wrapper interface is not designed to
//! be complete from the beginning. Feel free to extend it at need.
//! The first version as of May 2011 uses google's dense_hash_set.
//! Backing-up implementations may be changed in the future.
template<class T>
class HashSet
{
  public:
    HashSet()
    {
      _impl.set_empty_key(DefaultKeyProvider<T>::DEFAULT_EMPTY_KEY);
      _impl.set_deleted_key(DefaultKeyProvider<T>::DEFAULT_DELETED_KEY);
    }

    typedef typename google::dense_hash_set<T>::iterator iterator;

    void set_empty_key(const T& emptyKey)
    {
      _impl.set_empty_key(emptyKey);
    }

    void set_deleted_key(const T& emptyKey)
    {
      _impl.set_deleted_key(emptyKey);
    }

    iterator begin() const
    {
      return _impl.begin();
    }

    iterator end() const
    {
      return _impl.end();
    }

    void insert(const T& value)
    {
      _impl.insert(value);
    }

    size_t count(const T& value) const
    {
      return _impl.count(value);
    }

    size_t size() const
    {
      return _impl.size();
    }

    void clear()
    {
      _impl.clear();
    }

  private:
    google::dense_hash_set<T> _impl;
};
}
#endif  // SEMANTIC_WIKIPEDIA_UTILS_HASHSET_H_
