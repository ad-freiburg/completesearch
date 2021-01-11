// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>, Jens Hoffmann <hoffmaje>.

#include <getopt.h>
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include "parser/ParserBase.h"
#include "utility/StringConverter.h"
#include "utility/WkSupport.h"

// ____________________________________________________________________________
const unsigned int ParserBase::MAX_BUFFER_SIZE = 100 * 1000;

// ____________________________________________________________________________
void ParserBase::printUsage()
{
  cout << "--base-name                   : base name of all files (REQUIRED)"
       << endl
       << "--encoding                    : the encoding of the file content"
       <<                                " (utf8 or iso, default = iso)"
       << endl
       << "--normalize-words             : normalize text words"
       << endl
       << "--info-delimiter              : delimiter to split a multiple"
       <<                                " title field"
       << endl
       << "--write-docs-file             : write .docs-unsorted file (in ASCII)"
       <<                                " (default = false)"
       << endl
       << "--write-words-file-ascii      : write .words-unsorted.ascii file"
       <<                                " (default = false)"
       << endl
       << "--write-words-file-binary     : write .words-unsorted.binary file"
       <<                                " (default = false)"
       << endl
       << "--write-log-file              : write .parse-log file"
       <<                                " (default = true)"
       << endl
       << "--write-vocabulary            : write .vocabulary file"
       <<                                " (default = false)"
       << endl
       << "--read-vocabulary             : read .vocabulary file"
       <<                                " (default = false)"
       << endl
       << "--read-fuzzy-search-clusters  : read .fuzzysearch-clusters file"
       <<                                " (default = false)"
       << endl
       << "--read-synonym-groups         : read .synonym-groups file"
       <<                                " (default = false)"
       << endl
       << "--read-user-defined-words     : read .user-defined-words file"
       <<                                " (default = false)"
       << endl
       << "--output-word-frequencies     : write .vocabulary+frequencies file"
       <<                                " (default = false)"
       << endl
       << "--output-doc-frequencies      : do not count multiple occurrences"
       <<                                " of a word in a doc"
       <<                                " (default = false)"
       << endl
       << "--maps-directory              : the directory containing maps for"
       <<                                " utf8 and iso conversion"
       <<                                " (default = current directory)."
       << endl
       << "--word-part-separator-backend : separator used within the special"
       <<                                " words"
       << endl
       << "--csv-suffix                  : suffix of csv file (default = .csv)"
       << endl;
}


