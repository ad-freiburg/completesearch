#include "INVIndex.h"

//! Write inverted list to file; TODO: true? (was not commented before)
void INVIndex::writeCurrentDataToIndexFile(DocList& doclistForCurrentWord, Vector<DocId>& doclistLengthsForWordId, Vector<DiskScore>& scorelistForCurrentWord, Vector<Position>& positionlistForCurrentWord, File& indexStructureFile)
{
  assert(_byteOffsetsForDoclists.back() == indexStructureFile.tell());
  // TODO: FIRST WRITE AN INDEX WITH OFFSETS
  #define SIZE_COMPRESSION_BUFFER_FOR_POSITIONS ((MODE & WITH_POS) ?       \
    (positionlistForCurrentWord.size()*sizeof(Position)*_positionlistCompressionAlgorithm.getIncreaseFactor()) : (0))

  assert((!(MODE & WITH_SCORES)) || scorelistForCurrentWord.isPositive());

  // CHECK SORTEDNESS
  assert(((MODE & WITH_POS) && doclistForCurrentWord.isSorted()) || ( !(MODE & WITH_POS) && doclistForCurrentWord.isSorted(true)  ));
  unsigned char nofListsForWord = 1;
  if(MODE & WITH_POS) nofListsForWord++;
  if(MODE & WITH_SCORES) nofListsForWord++;


  assert((!(MODE & WITH_SCORES)) || (scorelistForCurrentWord.size() == doclistForCurrentWord.size()));
  assert((!(MODE & WITH_POS)) || (positionlistForCurrentWord.size() == doclistForCurrentWord.size()));

  off_t offsetForDoclist = 0;
  off_t offsetForPositionlist = 0;
  off_t offsetForScorelist = 0;

  // RESIZE COMPRESSION LIST IF APPROPRIATE
  if( doclistForCurrentWord.size()*sizeof(DOCID)*_doclistCompressionAlgorithm.getIncreaseFactor()
      + SIZE_COMPRESSION_BUFFER_FOR_POSITIONS > (compressionBufferSize()))
    resizeCompressionBuffer( 
	(long unsigned int) 
	ceil(2*( doclistForCurrentWord.size()*sizeof(DOCID)*_doclistCompressionAlgorithm.getIncreaseFactor()
	    + SIZE_COMPRESSION_BUFFER_FOR_POSITIONS)) );



  // COMPRESS DOC LIST
  doclistCompressionTimer.cont();
  // doc ids are converted to gaps
  const size_t compressedDoclistSize = _doclistCompressionAlgorithm.compress(doclistForCurrentWord, _compressionBuffer);
  doclistCompressionTimer.stop();
  assert(compressedDoclistSize > 0);
  assert(compressedDoclistSize <= compressionBufferSize());
  doclistVolumeCompressed += compressedDoclistSize;

  offsetForDoclist = nofListsForWord*sizeof(off_t) + _byteOffsetsForDoclists.back();
  assert(offsetForDoclist > _byteOffsetsForDoclists.back());

  size_t compressedPositionlistSize = 0;
  if(MODE & WITH_POS)
  {
    // COMPRESS POS LIST
    positionlistCompressionTimer.cont();
    // position ids are converted to gaps, with 0 in between. The 2 indicates this.
    compressedPositionlistSize = _positionlistCompressionAlgorithm.compress(positionlistForCurrentWord, _compressionBuffer+compressedDoclistSize,2);
    offsetForPositionlist = offsetForDoclist + sizeof(unsigned long) + compressedDoclistSize;
    positionlistCompressionTimer.stop();
    assert(compressedPositionlistSize > 0);
    assert(compressedPositionlistSize <= compressionBufferSize());
    positionlistVolumeCompressed += compressedPositionlistSize;
  }

  writeToDiskTimer.cont();
  if((MODE & WITH_SCORES) || (MODE & WITH_POS))
  {
    assert(offsetForDoclist > 0);
    assert(offsetForDoclist > _byteOffsetsForDoclists.back());
    assert(offsetForDoclist == _byteOffsetsForDoclists.back() + nofListsForWord*sizeof(off_t));
    indexStructureFile.write(&offsetForDoclist,(size_t) sizeof(off_t));
  }
  if(MODE & WITH_POS)
  {
    assert(offsetForPositionlist > offsetForDoclist + sizeof(unsigned long));
    indexStructureFile.write(&offsetForPositionlist,(size_t) sizeof(off_t));
  }
  if(MODE & WITH_SCORES)
  {
    if(MODE & WITH_POS)
    {
      assert(compressedPositionlistSize > 0);
      offsetForScorelist =  offsetForPositionlist + sizeof(unsigned long) + compressedPositionlistSize;
    }
    else 
    {
      assert(compressedDoclistSize > 0);
      offsetForScorelist =  offsetForDoclist + sizeof(unsigned long) + compressedDoclistSize;
    }
    assert(offsetForScorelist > offsetForDoclist + sizeof(unsigned long));
    assert(offsetForScorelist > offsetForPositionlist + sizeof(unsigned long));
    indexStructureFile.write(&offsetForScorelist,(size_t) sizeof(off_t));
  }

  if((MODE & WITH_SCORES) || (MODE & WITH_POS)) {assert(offsetForDoclist== indexStructureFile.tell());}

  const unsigned long lengthOfCurrentDoclist = doclistForCurrentWord.size();
  assert(lengthOfCurrentDoclist > 0);

  // WRITE DOC LIST TO INDEX FILE
  indexStructureFile.write(&lengthOfCurrentDoclist,(size_t) sizeof(unsigned long));
  indexStructureFile.write(_compressionBuffer,(size_t) compressedDoclistSize);
  if(MODE & WITH_POS)
  {
    assert(offsetForPositionlist == indexStructureFile.tell());
    // WRITE POS LIST TO INDEX FILE
    const unsigned long lengthOfCurrentPositionlist = positionlistForCurrentWord.size();
    assert(lengthOfCurrentPositionlist == lengthOfCurrentDoclist);
    writeToDiskTimer.cont();
    indexStructureFile.write(&lengthOfCurrentPositionlist,(size_t) sizeof(unsigned long));
    indexStructureFile.write(_compressionBuffer+compressedDoclistSize,(size_t) compressedPositionlistSize);
    writeToDiskTimer.stop();
  }
  size_t scorelistSize = 0;
  if(MODE & WITH_SCORES)
  {
    offsetForScorelist = _byteOffsetsForDoclists.back() + nofListsForWord*sizeof(off_t) + (nofListsForWord-1)*sizeof(unsigned long) + compressedDoclistSize + compressedPositionlistSize;
    assert(offsetForScorelist == indexStructureFile.tell());
    // WRITE SCORE LIST TO INDEX FILE
    const unsigned long lengthOfCurrentScorelist = scorelistForCurrentWord.size();
    assert(lengthOfCurrentScorelist == lengthOfCurrentDoclist);
    writeToDiskTimer.cont();
    indexStructureFile.write(&lengthOfCurrentScorelist,(size_t) sizeof(unsigned long));
    assert(scorelistForCurrentWord.isContiguous());
    scorelistSize = lengthOfCurrentScorelist*sizeof(DiskScore);
    indexStructureFile.write(&scorelistForCurrentWord.Vector<DiskScore>::operator[](0),(size_t) scorelistSize);
    writeToDiskTimer.stop();
  }
  writeToDiskTimer.stop();
  scorelistVolumeWritten += scorelistSize; /* in bytes */
  if((MODE & WITH_POS) || (MODE & WITH_SCORES))
  {
    assert(( !((MODE & WITH_POS) && (!(MODE & WITH_SCORES)  )  ) ) || (nofListsForWord == 2));
    assert((MODE & WITH_SCORES) || (scorelistSize == 0));
    assert((MODE & WITH_SCORES) || (nofListsForWord == 2));
    assert((MODE & WITH_POS) || (nofListsForWord == 2));
    _byteOffsetsForDoclists.push_back((off_t) nofListsForWord*(sizeof(unsigned long) + sizeof(off_t)) + compressedDoclistSize + compressedPositionlistSize + scorelistSize +  _byteOffsetsForDoclists.back());
  }
  else
  {
    assert(nofListsForWord == 1);
    _byteOffsetsForDoclists.push_back((off_t) nofListsForWord*(sizeof(unsigned long)) + compressedDoclistSize +  _byteOffsetsForDoclists.back());
  }
  assert(_byteOffsetsForDoclists.size()>=2);
  assert(_byteOffsetsForDoclists[_byteOffsetsForDoclists.size()-1] > _byteOffsetsForDoclists[_byteOffsetsForDoclists.size()-2]);
  assert(_byteOffsetsForDoclists.back() == indexStructureFile.tell());

  doclistLengthsForWordId.push_back( doclistForCurrentWord.size() );
  doclistForCurrentWord.clear();
  positionlistForCurrentWord.clear();
  scorelistForCurrentWord.clear();

  #ifdef SIZE_COMPRESSION_BUFFER_FOR_POSITIONS
  #undef SIZE_COMPRESSION_BUFFER_FOR_POSITIONS
  #endif
}

