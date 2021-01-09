#ifndef __INVCOMPLETER_H__
#define __INVCOMPLETER_H__

#include "INVIndex.h"
#include "CompleterBase.h"
#include "Query.h"
#include "DocList.h"
#include "QueryResult.h"
#include <string>
#include <iostream>
#include "File.h"
#include "Simple9CompressionAlgorithm.h"
#include "TrivialCompressionAlgorithm.h"
#include "DummyCompressionAlgorithm.h"


//! CLASS FOR INV INDEX (derived from CompleterBase)
//
//
template <unsigned char MODE> class INVCompleter : public CompleterBase<MODE>
{
  public:
  // ! DEFAULT CONSTRUCTOR
  //    INVCompleter(History& passedHistory);
  
  //! COPY CONSTRUCTOR
  INVCompleter(const INVCompleter& orig);

    const vector<off_t>& _byteOffsetsForDoclists;

  inline void getDataForWordId(WordId wordId, DocList& doclist, Vector<DiskScore>& scorelistForCurrentWord, Vector<Position>& positionlistForCurrentWord);

  private:

  // TODO: compression Algorithm could be a template parameter of class

  Simple9CompressionAlgorithm _doclistCompressionAlgorithm;
  Simple9CompressionAlgorithm _positionlistCompressionAlgorithm;
  /////  TrivialCompressionAlgorithm _positionlistCompressionAlgorithm;
  /////  DummyCompressionAlgorithm _positionlistCompressionAlgorithm;


  // NEW NEW NEW
    //  void allMatchesForWordRangeAndCandidates(unsigned char separatorMode, const WordRange& wordRange,
    void allMatchesForWordRangeAndCandidates(const Separator& splitSeparator, bool notIntersectionMode, const WordRange& wordRange,
                                           const QueryResult& candidateLists, QueryResult& matchingLists,
                                             const QueryResult* listsForPrefxFromHistory =  NULL);


    //! Compute completions and hits for given set of candidate documents and given word range
    /*!
     */
  // result is stored in buffers
  // candidateDocs no longer const due to push_back(INFTY_DOCID)
    void completionsForNonEmptyWordRange(unsigned char separatorMode, const WordRange& wordRange, QueryResult& candidateResult,  QueryResult* listsForPrefixFromHistory = (QueryResult*) NULL)
    {

          cout << "\n INV CURRENTLY NOT YET SUPPORTED!  " << endl << flush;
          exit(1);
          /*

      // if not using positions, have to use matchingDOcs
      // if using positions, have to use matchingDocsWithDuplicates ...
      // ... No! matchingDocsWithDuplicates is always used!
      DocList* candidateDocs = &candidateResult._docIds;
      assert((*candidateDocs).isSorted());

      assert((*candidateDocs).isFullList() || (*candidateDocs).size()>0);

      Vector<Position>* candidatePositions = &candidateResult._positions;
      //////      Vector<Score>* candidateScores = &candidateResult._scores; /// uncommented to surpress compiler warnings (as long as the interset signature in call is not changed)

      assert((MODE & WITH_POS) || ((*candidatePositions).size() == 0));

      assert(wordRange.size() > 0);

      clearBuffer();

      assert( (*_doclistIntersectionBuffer).size() == 0);
      assert( (*_wordlistIntersectionBuffer).size() == 0);
      assert( (*_doclistUnionBuffer).size() == 0);
      assert( (*_wordlistUnionBuffer).size() == 0);
      assert( (*_topWordScoresBuffer).size() == 0);


      //loop over wordRange
      DocList currentDoclist;
      Vector<Position> currentPositionlist;
      Vector<DiskScore> currentScorelist;

      //WordList dummyWordlist;

      //      cout << "\n WordRagne size = " << wordRange.size() << endl;

      for(signed long i=0; i<wordRange.size(); i++)
        {
	  // cout << "\n getting " << i << " of " << wordRange.size()-1 << " [" <<wordRange[i] <<"]\n" << flush;
          getDataForWordId(wordRange[i],currentDoclist, currentScorelist, currentPositionlist);
	  assert((!(MODE & WITH_SCORES))||(currentScorelist.isPositive()));

          assert(currentDoclist.size() >0);
          //          GlobalTimers::reserveTimer.cont();
          WordList dummyWordlist(wordRange[i],currentDoclist.size());
          //          GlobalTimers::reserveTimer.stop();

          assert(dummyWordlist.getNofElements() == currentDoclist.size());
          assert(dummyWordlist.getNofElements() > 0);

          assert((*_doclistIntersectionBuffer).capacity() > 0);

	  // intersect ONLY called here
          cout << "\n INV CURRENTLY NOT YET SUPPORTED: CHANGE SIGNATURE FOR INTERSECT CALL ! " << endl << flush;
          //          intersect(separatorMode, doNotCheckWordIds, LOCATION_SCORE_AGGREGATION, DiskScore(), (*candidateDocs), (*candidatePositions), (*candidateScores), currentDoclist, currentPositionlist, currentScorelist, dummyWordlist, WordRange(-1,1));
          exit(1);
        }
      // the bool is to indicate whether a sort (of the docs) is necessary
      // TODO 07March06: also include candidateScores in the list of arguments
      //      unite(MaxAggregation(), SumAggregation(), MaxAggregation(), SumAggregation(), wordRange.size() > 1, wordRange, candidateResult);
      cout << "\n INV CURRENTLY NOT YET SUPPORTED: REPLACE CALL TO UNITE BY TOP_COMPLETIONS_AND_HITS_FROM_ALL_MATCHES ! " << endl << flush;
      exit(1);
      //// unite(separatorMode , WITHIN_DOC_AGGREGATION, BETWEEN_DOCS_AGGREGATION, WITHIN_COMPLETION_AGGREGATION, BETWEEN_COMPLETIONS_AGGREGATION, BETWEEN_BLOCKS_AGGREGATION, wordRange.size() > 1, wordRange, candidateResult);
          
      return;
          */
    } // end: completionsForNonEmptyWordRange


 public:
   void printAllListLengths()
     {
       for(unsigned long int i=0;i<(unsigned long int) CompleterBase<MODE>::_metaInfo.getNofWords();i++)
	 {
	   printListLengthForWordId((WordId) i); 
	 }
     }


 private:
   void printListLengthForWordId(WordId wordId);

   Timer getDoclistsTimer;

  public:
   suseconds_t getGetDoclistsTimer() const {return getDoclistsTimer.value();}

    void resetTimers()
      {
        CompleterBase<MODE>::resetTimers();
        getDoclistsTimer.reset();
      }

    void resetCounters()
      {
        CompleterBase<MODE>::resetCounters();
      }

    void showPerformanceInfo(ostringstream& outputStream) const ;
 

   // for test purposes only. Not used in algorithm
    void getAllDoclists();

   //! CONSTRUCT INVCompleter from <db>.inverted and <db>.vocabulary
   //
   //   INVCompleter(const string indexStructureFile, const string vocabularyFile, History& passedHistory, unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT_DEFAULT, unsigned long int initUniBuffSize = UNION_BUFFER_INIT_DEFAULT, unsigned long int initCompressBuffSize = COMPRESSION_BUFFER_INIT_DEFAULT);


   //! CONSTRUCT INVCompleter from Index
   //
   INVCompleter(INVIndex& indexData, History& passedHistory, unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT_DEFAULT, unsigned long int initUniBuffSize = UNION_BUFFER_INIT_DEFAULT, unsigned long int initCompressBuffSize = COMPRESSION_BUFFER_INIT_DEFAULT);

    
   //! DESTRUCTOR
   ~INVCompleter()
   {
     if (CompleterBase<MODE>::_indexStructureFile.isOpen()) { CompleterBase<MODE>::_indexStructureFile.close(); }   
     CompleterBase<MODE>::freeCompressionBuffer();
   }
   
}; // class INVCompleter





  // ! DEFAULT CONSTRUCTOR
  //template <unsigned char MODE>
  //INVCompleter<MODE>::INVCompleter(History& passedHistory) : CompleterBase<MODE>::CompleterBase(passedHistory) {}


   //! CONSTRUCT INVCompleter from Index
   //
