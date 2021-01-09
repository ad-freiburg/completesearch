#ifndef CODEBASE_SERVER_VECTOR_H_
#define CODEBASE_SERVER_VECTOR_H_

// Chair of Algorithms and Data Structures
// Copyright 2010

// Base class for WordList and DocList

#include <gtest/gtest.h>
#include <assert.h>
#include <math.h>  // needed for log2() in CygWin
#include <algorithm>
#include <iostream>  // for cout
#include <vector>
#include <string>
#include <utility>
#include "./Globals.h"
#include "./Exception.h"

typedef unsigned long size_type;

using std::endl;
using std::sort;

enum SortOrderEnum
{
  SORT_ORDER_UNDEFINED  = 0,
  SORT_ORDER_ASCENDING  = 1,
  SORT_ORDER_DESCENDING = 2
};

// For sorting pairs in ascending order (second component breaks ties)
template <typename T, typename U> struct Pair_lt
{
  Pair_lt() {;}
  inline bool operator()(const pair<T, U>& a, const pair<T, U>& b) const
  {
    if (a.first < b.first) {return true;}
    else if (a.first == b.first) {return a.second < b.second;}
    else {return false;}
  }
};

// For sorting pairs in descending order (second component breaks ties, but
// ASCENDING)
template <typename T, typename U> struct Pair_gt
{
  Pair_gt() {;}
  inline bool operator()(const pair<T, U>& a, const pair<T, U>& b) const
  {
    if (a.first < b.first) {return false;}
    else if (a.first == b.first) {return (a.second < b.second);}
    else {return true;}
  }
};

// For sorting triples in ascending order (second component breaks ties)
template < typename T, typename V, typename U> struct Triple_lt
{
  Triple_lt() {;}
  inline bool operator()(const pair< pair<T, V> , U>& a,
                         const pair< pair<T, V>, U>& b) const
  {
    if (a.first.first < b.first.first)
      return true;
    else if (a.first.first == b.first.first)
      return a.first.second < b.first.second;
    else {return false;}
  }
};

// For sorting triples in descending order (second component breaks ties, but
// ASCENDING)
template < typename T, typename V, typename U> struct Triple_gt
{
  Triple_gt() {;}
  inline bool operator()(const pair< pair<T, V> , U>& a,
                         const pair< pair<T, V>, U>& b) const
  {
    if (a.first.first < b.first.first)
      return false;
    else if (a.first.first == b.first.first)
      return (a.first.second < b.first.second);
    else
      return true;
  }
};

// Class providing comparison for sorting quadruples in ascending order (second
// component breaks ties, ascending too).
template <typename T, typename V, typename U, typename W> class Quadruple_lt
{
 public:
  Quadruple_lt() { }
  inline bool operator()(const pair< pair<T, V> , pair <U, W> >& a,
                         const pair< pair<T, V>, pair<U, W> >& b) const
  {
    if (a.first.first < b.first.first) { return true; }
    else if (a.first.first == b.first.first)
      return a.first.second < b.first.second;
    else
      return false;
  }
};

// Class providing comparison for sorting quadruples in descending order (second
// component breaks ties, descending too).
template <typename T, typename V, typename U, typename W> class Quadruple_gt
{
 public:
  Quadruple_gt() {;}
  inline bool operator()(const pair< pair<T, V> , pair <U, W> >& a,
                         const pair< pair<T, V>, pair<U, W> >& b) const
  {
    if (a.first.first > b.first.first)
      return true;
    else if (a.first.first == b.first.first)
      return a.first.second > b.first.second;
    else
      return false;
  }
};


// static unsigned char vectorError;