INVIndex::INVIndex(const string& passedIndexStructureFile, const string& passedVocabularyFile, unsigned char mode)
  : IndexBase(passedIndexStructureFile, passedVocabularyFile, mode)
{
}

void INVIndex::read()
{
   // read offset table

   // also watch for adaptive buffer size a bit later
   // setInitialBufferSizes( initIntBuffSize, initUniBuffSize, initCompressBuffSize);
   reserveCompressionBuffersAndResetPointersAndCounters();
   File indexStructureFile(_indexFileName.c_str(), "rb");
   off_t indexTableOffset;
   const off_t lastOffsetOffset =  indexStructureFile.getLastOffset(indexTableOffset);
   assert(indexTableOffset > 0);
   assert(lastOffsetOffset > indexTableOffset);
   // read vocabulary
   readWordsFromFile(_vocabularyFileName, _vocabulary);
   //     CompleterBase<MODE>::_indexStructureFile.readVector< vector<off_t> , off_t >(_byteOffsetsForDoclists,(size_t) lastOffsetOffset - indexTableOffset, indexTableOffset);
   indexStructureFile.readVector(_byteOffsetsForDoclists,(size_t) (lastOffsetOffset - indexTableOffset), (off_t) indexTableOffset, off_t());
   cerr << "* read " << _byteOffsetsForDoclists.size()<< " doclist offsets" << endl;
   //     cout << "\n show offset: " << flush;
   //       for(unsigned int i=0; i< _byteOffsetsForDoclists.size() ;i++)  cout << _byteOffsetsForDoclists[i] << " ";


   // read meta information 
   char* metaInfoReadBuffer = new char[indexTableOffset - _byteOffsetsForDoclists.back()];
   indexStructureFile.read(metaInfoReadBuffer, (size_t) (size_t) indexTableOffset - _byteOffsetsForDoclists.back(), _byteOffsetsForDoclists.back());
   _metaInfo.readFromBuffer(metaInfoReadBuffer);
   // adaptive buffer size choice     
   //       if((initIntBuffSize == INTERSECTION_BUFFER_INIT_DEFAULT) && (initUniBuffSize == UNION_BUFFER_INIT_DEFAULT ))
   //       {
   //         setInitialBufferSizes((unsigned long int) _metaInfo.getNofDocs()/4  , (unsigned long int) _metaInfo.getNofDocs() /16);
   //       }
     
   delete[] metaInfoReadBuffer;
   metaInfoReadBuffer = NULL;
}

