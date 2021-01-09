// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#ifndef PARSER_PARSERBASE_H_
#define PARSER_PARSERBASE_H_

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "fuzzysearch/Utils.h"
#include "parser/SimpleTextParser.h"
#include "parser/UserDefinedIndexWords.h"
#include "synonymsearch/SynonymDictionary.h"
#include "utility/StringConverter.h"

using std::unordered_map;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// Hash function for strings.
// TODO(bast): this is used / needed in many places in the code. It should be
// implemented in one place (Globals.{h,cpp}?) and then included from there.
// TODO(celikik): Isn't it already there?
class HashString
{
 public:
  size_t operator()(const string& str) const
  {
    size_t x = 0;
    const char* s = str.c_str();
    while (*s != 0) { x = 31*x + *s++;}
    return x;
  }
};

// Abstract Parser base class.
// Contains functionality for reading and writing a vocabulary, for reading
// cluster ids needed for fuzzy search, and for reading a synonym dictionary.
// Abstract function parse() must be implemented by subclass.
class ParserBase
{
 public:

  enum Encoding
  {
    UTF8 = 0,
    ISO  = 1
  };

  // Constructor. Does not yet set the names of the various files (we need the
  // base name for that), this is done in init().
  ParserBase();

  // Virtual destructor doing nothing (needed because class contains virtual
  // functions, namely parse).
  virtual ~ParserBase() { }

  // Parse.
  virtual void parse() = 0;

  // Print options (to be used in printUsage of subclasses).
  static void printUsage();

  // Parse command line options (to be called by subclass before it parses its
  // options).
  void parseCommandLineOptions(int argc, char** argv);

  // Setter and getters.
  string getFileNameBase() const { return _fileNameBase; }
  char getWordPartSep() const { return _wordPartSep; }
  void setFileNameBase(const string& fnb) { _fileNameBase = fnb; }

  // Maximal length ofy a line that can be read in buffer at one time.
  static const unsigned int MAX_BUFFER_SIZE;

  // Empty function, which can be redefined to add application based
  // information to the index, e.g. date, name.
  virtual void addGlobalInformationToWordsFile();

 private:
  SynonymDictionary _synonymDictionary;
  void getClusterWords(const string& word, vector<string>* result);
  void getSynonymWords(const string& word, vector<string>* result);
  void stripOffWord(const string& word, string* wordPartUpToLastColon,
                    string* wordPartAfterLastColon);

 protected:
  // Base of the various file names.
  string _fileNameBase;
  Encoding _encoding;
  char _wordPartSep;
  // Various options for what to read and what to write. Default values are set
  // in the constructor.
  bool _writeLogFile;
  bool _writeDocsFile;
  bool _writeAsciiWordsFile;
  bool _writeBinaryWordsFile;
  bool _writeVocabulary;
  bool _readVocabulary;
  bool _readFuzzySearchClusters;
  bool _readUserDefinedWords;
  bool _readSynonymsGroups;
  bool _outputWordFrequencies;
  bool _normalizeWords;
  bool _countWordFrequencyDocsOnly;
  char _infoDelim;
  // File names.
  string _docsFileName;
  string _asciiWordsFileName;
  string _binaryWordsFileName;
  string _logFileName;
  string _vocabularyFileName;
  string _fuzzySearchClustersFileName;
  string _synonymGroupsFileName;
  string _userDefinedWordsFileName;
  string _pathToMaps;
  // Files.
  FILE* _docs_file;
  FILE* _ascii_words_file;
  FILE* _binary_words_file;
  FILE* _log_file;
  // Open output files and optionally read vocabulary, synonyn groups, fuzzy
  // search clusters, and user-defined words.
  // TODO(bast): I don't see why this must have the file name base as an
  // argument, given that _fileNameBase is a member of this class which is set
  // by parseCommandLineOptions, which happens before init().
  void init(const string& fileNameBase);
  // Close output files and optionall write the vocabulary.
  void done();

  SimpleTextParser _simpleTextParser;
  StringConverter _stringConverter;
  UserDefinedIndexWords _userDefinedIndexWords;

  // When operating in mode 32, this function must be called in the subclassed
  // parser whenever it finished parsing text.
  void flushSortedWordsFile(void);

  // Extend word to possbily user defined word.
  bool extendToUserDefinedWord(const string& text, size_t* word_start,
                               size_t* word_end);

  // Write word to words file.
  void writeToWordsFile(const string& word, unsigned int docId,
                        unsigned int score, unsigned int position);

  // Write the word ":info:<key>:<value>" to the word file for document 0 at
  // position 0 with score 0, where ":" is the configured word separator.
  // This method should be used to add global meta information (see
  // addGlobalInformationToWordsFile).
  void writeInfoWord(const string& key, const string &value);

  // Map words to word ids. Use to maintain or read or write the vocabulary.
  unordered_map<string, int, HashString> _wordsToWordsIds;
  // Map words to word frequencies.
  unordered_map<string, unsigned int, HashString> _wordToFrequency;
  // Write vocabulary to given file. Ignores the word ids and write out the
  // words in sorted order.
  // used for word frequency counting (needed for fuzzysearch)
  unordered_map<string, int, HashString> _wordToDocId;
  void writeVocabulary(const string& fileName);
  // Read vocabulary from given file. The file contains one word per line. The
  // word id is implicit in the line number, that is, the id of the word in the
  // first line is 1, the id of the word in the second line is 2, etc. The
  // typical file name is <db>.vocabulary.
  void readVocabulary(const string& fileName);

  // Current word id when maintaining vocabulary.
  size_t _wordId;

  // Map words to cluster ids. Most words (about 90%) get mapped to only one
  // cluster id. For this reason we have one map, which maps words to ints, and
  // a second map which maps words to vector<int>. The first map contains the
  // first (and usually only) cluster id for each word. The second map is only
  // for those words with more than one cluster id and it contains the second,
  // third, etc. cluster id. To indicate whether there is more than one cluster
  // id, the cluster ids in _wordsToClusterIds are multiplied by 2, and a 1 is
  // added iff there is more than one cluster id. This avoids unnecessary (and
  // expensive) lookups in _wordsToClusterIds2
  unordered_map<string, int,
           FuzzySearch::StringHashFunctionMarjan> _wordsToClusterIds;
  unordered_map<string, vector<int>,
           FuzzySearch::StringHashFunctionMarjan> _wordsToClusterIds2;
  // Read this map from given file. The file contains lines of the form
  // <word><TAB><cluster id>. The typical file name is <db>.cluster-ids.
  void readClusterIds(const string& fileName);

  // Read synonym dictionary.
  // TODO(bast): I don't think the name of the method is appropriate.
  void readSynonymIds(const string& fileName);
};

#endif  // PARSER_PARSERBASE_H_
