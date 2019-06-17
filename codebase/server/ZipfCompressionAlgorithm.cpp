// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include "./ZipfCompressionAlgorithm.h"
#include <vector>
#include <algorithm>
#include <utility>

// _____________________________________________________________________________
template<class T>
size_t ZipfCompressionAlgorithm<T>::compress(const Vector<T>& itemsToCompress,
                                             void* targetArray) const
{
  // COMPUTE MIN AND MAX
  T min = itemsToCompress[0];
  T max = itemsToCompress[0];
  for (unsigned int i = 1; i < itemsToCompress.size(); ++i)
  {
    if (itemsToCompress[i] > max) max = itemsToCompress[i];
    if (itemsToCompress[i] < min) min = itemsToCompress[i];
  }
  assert(max-min >= 0);

  // SORT ITEM IDS BY FREQUENCY (first = item id - min; second = frequency)
  //                             normalized item ids have range [0..max-min]
  unsigned int n = itemsToCompress.size();
  vector< pair<T, unsigned int> > freqs(max-min+1);
  for (unsigned int j = 0; j < freqs.size(); ++j)
  {
    freqs[j].first = j;
    freqs[j].second = 0;
  }
  for (unsigned int i = 0; i < n; ++i)
    ++freqs[ itemsToCompress[i] - min ].second;
#ifdef ZIPF_DEBUG
  cout << endl << endl;
  cout << "! " << flush;
  for (unsigned int j = 0; j < freqs.size(); ++j)
    cout << "[" << min + freqs[j].first << "|" << freqs[j].second << "]"
         << flush;
  cout << endl << endl;
#endif
  sort(freqs.begin(), freqs.end(), SortBySecondDescending<T>());
#ifdef ZIPF_DEBUG
  cout << "! " << flush;
  for (unsigned int j = 0; j < freqs.size(); ++j)
    cout << "[" << min + freqs[j].first << "|" << freqs[j].second << "]"
         << flush;
  cout << endl << endl;
#endif

  // MAP itemIdByRank :  i -> i-th most frequent item id (normalized)
  vector<unsigned int> itemIdByRank(freqs.size());
  for (unsigned int j = 0; j < freqs.size(); ++j)
    itemIdByRank[j] = freqs[j].first;
#ifdef ZIPF_DEBUG
  cout << "! rank -> item id : " << flush;
  for (unsigned int j = 0; j < freqs.size(); ++j)
    cout << "[" << j << "->" << min + itemIdByRank[j] << "]" << flush;
  cout << endl << endl;
#endif

  // MAP rankByItemId :  normalized item id -> frequency rank
  vector<unsigned int> rankByItemId(freqs.size());
  for (unsigned int j = 0; j < freqs.size(); ++j)
    freqs[j].second = j;
  sort(freqs.begin(), freqs.end(), SortByFirstAscending<T>());
  for (unsigned int j = 0; j < freqs.size(); ++j)
    rankByItemId[j] = freqs[j].second;
#ifdef ZIPF_DEBUG
  cout << "! item id -> rank : " << flush;
  for (unsigned int j = 0; j < freqs.size(); ++j)
    cout << "[" << min + j << "->" << rankByItemId[j] << "]" << flush;
  cout << endl << endl;
#endif

  // ENCODE THE *RANKS* OF THE ITEM IDS
  vector<unsigned int> itemRanks(n);
  for (unsigned int i = 0; i < n; ++i)
    itemRanks[i] = rankByItemId[ itemsToCompress[i] - min ];
#ifdef ZIPF_DEBUG
    cout << "! will compress ranks : " << flush;
    for (unsigned int i = 0; i < n; ++i)
      cout << "[" << itemRanks[i] << "]" << flush;
    cout << endl << endl;
#endif
  // CODEBOOK = max + min + itemIdByRank (much smaller than Huffman codebook!)
  unsigned int* code = (unsigned int*)(targetArray);
  code[0] = (unsigned int)(min);
  code[1] = (unsigned int)(max);
  memcpy(code + 2, &itemIdByRank[0], itemIdByRank.size()*sizeof(unsigned int));
  // code size in UNSIGNED INTS!
  unsigned int code_size = 2 + itemIdByRank.size();
  // ENCODE WITH SIMPLE9
  code_size += Simple9_enc(n, reinterpret_cast<int*>(&itemRanks[0]),
                           code + 2 + itemIdByRank.size());

  return sizeof(unsigned int)*code_size;
}

// _____________________________________________________________________________
template<class T>
void ZipfCompressionAlgorithm<T>::decompress(const void* sourceArray,
                                             T* targetArray,
                                             unsigned long n)
{
  unsigned int* code = (unsigned int*)(sourceArray);
  T min = (T)(code[0]);
  T max = (T)(code[1]);
  unsigned int* itemIdByRank = code + 2;
  Simple9_dec(n, reinterpret_cast<int*>(&targetArray[0]),
              code + 2 + max - min + 1);
#ifdef ZIPF_DEBUG
  cout << "! retrieved ranks : " << flush;
  for (unsigned int i = 0; i < n; ++i)
    cout << "[" << targetArray[i] << "]" << flush;
  cout << endl << endl;
#endif
  for (unsigned int i = 0; i < n; ++i)
    targetArray[i] = itemIdByRank[ targetArray[i] ] + min;
#ifdef ZIPF_DEBUG
  cout << "! item ids again : " << flush;
  for (unsigned int i = 0; i < n; ++i)
    cout << "[" << targetArray[i] << "]" << flush;
  cout << endl << endl;
#endif
}

//! Explicit instantiatoin, so that code gets generated.
template class ZipfCompressionAlgorithm<WordId>;
