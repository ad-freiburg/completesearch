#include "HYBIndex.h"
#include "../synonymsearch/SynonymDictionary.h"

// HACK(bast): Define stuff for fuzzy search and synonym search here. If
// fuzzySearchEnabled is true and synonym is true read the respective files and
// do the respective intializations in the HYBIndex constructor (at the end of
// this file).
// TODO(bast): Think about how to do this in a cleaner way.

extern bool fuzzySearchEnabled;
extern bool normalizeWords;
extern string baseName;

/*
int fuzzySearchNeighbourhoodSize = 50;
double fuzzySearchEditDistanceThreshold = 0.28;
FuzzySearch::ClosestWordsFinder* fuzzySearcher = NULL;
FuzzySearch::PermutedLexicon fuzzySearchPermutedLexicon;
vector<string> fuzzySearchClusterCenters;
vector<vector<int> > fuzzySearchClusterIdsPerClusterCenter;
bool fuzzySearchNormalizeWords = false;
*/

bool synonymSearchEnabled = false;
SynonymDictionary synonymDictionary;

void HYBIndex::showMetaInfo() const
{
  _metaInfo.show();
}

void HYBIndex::writeCurrentBlockToIndexFile(DocList& doclistForCurrentBlock, 
                                  WordList& wordlistForCurrentBlock, 
                                  Vector<DiskScore>& scorelistForCurrentBlock, 
                                  Vector<Position>& positionlistForCurrentBlock, 
                                  BlockId currentBlock,
                                  File& indexStructureFile)
{
  #define NUMBER_OF_LISTS_PER_BLOCK ( 2 + ((MODE & WITH_POS) ? (1) : (0)) + ((MODE & WITH_SCORES) ? (1) : (0)) )
  #define SIZE_COMPRESSION_BUFFER_FOR_POSITIONS ((MODE & WITH_POS) ? \
              (2*sizeof(Position)*MAX(_positionlistCompressionAlgorithm.getIncreaseFactor()*positionlistForCurrentBlock.size(),1000)) : (0))
  // Scores are currently NOT compressed

  assert((!(MODE & WITH_POS)) || (MODE & WITH_DUPS));
  assert(((!(MODE & WITH_POS ))&& (positionlistForCurrentBlock.size()==0 )) || ((MODE & WITH_POS) && (positionlistForCurrentBlock.size()==doclistForCurrentBlock.size()) ));
  assert(((!(MODE & WITH_SCORES ))&& (scorelistForCurrentBlock.size()==0 )) || ((MODE & WITH_SCORES) && (scorelistForCurrentBlock.size()==doclistForCurrentBlock.size()) ));
  assert((!(MODE & WITH_SCORES ))|| (scorelistForCurrentBlock.isPositive() ));
  // SORT THE WORD-DOC PAIRS BY DOC ID 
  sortTimer.cont(); 

  if((!(MODE & WITH_POS)) && (!(MODE & WITH_SCORES ))) {doclistForCurrentBlock.sortParallel(wordlistForCurrentBlock);}
  else if ((MODE & WITH_POS ) && (!(MODE & WITH_SCORES))) {doclistForCurrentBlock.sortParallel(positionlistForCurrentBlock, wordlistForCurrentBlock);}
  else if ((!(MODE & WITH_POS)) && (MODE & WITH_SCORES)) {doclistForCurrentBlock.sortParallel(scorelistForCurrentBlock, wordlistForCurrentBlock);}
  else {
    assert((MODE & WITH_POS) && (MODE & WITH_SCORES));
    doclistForCurrentBlock.sortParallel(positionlistForCurrentBlock, wordlistForCurrentBlock, scorelistForCurrentBlock);
    assert(((!(MODE & WITH_SCORES ))&& (scorelistForCurrentBlock.size()==0 )) || ((MODE & WITH_SCORES) && (scorelistForCurrentBlock.size()==doclistForCurrentBlock.size()) ));
    assert((!(MODE & WITH_SCORES ))|| (scorelistForCurrentBlock.isPositive() ));
  }
  sortTimer.stop();


 // RESIZE COMPRESSION LIST IF APPROPRIATE
  if( sizeof(DocId)*(MAX(doclistForCurrentBlock.size()*_doclistCompressionAlgorithm.getIncreaseFactor(),1000))        \
      + wordlistForCurrentBlock.size()*sizeof(WordId)*_wordlistCompressionAlgorithm.getIncreaseFactor()   \
      +  SIZE_COMPRESSION_BUFFER_FOR_POSITIONS >    compressionBufferSize() )
    { resizeCompressionBuffer( (unsigned long) ceil(1.3*( sizeof(DocId)*(MAX(doclistForCurrentBlock.size()*_doclistCompressionAlgorithm.getIncreaseFactor(),1000))        \
                             + wordlistForCurrentBlock.size()*sizeof(WordId)*_wordlistCompressionAlgorithm.getIncreaseFactor()   \
                             + SIZE_COMPRESSION_BUFFER_FOR_POSITIONS) ));}

 // COMPRESS DOC LIST
  doclistCompressionTimer.cont();

  //  const size_t compressedDoclistSize = 0;
  const size_t compressedDoclistSize = _doclistCompressionAlgorithm.compress(doclistForCurrentBlock, _compressionBuffer);
  assert(compressedDoclistSize > 0);
  doclistCompressionTimer.stop();
  doclistVolumeCompressed += compressedDoclistSize;

 // COMPRESS POSITION LIST
  size_t compressedPositionlistSize = 0;
  if(MODE & WITH_POS) {
    positionlistCompressionTimer.cont();
    compressedPositionlistSize  = _positionlistCompressionAlgorithm.                                         \
    compress(positionlistForCurrentBlock, _compressionBuffer +compressedDoclistSize,2);//2 indicates: gaps with boundaries
    assert(compressedPositionlistSize > 0);
    positionlistCompressionTimer.stop();
    positionlistVolumeCompressed += compressedPositionlistSize;}

  // DECOMPRESS DOC LIST FOR ERROR CHECKING ONLY
 #ifndef NDEBUG
  DocList uncompressedDocs;
  uncompressedDocs.resize(doclistForCurrentBlock.size());
  _doclistCompressionAlgorithm.decompress(_compressionBuffer,&uncompressedDocs[0],doclistForCurrentBlock.size());
     for(unsigned int i=0;i<doclistForCurrentBlock.size();i++)
       {
         assert(uncompressedDocs[i] == doclistForCurrentBlock[i]);
       }
  #endif


  // DECOMPRESS POSITION LIST FOR ERROR CHECKING ONLY
 #ifndef NDEBUG
 if(MODE & WITH_POS)
   {
     assert(positionlistForCurrentBlock.size() > 0);
     Vector<Position> uncompressedPositions;
     uncompressedPositions.resize(positionlistForCurrentBlock.size());
     ////     assert(*((unsigned long*) (CompleterBase<MODE>::_compressionBuffer + compressedDoclistSize  )) > positionlistForCurrentBlock.size()  ); // only hols for simple9 with gaps==2 and non-trivial list
     _positionlistCompressionAlgorithm.decompress(_compressionBuffer + compressedDoclistSize,&uncompressedPositions[0],positionlistForCurrentBlock.size() ,2);//2 indicates: gaps with artificial boundaries
     for(unsigned int i=0;i<positionlistForCurrentBlock.size();i++)
       {
         assert(uncompressedPositions[i] == positionlistForCurrentBlock[i]);
       }
   }
  #endif


  // COMPRESS WORD LIST
  #ifndef NDEBUG
  for(unsigned int i=0; i<wordlistForCurrentBlock.size();i++) {assert((currentBlock==0)||(wordlistForCurrentBlock[i]>0));}
  #endif
  wordlistCompressionTimer.cont();
  const size_t compressedWordlistSize                                                                                               \
   = _wordlistCompressionAlgorithm.                                                                                                \
       compress(wordlistForCurrentBlock,_compressionBuffer + compressedDoclistSize+compressedPositionlistSize);
  assert(compressedWordlistSize > 0);
  assert(compressedDoclistSize + compressedPositionlistSize + compressedWordlistSize <= compressionBufferSize());
  wordlistCompressionTimer.stop();
  wordlistVolumeCompressed += compressedWordlistSize;

  // DECOMPRESS WORD LIST FOR ERROR CHECKING ONLY
  #ifndef NDEBUG
  WordList uncompressedWords;
  uncompressedWords.resize(wordlistForCurrentBlock.size());
  _wordlistCompressionAlgorithm.decompress(_compressionBuffer + compressedDoclistSize + compressedPositionlistSize,&uncompressedWords[0],wordlistForCurrentBlock.size());
     for(unsigned int i=0;i<wordlistForCurrentBlock.size();i++)
       {
         assert(uncompressedWords[i] == wordlistForCurrentBlock[i]);
       }
  #endif

  const size_t scorelistSize = sizeof(DiskScore)*scorelistForCurrentBlock.size();
  assert(((MODE & WITH_SCORES) && (scorelistSize >0)) || ((!(MODE & WITH_SCORES)) && (scorelistSize == 0 )));
  scorelistVolumeWritten += scorelistSize;// in bytes 

    // WRITE TO INDEX FILE
  const off_t offsetForDoclist = NUMBER_OF_LISTS_PER_BLOCK*sizeof(off_t) + _byteOffsetsForBlocks.back();
  const off_t offsetForPositionlist = (off_t) sizeof(unsigned long int) + compressedDoclistSize + offsetForDoclist;// not written if no positions are used
  const off_t offsetForWordlist = (off_t) ((MODE & WITH_POS) ? (2) : (1))*sizeof(unsigned long int) + compressedDoclistSize + compressedPositionlistSize + offsetForDoclist;
  const off_t offsetForScorelist = (off_t) sizeof(unsigned long int) + offsetForWordlist + compressedWordlistSize;

  _byteOffsetsForBlocks.push_back( (off_t) NUMBER_OF_LISTS_PER_BLOCK*sizeof(off_t)          \
                                    + NUMBER_OF_LISTS_PER_BLOCK*sizeof(unsigned long int)   \
                                    + compressedDoclistSize                                 \
                                    + compressedPositionlistSize                            \
                                    + compressedWordlistSize                                \
                                    + scorelistSize                                         \
                                    + _byteOffsetsForBlocks.back());
  assert(_byteOffsetsForBlocks.size()>=2);
  assert(_byteOffsetsForBlocks[_byteOffsetsForBlocks.size()-1] > _byteOffsetsForBlocks[_byteOffsetsForBlocks.size()-2]);

  // the following four are identical. just clearer this way
  const unsigned long int lengthOfCurrentDoclist  = (unsigned long int) doclistForCurrentBlock.size();
  const unsigned long int lengthOfCurrentPositionlist = (unsigned long int) positionlistForCurrentBlock.size();
  const unsigned long int lengthOfCurrentWordlist = (unsigned long int) wordlistForCurrentBlock.size();
  const unsigned long int lengthOfCurrentScorelist = (unsigned long int) scorelistForCurrentBlock.size();

  assert(lengthOfCurrentDoclist > 0);
  assert(lengthOfCurrentDoclist == lengthOfCurrentWordlist);
  assert((!(MODE & WITH_POS))||(lengthOfCurrentDoclist == lengthOfCurrentPositionlist));
  assert(offsetForWordlist > offsetForDoclist);
  assert((!(MODE & WITH_POS)) || (offsetForWordlist > (off_t) sizeof(unsigned long int) + offsetForPositionlist));
  assert((!(MODE & WITH_POS)) || (offsetForDoclist + (off_t) sizeof(unsigned long int) < offsetForPositionlist));
  writeToDiskTimer.cont();

  // WRITE OFFSETS
  // ... FOR DOCLIST
  indexStructureFile.write(&offsetForDoclist,sizeof(off_t));
  // ... FOR POSITIONLIST
  if(MODE & WITH_POS) {indexStructureFile.write(&offsetForPositionlist,sizeof(off_t));}
  // ... FOR WORDLIST
  indexStructureFile.write(&offsetForWordlist,sizeof(off_t));
  // ... FOR SCORELIST
  if(MODE & WITH_SCORES) {indexStructureFile.write(&offsetForScorelist,sizeof(off_t));}

  // WRITE DOCLIST
  assert(offsetForDoclist == indexStructureFile.tell());
  indexStructureFile.write(&lengthOfCurrentDoclist,sizeof(unsigned long int));
  indexStructureFile.write(_compressionBuffer,compressedDoclistSize);
  // WRITE POSITIONLIST
  if(MODE & WITH_POS)
    {
      assert(offsetForPositionlist == indexStructureFile.tell());
      assert(offsetForPositionlist == offsetForDoclist + (off_t) sizeof(unsigned long int) + (off_t) compressedDoclistSize);
      indexStructureFile.write(&lengthOfCurrentPositionlist,sizeof(unsigned long int));
      indexStructureFile.write(_compressionBuffer+compressedDoclistSize,compressedPositionlistSize);
    }
  // WRITE WORDLIST
  assert(offsetForWordlist == indexStructureFile.tell());
  indexStructureFile.write(&lengthOfCurrentWordlist,sizeof(unsigned long int));
  assert((MODE & WITH_POS) || (compressedPositionlistSize == 0 ));
  indexStructureFile.write(_compressionBuffer+compressedDoclistSize+compressedPositionlistSize,compressedWordlistSize);
  // WRITE SCORELIST
  if(MODE & WITH_SCORES)
    {
      assert(offsetForScorelist == indexStructureFile.tell());
      indexStructureFile.write(&lengthOfCurrentScorelist,sizeof(unsigned long int));
      assert(scorelistForCurrentBlock.isContiguous());
      indexStructureFile.write(&scorelistForCurrentBlock[0],scorelistSize);
    }
  writeToDiskTimer.stop();

  // clear calls free  ->  memory fragmentation
  //doclistForCurrentBlock.clear();
  //wordlistForCurrentBlock.clear();
  //positionlistForCurrentBlock.clear();//
  //scorelistForCurrentBlock.clear();//
  
  doclistForCurrentBlock.resize(0);
  wordlistForCurrentBlock.resize(0);
  positionlistForCurrentBlock.resize(0);//
  scorelistForCurrentBlock.resize(0);

  #ifdef SIZE_COMPRESSION_BUFFER_FOR_POSITIONS
  #undef SIZE_COMPRESSION_BUFFER_FOR_POSITIONS
  #endif

  } // end: writeCurrentBlockToIndexFile(..)