template <unsigned char MODE>
  INVCompleter<MODE>::INVCompleter(INVIndex& indexData, History& passedHistory, unsigned long int initIntBuffSize, unsigned long int initUniBuffSize, unsigned long int initCompressBuffSize) : CompleterBase<MODE>::CompleterBase(passedHistory, indexData._vocabulary, indexData.getIndexFileName(), indexData._metaInfo), _byteOffsetsForDoclists(indexData._byteOffsetsForDoclists)
     {
       CompleterBase<MODE>::setInitialBufferSizes( initIntBuffSize, initUniBuffSize, initCompressBuffSize);
       CompleterBase<MODE>::reserveCompressionBuffersAndResetPointersAndCounters();
     }


    //! COPY CONSTRUCTOR
template <unsigned char MODE>
  INVCompleter<MODE>::INVCompleter(const INVCompleter<MODE>& orig) : CompleterBase<MODE>::CompleterBase(orig), _byteOffsetsForDoclists(orig._byteOffsetsForDoclists)
      {

      }



template <unsigned char MODE>
  void INVCompleter<MODE>::showPerformanceInfo(ostringstream& outputStream) const 
      {

#define FIRSTFIELD 30 

        double timeInCompletions =  CompleterBase<MODE>::getCompletionsForQueryTimer(); // NOT divided by 1000*1000

        outputStream << "* performance info for INVCompleter:" << endl << endl;
        outputStream << setw(6) << CompleterBase<MODE>::getNofQueries() << " queries answered in total \n";
        outputStream << setw(6) << CompleterBase<MODE>::getNofQueriesFromHistory() << " queries answered from history\n\n";

        
        outputStream << setw(FIRSTFIELD) << left << " completionsForQuery" << setw(8) << right << alignedDbl((double) CompleterBase<MODE>::getCompletionsForQueryTimer()/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(5) << alignedDbl(100*CompleterBase<MODE>::getCompletionsForQueryTimer()/timeInCompletions ,1) << "% )\n";
        outputStream << setw(FIRSTFIELD) << left << " completionsForWordRange" << setw(8) << right << alignedDbl((double) CompleterBase<MODE>::getCompletionsForWordRangeTimer()/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(5) << alignedDbl(100*CompleterBase<MODE>::getCompletionsForWordRangeTimer()/timeInCompletions ,1) << "% )\n\n";


        //      outputStream << setw(6) << alignedDbl (100*getCompletionsForWordRangeTimer()/timeInCompletions,1) << "% in completionsForWordrange\n";
        outputStream << " Coarse breakdown of times (no overlap between items): \n  ----------------------------------------- \n" ;

        double timeInGetting = getGetDoclistsTimer();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " reading+uncompressing+copy" << setw(8) << right << alignedDbl(timeInGetting/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInGetting/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) (CompleterBase<MODE>::getDoclistVolumeDecompressed()+CompleterBase<MODE>::getWordlistVolumeDecompressed())*1000*1000/(1024*1024*timeInGetting),1) << " MB/sec)\n";        

        double timeInIntersect = CompleterBase<MODE>::getIntersectionTimer();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " intersections" << setw(8) << right << alignedDbl(timeInIntersect/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInIntersect/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) CompleterBase<MODE>::getIntersectedVolume()*1000*1000/((double) 1024*1024*timeInIntersect),1) << " MB/sec)   [inputsize]\n";

        double timeInUniting = CompleterBase<MODE>::getUniteTimer(); // NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " sort+unique" << setw(8) << right << alignedDbl(timeInUniting/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*CompleterBase<MODE>::getUniteTimer()/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) (CompleterBase<MODE>::getDoclistUniteVolume()+CompleterBase<MODE>::getWordlistUniteVolume())*1000*1000/( 1024*1024*timeInUniting),1) << " MB/sec)   [inputsize]\n";

        double timeInHistory = CompleterBase<MODE>::getHistoryTimer();
        outputStream << setw(FIRSTFIELD) << left << " history" << setw(8) << right << alignedDbl(timeInHistory/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInHistory/timeInCompletions ,1) << "% )\n";

        double timeInPrefixToRange = CompleterBase<MODE>::getPrefixToRangeTimer();
        outputStream << setw(FIRSTFIELD) << left << " prefix to wordRange" << setw(8) << right << alignedDbl(timeInPrefixToRange/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInPrefixToRange/timeInCompletions ,1) << "% )\n";

        double timeInWriteBuffer = CompleterBase<MODE>::getWriteBuffersToQueryResultTimer();
        outputStream << setw(FIRSTFIELD) << left << " write buffer to result" << setw(8) << right << alignedDbl(timeInWriteBuffer/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInWriteBuffer/timeInCompletions ,1) << "% )\n";

        outputStream << "  ---------------------------------------------------\n";
        outputStream << "  ---------------------------------------------------\n";

        double totalTime = timeInGetting + timeInIntersect + timeInUniting + timeInHistory + timeInPrefixToRange + timeInWriteBuffer;
        outputStream << setw(FIRSTFIELD) << left << " total" << setw(8) << right << alignedDbl(totalTime/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*totalTime/timeInCompletions ,1) << "% )\n";


        outputStream << "  ---------------------------------------------------\n\n\n";

        outputStream << " Detailed breakdown of times: \n  ----------------------------------------- \n" ;

        double timeInStlSort =  CompleterBase<MODE>::stlSortTimer.value();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " stl sort   [docs]" << setw(8) << right << alignedDbl(timeInStlSort/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInStlSort/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) CompleterBase<MODE>::getDoclistUniteVolume()*1000*1000/(1024*1024*timeInStlSort),1) << " MB/sec)   [inputsize]\n";

        double timeInUnique =  CompleterBase<MODE>::uniqueTimer.value();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " stl unique [docs+words]" << setw(8) << right << alignedDbl(timeInUnique/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInUnique/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) (CompleterBase<MODE>::getDoclistUniteVolume()+CompleterBase<MODE>::getWordlistUniteVolume())*1000*1000/(1024*1024*timeInUnique),1) << " MB/sec)   [inputsize]\n";

        double timeInRead = CompleterBase<MODE>::getFileReadTimer();
        outputStream << setw(FIRSTFIELD) << left << " reading from file" << setw(8) << right << alignedDbl(timeInRead/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInRead/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) CompleterBase<MODE>::getVolumeReadFromFile()*1000*1000/(1024*1024*timeInRead),1) << " MB/sec)\n";

        double timeInDoclistUncompress = CompleterBase<MODE>::getDoclistDecompressionTimer();
        outputStream << setw(FIRSTFIELD) << left << " uncompress doclists" << setw(8) << right << alignedDbl(timeInDoclistUncompress/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInDoclistUncompress/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) CompleterBase<MODE>::getDoclistVolumeDecompressed()*1000*1000/(1024*1024*timeInDoclistUncompress),1) << " MB/sec)   [outputsize]\n";

        double timeInUsePointer = CompleterBase<MODE>::usePointerTimer.value();
        outputStream << setw(FIRSTFIELD) << left << " usePointer" << setw(8) << right << alignedDbl(timeInUsePointer/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInUsePointer/timeInCompletions ,1) << "% )   (" << setw(7) << right <<  alignedDbl((double) (CompleterBase<MODE>::getDoclistVolumeDecompressed()+CompleterBase<MODE>::getWordlistVolumeDecompressed())*1000*1000/(1024*1024*timeInUsePointer),1) << " MB/sec)\n";

        double timeInAppend =  CompleterBase<MODE>::appendTimer.value();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " append to buffer" << setw(8) << right << alignedDbl(timeInAppend/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInAppend/timeInCompletions ,1) << "% ) \n";

        double timeInCopy =  CompleterBase<MODE>::copyTimer.value();// NOT divided by 1000*1000
        outputStream << setw(FIRSTFIELD) << left << " copying Vector(s)" << setw(8) << right << alignedDbl(timeInCopy/(1000*1000)) << " secs  ( ";
        outputStream << right << setw(4) << alignedDbl(100*timeInCopy/timeInCompletions ,1) << "% ) \n";

        outputStream << "\n\n";
        
        outputStream << setw(8) << CompleterBase<MODE>::getNofIntersections() << " intersections computed (including trivial ones) \n";
        outputStream << setw(8) << CompleterBase<MODE>::getNofUnions() << " unions computed (including trivial ones) \n";
        outputStream << setw(8) << CompleterBase<MODE>::getNofBlocksReadFromFile() << " blocks (=lists) read from file\n";
        outputStream << setw(8) << alignedDbl( (double) CompleterBase<MODE>::getVolumeReadFromFile()/CompleterBase<MODE>::getNofBlocksReadFromFile(),1) << " Bytes per blocks (on average)\n";

      }// end: showPerformanceInfo(..)



