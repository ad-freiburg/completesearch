#ifndef __HYBCOMPLETER_H__
#define __HYBCOMPLETER_H__

#include <string>
#include <iostream>
#include "CompleterBase.h"
#include "Query.h"
#include "DocList.h"
#include "WordList.h"
#include "QueryResult.h"
#include "File.h"
#include "DummyCompressionAlgorithm.h"
//#include "HuffmanCompressionAlgorithm.h"
#include "Simple9CompressionAlgorithm.h"
#include "ZipfCompressionAlgorithm.h"
#include "TrivialCompressionAlgorithm.h"
#include "Vector.h"
#include "HYBIndex.h"

#include "../fuzzysearch/FuzzySearcher.h"

//! Needed for HybCompleter default constructor below
extern const vector<off_t>  emptyByteOffsetsForBlocks;
extern const Vector<WordId> emptyBoundaryWordIds;
extern off_t queryTimeout;


//! Main class for the completer based on the HYB index from Bast and Weber, SIGIR'06; derived from CompleterBase
template <unsigned char MODE> 
class HybCompleter : public CompleterBase<MODE>
{
  #define NUMBER_OF_LISTS_PER_BLOCK ( 2 + ((MODE & WITH_POS) ? (1) : (0)) + ((MODE & WITH_SCORES) ? (1) : (0)) )

 private:

  //! Compression algorithm to be used for the list of doc ids
  Simple9CompressionAlgorithm _doclistCompressionAlgorithm;
  //! Compression algorithm to be used for the list of positions
  Simple9CompressionAlgorithm _positionlistCompressionAlgorithm;
  //! Compression algorithm to be used for the list of word ids
  ZipfCompressionAlgorithm<WordId> _wordlistCompressionAlgorithm;
      //DummyCompressionAlgorithm _positionlistCompressionAlgorithm;
      //TrivialCompressionAlgorithm _wordlistCompressionAlgorithm;
      //HuffmanCompressionAlgorithm _wordlistCompressionAlgorithm;

  //! The offsets of the HYB blocks in the index file.
  // TODO(bast): relative to which position in the file?
  const vector<off_t>& _byteOffsetsForBlocks;

  // The ids of the first word in each HYB block.
  const Vector<WordId>& _boundaryWordIds;

 public:
  /// Default constructor. Useful for testing functions like intersect or
  // sortAndAggregateByWordId, where we do not actually need an index.
  // TODO(bast): causes a segmentation fault when delete[] _compressionBuffer
  // gets called in the destructor of CompleterBase, so should never be used and
  // eventually deleted.
  HybCompleter();

  //! Construct from given HYB Index and history (result cache)
  HybCompleter(
         HYBIndex*         indexData, 
	       TimedHistory*          passedHistory,
	       FuzzySearch::FuzzySearcherBase* fuzzySearcher,
	       unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT_DEFAULT, 
	       unsigned long int initUniBuffSize = UNION_BUFFER_INIT_DEFAULT,
	       unsigned long int initCompressBuffSize = COMPRESSION_BUFFER_INIT_DEFAULT);
  
  //! Copy constructor; TODO: used where?
  HybCompleter(const HybCompleter& orig);

  //! Destructor
  ~HybCompleter();

 private:

  // OLD (still from 2006 ... or older)
  //! COMPUTE COMPLETIONS AND HITS FOR GIVEN SET OF CANDIDATE DOCUMENTS AND GIVEN WORD RANGE
  /*
  void completionsForNonEmptyWordRange
	(      signed char  separatorMode, 
	 const WordRange&   wordRange, 
	       QueryResult& candidateResult, 
	       QueryResult* listsForPrefixFromHistory = NULL); 
  */
 

  //! Process basic prefix completion query (given doc ids D, and word range W)
  /*!
   *    Note: This is the HYB Index implementation of this abstract method from
   *    CompleterBase
   *
   *    \param inputList  the given posting list D
   *    \param wordRange  the given word range W
   *    \param result     the result list (gives D' and W')
   *    \param separator  determines the intersection mode
   */
  void processBasicQuery
        (const QueryResult& inputList,
         const WordRange&   wordRange,
               QueryResult& resultList,
         const Separator&   separator);


  //! Process basic prefix completion query; OLD IMPLEMENTATION BY INGMAR
  /*
  void allMatchesForWordRangeAndCandidates
        (const Separator&   splitSeparator,          //!< separator before last prefix 
               bool         notIntersectionMode,     //!< whether this is a "not" query or not
         const WordRange&   wordRange,               //!< the range W of word ids 
         const QueryResult& candidateLists,          //!< the input list of postings 
               QueryResult& matchingLists,           //!< the otput list of postings
         const QueryResult* listsForPrefixFromHistory =  NULL   //!< list of postings for word range, if available from history
        );
  */


 public:
    
  //! Compute range of block ids covering given word range
  /*
   *   Note: the range is [first, last] including both first and last!
   */
  void blockRangeForNonEmptyWordRange(const WordRange& wordRange, BlockId& first, BlockId& last) const;

  //! Read given block from disk; NEW INTERFACE: read into a QueryResult now
  /*!
   *    \param blockId  block id; must be less than [what?] 
   *    \param block    result lists; must be empty, exception thrown otherwise
   *
   *    TODO: don't call the old method, but integrate it into this method and
   *    then discard the old code.
   *
   *    TODO: copying of score list accounted for in resizeAndReserveTimer, is
   *    that a good idea? Also check whether the copying via reserve and push_back
   *    is really efficient.
   */
  void getDataForBlockId(BlockId blockId, QueryResult& block);
  
  //! Read block with given id from disk (doc ids, word ids, positions, scores)
  void getDataForBlockId(BlockId blockId, 
			 DocList& doclist, 
			 Vector<Position>& positionlist, 
			 Vector<DiskScore>& scorelist, 
			 WordList& wordlist);

 public:

    //! Get offsets of blocks in index file (needed by Holger for test-compression)
    const vector<off_t>& getByteOffsetsForBlocks() { return _byteOffsetsForBlocks; } 

    //! Print size (number of postings) of block with given id (for debugging)
    void printListLengthForBlockId(BlockId blockId);

    //! Print size (number of postings) for all blocks (for debugging)
    void printAllListLengths();

    //! Get data for all blocks and print it (for debugging)
    void getAllBlocks();

}; 

#endif
