#ifndef __DUMMYCOMPRESSIONALGORITHM_H__
#define __DUMMYCOMPRESSIONALGORITHM_H__

#include "CompressionAlgorithm.h"

class DummyCompressionAlgorithm : public CompressionAlgorithm
{
 public:
  DummyCompressionAlgorithm() :  CompressionAlgorithm(1.0) {};

  // can't have virtual template functions in c++
  // returns size of compressed array = NOF BYTES USED
  template <typename T> size_t compress(const vector<T>& vectorToCompress, void* targetArray) const
    {
      assert(vectorToCompress.size() > 0);
      assert(targetArray);
      for(unsigned int i=0; i<vectorToCompress.size(); i++)
      	{
	  assert(vectorToCompress[i] >= 0);
	  *((T*) targetArray + i) = vectorToCompress[i];
      	}
      return vectorToCompress.size()*sizeof(T);
    }

  // returns NOTHING
  // NUMBER OF ELEMENT of uncompressed array HAS TO BE KNOWN!!
  template <typename T> void decompress(const void* sourceArray, T* targetArray, unsigned long nofElements) const
    {
      assert(nofElements > 0);
      assert(targetArray);
      assert(sourceArray);

      for(unsigned long i=0; i<nofElements; i++)
	{ 
	  assert(*((T*) sourceArray + i) >= 0);
	  targetArray[i] = *((T*) sourceArray + i);
	}

    }


};

#endif