// ____________________________________________________________________________
void ParserBase::parseCommandLineOptions(int argc, char** argv)
{
  #ifdef DEBUG_PARSER_BASE
  cout << "ParserBase::parseCommandLineOptions ... " << flush;
  #endif

  opterr = 0;
  optind = 1;
  while (true)
  {
    static struct option long_options[] =
    {
      {"base-name"                   , 1, NULL, 'n'},
      {"encoding"                    , 1, NULL, 'E'},
      {"maps-directory"              , 1, NULL, 'm'},
      {"info-delimiter"              , 1, NULL, 'i'},
      {"csv-suffix"                  , 1, NULL, 'x'},
      {"word-part-separator-backend" , 1, NULL, 'B'},
      {"normalize-words"             , 0, NULL, 's'},
      {"write-docs-file"             , 0, NULL, 'd'},
      {"write-words-file-ascii"      , 0, NULL, 'w'},
      {"write-words-file-binary"     , 0, NULL, 'b'},
      {"write-log-file"              , 0, NULL, 'l'},
      {"write-vocabulary"            , 0, NULL, 'v'},
      {"read-vocabulary"             , 0, NULL, 'V'},
      {"read-fuzzy-search-clusters"  , 0, NULL, 'Y'},
      {"read-synonym-groups"         , 0, NULL, 'S'},
      {"read-user-defined-words"     , 0, NULL, 'U'},
      {"output-word-frequencies"     , 0, NULL, 'f'},
      {"output-doc-frequencies"      , 0, NULL, 'z'},
      { NULL                         , 0, NULL,  0 }
    };
    int c = getopt_long(argc, argv, "EmisdwblvVYSUfz", long_options, NULL);
    if (c == -1) break;
    // cout << "ParserBase::parseCommandLineOptions ["
    //      << c << "|" << (char)(c) << "]" << endl;
    switch (c)
    {
      case 'n': _fileNameBase = optarg; break;
      case 'E':
      {
        string encoding = string(optarg);
        if (encoding == "utf8" || encoding == "UTF8") _encoding = UTF8;
        else if (encoding == "iso" || encoding == "ISO") _encoding = ISO;
        else
        {
          cerr << "Unknown encoding: " << encoding << endl;
          printUsage();
          exit(1);
        }
        break;
      }
      case 's': _normalizeWords = true; break;
      case 'i': _infoDelim = optarg[0]; break;
      case 'B': _wordPartSep = optarg[0]; break;
      case 'd': _writeDocsFile = true; break;
      case 'w': _writeAsciiWordsFile = true; break;
      case 'b': _writeBinaryWordsFile = true; break;
      case 'l': _writeLogFile = true; break;
      case 'v': _writeVocabulary = true; break;
      case 'V': _readVocabulary = true; break;
      case 'Y': _readFuzzySearchClusters = true; break;
      case 'S': _readSynonymsGroups = true; break;
      case 'U': _readUserDefinedWords = true; break;
      case 'f': _outputWordFrequencies = true; break;
      case 'z': _countWordFrequencyDocsOnly = true; break;
      case 'm': _pathToMaps = string(optarg); break;
      case 'x': _csvFileNameSuffix = string(optarg); break;
    }
  }
  // Print information about the options and about the current locale.
  cout << "Base name of input and output files is \"" << _fileNameBase
       << "\"" << endl;
  cout << "Encoding is ";
  if (_encoding == UTF8)
    cout << "UTF8" << endl;
  else
    cout << "ISO" << endl;
  cout << boolalpha << "Will write: docs file = " << _writeDocsFile
       << ", words file = " << _writeAsciiWordsFile
       << ", binary words file = " << _writeBinaryWordsFile
       << ", vocabulary file = " << _writeVocabulary
       << ", vocabulary+frequencies file = " << _outputWordFrequencies << endl;
  cout << "Will read: vocabulary = " << _readVocabulary
       << ", fuzzy search clusters = " << _readFuzzySearchClusters
       << ", user-defined words = " << _readUserDefinedWords
       << ", synonym groups = " << _readSynonymsGroups << endl;
  cout << "Will do: normalize words = " << _normalizeWords
       << ", count doc frequencies only = " << _countWordFrequencyDocsOnly
       << noboolalpha << endl;
  cout << "Locale is: " << setlocale(LC_CTYPE, NULL) << " (CTYPE)"
       << ", " << setlocale(LC_COLLATE, NULL) << " (COLLATE)" << endl;
  cout << endl;

  #ifdef DEBUG_PARSER_BASE
  cout << "done." << endl;
  #endif
}


// ____________________________________________________________________________
ParserBase::ParserBase()
{
  _wordId = 0;
  // Set default options for what to read and what to write. The default is to
  // require no special input and write the words and docs file, both in ASCII.
  _writeLogFile = true;
  _writeDocsFile = false;
  _writeAsciiWordsFile = false;
  _writeBinaryWordsFile = false;
  _writeVocabulary = false;
  _readVocabulary = false;
  _readFuzzySearchClusters = false;
  _readUserDefinedWords = false;
  _readSynonymsGroups = false;
  _outputWordFrequencies = false;
  _normalizeWords = false;
  _countWordFrequencyDocsOnly = false;
  _infoDelim = '';
  // By default set all file handles to NULL. Will be opened according to
  // read/write options in openOutputFiles.
  _docs_file = NULL;
  _ascii_words_file = NULL;
  _binary_words_file = NULL;
  _log_file = NULL;
  _encoding = ISO;
  _csvFileNameSuffix = ".csv";

  _wordPartSep   = '!';
  _pathToMaps="";
  // TODO(bast): This should be moved to a separate init function. A constructor
  // should not do real work, see http://google-styleguide.googlecode.com/svn/
  // trunk/cppguide.xml#Doing_Work_in_Constructors
  // if (_readUserDefinedWords)
  // {
  //  _userDefinedIndexWords.init(_userDefinedWordsFileName);
  // }
}


