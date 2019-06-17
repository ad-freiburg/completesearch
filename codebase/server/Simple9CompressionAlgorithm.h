#ifndef __SIMPLE9COMPRESSIONALGORITHM_H__
#define __SIMPLE9COMPRESSIONALGORITHM_H__

#include "CompressionAlgorithm.h"
#include "codes.h"
#include "Vector.h"

using namespace std;

class Simple9CompressionAlgorithm : public CompressionAlgorithm
{
 public:

  // set the increaseFactor (which is 2 for simple 9 [for a single 32 bit number])
  Simple9CompressionAlgorithm() : CompressionAlgorithm(2) {};
  template <typename T> void decompress(const void* sourceArray, T* targetArray, unsigned long nofElements, unsigned char useGaps = 1) const;
  template <typename T> size_t compress(const Vector<T>& vectorToCompress, void* targetArray, unsigned char useGaps = 1) const;


}; // end class declaration


  // returns size of compressed array = NOF BYTES USED
  // useGaps==0  => no gaps,  useGaps==1  =>  normal gaps (for doclists, useGaps == 2  => gaps w/ boundaries (for positions)
 template <typename T> size_t Simple9CompressionAlgorithm::compress(const Vector<T>& vectorToCompress, void* targetArray, unsigned char useGaps) const
    {
      assert(vectorToCompress.size() > 0);
      assert(targetArray);
      assert(sizeof(T)==4);//32 bits are hard-coded in the code!

      unsigned long returnValue;
      if(useGaps==1)
	{
	  T* arrayToCompress = new T[vectorToCompress.size()];
	  // if useGaps then the differences are encoded rather than the raw ids
	  vectorToCompress.writeToPointer(arrayToCompress,useGaps);
	  returnValue = Simple9_enc(vectorToCompress.size(), (int*) arrayToCompress, (unsigned int*) targetArray )*sizeof(int);
	  delete[] arrayToCompress;
	  arrayToCompress = NULL;
	}
      else if (useGaps == 0)
	{
	  // in this case we can simply use the vector directly
	  assert(vectorToCompress.isContiguous());
	  returnValue = Simple9_enc(vectorToCompress.size(), (int*) &vectorToCompress.Vector<T>::operator[](0), (unsigned int*) targetArray )*sizeof(int);
	}
      else
	{
	  assert(useGaps == 2);
	  ///	  T* arrayToCompress = new T[ 2 *vectorToCompress.size()];//Factor two is needed due to gaps
	  Vector<T> arrayToCompress;
	  arrayToCompress.resize(3 *vectorToCompress.size(),0);//Factor two is needed due to gaps
	  // if useGaps==2 then the differences (with boundaries) are encoded rather than the raw ids
	  assert(arrayToCompress.isContiguous());
	  const unsigned long nofElementsEncoded = vectorToCompress.writeToPointer(&arrayToCompress[0],useGaps);
	  assert( nofElementsEncoded  <= 2 *vectorToCompress.size());
	  assert(arrayToCompress.isContiguous());
	  assert(arrayToCompress[nofElementsEncoded-1] != 0);
	  assert(arrayToCompress[nofElementsEncoded] == 0);
	  #ifndef NDEBUG
	  for(unsigned int i=1; i<nofElementsEncoded; i++) {assert((arrayToCompress[i]!=0)||(arrayToCompress[i-1] != 0));}
          #endif 
	  assert(vectorToCompress.isSorted(true)||(nofElementsEncoded > vectorToCompress.size()));
	  *((unsigned long*) targetArray) = nofElementsEncoded;
	  returnValue = Simple9_enc(nofElementsEncoded, (int*) &arrayToCompress[0], (unsigned int*) ((unsigned char*) targetArray + sizeof(unsigned long)) )*sizeof(int);
	  returnValue += sizeof(unsigned long);
	  assert(returnValue > sizeof(unsigned long));
	  assert(  *((unsigned int*) ((unsigned char*) targetArray + (returnValue -sizeof(unsigned int)))) != 0);

	  ////	  delete[] arrayToCompress;
	  ////	  arrayToCompress = NULL;
	}
      assert((returnValue >0) || (vectorToCompress.size()==0));

      return returnValue;
    }// end: compress(..)

  // returns NOTHING
  // NUMBER OF ELEMENT of uncompressed array HAS TO BE KNOWN!!
  // compresses size in bytes is not relevant
  // useGaps == 1 is used for doclists, useGaps==2 for positions (where boundaries are introduced)
  template <typename T> void Simple9CompressionAlgorithm::decompress(const void* sourceArray, T* targetArray, unsigned long nofElements, unsigned char useGaps) const
    {
      assert(nofElements > 0);
      assert(sizeof(T)==sizeof(int));
      assert(targetArray);
      assert(sourceArray);
      if(useGaps==1) {Simple9_dec_gaps(nofElements,(int*) targetArray,(unsigned int*) sourceArray);}
      else if(useGaps==0) { Simple9_dec(nofElements,(int*) targetArray,(unsigned int*) sourceArray);}
      else
	{
	  assert(useGaps==2);
	  const unsigned long nofElementsWithBoundaries = *((unsigned long *) sourceArray);
	  Simple9_dec_gaps_with_boundaries(nofElementsWithBoundaries,(int*) targetArray, (unsigned int*) ((unsigned char*) sourceArray +sizeof(unsigned long) ));
	}
    }// end: decompress


#endif