void INVIndex::build(const string& wordsFileName, const string& format)
{
  cout << "* build INV index from file \"" << wordsFileName << "\"" << endl 
       << endl;

  // FOR BINARY : LOAD VOCABULARY WHICH IS ALREADY THERE 
  // (this saves constructing it again, and allows to show [geheim-glutre] progress)
  if (format == "BINARY") 
    readWordsFromFile(_vocabularyFileName, _vocabulary, "vocabulary"); 
  //       #define WORD(x) ( format != "BINARY" ? x : _vocabulary[atoi(x.c_str())-1] )
  #define WORD(x) ( format != "BINARY" ? x : (_vocabulary.operator[](atoi(x.c_str())-1) ))

  // INIT TIMERS AND COUNTERS (declared in HYBCompleter above)
  buildIndexTimer.start();
  readFromDiskTimer.reset(); /* not used, slows down indexing by 50% */
  writeToDiskTimer.reset();
  doclistCompressionTimer.reset();
  positionlistCompressionTimer.reset();
  doclistVolumeCompressed = 0;  
  scorelistVolumeWritten = 0;
  positionlistVolumeCompressed = 0;

  // -> INGMAR: Please give a short explanation, what this does
  reserveCompressionBuffersAndResetPointersAndCounters();

  // the index file for INV
  File indexStructureFile(_indexFileName.c_str(), "wb");

  string word, oldWord = "";
  WordId wordId;
  Position location;
  DocId docId;
  unsigned long lineNumber = 0;

  // Go through the worddump file and assign word-ids to the words and count the documents
  // --> wordIds, _maxDocID

  // use Vector to have entropy method later (and possible more statistics)
  Vector<DocId> doclistLengthsForWordId;


  _byteOffsetsForDoclists.push_back((off_t) 0);

  WordsFile wordsFile(wordsFileName.c_str());
  wordsFile.setFormat(WordsFile::FORMAT_HTDIG);
   wordsFile.setSkipLineWithSameWordAndDoc(MODE & WITH_POS ? false : true);

  DocId maxDocId, numDocs;
  WordId numWords;
  DiskScore score;
  unsigned long nofTokens = 0;

  DocList doclistForCurrentWord;
  WordList wordlistForCurrentWord;
  // only used if MODE == WITH_DUPS_WITH_POS
  Vector<Position> positionlistForCurrentWord;
  Vector<DiskScore> scorelistForCurrentWord;

  //
  // MAIN LOOP : process words file line by line writing out block after block
  //
  bool eofReached = false;
  MSG_BEG = " ";  /* let buffer resize messages come nicely */
  MSG_END = " "; /* between the [...] progress indicator */
  bool shownIgnoreMessage = false;
  while (true)
  {
    lineNumber++;

    // Read next line from words file; 0 indicates that lines are not skipped
    bool ret = wordsFile.getNextLine(word, wordId, docId, score, location);

    // End of file or error reading line
    if (ret == false)
    {
      if (!wordsFile.isEof()) {continue;} // case: not at end of file but invalid line read
      else
      {
	word = "";
	eofReached = true;
      } 
    }

    // Progress indicator
    if ((lineNumber % 10000000) == 2 ) {cout << "[" << WORD(word) << "]..." << flush;}

    // New word encountered -> write old list and start a new one
    if (word != oldWord)
    {
      if (oldWord != "")
      {
	// Also pushes to the end of the byte offsets and adds size of doclist 
	writeCurrentDataToIndexFile(doclistForCurrentWord, doclistLengthsForWordId, scorelistForCurrentWord, positionlistForCurrentWord, indexStructureFile);
      }
      // add word to vocabulary (unless empty or reading from BINARY file)
      if (word != "" && format != "BINARY") addWordToVocabulary(word);	       
      oldWord = word;
      shownIgnoreMessage = false;
    }

    // If end of file exit loop now
    if (eofReached) break;

    // Add a new posting  NEW 20Jan08 (Holger): optionally truncate very long lists
    if (doclistForCurrentWord.size() < maxBlockVolume)
    {
      doclistForCurrentWord.push_back(docId);
      if(MODE & WITH_POS) {positionlistForCurrentWord.push_back(location);} 
      if(MODE & WITH_SCORES) {assert(score>0); scorelistForCurrentWord.push_back(score);} 
      nofTokens++;
    }
    else if (shownIgnoreMessage == false)
    {
      cout << "{WARNING: list volume has reached " << commaStr(maxBlockVolume)
	   << ", ignoring remaining items from this list, current word is: "
	   << EMPH_ON << WORD(word) << EMPH_OFF << "}..." << flush;
      shownIgnoreMessage = true;
    }

  } /* END OF WHILE LOOP reading line by line */

  maxDocId = wordsFile.maxDocId();
  numDocs = wordsFile.numDocs();       
  numWords = _vocabulary.size();

  const off_t sizeOfCompressedDocLists = indexStructureFile.tell();
  _byteOffsetsForDoclists.push_back(sizeOfCompressedDocLists );

  // WRITE META INFO
  _metaInfo.setMaxDocID(maxDocId);
  _metaInfo.setNofDocs(numDocs);
  _metaInfo.setNofWords(numWords);
  _metaInfo.setNofWordInDocPairs(nofTokens);
  char* metaInfoBuffer = new char[_metaInfo.getSizeInBytes()];
  assert(_metaInfo.getSizeInBytes() > 0);
  _metaInfo.writeToBuffer(metaInfoBuffer);       
  indexStructureFile.write(metaInfoBuffer,_metaInfo.getSizeInBytes());
  delete[] metaInfoBuffer;
  metaInfoBuffer = NULL;

  // WRITE OFFSET OF INDEX TABLE (and then close file)
  const off_t offsetForIndexTable = indexStructureFile.tell();
  // The last element of this table will be the index for the index table
  indexStructureFile.writeVector(_byteOffsetsForDoclists, off_t());
  //cout << "\n show offset: " << flush;
  //for(unsigned int i=0; i< _byteOffsetsForDoclists.size() ;i++)  cout << _byteOffsetsForDoclists[i] << " ";
  indexStructureFile.write(&offsetForIndexTable,sizeof(offsetForIndexTable));
  buildIndexTimer.stop();

  freeCompressionBuffer();

  // WRITE VOCABULARY to separate file (not for BINARY!)
  cout << endl << endl;
  if (format != "BINARY") writeVocabularyToFile();
  else cout << "! no vocabulary written when reading BINARY format" << endl;
  cout << endl;   

  // SHOW TIMINGS AND STATISTICS 
  off_t fileSizeInBytes = indexStructureFile.tell();
  //double entropy;
  //off_t nofBits;
  //off_t totalDoclistLength =  doclistLengthsForWordId.entropy(numDocs, entropy, nofBits);
  cout.setf(ios::fixed);
  cout.precision(2);
#define HF1 setw(40) << right
#define HF2T " : " << setw(7) << right << setprecision(2)
#define HF2N " : " << setw(14) << right
#define HF2S " : " << setw(6) << right << setprecision(1)
  cout << EMPH_ON << "TIMINGS" << EMPH_OFF << endl << endl
    << HF1 << "total time to build index" << HF2T << buildIndexTimer.secs() << " seconds" << endl
    << HF1 << "time for reading from disk" << HF2T << "----" << " not measured (would slow down indexing by 50%)" << endl
    //readFromDiskTimer.secs() << " seconds" << endl
    << HF1 << "time for writing to disk" << HF2T << writeToDiskTimer.secs() << " seconds" 
    << " (" << fileSizeInBytes/writeToDiskTimer.usecs() << " MiB per second)" << endl  
    << HF1 << "time for compressing doclists" << HF2T << doclistCompressionTimer.secs() << " seconds"
    << " (" << setw(2) << nofTokens/doclistCompressionTimer.usecs() 
    << " million doc ids per second)" << endl;
  if(MODE & WITH_POS){cout
    << HF1 << "time for compressing positionlists" << HF2T << positionlistCompressionTimer.secs() << " seconds"
      << " (" << setw(2) << nofTokens/positionlistCompressionTimer.usecs()
      << " million position ids per second)" << endl;}
  cout << endl;
  cout << EMPH_ON << "STATISTICS" << EMPH_OFF << endl << endl
    << HF1 << "number of documents" << HF2N << commaStr(numDocs) << endl
    << HF1 << "number of words" << HF2N << commaStr(numWords) << endl
    << HF1 << "number of word-in-doc pairs" << HF2N << commaStr(nofTokens) << endl
    << endl
    << HF1 << "total index file size" << HF2S << fileSizeInBytes/(1.0*1024*1024) << " MB" 
    << " (" << setw(4) << setprecision(1) 
    << 8.0*fileSizeInBytes/nofTokens << " bits per word-in-doc pair)" << endl 
    << HF1 << "total size of doclists" << HF2S << doclistVolumeCompressed/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*doclistVolumeCompressed/nofTokens << " bits per doc id)" << endl
    << HF1 << "total size of positionlists" << HF2S << positionlistVolumeCompressed/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*positionlistVolumeCompressed/nofTokens << " bits per position id)" << endl
    << HF1 << "total size of scorelists" << HF2S << scorelistVolumeWritten/(1.0*1024*1024) << " MB"
    << " (" << setw(4) << setprecision(1) 
    << 8.0*scorelistVolumeWritten/nofTokens << " bits per score id)" << endl 
    // Commented out by Holger 15Jan08, because it produces nonsense numbers (very large)
    //<< HF1 << "entropy-optimal size of doclists" << HF2S << nofBits/(8.0*1024*1024) << " MB"
    //       << " (" << setw(4) << setprecision(1) 
    //       << entropy << " bits per doc id)" << endl
    //<< HF1 << "total length of doclists" << HF2S << totalDoclistLength << " elements" << endl
    //<< HF1 << "total number of bits (in entropy coding)" << HF2S << nofBits << " bits" << endl
    << endl;

  /*
   * the old output
   *
   cout << endl << endl;

   const off_t totalFileSize = CompleterBase<MODE>::_indexStructureFile.tell();

   cout << setw(25) <<  commaStr(nofBits) << " bits for document lists (according to \"entropy\") \n";
   cout << setw(25) << alignedDbl(entropy,1)  <<  " \"entropy\" of collection (= bits per pointer) \n";
   cout << setw(25) <<  commaStr( (unsigned long) nofBits/8) <<  " theoretical file size in bytes (doclists only) \n\n";

   cout << setw(25) <<  commaStr(sizeOfCompressedDocLists - numDocs * sizeof(unsigned long)) <<  " real size for compressed doLists in bytes\n";
   cout << setw(25) <<  commaStr(_metaInfo.getSizeInBytes()) << " size of metaInfo in bytes\n";
   cout <<  setw(25) <<  commaStr(numDocs * sizeof(unsigned long)) << " size for size indices (of doclists) in bytes \n";
   cout << setw(25) <<  commaStr(totalFileSize - offsetForIndexTable -sizeof(unsigned long)) << " size for byte offsets in bytes \n";

   cout << setw(25) <<  commaStr(totalFileSize) << " total file size in bytes" << endl;
   cout << endl;
   */    

#undef WORD
}


//! Show meta information of index: number of docs, words, etc.
void INVIndex::showMetaInfo() const
{
  _metaInfo.show();
  cout << endl;
}