// ____________________________________________________________________________
void ParserBase::init(const string& fileNameBase)
{
  // Set the default file names. They all start with the same base name and then
  // have suffixes indicating their contents and type of contents.
  _fileNameBase = fileNameBase;
  _docsFileName = fileNameBase + ".docs-unsorted";
  _asciiWordsFileName = fileNameBase + ".words-unsorted.ascii";
  _binaryWordsFileName = fileNameBase + ".words-unsorted.binary";
  _logFileName = fileNameBase + ".parse-log";
  _vocabularyFileName = fileNameBase + ".vocabulary";
  _fuzzySearchClustersFileName = fileNameBase + ".fuzzysearch-clusters";
  _synonymGroupsFileName = fileNameBase + ".synonym-groups";
  _userDefinedWordsFileName = fileNameBase + ".user-defined-words";

  #ifdef DEBUG_PARSER_BASE
  printf("Opening files ...");
  fflush(stdout);
  #endif

  // Optionally open the files with these names.
  if (_writeDocsFile)
  {
    _docs_file  = fopen(_docsFileName.c_str(), "w");
    if (_docs_file == NULL)
    {
      perror("fopen docs file:");
      exit(1);
    }
  }
  if (_writeAsciiWordsFile)
  {
    _ascii_words_file = fopen(_asciiWordsFileName.c_str(), "w");
    if (_ascii_words_file == NULL)
    {
      perror("fopen ascii words file:");
      exit(1);
    }
  }
  if (_writeBinaryWordsFile)
  {
    _binary_words_file = fopen(_binaryWordsFileName.c_str(), "wb");
    if (_binary_words_file == NULL)
    {
      perror("fopen binary words file:");
      exit(1);
    }
  }
  if (_writeLogFile)
  {
    _log_file  = fopen(_logFileName.c_str(), "w");
    if (_log_file == NULL)
    {
      perror("fopen log file:");
      exit(1);
    }
  }

  // Optionally read vocabulary, cluster ids, synonym groups, and user-defined
  // words.
  if (_readVocabulary) readVocabulary(_vocabularyFileName);
  if (_readFuzzySearchClusters) readClusterIds(_fuzzySearchClustersFileName);
  if (_readSynonymsGroups) readSynonymIds(_synonymGroupsFileName);
  if (_readUserDefinedWords)
  {
    _userDefinedIndexWords.init(_userDefinedWordsFileName);
  }
  if (_stringConverter.init(_pathToMaps) == false)
  {
    cerr << _stringConverter.getLastError() << endl;
    exit(1);
  }

  #ifdef DEBUG_PARSER_BASE
  printf("done.\n");
  #endif
}


// ____________________________________________________________________________
void ParserBase::done()
{
  if (_docs_file) fclose(_docs_file);
  if (_ascii_words_file) fclose(_ascii_words_file);
  if (_binary_words_file) fclose(_binary_words_file);
  if (_log_file) fclose(_log_file);

  if (_writeVocabulary) writeVocabulary(_vocabularyFileName);
}

// ____________________________________________________________________________
bool orderWithFrequencies(const pair<string, unsigned int>& x,
                          const pair<string, unsigned int>& y)
{
  // return x.first < y.first;  // lexicographically
  return x.second < y.second;  // w.r.t. frequencies
}

