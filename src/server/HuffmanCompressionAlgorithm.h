#ifndef __HUFFMANCOMPRESSIONALGORITHM_H__
#define __HUFFMANCOMPRESSIONALGORITHM_H__

//#include "NumericalRecipes/huffman.h"
#include "Compress/huffman.h"
#include "CompressionAlgorithm.h"

class HuffmanCompressionAlgorithm : public CompressionAlgorithm
{
 public:
  // could increase things for a small vocabulary size
  // theoretical factor (for nr implementation) is 20 (or slightly less)
  // work with 10 here
  // close to 8 could happen in practice (for blocks with only distinct words)
  // again, this is just the NR implementation of the codebook
  // could pass buffer size to compress and check if it's sufficient
  HuffmanCompressionAlgorithm() :  CompressionAlgorithm(10.0) {};

  // can't have virtual template functions in c++
  // returns size of compressed array = NOF BYTES USED
  template <typename T> size_t compress(const vector<T>& vectorToCompress, void* targetArray) const
    {
      
      assert(vectorToCompress.size() > 0);
      assert(targetArray);

      // 1. pass over vector and get min and max elements
      T min = vectorToCompress[0];
      T max = vectorToCompress[0];
      for(unsigned int i=1; i<vectorToCompress.size(); i++)
	{
	  if(vectorToCompress[i] > max) max = vectorToCompress[i];
	  if(vectorToCompress[i] < min) min = vectorToCompress[i];
	}
      assert(max-min>=0);

      // 2. pass over vector (again) and get counters for elements
      unsigned long int* nfreq = new unsigned long int[max-min+2];// the 0-th field is not used by hufmak
      for(unsigned int i=1;i<(unsigned int) max-min+2;i++) {nfreq[i] = 0;} // initialize
      for(unsigned int i=0; i<vectorToCompress.size(); i++)
	{
	  ++nfreq[vectorToCompress[i]-min+1]; // should not use the 0-th element
	}
#ifndef NDEBUG
      for(unsigned int i=1;i<(unsigned int) max-min+2;i++) {assert(nfreq[i]>0);} 
#endif

      // 3. Make Huffman code with these counters
      const unsigned long nchin = max-min+1; // number of distinct characters to encode
      unsigned long ilong; // will hold (one of) the cleartext codewords(=numbers) which gives the longest code
      unsigned long nlong; // holds the number of bits to encode (one of) the rarest codewords
      /* huffcode is the following struct:
	typedef struct {
	unsigned long *icod,*ncod,*left,*right,nch,nodemax;
	} huffcode;
      */
      huffcode huffmanStruct;
      // TODO: find out what an lveCTor is
      // Answer: function which mallocs a block of size arg2-arg1+1+NR_END
      // NR_END is 1 (why??)... i.e. wastes the first unsigned long
      //arguments are subscript ranges of vector
      /* the code from nrutil.c  (just to show, what lveCTor does
	unsigned long *lveCTor(long nl, long nh)
	// allocate an unsigned long veCTor with subscript range v[nl..nh] 
	{
	unsigned long *v;
	v=(unsigned long *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(long)));
	if (!v) nrerror("allocation failure in lveCTor()");
	return v-nl+NR_END;
	}
      */
      // initialize 'huffcode' struct.
      // These four things need to be written to disk later
      // all of these are pointers to the 0-th elements (which aren't used)
      // for each 2*nchin elements are allocated
      huffmanStruct.icod = (unsigned long*)lveCTor(1,2*nchin-1);
      huffmanStruct.ncod = (unsigned long*)lveCTor(1,2*nchin-1);
      huffmanStruct.left = (unsigned long*)lveCTor(1,2*nchin-1);
      huffmanStruct.right = (unsigned long*)lveCTor(1,2*nchin-1);
      /* This is NOT NEEDED FOR CORRECTNESS!
	 But it ensures that valgrind does not complain about writing
	 uninitalized bytes (... which are never used). Apparently, 
	 the huffmanStruct only uses some of the memory it's allocated. */
      for (unsigned int j=0;j<=(2*nchin-1);j++)
      	{
	  huffmanStruct.icod[j]=huffmanStruct.ncod[j]=huffmanStruct.left[j]=huffmanStruct.right[j]=0;
	}

      /* At this point we need
	 4*sizeof(unsigned long)*2*nchin + 2*sizeof(unsigned long)
	 = sizeof(unsigned long)*(8*nchin + 2) bytes to store the 
	 huffmanStruct on disk  PLUS sizeof(unsigned long) for min */
      hufmak(nfreq,nchin,&ilong,&nlong,&huffmanStruct);
      delete[] nfreq;
      assert(nlong<8*sizeof(unsigned long));

      // 4.Write Huffman codebook (=coding tree) to memory
      // Write it in the format:
      // min, nch, nodemax, icod[], ncod[], left[], right[]
      *((unsigned long*) targetArray + 0) = min;
      *((unsigned long*) targetArray + 1) = huffmanStruct.nch;
      *((unsigned long*) targetArray + 2) = huffmanStruct.nodemax;
      
      if(nchin > 1)
      	{//begin case: more than one distinct element to encode
	  memcpy((void*) ((unsigned long*) targetArray +3), huffmanStruct.icod, (size_t) 2*nchin*sizeof(unsigned long));
	  memcpy((void*) ((unsigned long*) targetArray +3+2*nchin), huffmanStruct.ncod, (size_t) 2*nchin*sizeof(unsigned long));
	  memcpy((void*) ((unsigned long*) targetArray +3+4*nchin), huffmanStruct.left, (size_t) 2*nchin*sizeof(unsigned long));
	  memcpy((void*) ((unsigned long*) targetArray +3+6*nchin), huffmanStruct.right, (size_t) 2*nchin*sizeof(unsigned long));
	  
	  // 5. pass over vector (yet again) and compress to array
	  // hufenc() only supports element by element compression
	  // When compressing only numbers up to max-min (without any +/- 1) can be encoded.
	  // The smallest element to encode is 0, not 1
	  // REMEMBER: It is assumed that the buffer memory passed to the compress() method is sufficient
	  // (but a check is also done here, assuming the increaseFactor was used correctly)
	  // This is ensured by (externally) using the increaseFactor
	  assert(sizeof(T)==sizeof(unsigned long));
	  const unsigned long lcode = (unsigned long) floor(vectorToCompress.size()*sizeof(T)*getIncreaseFactor() - sizeof(unsigned long)*(8*nchin + 3)); // A lower bound on the memory available (for error checking only)
	  assert(lcode > vectorToCompress.size()*sizeof(T));
	  unsigned long nb=0; // start writing at bit nb, increment afterwards

	  for(unsigned i=0; i<vectorToCompress.size(); i++)
	    // compresses to targetArray shifted by one byte??
	    { hufenc((unsigned long) vectorToCompress[i]-min,(unsigned char*) targetArray + sizeof(unsigned long)*(8*nchin + 3) -1,lcode,&nb,&huffmanStruct); }
	  assert(nb>0);

 // This decodes immediately (with a 'fresh' huffcode struct) and compares results
	  /*
#ifndef NDEBUG
      unsigned long nb2 = nb;
      huffcode huffmanStruct2;
      const T min2 = *((unsigned long*) targetArray + 0);
      assert(min2>=0); 
      huffmanStruct2.nch = *((unsigned long*) targetArray + 1);
      assert( *((unsigned long*) targetArray + 1) > 0);
      const unsigned long nchin2 = *((unsigned long*) targetArray + 1);
      assert(nchin2 == nchin);
      huffmanStruct2.nodemax = *((unsigned long*) targetArray + 2);
      assert( huffmanStruct2.nodemax ==  huffmanStruct.nodemax);
      huffmanStruct2.icod = (unsigned long*) targetArray +3;
      huffmanStruct2.ncod = (unsigned long*) targetArray +3+2*nchin2;
      huffmanStruct2.left = (unsigned long*) targetArray +3+4*nchin2;
      huffmanStruct2.right = (unsigned long*) targetArray +3+6*nchin2;
      cout << "\n start comparing (nchin = " << nchin << ")\n" << flush;

      for(unsigned i =0; i<2*nchin; i++)
	{
	  assert(*(huffmanStruct2.icod +i) == huffmanStruct.icod[i]);
	  assert(*(huffmanStruct2.ncod +i) == huffmanStruct.ncod[i]);
	  assert(*(huffmanStruct2.left +i) == huffmanStruct.left[i]);
	  assert(*(huffmanStruct2.right +i) == huffmanStruct.right[i]);
	}
      cout << "\n done comparing \n" << flush;

      // All tests above go through, i.e., the codes are the same

      // 2. Decode element by element
      nb2 = 0;
      register T temp;
      for(register unsigned long i=0; i<vectorToCompress.size(); i++)
	{
	  // unfortunately, have to add min to each element
	  // this slows down the process.
	  hufdec((long unsigned int*) &temp,(unsigned char*) targetArray-1 +  sizeof(unsigned long)*(8*nchin + 3),(unsigned long) ceil(vectorToCompress.size()*sizeof(T)*getIncreaseFactor()),&nb2, &huffmanStruct2);
	  if(temp+min2 != vectorToCompress[i])
	    {
	      cout << "\n ERROR ! : " << endl;
	      cout << " nb = " << nb << " = nof bits written by compressor" << endl;
	      cout << "  sizeof(unsigned long)*(8*nchin + 3) -1 " <<  sizeof(unsigned long)*(8*nchin + 3) -1 << endl;
	      cout << " compressed size in bytes: " << (size_t) (nb/8) + sizeof(unsigned long)*(8*nchin + 3) << endl;
	      cout << " vectorToCompress[i] = " << vectorToCompress[i] <<", temp+min2 = " << temp+min2 << ", min2 = " << min2 << ", i= " << i << endl;
	      cout << " size of vector (nofElements): " << vectorToCompress.size() << ", nb2 = " << nb2 << ", temp= " << temp << endl;
	      cout << " nb2/8 = " << nb2/8 <<  endl;
	      cout << " nchin2 = " << nchin2 << endl;
	      if(nb2 == nchin2) {cout << "\n ERROR: END OF MESSAGE !! \n" << endl;}

	    }
	  assert(temp+min2 == vectorToCompress[i]);
	}
#endif 
      */

      // SHOW DETAILED HUFFMAN STATISTICS if specified on command line (buildIndex -C)
      if (SHOW_HUFFMAN_STAT)
      {
        cout << endl << " Huffman compression statistics (for one array) " << endl;
        cout << setw(40) << " Number of elements : " << commaStr(vectorToCompress.size()) << endl;
        cout << setw(40) << " Range = max-min+1 : " << commaStr(max-min+1) << endl;
        cout << setw(40) << " Uncompressed size (as unlongs) : " << commaStr(vectorToCompress.size()*sizeof(T)) << " bytes" << endl;
        cout << setw(40) << " Compressed size : " << commaStr((size_t) ((nb+7)/8) + sizeof(unsigned long)*(8*nchin + 3)) << " bytes (including codebook) " << endl;
        cout << setw(40) << " Compressed size : " << commaStr((size_t) ((nb+7)/8)) << " bytes (WITHOUT codebook) " << endl;
        cout << setw(40) << " Size with (reduced) binary encoding : " << commaStr((unsigned long) ceil(vectorToCompress.size()*ceil(log2(max-min+1))/8)) << " bytes " << endl;
        cout << setw(40) << " Better : ";
        if((size_t) ((nb+7)/8) + sizeof(unsigned long)*(8*nchin + 3) <  ceil(vectorToCompress.size()*ceil(log2(max-min+1))/8) )
          { cout << " Huffman" << endl;}
        else { cout << " Trivial" << endl;}
      }

      free_lveCTor(huffmanStruct.icod,1,2*nchin-1);
      free_lveCTor(huffmanStruct.ncod,1,2*nchin-1);
      free_lveCTor(huffmanStruct.left,1,2*nchin-1);
      free_lveCTor(huffmanStruct.right,1,2*nchin-1);

        return (size_t) ((nb+7)/8) + sizeof(unsigned long)*(8*nchin + 3);
	} // end case: more than one distinct element to encode
      else
	{// begin case: nchin==1. If nching == 1, then nothing to do, as only one distinct element
	  return (size_t) sizeof(unsigned long)*3;
	}
	
    }// end: compress()