template <unsigned char MODE>
  inline void INVCompleter<MODE>::getDataForWordId(WordId wordId, DocList& doclist, Vector<DiskScore>& scorelistForCurrentWord, Vector<Position>& positionlistForCurrentWord)
     {

     /* TODO: INTRODUCED POSITIONS + SCORES HERE ! */
       // TODO: FIRST READ AN INDEX WITH OFFSETS!!
       off_t offsetForDoclist = 0;
       off_t offsetForPositionlist = 0;
       off_t offsetForScorelist = 0;
       ++CompleterBase<MODE>::nofBlocksReadFromFile;
       getDoclistsTimer.cont();
       assert(wordId >= 0);
       //              cout << "\n getting doclist for wordid : " << wordId << flush;
       //       assert(doclist.isEmpty());
       assert((WordId) CompleterBase<MODE>::_metaInfo.getNofWords() > wordId);
       assert(_byteOffsetsForDoclists.size() > (unsigned long) wordId);

       assert(wordId+1 < (WordId) _byteOffsetsForDoclists.size());

       //       cout << "\n size in bytes to read from disk: " << _byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId] -sizeof(unsigned long)<< "\n" << flush;       
       if(_byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId] > (size_t) CompleterBase<MODE>::compressionBufferSize())
         {
           CompleterBase<MODE>::resizeCompressionBuffer(2*((size_t)_byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId]));
         }

       assert(CompleterBase<MODE>::compressionBufferSize() >= _byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId] -sizeof(unsigned long));
       assert(_byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId]-sizeof(unsigned long) > 0);


       /* random initilizatino (used for error checking only) */
       unsigned long  nofDocsInCompressedList=0;
       unsigned long nofPositionsInCompressedList=1;
       unsigned long nofScoresInList=0;

      unsigned char nofListsForWord = 1;
      if(MODE & WITH_POS) nofListsForWord++;
      if(MODE & WITH_SCORES) nofListsForWord++;

       /* Read compressed doclist from file */
      CompleterBase<MODE>::fileReadTimer.cont(); ///////

       // the read here has to to a seek
       if((MODE & WITH_POS) || (MODE & WITH_SCORES))
         {
           CompleterBase<MODE>::_indexStructureFile.read(&offsetForDoclist, sizeof(off_t), _byteOffsetsForDoclists[wordId]);
           assert(offsetForDoclist >  _byteOffsetsForDoclists[wordId]);
           assert(offsetForDoclist == (nofListsForWord*sizeof(off_t) + _byteOffsetsForDoclists[wordId]));
           
         }
       else {CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList, sizeof(unsigned long), _byteOffsetsForDoclists[wordId]);}
       
       if(MODE & WITH_POS) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForPositionlist, sizeof(off_t)); assert(offsetForPositionlist > offsetForDoclist+sizeof(unsigned long));}
       if(MODE & WITH_SCORES) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForScorelist, sizeof(off_t)); assert(offsetForScorelist > offsetForDoclist+sizeof(unsigned long)); assert(offsetForScorelist > sizeof(unsigned long) + offsetForPositionlist);}
       
       if((MODE & WITH_POS) || (MODE & WITH_SCORES))
         {
           assert(nofDocsInCompressedList == 0);
           CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList, sizeof(unsigned long));
         }
       assert(nofDocsInCompressedList > 0);
       // READ DOCLIST
       if((!(MODE & WITH_SCORES)) && (!(MODE & WITH_POS)))
         {
           assert (  _byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId]-sizeof(unsigned long)  > 0);
           CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer,(size_t) _byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId]-sizeof(unsigned long));
         }
       else if(MODE & WITH_POS)
         {
           assert( offsetForPositionlist-offsetForDoclist-sizeof(unsigned long) > 0);
           CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer,(size_t) offsetForPositionlist-offsetForDoclist-sizeof(unsigned long));
         }
       else
         {
           assert(  offsetForScorelist-offsetForDoclist-sizeof(unsigned long) > 0 );
           CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer,(size_t) offsetForScorelist-offsetForDoclist-sizeof(unsigned long));
         }
       
       // READ POS LIST
       if(MODE & WITH_POS)
         {
           CompleterBase<MODE>::_indexStructureFile.read(&nofPositionsInCompressedList, sizeof(unsigned long));
           assert(nofPositionsInCompressedList > 0);
           assert(nofPositionsInCompressedList == nofDocsInCompressedList);
           assert( _byteOffsetsForDoclists[wordId+1] > offsetForPositionlist+sizeof(unsigned long));
           if(MODE & WITH_SCORES)
             {
               CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer+(offsetForPositionlist-offsetForDoclist-sizeof(unsigned long)),(size_t) offsetForScorelist - offsetForPositionlist-sizeof(unsigned long));
             }
           else
             {
               CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer+(offsetForPositionlist-offsetForDoclist-sizeof(unsigned long)),(size_t) _byteOffsetsForDoclists[wordId+1] - offsetForPositionlist-sizeof(unsigned long));
             }
         }
       // READ SCORE LIST
       if(MODE & WITH_SCORES)
         {
           CompleterBase<MODE>::_indexStructureFile.read(&nofScoresInList, sizeof(unsigned long));
           assert(nofScoresInList > 0);
           assert(nofScoresInList == nofDocsInCompressedList);
           assert( _byteOffsetsForDoclists[wordId+1] > offsetForScorelist);
           scorelistForCurrentWord.resize(nofScoresInList);
           assert(nofScoresInList * sizeof(DiskScore) ==  _byteOffsetsForDoclists[wordId+1] - offsetForScorelist -sizeof(unsigned long));
           assert(scorelistForCurrentWord.isContiguous());
           CompleterBase<MODE>::_indexStructureFile.read(&scorelistForCurrentWord.Vector<DiskScore>::operator[](0),(size_t) _byteOffsetsForDoclists[wordId+1] - offsetForScorelist -sizeof(unsigned long));
           assert(scorelistForCurrentWord.isContiguous());
           assert(scorelistForCurrentWord.isPositive());
           CompleterBase<MODE>::scorelistVolumeRead += nofScoresInList*sizeof(DiskScore);
         }

       CompleterBase<MODE>::fileReadTimer.stop(); ///////


       assert(nofDocsInCompressedList > 0);
       CompleterBase<MODE>::volumeReadFromFile += (_byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId] );

       // * UNCOMPRESS DOC LIST *
       CompleterBase<MODE>::resizeAndReserveTimer.cont();
       doclist.resize(nofDocsInCompressedList);
       CompleterBase<MODE>::resizeAndReserveTimer.stop();

       CompleterBase<MODE>::doclistDecompressionTimer.cont();
       _doclistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer, &doclist[0], nofDocsInCompressedList );
       CompleterBase<MODE>::doclistDecompressionTimer.stop();

       assert(doclist.isContiguous());
       assert(((!(MODE & WITH_POS ) ) && doclist.isSorted(true)) || ((MODE & WITH_POS) && (doclist.isSorted() )));
       CompleterBase<MODE>::doclistVolumeDecompressed += (nofDocsInCompressedList*sizeof(DocId));
       // * UNCOMPRESS POS LIST *
       if(MODE & WITH_POS)   
         {   
           CompleterBase<MODE>::resizeAndReserveTimer.cont();
           positionlistForCurrentWord.resize(nofPositionsInCompressedList);
           CompleterBase<MODE>::resizeAndReserveTimer.stop();
           CompleterBase<MODE>::positionlistDecompressionTimer.cont();
           _positionlistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer +(offsetForPositionlist-offsetForDoclist-sizeof(unsigned long)), &positionlistForCurrentWord[0], nofPositionsInCompressedList,2);
           CompleterBase<MODE>::positionlistDecompressionTimer.stop();

           assert(positionlistForCurrentWord.isContiguous());
           CompleterBase<MODE>::positionlistVolumeDecompressed += (nofPositionsInCompressedList*sizeof(Position));
         }
       getDoclistsTimer.stop();

     }//end: getDataForWOrdId(..)