// ____________________________________________________________________________
void ParserBase::writeVocabulary(const string& fileName)
{
  assert(_writeVocabulary);
  cout << "Writing vocabulary to file \"" << fileName << "\" ... " << flush;

  // First copy words into vector and sort them lexicographically.
  cout << "[sorting] ... " << flush;
  vector<string> vocabulary;
  vocabulary.reserve(_wordsToWordsIds.size());
  for (auto it = _wordsToWordsIds.begin(); it != _wordsToWordsIds.end(); ++it)
  {
    vocabulary.push_back(it->first);
  }
  std::sort(vocabulary.begin(), vocabulary.end());
  // Now write to file and to hash_map in sorted order.
  FILE* file = fopen(fileName.c_str(), "w");
  if (file == NULL)
  {
    perror("fopen");
    exit(1);
  }
  for (size_t i = 0; i < vocabulary.size(); ++i)
  {
    fprintf(file, "%s\n", vocabulary[i].c_str());
  }
  cout << "done, #words written = " << vocabulary.size() << endl;
  fclose(file);

  // Output the word frequencies + vocabulary if needed
  if (_writeVocabulary && _outputWordFrequencies)
  {
    // First copy words and frequencies into vector and sort them
    // lexicographically.
    string fileNameWithFrequencies = fileName+"+frequencies";
    cout << "Writing .vocabulary+frequencies to file \"" <<
        fileNameWithFrequencies << "\" ... " << flush;
    cout << "[sorting] ... " << flush;
    vector<pair<string, unsigned int> > vocabulary;
    for (auto it = _wordToFrequency.begin();
        it != _wordToFrequency.end(); ++it)
    {
      vocabulary.push_back(std::make_pair(it->first, it->second));
    }
    std::sort(vocabulary.begin(), vocabulary.end(), orderWithFrequencies);
    FILE* file = fopen(fileNameWithFrequencies.c_str(), "w");
    if (file == NULL)
    {
      perror("fopen");
      exit(1);
    }
    for (size_t i = 0; i < vocabulary.size(); ++i)
      fprintf(file, "%s\t%d\n", vocabulary[i].first.c_str(),
          vocabulary[i].second);
    fclose(file);
    cout << "done, #words with frequencies written = "
        << vocabulary.size() << endl;
  }
}


// ____________________________________________________________________________
void ParserBase::readVocabulary(const string& fileName)
{
  assert(_writeBinaryWordsFile);
  cout << "Reading vocabulary from file \"" << fileName << "\" ... " << flush;
  assert(_wordsToWordsIds.size() == 0);
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL)
  {
    perror("fopen");
    exit(1);
  }
  char* line = new char[MAX_BUFFER_SIZE + 2];
  unsigned int wordId = 0;
  while (true)
  {
    char* ret = fgets(line, MAX_BUFFER_SIZE + 2, file);
    if (ret == NULL) break;
    string word = line;
    assert(word.length() > 0);
    word.erase(word.length() - 1);
    assert(_wordsToWordsIds.count(word) == 0);
    // cout << word << endl;
    _wordsToWordsIds[word] = wordId;
    ++wordId;
  }
  cout << "done, #words = "  << _wordsToWordsIds.size() << endl;
  delete[] line;
  fclose(file);
}


// ____________________________________________________________________________
void ParserBase::readClusterIds(const string& fileName)
{
  cout << "Reading cluster ids from file \"" << fileName << "\" ... " << flush;
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL)
  {
    perror("fopen");
    exit(1);
  }
  char* line = new char[MAX_BUFFER_SIZE + 2];
  size_t lineNumber = 0;
  string previousWord;
  while (true)
  {
    char* ret = fgets(line, MAX_BUFFER_SIZE + 2, file);
    if (ret == NULL) break;
    lineNumber++;
    size_t pos = 0;
    while (line[pos] != '\t' && pos < MAX_BUFFER_SIZE) pos++;
    if (pos == MAX_BUFFER_SIZE)
    {
      perror("ERROR: missing tab in line");
      exit(1);
    }
    assert(line[pos] == '\t');
    line[pos] = 0;
    string word = line;
    int clusterId = atoi(line + pos + 1);
    // If word occurs for the first time, put it into _wordsToClusterIds.
    if (_wordsToClusterIds.count(word) == 0)
    {
      _wordsToClusterIds[word] = 2 * clusterId;
    }
    // If word occurs for the first, second, etc. time, put it into
    // _wordsToClusterIds2. If it's the first cluster id there, add 1 to the
    // corresponding entry in _wordsToClusterIds.
    else
    {
      if (_wordsToClusterIds2.count(word) == 0) _wordsToClusterIds[word] += 1;
      _wordsToClusterIds2[word].push_back(clusterId);
    }
  }
  delete[] line;
  cout << "done, #clusters = "  << _wordsToClusterIds.size() << endl << endl;
}

void ParserBase::readSynonymIds(const string& fileName)
{
  _synonymDictionary.readFromFile(fileName);
}