  // returns NOTHING
  // NUMBER OF ELEMENT of uncompressed array HAS TO BE KNOWN!!
  template <typename T> inline void decompress(const void* sourceArray, T* targetArray, unsigned long nofElements) const
    {
      assert(sizeof(long unsigned int)==sizeof(T));
      // encoded elements: (unsigned long) vectorToCompress[i]-min
      /* huffcode is the following struct:
	typedef struct {
	unsigned long *icod,*ncod,*left,*right,nch,nodemax;
	} huffcode;
      */

      // 1. Read Huffman tree from Array
      huffcode huffmanStruct;
      register const T min = *((unsigned long*) sourceArray + 0);
      assert(min>=0); 
      huffmanStruct.nch = *((unsigned long*) sourceArray + 1);
      assert( *((unsigned long*) sourceArray + 1) > 0);
      const unsigned long nchin = *((unsigned long*) sourceArray + 1);
      huffmanStruct.nodemax = *((unsigned long*) sourceArray + 2);

      /* begin case: more than one distinct element to decode */
      if(nchin>1)
      	{
      huffmanStruct.icod = (unsigned long*) sourceArray +3;
      huffmanStruct.ncod = (unsigned long*) sourceArray +3+2*nchin;
      huffmanStruct.left = (unsigned long*) sourceArray +3+4*nchin;
      huffmanStruct.right = (unsigned long*) sourceArray +3+6*nchin;

      // 2. Decode element by element
      unsigned long nb = 0;
      register T temp=0;
      const unsigned long lcode = (unsigned long) ceil(nofElements*sizeof(T)*getIncreaseFactor());

      for(register unsigned long i=0; i<nofElements; i++)
	{
	  //cout << "[" << i <<"]" << flush;
	  // unfortunately, have to add min to each element
	  // also, only element by element decoding is supported
	  // this slows down the process.
	  // -1 for "1-based" array
	  hufdec((long unsigned int*) &temp,(unsigned char*) sourceArray+ sizeof(unsigned long)*(8*nchin + 3) -1,lcode,&nb, &huffmanStruct);
	  targetArray[i] = temp+min;
	}
      assert(nb>0);
      	}// end case: more than one distinct element to decode

      /* begin case: nchin ==1, i.e. only one distinct element to decode */
      else
	{
	  for(register unsigned long i=0; i<nofElements; i++)
	    {
	      targetArray[i] = min;
	    }
	}// end case: only one disticnt element to decode  

    }// end: decompress()


}; // end: class HuffmanCompressionAlgorithm

#endif