template <unsigned char MODE>
  void INVCompleter<MODE>::printListLengthForWordId(WordId wordId)
     {
      DocList doclist;
       Vector<DiskScore> scorelistForCurrentWord;
       Vector<Position> positionlistForCurrentWord;

       /* TODO: INTRODUCED POSITIONS + SCORES HERE ! */
       // TODO: FIRST READ AN INDEX WITH OFFSETS!!
       off_t offsetForDoclist = 0;
       off_t offsetForPositionlist = 0;
       off_t offsetForScorelist = 0;

       assert(wordId >= 0);
       //              cout << "\n getting doclist for wordid : " << wordId << flush;
       //       assert(doclist.isEmpty());
       assert((WordId) CompleterBase<MODE>::_metaInfo.getNofWords() > wordId);
       assert(_byteOffsetsForDoclists.size() > (unsigned long) wordId);

       assert(wordId+1 < (WordId) _byteOffsetsForDoclists.size());

  
         assert(_byteOffsetsForDoclists[wordId+1]-_byteOffsetsForDoclists[wordId]-sizeof(unsigned long) > 0);


       /* random initilizatino (used for error checking only) */
       unsigned long  nofDocsInCompressedList=0;


      unsigned char nofListsForWord = 1;
      if(MODE & WITH_POS) nofListsForWord++;
      if(MODE & WITH_SCORES) nofListsForWord++;

       /* Read compressed doclist from file */

       // the read here has to to a seek
       if((MODE & WITH_POS) || (MODE & WITH_SCORES))
	 {
	   CompleterBase<MODE>::_indexStructureFile.read(&offsetForDoclist, sizeof(off_t), _byteOffsetsForDoclists[wordId]);
	   assert(offsetForDoclist >  _byteOffsetsForDoclists[wordId]);
	   assert(offsetForDoclist == (nofListsForWord*sizeof(off_t) + _byteOffsetsForDoclists[wordId]));

	 }
       else {CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList, sizeof(unsigned long), _byteOffsetsForDoclists[wordId]);}

       if(MODE & WITH_POS) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForPositionlist, sizeof(off_t)); assert(offsetForPositionlist > offsetForDoclist+sizeof(unsigned long));}
       if(MODE & WITH_SCORES) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForScorelist, sizeof(off_t)); assert(offsetForScorelist > offsetForDoclist+sizeof(unsigned long)); assert(offsetForScorelist > sizeof(unsigned long) + offsetForPositionlist);}

       if((MODE & WITH_POS) || (MODE & WITH_SCORES))
	 {
	   assert(nofDocsInCompressedList == 0);
	   CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList, sizeof(unsigned long));
	 }
       assert(nofDocsInCompressedList > 0);

       cout << nofDocsInCompressedList << endl;
     } // end:printListLengthForWordId(...)