void ParserBase::getSynonymWords(const string& word, vector<string>* result)
{
  #ifdef DEBUG_PARSER_BASE_
  cout << "ParserBase::getSynonymWords for " << word << " ... "
       << "MAX_BUFFER_SIZE is " << MAX_BUFFER_SIZE << " " << flush;
  #endif
  // printf("ParserBase.cpp:172: Enter getSynonymWords.\n");
  vector<uint32_t> groupIds;
  string wordPartUpToLastColon;
  string wordPartAfterLastColon;
  char *buffer = new char[MAX_BUFFER_SIZE];
  uint32_t id;
  // NEW 11Oct13 (baumgari): Uncommented (here and line 488) since unused
  // variable.
  // bool asteriskBit;

  // Strip off word in :filter:blub:word.
  stripOffWord(word, &wordPartUpToLastColon, &wordPartAfterLastColon);
  // Get the group ids.
  _synonymDictionary.getSynonymGroupIds(wordPartAfterLastColon, groupIds);
  // Go through resulting ids and write word.
  for (size_t i = 0; i < groupIds.size(); i++)
  {
    #ifdef DEBUG_PARSER_BASE
    cout << "Found groups. " << flush;
    cout << "Wordpart: " << wordPartAfterLastColon
         <<  ", into buffer with size " << sizeof(buffer) << endl;
    #endif
    id = groupIds[i];
    // See l. 468. But: stripAsteriskBitFromGroupId has side effects, so we need
    // to call the function!!
    // asteriskBit = _synonymDictionary.stripAsteriskBitFromGroupId(&id);
    _synonymDictionary.stripAsteriskBitFromGroupId(&id);
    // When group id has the 31th bit set (so it was marked with an asterisk) -
    // do noting.
    // if (asteriskBit) continue;

    snprintf(buffer, MAX_BUFFER_SIZE, "%sS%c%u%c%s",
             wordPartUpToLastColon.c_str(),
             _wordPartSep,
             id,
             _wordPartSep,
             wordPartAfterLastColon.c_str());
    result->push_back(string(buffer));

    // If group id has not the 31. bit set (so it was NOT marked with an
    // asterix) then also add the words translation.
    // TODO(hoffmaje): This code might be obsolote: The line with format
    // S:<word>:<groupId> is only needed when having the UI doing the job of
    // handling the search for group-members. In the moment this
    // job is done in CompleterBase code.
    // if (!asteriskBit)
    // {
    //   snprintf(buffer, MAX_BUFFER_SIZE, "S%c%s%c%u",
    //            _wordPartSep,
    //            wordPartAfterLastColon.c_str(),
    //            _wordPartSep,
    //            id);
    //   result->push_back(string(buffer));
    // }
  }
  #ifdef DEBUG_PARSER_BASE
  cout << "done." << endl;
  #endif
  delete[] buffer;
}

void ParserBase::stripOffWord(const string& word,
                              string* wordPartUpToLastColon,
                              string* wordPartAfterLastColon)
{
  size_t pos;

  // Make sure strings are empty.
  wordPartUpToLastColon->clear();
  wordPartAfterLastColon->clear();
  // Find last colon.
  pos = word.rfind(_wordPartSep);
  if (pos == std::string::npos)
  {
    *wordPartAfterLastColon = word;
  }
  else
  {
    *wordPartUpToLastColon = word.substr(0, pos + 1);
    *wordPartAfterLastColon = word.substr(pos + 1);
  }
}


