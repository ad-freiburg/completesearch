#ifndef __COMPRESSIONALGORITHM_H__
#define __COMPRESSIONALGORITHM_H__
#include "Globals.h"

using namespace std;


class CompressionAlgorithm
{
 public:
  
  CompressionAlgorithm(double increaseFactor) : _increaseFactor(increaseFactor) {};
  CompressionAlgorithm() : _increaseFactor(-1.0) {};

  // returns the number of bytes written to targetArray
  // 0 indicates failure
  // memory for (un)compression needs to be allocated before callign compress

  // can't have virtual template functions in C++ !!
  // returns size of compressed array = NOF BYTES USED
  //  template <typename T> size_t compress(const Vector<T>& vectorToCompress, void* targetArray) const {return 0;}
  //size_t compress(const Vector<int>& vectorToCompress, void* targetArray) const {return 0;}

  // can't have virtual template functions in C++ !!
  // returns NOTHING
  // NUMBER OF ELEMENT of uncompressed array HAS TO BE KNOWN!!
  //template <typename T> void decompress(const void* sourceArray, T* targetArray, unsigned long nofElements) const {return;}

  double getIncreaseFactor() const {assert(_increaseFactor > 0); return _increaseFactor;}

  // factor by which things could get INCREASED during compression (in the worst case)
  // Just to ensure the program ALWAYS works and no realloc is necessary.
  // Has to include codebook for Huffman
 private:
  const double _increaseFactor;


  
}; // end class declaration


#endif