void HYBIndex::build(const string& wordsFileName, const string& format)
{
  cout << endl 
        << "* build HYB index from file \"" << wordsFileName << "\"" << " in format " << format 
        << endl << endl;

   // INIT TIMERS AND COUNTERS (declared in HYBCompleter above)
   buildIndexTimer.start();
   readFromDiskTimer.reset();
   writeToDiskTimer.reset();
   sortTimer.reset();
   doclistCompressionTimer.reset();
   positionlistCompressionTimer.reset();
   wordlistCompressionTimer.reset();
   resetCounters();

   // NEW 08Oct06 (Holger): three modes now for how to choose block sizes (BY_PREFIX_SIZE is new)
   // HYB_BOUNDARY_WORDS_FILE_NAME and HYB_BLOCK_VOLUME declared in Globals.h and set in buildIndex.cpp
   enum { BY_PREFIXES, BY_PREFIX_SIZE, BY_VOLUME } howToChooseBlocks; 
   if (HYB_BOUNDARY_WORDS_FILE_NAME != "") 
   {
     howToChooseBlocks = BY_PREFIXES;
     cout << "* form blocks according to prefixes from file \"" << HYB_BOUNDARY_WORDS_FILE_NAME << "\"" << endl;
   }
   else if (HYB_BLOCK_VOLUME < 10) 
   {
     howToChooseBlocks = BY_PREFIX_SIZE;
     cout << "* one block for each prefix of size " << HYB_BLOCK_VOLUME << endl;
   }
   else 
   {
     howToChooseBlocks = BY_VOLUME;
     cout << "* form blocks of volume approximately " << commaStr(HYB_BLOCK_VOLUME) << endl << endl;
   }

   // MORE INITIALIZATION
   reserveCompressionBuffersAndResetPointersAndCounters(); // this also reserves the compression buffer
   File indexStructureFile(_indexFileName.c_str(), "wb");
   string nextPrefix; /* nextPrefix iterates over the prefixes. Once it matches 
                         the beginning of a new word, a new block is created */
   // the first element must be larger than the first word
   // a new block is started once a word gets larger/equal to the current prefix
   Vocabulary boundaryPrefixes;
   boundaryPrefixes.reserve(800000);// an attempt to avoid unnecessary dynamic memory handling
   unsigned long prefixIndex = 0;

   // READ BLOCK BOUDANRIES FROM FILE (buildIndex with -b filename option)
   if (howToChooseBlocks == BY_PREFIXES)
   {
     //cout << "* reading block boundary words from file \"" << HYB_BOUNDARY_WORDS_FILE_NAME << "\" ... ";
     readWordsFromFile(HYB_BOUNDARY_WORDS_FILE_NAME, boundaryPrefixes, "prefixes");
     assert(boundaryPrefixes.size() > 0);
     nextPrefix = boundaryPrefixes[prefixIndex];
     
     // This last artificial prefix needs to be alphabetically before the very
     // last prefix this ensures, that all of the last terms are attributed to
     // the last prefix in fact enough if the at the end no word looks like this
     // exactly.
     // TODO(bast): bad hack by Ingmar, can't this be done in a cleaner way?
     boundaryPrefixes.push_back("0000NO_WORD0000000000"); 
   }

   WordsFile wordsFile(wordsFileName.c_str());
   if (format == "ASCII") wordsFile.setFormat(WordsFile::FORMAT_HTDIG);
   else if (format == "BINARY") wordsFile.setFormat(WordsFile::FORMAT_BINARY);
   wordsFile.setSkipLineWithSameWordAndDoc(MODE & WITH_POS ? false : true);
   // NOTE(bast, 8Jul11): Max doc id by default was no longer maintained after
   // r244 (where Hannah re-factored WordsFile). However, DBLP produces
   // non-consecutive doc ids, and since r244 until now was displaying the wrong
   // "number of documents" in the UI (> 2M instead of 1.xM). So if you change
   // this in the future, or add a command-line argument for this, make sure
   // that for DBLP this option is set to true.
   wordsFile.setMaintainSetOfDistinctDocIds(true);

   // If binary format, load the vocabulary (otherwise we can easily construct
   // it from the words file). Otherwise reserve some space for it.
   assert(_vocabulary.size() == 0);
   if (wordsFile.formatIsBinary()) readWordsFromFile(_vocabularyFileName, _vocabulary, "vocabulary"); 
   else _vocabulary.reserve(10 * 1000 * 1000);

   unsigned long nofTokens = 0;

   string word; 
   string previousWord;
   WordId previousWordId = -1;
   WordId wordId = 0;
   BlockId blockId = 0;
   Position location;
   DocId docId;
   DiskScore score;
  
   unsigned long currentVolume = 0;

   _byteOffsetsForBlocks.push_back(0);

   DocList doclistForCurrentBlock;
   WordList wordlistForCurrentBlock;
   // only used if(MODE & WITH_POS)
   Vector<Position> positionlistForCurrentBlock;
   Vector<DiskScore> scorelistForCurrentBlock;

   _boundaryWordIds.push_back(0);     

   #define INITIAL_BLOCK_SIZE (10*1000*1000)
   doclistForCurrentBlock.reserve(INITIAL_BLOCK_SIZE);
   wordlistForCurrentBlock.reserve(INITIAL_BLOCK_SIZE);
   if (MODE & WITH_POS) { positionlistForCurrentBlock.reserve(INITIAL_BLOCK_SIZE); }
   if (MODE & WITH_SCORES) { scorelistForCurrentBlock.reserve(INITIAL_BLOCK_SIZE); }
   #ifdef INITIAL_BLOCK_SIZE
   #undef INITIAL_BLOCK_SIZE
   #endif


   //
   // MAIN LOOP : process words file line by line writing out block after block
   //
   bool eofReached = false;
   MSG_BEG = " ";  /* let buffer resize messages come nicely */
   MSG_END = " "; /* between the [...] progress indicator */
   bool shownIgnoreMessage = false;
   // NEW(Hannah): do not show the block boundaries anymore.
   // cout << endl; 
   #define WORD(x) (x)
   while (true)
   {
     // Get next line (duplicate lines skipped if _skipLineWithSameWordAndDoc).
     bool ret = wordsFile.getNextLine(word, wordId, docId, score, location);

     // If getNextLine failed -> either end of line, or parse error or duplicate
     // line skipped.
     if (ret == false)
     {
       if (not wordsFile.isEof()) 
       {
         continue;
       }
       // TODO(bast): What is supposed to happen in this case?
       else
       {
         if (wordsFile.getLineNumber() == 1)
         {
           cerr << "! words file is empty" << endl << endl;
           exit(1);
         }
         currentVolume = ULONG_MAX;
         word = "";
         nextPrefix = "";  // Ensures match even if prefixes are used.
         eofReached = true;
       } 
     }

     // If format is binary, get word from vocabulary. If format is not binary,
     // compute word id at this point (easy, since words file is sorted by
     // words). Check that in binary format word ids increase by at most one,
     // and that in ascii format, words are indeed sorted in ascending order.
     if (wordsFile.formatIsBinary())
     {
       if (wordId == previousWordId) 
       {
         word = previousWord;
       }
       else 
       {
         if (wordId != previousWordId + 1)
         {
           if (previousWordId == -1)
           {
             cerr << "! First word id must be 0 (is " << wordId 
                  << ") at record #" << wordsFile.getLineNumber() 
                  << endl << endl;
           }
           else
           {
             cerr << "! Non-consecutive word ids in binary words file ("
                  << previousWordId << " -> " << wordId
                  << ") at record #" << wordsFile.getLineNumber() 
                  << endl << endl;
           }
           // exit(1);
         }
         // TODO(hoffmaje): Segmentation fault at this point, when running
         // buildIndex on data that contains non text characters. On my test
         // data the segfault occurs right in the last run of the loop wraped
         // around all this.
         assert(wordId < static_cast<int>(_vocabulary.size()));
         word = _vocabulary[wordId];
       }
     }
     else
     {
       // After the call of  wordsFile.getNextLine(word, ...) above, 
       // word is set to the empty string if eof is reached.
       // In this case we produce one word ID more than we have different words.
       // This caused a "Assertion `numWords == wordId + 1' failed." assertion
       // Hence we check for the empty string here: 
       if (word != "")
       {
         if (word == previousWord) 
         {
           wordId = previousWordId;
         }
         else 
         {
           if (!eofReached && word < previousWord)
           {
             cerr << "! Words not in sorted order in words file ("
                  << previousWord << " -> " << word 
                  << ") at line#" << wordsFile.getLineNumber() 
                  << endl << endl;
             exit(1);
           }
           wordId = previousWordId + 1;
         }
       }
     }

     // A new word was encountered or end of file.
     if (wordId != previousWordId || eofReached)
     {
       // If first word of first block, print it.
       // NEW(Hannah): do not show the block boundaries anymore.
       // if (wordId == 0) cout << "[" << printable(WORD(word)) << " - " << flush;

       // For mode BY_PREFIX_SIZE, nextPrefix is actually the prefix of the *current* block
       if (howToChooseBlocks == BY_PREFIX_SIZE && wordId == 0) nextPrefix = WORD(word).substr(0, HYB_BLOCK_VOLUME);
       if (howToChooseBlocks == BY_PREFIX_SIZE && !eofReached) assert(nextPrefix.size() > 0);

       // If block is finished, write it to disk.
       if (eofReached
           || (howToChooseBlocks == BY_PREFIXES && WORD(word).substr(0, nextPrefix.length()) == nextPrefix) 
           || (howToChooseBlocks == BY_PREFIX_SIZE && WORD(word).substr(0, nextPrefix.length()) != nextPrefix)
           || (howToChooseBlocks == BY_PREFIX_SIZE && nextPrefix.size() < HYB_BLOCK_VOLUME && WORD(word).size() >= HYB_BLOCK_VOLUME)
           || (howToChooseBlocks == BY_VOLUME && previousWord != "" && currentVolume >= HYB_BLOCK_VOLUME))
       {
         // Write block to disk.
         if (currentVolume > 0)
         {
           // NEW(Hannah): do not show the block boundaries anymore.
           // cout << printable(WORD(previousWord)) << "]" << flush;
           writeCurrentBlockToIndexFile(doclistForCurrentBlock, wordlistForCurrentBlock, 
      	                                scorelistForCurrentBlock, positionlistForCurrentBlock, blockId, indexStructureFile);
           blockId++;
         }

         // If not end of file, current word starts a new block.
         if (!wordsFile.isEof())
         {
           if (currentVolume > 0)
           {
             _boundaryWordIds.push_back(wordId);
             // NEW(Hannah): do not show the block boundaries anymore.
             // cout << "[" << printable(WORD(word)) << " - " << flush;
           }
           if (howToChooseBlocks == BY_PREFIXES)
           {
             prefixIndex++;
             assert(prefixIndex < boundaryPrefixes.size());
             string previousPrefix = nextPrefix;
             nextPrefix = boundaryPrefixes[prefixIndex];
             if (prefixIndex + 1 < boundaryPrefixes.size() && nextPrefix < previousPrefix)
             {
               cerr << "! boundary prefixes not in sorted order (\"" 
                    << previousPrefix << "\" -> \"" << nextPrefix << "\")"
                    << " at prefix #" << prefixIndex << endl << endl;
               exit(1);
             }
           }                     
           if (howToChooseBlocks == BY_PREFIX_SIZE)
           {
             nextPrefix = WORD(word).substr(0, HYB_BLOCK_VOLUME);
           }
         }

         // Newly started block has volume zero.
         currentVolume = 0;
       }

       // Add word to vocabulary (unless empty or reading from binary file)
       if (!wordsFile.formatIsBinary() && word != "") addWordToVocabulary(word);
       previousWord = word;
       previousWordId = wordId;
       shownIgnoreMessage = false;
     } 

     // If end of file, exit loop now (we have just written the last block then).
     if (eofReached) break;

     // Add the current posting to the current block, except when maxBlockVolume
     // is reached (which, by default, is infinity).
     if (doclistForCurrentBlock.size() < maxBlockVolume)
     {
       doclistForCurrentBlock.push_back(docId);
       wordlistForCurrentBlock.push_back(wordId);
       if (MODE & WITH_POS) { positionlistForCurrentBlock.push_back(location); } 
       if (MODE & WITH_SCORES) { /*assert(score>0);*/ scorelistForCurrentBlock.push_back(score); } 
       currentVolume++;
       nofTokens++;
     }
     // If block volume is already at maxBlockVolume, show message (but only
     // once, not per ignored posting, hence the shownIgnoreMessage).
     else if (shownIgnoreMessage == false)
     {
       cout << "{WARNING: list volume has reached " << commaStr(maxBlockVolume)
            << ", ignoring remaining items from this list, current word is: "
            << EMPH_ON << WORD(word) << EMPH_OFF << "}" << flush;
       shownIgnoreMessage = true;
     }

   }  // End of main loop reading the words file line by line / record by record.
   
   //cout << "\n size of boundary wordsIds : " <<  _boundaryWordIds.size() << "\n id of current block : " << currentBlock;
   assert(  _boundaryWordIds.size() == blockId);
   assert(_vocabulary.size() == wordId + 1);
   _byteOffsetsForBlocks.push_back((off_t) sizeof(WordId)*_boundaryWordIds.size()+ _byteOffsetsForBlocks.back());
   indexStructureFile.writeVector(_boundaryWordIds,WordId());
     //off_t sizeOfCompressedLists = indexStructureFile.tell();
    
   // WRITE META INFO
   _metaInfo.setMaxDocID(wordsFile.maxDocId());
   _metaInfo.setNofDocs(wordsFile.numDocs());
   _metaInfo.setNofWords(_vocabulary.size());
   _metaInfo.setNofWordInDocPairs(nofTokens);
   _metaInfo.setNofBlocks(blockId);
   assert(_metaInfo.getNofBlocks() + 2 == _byteOffsetsForBlocks.size());
   writeMetaInfo(&indexStructureFile);

   // WRITE OFFSET OF INDEX TABLE (and then close file)
   writeToDiskTimer.cont();
   off_t offsetForIndexTable = indexStructureFile.tell();
   indexStructureFile.writeVector(_byteOffsetsForBlocks, off_t());
   assert(indexStructureFile.tell() > offsetForIndexTable);
   indexStructureFile.write(&offsetForIndexTable,sizeof(off_t));
   indexStructureFile.flush();
   writeToDiskTimer.stop();
   buildIndexTimer.stop();
   freeCompressionBuffer();

   // WRITE VOCABULARY to separate file (not for BINARY!)
   // NEW(Hannah): do not show the block boundaries anymore.
   // cout << endl << endl;
   if (format != "BINARY") writeVocabularyToFile();
   else cout << "! no vocabulary written when reading BINARY format" << endl;
   cout << endl;     
   
   // SHOW TIMINGS AND STATISTICS 
   off_t fileSizeInBytes = indexStructureFile.tell();
   showBuildStatistics(fileSizeInBytes);

   #undef WORD
}