#ifdef STL_VECTOR
template <class T> class Vector : public vector<T>
#else
template <class T> class Vector
#endif
{
 public:
  // Standard constructor.
  FRIEND_TEST(VectorTest, constructor);
  Vector();
  // Constructor.
  // element: Element to be repeated nofRepetitions times.
  // nofRepetitions: Number of repetitions. TODO(hoffmaje): should be size_t.
  Vector(const T& element, unsigned long nofRepetitions);
  // Destructor.
  ~Vector();

  // NEW(bast, 7Dec10): convert to string and parse from string. Useful for
  // testing.
  string asString() const;
  void parseFromString(const string& vectorAsString);

#ifdef STL_VECTOR
    size_t size() const { return vector<T>::size(); }
    size_t capacity() const { return vector<T>::capacity(); }
    void   resize(size_t n, T x = 0) { return vector<T>::resize(n, x); }
    void   reserve(size_t n) { return vector<T>::reserve(n); }
    const  T& operator[](size_t i) const { return vector<T>::operator[](i); }
           T& operator[](size_t i) { return vector<T>::operator[](i); }
#endif

    size_t sizeInBytes() const { return sizeof(T)*size(); }

#ifndef STL_VECTOR

 public:
    
    // Get entry at given position (as const)
    inline const T& operator[](size_type n) const
    {
      // actually, in some cases we do want n < _size
      assert(n < _capacity);
      return *(_data + n);
    }

    // Get entry at given position (as lvalue)
    inline T& operator[](size_type n)
    {
      // actually, in some cases we do want n < _size
      assert(n < _capacity);
      return *(_data + n);
    }

    // Get last element (as const)
    const T& back() const
    {
      assert(_size > 0);
      return _data[_size-1];
    }

    // Get last element (as lvalue)
    T& back()
    {
      assert(_size > 0);
      return _data[_size-1];
    }

    // Get number of elements
    size_type size() const
    {
      return _size;
    }

    // Get capacity = for how many elements has space been reserved
    size_type capacity() const
    {
      return _capacity;
    }


    // Append an element
    void push_back(const T& x)
    {
      if (_capacity < _size + 1)
      {
        reserve(2*(_capacity+1));
      }
      assert(_capacity >= _size + 1);
      _data[_size] = x;
      ++_size;
      assert(back() == x);
    }


    // Get last element and remove it
    void pop_back()
    {
      --_size;
    }


    // Reserve space for the given number of elements (does nothing if enough
    // space is already reserved)
    /*
     *  Note: The reserve does currently NOT do the initialization by memset
     */
    void reserve(size_type n);


    // Resize vector and set new elements to specified value (default = 0)
    void resize(size_type n, T inEl = 0);

#endif  // end: reimplementation of stl::vector


  public:
    // could move this to DocList
    bool _isFullList;

#ifndef STL_VECTOR

  private:

    size_type _size;
    size_type _capacity;
    T* _data;

#endif


  public:

    void unreserve()
    {
#ifndef STL_VECTOR
      reserve(size());
#else
      vector<T>(*this).swap(*this);
#endif
    }



#ifndef STL_VECTOR

    // Copy Constructor
    Vector(const Vector& orig);

    // Assignment operator
    Vector& operator= (const Vector& orig);

#endif
    // Given a set of indices, this method returns the subvector corresponding
    // to these indices.
    void slice(const Vector<unsigned long>& indices, Vector<T>& subset) const
    {
      assert(indices.isSorted(true));
      assert(indices.size() <= size());
      if (indices.size() != subset.size()) {subset.resize(indices.size());}
      assert(indices.size() == subset.size());
      assert(!_isFullList);
      assert(!subset._isFullList);
      if (subset.size() == 0) {return;}
      assert(indices[indices.size() -1] < size());
      assert(indices.isSorted(true));

      for (register unsigned long i = 0; i < indices.size(); i++)
      {
        subset[i] = Vector<T>::operator[](indices[i]);
      }
    }

    void indicesOfSubset(const Vector<T>& subset,
                         Vector<unsigned long>& indices) const
    {
      assert(size() >= subset.size());

      if (subset.size() != indices.size()) {indices.resize(subset.size());}
      assert(subset.size() == indices.size());
      const unsigned long subsetSize = subset.size();

      if (subsetSize == 0) {return;}

      assert(subset.size() > 0);
      assert(subset[0] >= Vector<T>::operator[](0));
      assert(subset[subset.size()-1] <= Vector<T>::operator[](size()-1));

      register unsigned long i = 0, j = 0;
      while (i < subsetSize)
      {
        assert(j < size());
        if (Vector<T>::operator[](j) < subset[i] )
        {
          ++j;
        }
        else
        {
          assert(subset[i] == Vector<T>::operator[](j));
          indices[i] = j;
          ++i;
          ++j;
        }
      }
      assert(indices.isSorted(true));
    }  // end: indicesOfSubset(..)

    explicit Vector(bool isFull)
    {
      _isFullList = isFull;
#ifndef STL_VECTOR
      _size = 0;
      _capacity = 0;
      _data = NULL;
#endif
    };


    bool isContiguous() const
    {
#ifndef STL_VECTOR
      return true;
#else
      assert(vector<T>::size()>0);
      T* array = (T*) &vector<T>::operator[](0);

      // actually checks one position more than this
      const unsigned short nofPositionsToCheck = 10;
      // const unsigned short nofPositionsToCheck = vector<T>::size() -1;

      for (unsigned short k = 0; k <= nofPositionsToCheck; k++)
      {
        if (array[k*(vector<T>::size()-1)/nofPositionsToCheck] !=
            vector<T>::operator[](k*(vector<T>::size()-1)/nofPositionsToCheck))
        { return false; }
      }
      return true;
#endif
    }  // end: isContiguous()



    void clear()
    {
#ifndef STL_VECTOR
      if (_data)
      {
        free(_data);
        _data = NULL;
      }
      _size = 0;
      _capacity = 0;
#else
      vector<T>::clear();
#endif
      _isFullList = false;
    }

    // Sort TWO vectors in parallel, in ascending order of first component
    /*!
     *   Used in the following places (as of 25Apr08):
     *   - in HYB index building,
     *   - in CompleterBase.TopK.cpp, for ranking words (case: small result)
     *   - in QueryResult.cpp, for sorting large results (more than one block)
     *   - in QueryResult.cpp, for merge join
     */
    template <typename U>
    void sortParallel(Vector<U>& parallelVector)
    {
      assert(size() == parallelVector.size());

      vector<pair<T, U> > pairedVector;
      pairedVector.resize(size());

      // build pairs of docid, wordid
      for (unsigned long i = 0; i < size(); i++)
      {
        pairedVector[i].first  = (*this)[i];
        pairedVector[i].second = parallelVector[i];
      }


      // sort in ascending order
      sort(pairedVector.begin(), pairedVector.end(), Pair_lt<T, U>());

      // split into two vectors again
      for (unsigned long i = 0; i < size(); i++)
      {
        (*this)[i]        = pairedVector[i].first;
        parallelVector[i] = pairedVector[i].second;
      }

      assert(isSorted());
    }



    // Sort THREE vectors in parallel, in ascending order of first component
    /*!
     *    See the two vectos sort, for where this is used
     */
    template <typename U, typename V>
    void sortParallel(Vector<V>& secondaryVector, Vector<U>& parallelVector)
    {
      assert(size() == secondaryVector.size());
      assert(size() == parallelVector.size());

      vector< pair< pair<T, V>, U > > tripledVector;
      tripledVector.resize(size());

      /* build (docid, position, wordid) triples */
      for (unsigned long i = 0; i < size(); i++)
      {
        tripledVector[i].first.first  = Vector<T>::operator[](i);
        tripledVector[i].first.second  = secondaryVector.operator[](i);
        tripledVector[i].second  = parallelVector.operator[](i);
      }

      // sort pairs by first key, then by secondaryKey
      sort(tripledVector.begin(), tripledVector.end(), Triple_lt<T, V, U>());

      /* build three vectors from vector of triples */
      for (unsigned long i = 0; i < size(); i++)
      {
        Vector<T>::operator[](i) = tripledVector[i].first.first;
        secondaryVector.operator[](i) = tripledVector[i].first.second;
        parallelVector.operator[](i) = tripledVector[i].second;
      }

      assert(isSorted());
    }



    // Sort FOUR vectors in parallel, in ascending order of first component
    /*!
     *    See the two vectos sort, for where this is used
     */
    template <typename U, typename V, typename W>
    void sortParallel(Vector<V>& secondaryVector,
                      Vector<U>& firstParallelVector,
                      Vector<W>& secondParallelVector)
    {
      assert(size() == secondaryVector.size());
      assert(size() == firstParallelVector.size());
      assert(size() == secondParallelVector.size());

      // a vector of pairs of pairs
      vector< pair< pair<T, V>, pair<U, W> > > quadrupledVector;
      quadrupledVector.resize(size());

      /* build (docid, position, wordid) triples */
      for (unsigned long i = 0; i < size(); i++)
      {
        quadrupledVector[i].first.first  = Vector<T>::operator[](i);
        quadrupledVector[i].first.second  = secondaryVector.operator[](i);
        quadrupledVector[i].second.first  = firstParallelVector.operator[](i);
        quadrupledVector[i].second.second  = secondParallelVector.operator[](i);
      }

      // sort pairs by first key, then by secondaryKey
      sort(quadrupledVector.begin(), quadrupledVector.end(),
           Quadruple_lt<T, V, U, W>());

      /* build three vectors from vector of triples */
      for (unsigned long i = 0; i < size(); i++)
      {
        Vector<T>::operator[](i) = quadrupledVector[i].first.first;
        secondaryVector.operator[](i) = quadrupledVector[i].first.second;
        firstParallelVector.operator[](i) = quadrupledVector[i].second.first;
        secondParallelVector.operator[](i) = quadrupledVector[i].second.second;
      }
      assert(isSorted());
    }



    // Partial sort TWO vectors. considering them as a vector of pairs
    /*!
     *    Note: The partial sorts are used for the top-k computation (and
     *    nowhere else, as of 25Apr08). The routine used to sort in *descending*
     *    order only.
     */
    template <typename U>
    void partialSortParallel(Vector<U>&    parallelVector,
                             unsigned int  k,
                             SortOrderEnum sortOrder)
    {
      assert(size() == parallelVector.size());
      if (k == 0) {k = size();}
      k = MIN(k, size());

      vector< pair< T, U > > pairedVector;
      pairedVector.resize(size());

      // build pairs of docid, wordid
      for (unsigned int i = 0; i < size(); i++)
      {
        pairedVector[i].first   = (*this)[i];
        pairedVector[i].second  = parallelVector[i];
      }

      // sort according to specified order
      if (sortOrder == SORT_ORDER_ASCENDING)
        partial_sort(pairedVector.begin(),
                     pairedVector.begin() + k,
                     pairedVector.end(),
                     Pair_lt<T, U>());
      else if (sortOrder == SORT_ORDER_DESCENDING)
        partial_sort(pairedVector.begin(),
                     pairedVector.begin() + k,
                     pairedVector.end(),
                     Pair_gt<T, U>());
      else
        CS_THROW(Exception::OTHER, "invalid sort order: " << sortOrder);
      // partial_sort(pairedVector.begin(),
      //             pairedVector.begin() + k,
      //             pairedVector.end(),
      //             Pair_gt<T, U>());

      pairedVector.resize(k);

      resize(k);
      parallelVector.resize(k);

      // split into two vectors again
      for (unsigned int i = 0; i < k; i++)
      {
        (*this)[i]        = pairedVector[i].first;
        parallelVector[i] = pairedVector[i].second;
      }

      // assert(isReverseSorted());
    }


    // Partial sort THREE vectors in parallel, considering them as a vector of
    // triples
    /*!
     *  Note: The partial sorts are used for the top-k computation (and
     *  nowhere else, as of 25Apr08). The routine used to sort in *descending*
     *  order only.
     */
    template <typename U, typename V>
    void partialSortParallel(Vector<V>&    secondaryVector,
                             Vector<U>&    parallelVector,
                             unsigned int  k,
                             SortOrderEnum sortOrder)
    {
      assert(size() == secondaryVector.size());
      assert(size() == parallelVector.size());
      if (k == 0) {k = size();}
      k = MIN(k, size());

      vector< pair< pair<T, V>, U > > tripledVector;
      tripledVector.resize(size());

      // build triples
      for (unsigned long i = 0; i < size(); i++)
      {
        tripledVector[i].first.first   = (*this)[i];
        tripledVector[i].first.second  = secondaryVector[i];
        tripledVector[i].second        = parallelVector[i];
      }

      // sort according to specified order
      if (sortOrder == SORT_ORDER_ASCENDING)
        partial_sort(tripledVector.begin(),
                     tripledVector.begin() + k,
                     tripledVector.end(),
                     Triple_lt<T, V, U>());
      else if (sortOrder == SORT_ORDER_DESCENDING)
        partial_sort(tripledVector.begin(),
                     tripledVector.begin() + k,
                     tripledVector.end(),
                     Triple_gt<T, V, U>());
      else
        CS_THROW(Exception::OTHER, "invalid sort order: " << sortOrder);
      // partial_sort(tripledVector.begin(),
      //             tripledVector.begin() + k,
      //             tripledVector.end(),
      //             Triple_gt<T, V, U>());

      tripledVector.resize(k);

      resize(k);
      secondaryVector.resize(k);
      parallelVector.resize(k);

      // split into three vectors again
      for (unsigned int i = 0; i < k; i++)
      {
        (*this)[i]         = tripledVector[i].first.first;
        secondaryVector[i] = tripledVector[i].first.second;
        parallelVector[i]  = tripledVector[i].second;
      }

      // assert(isReverseSorted());
    }


    // Partial sort FOUR vectors in parallel, consdering them as a vector of
    // 4-tuples
    /*!
     *  Note: The partial sorts are used for the top-k computation (and
     *  nowhere else, as of 25Apr08). The routine used to sort in *descending*
     *  order only.
     */
    template <typename U, typename V, typename W>
    void partialSortParallel(Vector<V>&    secondaryVector,
                             Vector<U>&    firstParallelVector,
                             Vector<W>&    secondParallelVector,
                             unsigned int  k,
                             SortOrderEnum sortOrder)
    {
      assert(size() == secondaryVector.size());
      assert(size() == firstParallelVector.size());
      assert(size() == secondParallelVector.size());
      if (k == 0) {k = size();}
      k = MIN(k, size());
      // A vector of pairs of pairs
      vector<pair<pair<T, V>, pair<U, W> > > quadrupledVector;
      quadrupledVector.resize(size());

      // build vector of quadruples
      for (unsigned long i = 0; i < size(); i++)
      {
        quadrupledVector[i].first.first   = (*this)[i];
        quadrupledVector[i].first.second  = secondaryVector[i];
        quadrupledVector[i].second.first  = firstParallelVector[i];
        quadrupledVector[i].second.second = secondParallelVector[i];
      }

      // sort according to specified order
      if (sortOrder == SORT_ORDER_ASCENDING)
        partial_sort(quadrupledVector.begin(),
                     quadrupledVector.begin() + k,
                     quadrupledVector.end(),
                     Quadruple_lt<T, V, U, W>());
      else if (sortOrder == SORT_ORDER_DESCENDING)
        partial_sort(quadrupledVector.begin(),
                     quadrupledVector.begin() + k,
                     quadrupledVector.end(),
                     Quadruple_gt<T, V, U, W>());
      else
        CS_THROW(Exception::OTHER, "invalid sort order: " << sortOrder);
      // partial_sort(quadrupledVector.begin(),
      //             quadrupledVector.begin()+k,
      //             quadrupledVector.end(),
      //             Quadruple_gt<T, V, U, W>());

      quadrupledVector.resize(k);

      resize(k);
      secondaryVector.resize(k);
      firstParallelVector.resize(k);
      secondParallelVector.resize(k);

      // split into four vectors again
      for (unsigned int i = 0; i < k; i++)
      {
        Vector<T>::operator[](i) = quadrupledVector[i].first.first;
        secondaryVector.operator[](i) = quadrupledVector[i].first.second;
        firstParallelVector.operator[](i) = quadrupledVector[i].second.first;
        secondParallelVector.operator[](i) = quadrupledVector[i].second.second;
      }

      // assert(isReverseSorted());
    }






    // Simply checks if all the elements are > 0. Also returns true for empty
    // vector.
    bool isPositive() const
    {
      for (unsigned long i = 0; i < size(); i++)
      {
        if (operator[](i) <=0) return false;
      }
      return true;
    }

    bool hasEvenSize() const
    {
      if (size()%2 == 0) { return true;}
      else
        return false;
    }

    bool checkPairedness() const
    {
      if (size() == 0) { return true;}
      if (!hasEvenSize()) { return false; }
      for (unsigned long i = 0; i < size(); i += 2)
      {
        if (operator[](i) != operator[](i+1))
        {
          return false;
        }
      }
      return true;
    }


    // currently this is only used by compress algorithms
    // repeated is never written to disk
    // if gaps is true then the 1, 4, 9... in Vector gives:
    //   1, 3, 5, ... in data
    // gaps == 0 => no gaps,
    // gaps == 1 => ordinary doclist gaps,
    // gaps == 2 => gaps with boundaries (for positions)
    // returns the number of elements written (which is not clear in advance
    // if gaps==2)
    unsigned long writeToPointer(T* outputData, unsigned char gaps = 0) const
    {
      assert(outputData);
      assert(!_isFullList);

      if (size()>0)
      {
        // first element is always encoded directly
        if (gaps != 2) {outputData[0] = Vector<T>::operator[](0);}
        else {outputData[0] = Vector<T>::operator[](0) +1;}
      }
      if (gaps == 0)
      {
        for (unsigned long i = 1; i < size(); i++)
        {
          outputData[i] = Vector<T>::operator[](i);
          // should at least hold for doclist without duplicates,
          // but not for wordlists:
          // assert(outputData[i] > outputData[i-1]);
        }
        return  size();
      }  // end case: not using gaps
      else if (gaps == 1)
      {
        for (unsigned long i = 1; i < size(); i++)
        {
          outputData[i] = (Vector<T>::operator[](i) -
                           Vector<T>::operator[](i-1));
          // should at least hold for doclist without duplicates,
          // but not for wordlists
          assert(Vector<T>::operator[](i) >= Vector<T>::operator[](i-1));
        }
        return size();
      }  // end case: using gaps = 1 (for doclists)
      else
      {
        assert(gaps == 2);

        unsigned long j = 1;
        for (unsigned long i = 1; i < size(); i++, j++)
        {
          if (Vector<T>::operator[](i) > Vector<T>::operator[](i-1) )
          {
            outputData[j] = (T) 1 + operator[](i) - operator[](i-1);
          }
          else
          {
            outputData[j] = (T) 0;
            j++;
            outputData[j] = (T) 1 + operator[](i);
          }
        }
#ifndef NDEBUG
        for (unsigned long i = 1; i < j; i++)
        {
          assert(((T) outputData[i] != (T) 0) ||
                 ((T) outputData[i-1] != (T) 0));
        }
#endif
        assert((j>size())||(isSorted(true)));
        return  j;
      }  // end case: using gaps = 2 (for positions)
    }


    bool isEmpty() const
    {
      if (_isFullList) {return false;}
      return (size()>0) ? false : true;
    }

    off_t entropy(unsigned long n, double& entropy, off_t& nofBits) const
    {
      assert(!_isFullList);
      assert(size()>0);

      off_t totalSum = 0;  // will hold sum_i n_i

      entropy = 0;
      nofBits = 0;
      T n_i;

      for (unsigned int i = 0; i < size(); i++)
      {
        n_i = Vector<T>::operator[](i);

        totalSum += n_i;
        nofBits += (off_t) ceil((
              (double) n_i*log2((double) n/n_i) +   // NOLINT
              (n-n_i)*log2((double) n/(n-n_i)) ));  // NOLINT 
      }

      entropy = static_cast<double>(nofBits/totalSum);
      return totalSum;
    }  // end: entropy(..)

    /* Checks if a Vector is sorted *
     * if strict is true, then also *
     * checks for strict sortedness *
     * wihout duplicates            */

    bool isSorted(bool strict = false) const
    {
      //  for (unsigned int i = 1; i<getNofElements(); i++)
      for (unsigned int i = 1; i < Vector<T>::size(); i++)
      {
        if (strict)
        {
          if (operator[](i) <= operator[](i-1))
          {
            // cerr << " at position i = " << i
            //      << ", with size = " << size() << endl;
            // cerr << " read : " << operator[](i)
            //      << ", but at last position I read : "
            //      << operator[](i-1) << endl;
            return false;
          }
        }
        else
        {
          if (operator[](i) < operator[](i-1))
          {
            // cerr << " at position i = " << i
            //      << ", with size = " << size() << endl;
            // cerr << " read : " <<operator[](i)
            //      << ", but at last position I read : "
            //      << operator[](i-1) << endl;
            return false;
          }
        }
      }
      return true;
    }  // end: isSorted(..)



    bool isReverseSorted(bool strict = false) const
    {
      //  for (unsigned int i = 1; i < getNofElements(); i++)
      for (unsigned int i = 1; i < Vector<T>::size(); i++)
      {
        if (strict)
        {
          if (operator[](i) >= operator[](i-1))
          {
            // cerr << " at position i = " << i
            //      << ", with size = " << size() << endl;
            // cerr << " read : " <<operator[](i)
            //      << ", but at last position I read : "
            //      << operator[](i-1) << endl;
            return false;
          }
        }
        else
        {
          if (operator[](i)>operator[](i-1))
          {
            // cerr << " at position i = " << i <<", with size = "
            //      << size() << endl;
            // cerr << " read : " << operator[](i)
            //      << ", but at last position I read : "
            //      << operator[](i-1) << endl;
            return false;
          }
        }
      }
      return true;
    }  // end: isReverseSorted(..)


    template <typename U> void copy(const Vector<U>& source)
    {
      _isFullList = source.isFullList();
      resize(source.size());
      if (sizeof(U) != sizeof(T))
      {  // have to copy element by element
        for (register unsigned int i = 0; i < source.size();i++)
        {
          Vector<T>::operator[](i) = (T) source.Vector<U>::operator[](i);
        }
      }
      else
      {  // do a memcpy
        if (size() >0)
        {
          assert(source.isContiguous());
          memcpy(&Vector<T>::operator[](0), &source.Vector<U>::operator[](0),
                 size()*sizeof(T));
          assert(isContiguous());
        }
      }
    }



    /* This rountine was used for documents
     * Is obsolete now, but keep, as we might want to "turn off" scoring
     * for docs
     *
       void uniqueElements(const Vector<T>& source, bool doSort = true)
       {
    // first copy elements

    clear();
    if (source.getNofElements() == 0) {return;}
    copy(source);

    assert(size()>0);

    assert(doSort||isSorted());

    if (doSort)
    {
    GlobalTimers::stlSortTimer.cont();
    // then sort
    sort(begin(), end());
    GlobalTimers::stlSortTimer.stop();
    }
    assert(isSorted());
    // then remove duplicates
    // TODO(INGMAR): Write a user defined unique function, which could also do
    // scoring e.g. by counting how often something occurs and returning pairs
    GlobalTimers::uniqueTimer.cont();
    const typename vector<T>::iterator new_end
      = unique(vector<T>::begin(), vector<T>::end());
    GlobalTimers::uniqueTimer.stop();
    // then cut to right length
    resize(distance(vector<T>::begin(), new_end));  // yes, no +1 here!
    assert(isSorted(true));
    }
    */

    T max() const
    {
      if (size() == 0) return 0;
      T max = operator[](0);
      for (unsigned int i = 1;i < size();i++)
      {
        max = MAX(max, operator[](i));
      }
      return max;
    }

    T min() const
    {
      if (size() == 0) return 0;
      T min = operator[](0);
      for (unsigned int i = 1;i < size();i++)
      {
        min = MIN(min, operator[](i));
      }
      return min;
    }


    void show(unsigned int nof_items_to_show = 20) const
    {
      for (unsigned int i = 0; i < MIN(size(), nof_items_to_show);i++)
      {
        if (sizeof(T) > 1)
        {
          cout << Vector<T>::operator[](i);
        }
        else
        {
          cout << (unsigned int) Vector<T>::operator[](i);
        }
        if ((i+1) < MIN(size(), nof_items_to_show))
        {
          cout << "\t";
        }
      }
      cout << "\n";
    }  // end: show()


    void show(ostringstream& outputStream,
              unsigned int nof_items_to_show = 20) const
    {
      for (unsigned int i = 0; i < MIN(size(), nof_items_to_show);i++)
      {
        if (sizeof(T) > 1)
        {
          outputStream << Vector<T>::operator[](i);
        }
        else
        {
          outputStream << (unsigned int) Vector<T>::operator[](i);
        }
        if ((i+1) < MIN(size(), nof_items_to_show))
        {
          outputStream << "\t";
        }
      }
      outputStream << "\n";
    }  // end: show()


    // Contents of the vector in human-readable form.
    string debugString();


    // TODO(INGMAR): for some reason the bool constructor is not accessible
    // in doclist!
    void setFull(bool isFull)
    {
      _isFullList = isFull;
    }

    template <class S>
      inline void append(const Vector<S>& source)
      {
        assert(!_isFullList);

#ifndef NDEBUG
        const bool shouldBePositive = source.isPositive() && isPositive();
#endif

        const unsigned long int oldSize = size();

        if (capacity() < (source.size() + size()))
        {
          Vector<T>::reserve(2* (source.size() + size()));
        }

        Vector<T>::resize(Vector<T>::size() + source.size());  // crucial!!!!

        if (sizeof(T) != sizeof(S))
        {  // case: types differ in size. Have to copy element by element
          register unsigned long j = oldSize;
          for (unsigned long i = 0; i < source.size(); i++, j++)
          {
            operator[](j) = (T) source[i];
          }
        }  // end case: types differ in size. Have to copy element by element
        else
        {  // case: types ahve the same size. Can use memcpy
          assert(source.isContiguous());
          memcpy(&Vector<T>::operator[](size()-source.size()),
              &source.Vector<S>::operator[](0), source.size()*sizeof(T));
          assert(isContiguous());
        }
#ifndef NDEBUG
        assert(!shouldBePositive || isPositive());
#endif
        // do NOT resize after assignments!!
      }

    bool isFullList() const
    {
      return _isFullList;
    }
};  // class Vector