// ____________________________________________________________________________
void ParserBase::getClusterWords(const string& word, vector<string>* result)
{
  // For special words, like :filter:autor:jens, do the following:
  // 1. strip of the :filter:autor:
  // 2. Do the lookup for jens.
  // 3. For every cluster we find, prepend the stripped of prefix again, so
  // that we get, for example :filter:autor:C:2345:jens.

  // bartsch:
  // 1. The algorithm did not work for phrases.
  // Expl.: phrase field contains: "Costa Rica"
  // vocabulary file will contain:
  // costa:costa_rica
  // rica:costa_rica
  // In this cases costa_rica is the last word part and we have no
  // cluster id for costa_rica. But we have  cluster ids for costa
  // and rica,  so we will take in the phrase case the last but
  // one part of the word for cluster id lookup.
  //
  // 2. We add no cluster ids to facet fields
  //

  string         wordPartUpToLastColon;
  string         wordPartAfterLastColon;
  string         wordPartForClusterLookup;
  vector<string> vs;
  char           buffer[256];

  if (word.find(_wordPartSep + string("facet") + _wordPartSep)
      != string::npos)
  {
    return;
  }

  stripOffWord(word, &wordPartUpToLastColon, &wordPartAfterLastColon);

  putTokenInVector(word, _wordPartSep, &vs);
  if (vs.size() > 1)
  {
    if (vs[vs.size()-1].find("_") != string::npos)
    {
      // last part is a phrase, so we take the last but one part
      // for cluster lookup:
      wordPartForClusterLookup = vs[vs.size() - 2];
      wordPartUpToLastColon = "";
      if (vs.size() > 2)
      {
        for (size_t n = 0; n < vs.size() - 2; n++)
        {
          wordPartUpToLastColon += vs[n];
          wordPartUpToLastColon += _wordPartSep;
        }
      }
    }
    else
    {
      wordPartForClusterLookup = vs[vs.size() - 1];
    }
  }
  else
  {
    wordPartForClusterLookup = word;
  }

  // No cluster id. TODO: should we really write C:0:... then?
  if (_wordsToClusterIds.count(wordPartForClusterLookup) == 0)
  {
    // TODO(bast): currently don't do it, since we don't need it and it
    // costs time. Have this as an option.
    // int clusterId = 0;
    // fprintf(_words_file, "C%c%d%c%s\t%u\t%u\t%u\n",
    //         _wordPartSep, clusterId, _wordPartSep, word.c_str(),
    //         docId, score, position);
  }
  else
  {
    int clusterId = _wordsToClusterIds[wordPartForClusterLookup];
    bool moreThanOneClusterId = (clusterId % 2) == 1;
    clusterId = moreThanOneClusterId ? (clusterId - 1) / 2 : clusterId / 2;
    // clusterId /= 2;

    snprintf(buffer, sizeof(buffer), "%sC%c%d%c%s",
                                     wordPartUpToLastColon.c_str(),
                                     _wordPartSep,
                                     clusterId,
                                     _wordPartSep,
                                     wordPartAfterLastColon.c_str());
    result->push_back(string(buffer));
    // Write first cluster id.
    // fprintf(_words_file, "%sC%c%d%c%s\t%u\t%u\t%u\n",
    //         wordPartUpToLastColon.c_str(),
    //         _wordPartSep,
    //         clusterId,
    //         _wordPartSep,
    //         wordPartAfterLastColon.c_str(),
    //         docId, score, position);
    //         Write additional cluster ids if they exist.
    if (moreThanOneClusterId)
    {
      assert(_wordsToClusterIds2.count(wordPartForClusterLookup) > 0);
      const vector<int> clusterIds =
      _wordsToClusterIds2[wordPartForClusterLookup];
      for (size_t k = 0; k < clusterIds.size(); ++k)
      {
        clusterId = clusterIds[k];
        snprintf(buffer, sizeof(buffer), "%sC%c%d%c%s",
                                         wordPartUpToLastColon.c_str(),
                                         _wordPartSep,
                                         clusterId,
                                         _wordPartSep,
                                         wordPartAfterLastColon.c_str());
        result->push_back(string(buffer));
        // fprintf(_words_file, "%sC%c%d%c%s\t%u\t%u\t%u\n",
        //         wordPartUpToLastColon.c_str(),
        //         _wordPartSep,
        //         clusterIds[k],
        //         _wordPartSep,
        //         wordPartAfterLastColon.c_str(),
        //         docId, score, position);
      }
    }
  }
}

// ____________________________________________________________________________
void ParserBase::writeInfoWord(const string& key, const string &value)
{
  const string word = _wordPartSep + string("info") + _wordPartSep + key +
    _wordPartSep + value;
  writeToWordsFile(word, 0, 0, 0);
}