void HYBIndex::showBuildStatistics(off_t fileSizeInBytes)
{
  unsigned long nofTokens = _metaInfo.getNofWordInDocPairs();
  DocId numDocs = _metaInfo.getNofDocs();
  WordId numWords = _metaInfo.getNofWords();

  cout.setf(ios::fixed);
  cout.precision(2);
#define HF1 setw(40) << right
#define HF2T " : " << setw(7) << right << setprecision(2)
#define HF2N " : " << setw(14) << right
#define HF2S " : " << setw(6) << right << setprecision(1)
  cout << EMPH_ON << "TIMINGS" << EMPH_OFF << endl << endl;
  cout << HF1 << "total time to build index" << HF2T << buildIndexTimer.secs() << " seconds" << endl;
  cout << HF1 << "time for reading from disk" << HF2T << "----" << " not measured (would slow down indexing by 50%)" << endl;
  //readFromDiskTimer.secs() << " seconds" << endl
  cout << HF1 << "time for writing to disk" << HF2T << writeToDiskTimer.secs() << " seconds";
  if (writeToDiskTimer.usecs() == 0) cout << endl;
  else cout   << " (" << fileSizeInBytes/writeToDiskTimer.usecs() << " MiB per second)" << endl;
  cout << HF1 << "time for sorting" << HF2T << sortTimer.secs() << " seconds";
  if (sortTimer.usecs() == 0) cout << endl;
  else cout   << " (" << setw(2) << nofTokens/sortTimer.usecs() 
    << " million word-in-doc pairs per second)" << endl;
  cout << HF1 << "time for compressing doclists" << HF2T << doclistCompressionTimer.secs() << " seconds";
  if (doclistCompressionTimer.usecs() == 0) cout << endl;
  else cout   << " (" << setw(2) << nofTokens/doclistCompressionTimer.usecs() 
    << " million doc ids per second)" << endl;
  cout << HF1 << "time for compressing wordlists" << HF2T << wordlistCompressionTimer.secs() << " seconds";
  if (wordlistCompressionTimer.usecs() == 0) cout << endl;
  else cout   << " (" << setw(2) << nofTokens/wordlistCompressionTimer.usecs() 
    << " million word ids per second)" << endl;
  cout << HF1 << "time for compressing positionlists" << HF2T << positionlistCompressionTimer.secs() << " seconds";
  if (positionlistCompressionTimer.usecs() == 0) cout << endl;
  else cout   << " (" << setw(2) << nofTokens/positionlistCompressionTimer.usecs()
    << " million position ids per second)" << endl;
  cout << endl;
  assert(_metaInfo.getNofBlocks() > 0); 
  assert(nofTokens > 0); 

  cout << EMPH_ON << "STATISTICS" << EMPH_OFF << endl << endl
    << HF1 << "number of documents" << HF2N << commaStr(numDocs) << endl
    << HF1 << "number of words" << HF2N << commaStr(numWords) << endl
    << HF1 << "number of word-in-doc pairs" << HF2N << commaStr(nofTokens) << endl
    << HF1 << "number of blocks" << HF2N << commaStr(_metaInfo.getNofBlocks()) << endl
    << HF1 << "average block volume" << HF2N << commaStr(nofTokens/_metaInfo.getNofBlocks()) << endl
    << endl
    << HF1 << "total index file size" << HF2S << fileSizeInBytes/(1.0*1024*1024) << " MB" 
    << " (" << setw(4) << setprecision(1) 
    << 8.0*fileSizeInBytes/nofTokens << " bits per word-in-doc pair)" << endl 
    << HF1 << "total size of doclists" << HF2S << doclistVolumeCompressed/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*doclistVolumeCompressed/nofTokens << " bits per doc id)" << endl 
    << HF1 << "total size of wordlists" << HF2S << wordlistVolumeCompressed/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*wordlistVolumeCompressed/nofTokens << " bits per word id)" << endl
    << HF1 << "total size of positionlists" << HF2S << positionlistVolumeCompressed/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*positionlistVolumeCompressed/nofTokens << " bits per position id)" << endl
    << HF1 << "total size of scorelists" << HF2S << scorelistVolumeWritten/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*scorelistVolumeWritten/nofTokens << " bits per score id)" << endl;
  cout << endl;
}

