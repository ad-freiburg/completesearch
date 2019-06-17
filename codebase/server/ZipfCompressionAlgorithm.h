// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#ifndef SERVER_ZIPFCOMPRESSIONALGORITHM_H__
#define SERVER_ZIPFCOMPRESSIONALGORITHM_H__

#include <vector>
#include <algorithm>
#include <utility>
#include "./CompressionAlgorithm.h"
#include "./Vector.h"
#include "./codes.h"

//! Sorting class (needed in zipfcompressionalgorithm::compress).
template<class T> class SortBySecondDescending
{
  public: bool operator()(const pair<T, unsigned>& x,
                          const pair<T, unsigned>& y) const
          { return x.second > y.second; }
};

//! Sorting class (needed in zipfcompressionalgorithm::compress).
template<class T> class SortByFirstAscending
{
  public:
    bool operator()(const pair<T, unsigned>& x,
                    const pair<T, unsigned>& y) const
          { return x.first < y.first; }
};

/** Class for (de)compression of word lists with a zipf-like distribution.
 *
 *    added by Holger 21Jan06
 *
 *    for a Zipf-like distribution of frequencies, an entropy-optimal
 *    encoding is obtained by sorting the items in order of descending
 *    frequencies (most frequent item first), and then encode the i-th
 *    item in this list with the elias-delta code for i.
 *
 *    The sorting permutation has to be stored, but in our application
 *    that will typically be small (number of distinct words in a block
 *    versus total volume of a block)
 */
template<class T>
class ZipfCompressionAlgorithm : public CompressionAlgorithm
{
  public:
    /** Constructor.
     * 
     *  The factor 1.0 says that encoding will never increase size (in bytes)
     */
    ZipfCompressionAlgorithm() :  CompressionAlgorithm(1.0) {}

    /** Compress stl::vector of items to an array of bytes.
     *
     *    the compress byte sequence has the following structure:
     *      1. min item
     *      2. max item
     *      3. item[] of size max-min+1; item[i] = i-th largest item - min
     *      4. the coded items
     *
     *  Note : casting item type t to unsigned int must make sense.
     */
    size_t compress(const Vector<T>& itemsToCompress, void* targetArray) const;
    //! Decompress the given compressed field sourceArray.
    void decompress(const void* sourceArray, T* targetArray, unsigned long n);
};

#endif  // SERVER_ZIPFCOMPRESSIONALGORITHM_H__