// ____________________________________________________________________________
void ParserBase::writeToWordsFile(const string& word, unsigned int docId,
                                  unsigned int score, unsigned int position)
{
  #ifdef DEBUG_PARSER_BASE
  cout << "ParserBase::writeToWordsFile " << word << " ... " << endl;
  #endif
  // When none of the corresponding write options is set, there is nothing to
  // do.
  if (_writeAsciiWordsFile == false &&
      _writeBinaryWordsFile == false &&
      _writeVocabulary == false) return;

  // TODO(celikik): this is a hack which ignores very long words that might
  // cause trouble for buildIndex (e.g. when indexing Wikipedia)
  if (word.length() == 0 || word.length() > 1000)
  {
    // Empty words can occur because the normalization replaces control
    // characters by the empty string and there are words consisting
    // only of control characters out there.
    // An empty entry in vocabulary or words file crashes the server,
    // therefore we won't write it.
    return;
  }
  if (word.find(" ") != string::npos)
  {
    // A space in output can send buildFuzzySearchClusters to eternity
    // It shouldn't occur anymore, but just to be sure at the moment:
    return;
  }
  // Get all words to be written: the original word + optionally words from
  // related fuzzy search clusters + optionally words from related synonym
  // groups.
  vector<string> resulting_words;
  resulting_words.push_back(word);
  if (_readFuzzySearchClusters) getClusterWords(word, &resulting_words);
  if (_readSynonymsGroups) getSynonymWords(word, &resulting_words);

  // Now write these words (in ASCII and / or binary). Optionally also maintain
  // the vocabulary in case it should be written out in the end.
  for (size_t i = 0; i < resulting_words.size(); ++i)
  {
    // Optionally add word to vocabulary, if not already there.
    // NOTE(bast): For our purposes so far, we would actually only need a hash
    // set, since all we do in writeVocabulary is write the distinct words to
    // a file in sorted order, ignoring the ids assigned to them here. Anyway,
    // in order to avoid having both a hash set and a hash map and since it is
    // so natural to assign an id here (namely i if this word is the i-th
    // distinct word encountered), and we just do it.
    if (_writeVocabulary)
    {
      if (_wordsToWordsIds.count(resulting_words[i]) == 0)
      {
        ++_wordId;
        _wordsToWordsIds[resulting_words[i]] = _wordId;
        if (_outputWordFrequencies)
        {
          _wordToFrequency[resulting_words[i]] = 1;
          if (_countWordFrequencyDocsOnly)
            _wordToDocId[resulting_words[i]] = docId;
        }
      }
      else
      {
        if (_outputWordFrequencies)
        {
          if (_countWordFrequencyDocsOnly)
          {
            if (_wordToDocId[resulting_words[i]] != static_cast<int>(docId))
            {
              _wordToFrequency[resulting_words[i]]++;
              _wordToDocId[resulting_words[i]] = docId;
            }
          }
          else
            _wordToFrequency[resulting_words[i]]++;
        }
      }
    }
    // Write to words file in ASCII.
    if (_writeAsciiWordsFile)
    {
      assert(_ascii_words_file);
      fprintf(_ascii_words_file, "%s\t%d\t%d\t%d\n",
              resulting_words[i].c_str(), docId, score, position);
    }
    // Write to words file in binary.
    if (_writeBinaryWordsFile)
    {
      const string& word = resulting_words[i];
      if (_wordsToWordsIds.count(word) == 0)
      {
        cerr << "! word not found in vocabulary (" << word
             << "), can't write word id." << endl << endl;
        // exit(1);
      }
      // assert(_wordsToWordsIds.count(word) > 0);
      // NOTE: writing the four integers with four separate fwrites turned out
      // to be two times slower.
      int buf[4];
      buf[0] = _wordsToWordsIds[word];
      buf[1] = docId;
      buf[2] = score;
      buf[3] = position;
      assert(_binary_words_file);
      fwrite(&buf, 1, sizeof(buf), _binary_words_file);
    }
  }
}

// ____________________________________________________________________________
void ParserBase::addGlobalInformationToWordsFile()
{
  // This method can be filled by redefining to add application based / global
  // information.

  // Date
  time_t rawtime;
  time_t now = time(&rawtime);
  tm time;
  localtime_r(&now, &time);
  char date[100];
  // HACK(bast): omit the time, otherwise the special "date" word is not found
  // in binary sort mode, which requires two consecutive parses in a row, one
  // for construction the vocabulary and the second one mapping words from this
  // vocabular to ids.
  strftime(date, 100, "%d%b%y", &time);
  // strftime(date, 100, "%d%b%y_%H:%M:%S", &time);
  writeInfoWord("date", date);
}

// ____________________________________________________________________________
bool ParserBase::extendToUserDefinedWord(const string& text,
                                         size_t* word_start,
                                         size_t* word_end)
{
  assert(_readUserDefinedWords);
  return _userDefinedIndexWords.
      extendToUserDefinedWord(text, word_start, word_end);
}