HYBIndex::~HYBIndex()
{
  #ifndef NDEBUG
  cout << endl << endl << "************** HYBINDEX DESTRUCTED ***********" << endl << endl;
  #endif
}

HYBIndex::HYBIndex(const string& passedIndexStructureFile, const string& passedVocabularyFile, unsigned char mode)
  : IndexBase(passedIndexStructureFile, passedVocabularyFile, mode)
{
}

void HYBIndex::read()
{
   File indexStructureFile(_indexFileName.c_str(), "rb");
   
   off_t indexTableOffset;

   const off_t lastOffsetOffset = indexStructureFile.getLastOffset(indexTableOffset);

   assert(indexTableOffset > 0);

   assert(lastOffsetOffset > indexTableOffset + (off_t) sizeof(off_t));

   // Read vocabulary. NEW(Hannah, 10Jul10): If fuzzy search enabled (C:...
   // words) or synonym search enabled (S:... words), precompute word id map.
   readWordsFromFile(_vocabularyFileName, _vocabulary);
   if (fuzzySearchEnabled || synonymSearchEnabled || normalizeWords)
     _vocabulary.precomputeWordIdMap();

   readBlockOffsets(&indexStructureFile, lastOffsetOffset, indexTableOffset);
   readBoundaryWordIds(&indexStructureFile);
   readMetaInfo(&indexStructureFile, indexTableOffset);

  // HACK(Hannah 6Nov09): if file name for fuzzy search data structure specified
  // on command line (via -Y option), then read it in here and have the
  // fuzzySearcher object ready for processing fuzzy search queries.
  // TODO: declare and create this object at the proper place.
   /*
  if (fuzzySearchEnabled)
  {
    fuzzySearcher = new FuzzySearch::ClosestWordsFinder(fuzzySearchEditDistanceThreshold);
    // Set word normalization (true or false, given by command line option).
    fuzzySearcher->useNormalization(fuzzySearchNormalizeWords);
    if (fuzzySearcher->usingNormalization())
      cout << "* NEW FUZZY: using word normalization" << endl;
    // Use prefix edit distance instead of normal edit distance. For example,
    // prefix edit distance of audo to autocompleteion is 3, while the normal
    // edit distance from audo to autocompletion is 12.
    fuzzySearcher->setAutocompletion(true);
    // Set the encoding.
    // TODO(bast): this is the same code used at the beginning of
    // CompletionServer.cpp. Unfortunately, this code here gets executed before
    // the code in CompletionServer.cpp, so we can't use the globale variable
    // encoding here yet. However, here is certainly not the right place to set
    // the global encoding for the first time.
    size_t pos = localeString.find(".");
    string encoding = pos != string::npos ? localeString.substr(pos + 1) : "UTF8";
    for (size_t i = 0; i < encoding.size(); ++i) encoding[i] = toupper(encoding[i]);
    if (encoding == "UTF8") fuzzySearcher->setEncoding(FuzzySearch::ClosestWordsFinder::UTF_8);
    else if (encoding == "ISO88591") fuzzySearcher->setEncoding(FuzzySearch::ClosestWordsFinder::ISO_8859_1);
    else fuzzySearcher->setEncoding(FuzzySearch::ClosestWordsFinder::UTF_8);
    // Load the fuzzy search data structure.
    cout << "* NEW FUZZY: " << flush;
    string fuzzySearchDataStructureFileName = baseName +
                                              ".fuzzysearch-datastructure";
    fuzzySearcher->loadPermutedLexiconFromFile(
	fuzzySearchDataStructureFileName,
	&fuzzySearchPermutedLexicon,
	&fuzzySearchClusterCenters);
    // Load the cluster ids (one list for each cluster center).
    string fuzzySearchClusterIdsFileName = baseName + ".fuzzysearch-clusters";
    cout << "* NEW FUZZY: " << flush;
    fuzzySearcher->readClusterIds(fuzzySearchClusterIdsFileName,
	                          fuzzySearchClusterCenters,
				  &fuzzySearchClusterIdsPerClusterCenter);
    cout << "* NEW: edit distance threshold is " << fuzzySearchEditDistanceThreshold << endl;
  }
  */

  // Optionally read synonym dictionary.
  if (synonymSearchEnabled)
  {
    string synonymDictionaryFileName = baseName + ".synonym-groups";
    cout << "* NEW SYN: read synonym groups from \"" 
         << synonymDictionaryFileName << "\"" << endl;
    synonymDictionary.readFromFile(synonymDictionaryFileName);
  }

}