// A "repeatedVector" is a Vector which only contains a single element but has
// it repeated many times. However, it can also be used as a normal vector used
// for WordLists in the case of INV and for scores when the candidate list is
// the full list.
template <class T> class repeatedVector : public Vector<T>
{
  protected:

    bool _isRepeated;  // to indicate that only one element is stored repeatedly
    unsigned long _size;  // only used if _isRepeated

  public:

    // Create empty, non-repeated vector
    repeatedVector(): Vector<T>()
  {
    // Vector<T>::Vector<T>();
    _size = 0;
    _isRepeated = false;
  }


    // Write all elements to stdout (for debugging)
    void show() const
    {
      for (unsigned int i = 0; i < getNofElements();i++)
      {
        cout << " " << repeatedVector<T>::operator[](i);
      }
      cout << "\n";
    }


    // Append elements of \c source to \c this
    inline void append(const repeatedVector<T>& source)
    {
      //      GlobalTimers::appendTimer.cont();
      assert(!Vector<T>::_isFullList);

      register unsigned long j = Vector<T>::size(); /* CYGWIN */

      if (Vector<T>::capacity() < (source.getNofElements() + Vector<T>::size()))
      {
        Vector<T>::reserve(2* (source.getNofElements() + Vector<T>::size()));
      }

      // crucial!
      Vector<T>::resize(Vector<T>::size() + source.getNofElements());

      for (unsigned long i = 0; i < source.getNofElements(); i++, j++)
      {
        Vector<T>::operator[](j) = source.repeatedVector<T>::operator[](i);
      }
      // do NOT resize after assignments!!
      //      GlobalTimers::appendTimer.stop();
    }


    // Create with given element repeated given number of times
    repeatedVector(const T& element, unsigned long nofRepetitions)
    {
      assert(nofRepetitions>0);
      Vector<T>::resize(1);
      Vector<T>::operator[](0) = element;

      // CYGWIN
      Vector<T>::_isFullList = false;
      Vector<T>::_isRepeated = true;
      Vector<T>::_size = nofRepetitions;

      assert(getNofElements() == nofRepetitions);
    }


    // Get number of elements
    inline unsigned long getNofElements() const
    {
      assert((!_isRepeated)||(_size > 0));
      return (_isRepeated) ? (_size) : (Vector<T>::size());
    }

    // Get element at given position (reference)
    inline const T& operator[](unsigned long n) const
    {
      assert(!_isRepeated||(n < _size));
      return (_isRepeated) ? (Vector<T>::operator[](0))
                           : (Vector<T>::operator[](n));
    }

    // Get element at given position (const reference)
    inline T& operator[](unsigned long n)
    {
      assert(!_isRepeated);
      return Vector<T>::operator[](n);
    }


    // Copy given vector to this; only for non-repeated vector!
    void copy(const repeatedVector<T>& source)
    {
      _isRepeated = source._isRepeated;
      _size = source._size;
      assert(!_isRepeated);
      Vector<T>::copy(source);
    }

    // Set to empty vector
    void clear()
    {
      Vector<T>::clear();
      _size = 0;
      _isRepeated = false;
    }
};  // class repeatedVector

// Vector of positions (Note: Doclist and WordList are proper classes derived
// from Vector<DocId> and Vector<WordId>)
typedef Vector<Position> PositionList;

// Vector of scores (Note: Doclist and WordList are proper classes derived
// from Vector<DocId> and Vector<WordId>)
typedef Vector<Score> ScoreList;

#endif  // CODEBASE_SERVER_VECTOR_H_
