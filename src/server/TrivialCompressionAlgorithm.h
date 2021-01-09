#ifndef __TRIVIALCOMPRESSIONALGORITHM_H__
#define __TRIVIALCOMPRESSIONALGORITHM_H__

#include <math.h> // needed for log2
#include "assert.h"
#include "CompressionAlgorithm.h"
#include "Vector.h"

using namespace std;

class TrivialCompressionAlgorithm : public CompressionAlgorithm
{
 public:

  // is actually and additive factor of 1 byte (for the windowSize)
  TrivialCompressionAlgorithm() : CompressionAlgorithm(1.25) {}; 

  // returns size of compressed array = NOF BYTES USED
  template <typename T> size_t compress(const Vector<T>& vectorToCompress, void* targetArray) const
    {
      assert(vectorToCompress.size() > 0);
      assert(targetArray);
      assert(sizeof(T)==4);

      const unsigned long int nofNumbers = vectorToCompress.size();
      
      /* GET MAXIMUM NUMBER IN VECTOR */
      // The biggest number in the vector determines how many bits need to be used
      T max = 0;      
      T min = MAX_WORD_ID;
      for(unsigned long int i=0; i<nofNumbers; i++)
	{
	  if(vectorToCompress[i]>max ) {max = vectorToCompress[i];}
	  if(vectorToCompress[i]<min ) {min = vectorToCompress[i];}
	}
      
      // This is the number of bits needed to represent the largest number in binary
      // Can be zero if min == max !!! (a problem for INV with a single entry in a list)
      const unsigned char maxLog2 = (unsigned char) ceil(log2(max-min+1));
      assert(maxLog2 > 0);
      assert(maxLog2 <= 32);

      /* GET WINDOW SIZE (= number of bits per number) */
      // for large Numbers (>= 2^26) it could happen that, due to shifts, more than 4 bytes need to be manipulated simulatenously
      // As this is not possible, avoid this case.
      unsigned char windowS; 
      if(maxLog2 <= 26) windowS = maxLog2;
      else
	{
	  switch (maxLog2)
	    {
	    case 27:
	      windowS = 28;
	    case 28:
	      windowS = 28;
	    default:
	      windowS = 32;
	    }
	}
      
      // These many bits are used per number
      register const unsigned char windowSize = windowS;
      
      // The "Codebook" consists only of the windowSize ...
      *((unsigned char*) targetArray) = windowSize;
      // .. plus the minimum
      *((T*) ((unsigned char*) targetArray + 1)) = min;
      assert((min ==0)|| (*((T*) ((unsigned char*) targetArray +1 )) >0 ));

      /* INITIALIZE TARGET BUFFER */
      // +2 here as first unsigened char is used to write the windowSize
      for(unsigned long int i=1; i< ((nofNumbers*windowSize-1)/8 +2+sizeof(T)); i++) *((unsigned char*) targetArray + i + sizeof(T))  = 0;
      assert((min ==0)|| (*((T*) ((unsigned char*) targetArray +1 )) >0 ));            
      /* ENCODE ELEMENT ONE-BY-ONE */
      register unsigned long int bitPos = 0;
      for(register unsigned long int i=0; i< nofNumbers; i++, bitPos += windowSize)
	{
	  // Number has to fit within window
	  assert(((unsigned int) (vectorToCompress[i]-min) & (  (windowSize == 32) ? ((unsigned int) -1) : ((1 << windowSize)-1)    )  ) == (unsigned int) (vectorToCompress[i]-min));      
	  // last byte should be untouched
	  assert((windowSize < 8) || ( *((unsigned char*) targetArray +1 +sizeof(T)+ (bitPos+windowSize -1)/8)    == 0));
	  // A windows size of 31 won't be possible, as otherwise we'd need to manipulate more than 4 bytes simultaneously
	  assert( (bitPos % 8) <=  (unsigned long int) (32-windowSize)); // make sure that no high order bits are lost
	  // ensure that the low order bits are unused
	  assert( ((*((T*) ((unsigned char*) targetArray + 1 + sizeof(T) + (  (bitPos / 8)  )))) & ( ( (windowSize == 32) ? ((unsigned int) -1) : ((1 << windowSize)-1)    ) << (  (bitPos) % 8 ))) == 0 ); 
	  // Encode by a shift followed by an OR
	  (*((T*) ((unsigned char*) targetArray + 1 + sizeof(T) + ( (bitPos >> 3)   ) ))) |= (  (vectorToCompress[i]-min) << ((bitPos) % 8  ) );
	}
      
      assert((min ==0)|| (*((T*) ((unsigned char*) targetArray +1 )) >0 ));

      return (size_t) 1 +sizeof(T)+ (bitPos+7)/8;
    } // end: Compress
  

  // returns NOTHING
  // NUMBER OF ELEMENT of uncompressed array HAS TO BE KNOWN!!
  // compresses size in bytes is not relevant
  template <typename T> void decompress(const void* sourceArray, T* targetArray, unsigned long int nofElements) const
    {
      
      register const unsigned char windowSize = *((unsigned char*) sourceArray);
      register const T min = *((T*) ((unsigned char*) sourceArray +1 ));
      assert(windowSize > 0);
      assert(windowSize <= 32);
      assert(sizeof(T) == 4);
      assert(min>=0);

      register const unsigned int mask = ((windowSize == 32) ? ((unsigned int) -1) : ((1 << windowSize)-1));
      
      register unsigned long int bitPos = 0;
      for(unsigned long int i=0; i< nofElements; i++, bitPos += windowSize)
	{
	  // make sure that no high order bits are lost in bit shift operations
	  assert( ( bitPos % 8  ) <= (unsigned long) (32-windowSize));
	  targetArray[i] = min + (( (*((T*) ((unsigned char*) sourceArray + 1 + sizeof(T)+ ( bitPos >> 3   )  ))) >> ( bitPos % 8  ) ) & mask);
	}
      
    }
  
}; // end class TrivialCompressionAlgorithm

#endif