void HYBIndex::writeMetaInfo(File *file)
{
   char metaInfoBuffer[_metaInfo.getSizeInBytes()];
   _metaInfo.writeToBuffer(metaInfoBuffer);       
   file->write(metaInfoBuffer, _metaInfo.getSizeInBytes());
}

void HYBIndex::readBlockOffsets(File* file, off_t lastOffsetOffset,
    off_t indexTableOffset)
{
  cout << "* reading hybrid index block offsets from file "
    << file->getFileName() << " ... " << flush;

  Timer timer;
  timer.start();

  assert(((size_t) lastOffsetOffset - indexTableOffset) /(sizeof(off_t))*sizeof(off_t) ==
      (size_t) lastOffsetOffset - indexTableOffset);

  file->readVector(_byteOffsetsForBlocks,
      static_cast<size_t>(lastOffsetOffset - indexTableOffset),
      indexTableOffset, off_t());

  timer.stop();
  cout << "* done in " << timer << ", #offsets = "
    << _byteOffsetsForBlocks.size() << endl;

  assert(_byteOffsetsForBlocks.size() > 2);
  assert(_byteOffsetsForBlocks[_byteOffsetsForBlocks.size() - 1] >
      _byteOffsetsForBlocks[_byteOffsetsForBlocks.size() - 2]);
}

void HYBIndex::readBoundaryWordIds(File* file)
{
  file->readVector(_boundaryWordIds,
      (size_t) (_byteOffsetsForBlocks[_byteOffsetsForBlocks.size()-1] -_byteOffsetsForBlocks[_byteOffsetsForBlocks.size()-2]),
      _byteOffsetsForBlocks[_byteOffsetsForBlocks.size()-2], WordId());
}

void HYBIndex::readMetaInfo(File* file, off_t indexTableOffset)
{
   char metaInfoReadBuffer[indexTableOffset - _byteOffsetsForBlocks.back()];

   file->read(metaInfoReadBuffer,
       (size_t) indexTableOffset - _byteOffsetsForBlocks.back(),
       _byteOffsetsForBlocks.back());

   _metaInfo.readFromBuffer(metaInfoReadBuffer);

   // HOLGER 25Jan06: if _nofWords is zero (was a bug in buildIndex), set to size of vocabulary.
   if (_metaInfo.getNofWords() == 0)
   {
     _metaInfo.setNofWords(_vocabulary.size());
     cout << "! NOTICE : number of words in meta info was zero, set it to vocabulary size "
          << commaStr(_metaInfo.getNofWords()) << endl;
   }
}