template <unsigned char MODE>
  void INVCompleter<MODE>::allMatchesForWordRangeAndCandidates(const Separator& splitSeparator, bool notIntersectionMode, const WordRange& wordRange,
                                           const QueryResult& candidateLists, QueryResult& matchingLists,
                                           const QueryResult* listsForPrefxFromHistory)
    {

   // if not using positions, have to use matchingDocs
      // if using positions, have to use matchingDocsWithDuplicates ...
      // ... No! matchingDocsWithDuplicates is always used!
      //      assert(!errorStatus);
      assert(candidateLists._docIds.isSorted());
      assert(candidateLists._docIds.isFullList() || candidateLists._docIds.size()>0);
      assert((MODE & WITH_POS) || (candidateLists._positions.size() == 0));
  #ifndef NDEBUG
  signed char separatorMode = splitSeparator._separatorIndex;
  #endif
      assert(!notIntersectionMode);
      assert(notIntersection());
      assert(!andIntersection());
      assert( (separatorMode != FULL) || (!notIntersectionMode));


      assert(wordRange.size() > 0);

      //      CompleterBase<MODE>::clearBuffer();

      assert( (*CompleterBase<MODE>::_doclistIntersectionBuffer).size() == 0);
      assert( (*CompleterBase<MODE>::_wordlistIntersectionBuffer).size() == 0);
      assert( (*CompleterBase<MODE>::_doclistUnionBuffer).size() == 0);
      assert( (*CompleterBase<MODE>::_wordlistUnionBuffer).size() == 0);
      assert( (*CompleterBase<MODE>::_topWordScoresBuffer).size() == 0);


      //loop over wordRange
      DocList currentDoclist;
      Vector<Position> currentPositionlist;
      Vector<DiskScore> currentScorelist;

      //WordList dummyWordlist;

      //      cout << "\n WordRagne size = " << wordRange.size() << endl;

      //      assert(!errorStatus);

      for(signed long i=0; i<wordRange.size(); i++)
        {
	  // cout << "\n getting " << i << " of " << wordRange.size()-1 << " [" <<wordRange[i] <<"]\n" << flush;
          getDataForWordId(wordRange[i],currentDoclist, currentScorelist, currentPositionlist);
	  assert((!(MODE & WITH_SCORES))||(currentScorelist.isPositive()));


          assert(currentDoclist.size() >0);
          WordList dummyWordlist(wordRange[i],currentDoclist.size());

          assert(dummyWordlist.getNofElements() == currentDoclist.size());
          assert(dummyWordlist.getNofElements() > 0);

          assert((*CompleterBase<MODE>::_doclistIntersectionBuffer).capacity() > 0);

          // intersect ONLY called here
          #ifndef NDEBUG
          cout << "! not intersectionMode in INVCompleter (just before intersect): " << andIntersection() << endl;
          #endif
          CompleterBase<MODE>::intersect(splitSeparator, doNotCheckWordIds, andIntersection, LOCATION_SCORE_AGGREGATION, DiskScore(),
                    candidateLists,
                    currentDoclist, currentPositionlist, currentScorelist, dummyWordlist,
                    matchingLists, WordRange(-1,1));
        }

      // SORT THE LISTS IF NECESSARY
      if (wordRange.size() > 1)
        {
          #ifndef NDEBUG
          cout << "! start sorting/merging lists" << endl;
          #endif
          CompleterBase<MODE>::invMergeTimer.cont();
          matchingLists.sortLists();
          CompleterBase<MODE>::invMergeTimer.stop();
          #ifndef NDEBUG
          cout << "! done sorting/merging lists" << endl;
          #endif
        }
      //      assert(!errorStatus);

    }//end: allMatchesForWordRangeAndCandidates(...)


   // for test purposes only. Not used in algorithm
template <unsigned char MODE>
  void INVCompleter<MODE>::getAllDoclists()
     {
       assert(CompleterBase<MODE>::_metaInfo.getNofWords() > 0);
       DocList dummyDoclist;

       CompleterBase<MODE>::_metaInfo.getNofWords();

       
       for(unsigned long i=0; i< MIN((unsigned long)CompleterBase<MODE>::_metaInfo.getNofWords(), 10); i++)
         {
           CompleterBase<MODE>::getDoclistForWordId(i,dummyDoclist); 
           dummyDoclist.clear();
         }
       
     } // end: getAllDoclists


#endif
