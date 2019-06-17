// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Timer.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./VocabularyBuilder.h"

using std::string;
using std::vector;
using std::endl;
using std::flush;

using ad_utility::HashSet;
using ad_utility::Timer;
using ad_utility::File;

namespace ad_semsearch
{
// _____________________________________________________________________________
VocabularyBuilder::VocabularyBuilder()
  : _vocabulary()
{
}
// _____________________________________________________________________________
void VocabularyBuilder::constructVocabularyFromASCIIWordsFile(
    const string& inputFileName)
{
  DefaultWordExtractorNoEntities we;
  vector<string> additions;
  constructVocabularyFromASCIIFile(inputFileName, additions, we,
      std::less<string>());
}
// _____________________________________________________________________________
void VocabularyBuilder::constructVocabularyFromOntology(
    const string& inputFileName)
{
  OntologyWordExtractor we;
  vector<string> additions;
  additions.push_back(string(RELATION_PREFIX) + HAS_RELATIONS_RELATION);
  additions.push_back(string(TYPE_PREFIX) + "entity");
  additions.push_back(string(TYPE_PREFIX) + "relation");
  additions.push_back(string(RELATION_PREFIX) + HAS_INSTANCES_RELATION);
  additions.push_back(string(TYPE_PREFIX) + "count");
  additions.push_back(string(TYPE_PREFIX) + "class");

  constructVocabularyFromASCIIFile(inputFileName, additions, we,
      std::less<string>());
}
// _____________________________________________________________________________
template<class WordExtractor, class DesiredSortOrderComparator>
void VocabularyBuilder::constructVocabularyFromASCIIFile(
    const string& inputFileName,
    const vector<string>& additions,
    const WordExtractor& wordExtractor,
    const DesiredSortOrderComparator& comp)
{
  // Clear old data
  _vocabulary.clear();

  // Collect all words in a HashSet
  HashSet<string> words;

  // Add the additions
  for (size_t i = 0; i < additions.size(); ++i)
  {
    words.insert(additions[i]);
  }
  char buf[ASCII_LINE_BUFFER_SIZE];



  int lines = 0;
  LOG(INFO) << "Processing input file \"" << inputFileName
      << "\" and mapping words... " << std::endl;
  File file(inputFileName.c_str(), "r");
  Timer vocabularyBuildTimer;
  // Read the file line-wise.
  string line;
  vocabularyBuildTimer.start();
  while (file.readLine(&line, buf, ASCII_LINE_BUFFER_SIZE))
  {
    if (line.size() == 0) continue;
    // Extract the word and add it to the HashSet.
    vector<string> extracted;
    wordExtractor(line, &extracted);
    for (size_t i = 0; i < extracted.size(); ++i)
    {
      words.insert(extracted[i]);
    }
    lines++;
  }
  vocabularyBuildTimer.stop();
  LOG(INFO) << "done in " << vocabularyBuildTimer.secs() << " seconds."
      << " Lines read: " << lines << endl << flush;

  LOG(INFO)
      << "Converted HashSet into vector (" << words.size()
      << " elements)... " << endl;
  vocabularyBuildTimer.start();
  // Store the content of the HashSet in the vector member.
  _vocabulary.reserve(words.size());
  for (HashSet<string>::iterator it = words.begin(); it != words.end(); ++it)
  {
    _vocabulary.push_back(*it);
  }
  LOG(INFO)
    << "Sorting vocabulary... " << endl;
  // Sort the vocabulary.
  std::sort(_vocabulary.begin(), _vocabulary.end(), comp);
  vocabularyBuildTimer.stop();
  LOG(INFO)
      << "done in " << vocabularyBuildTimer.secs() << " seconds. Now there are "
      << _vocabulary.size() << " elements." << endl
      << flush;
}
// _____________________________________________________________________________
template<class SortOrderComparator>
signed int VocabularyBuilder::getWordId(const std::string& word,
    const SortOrderComparator& comp) const
{
  vector<string>::const_iterator i = std::lower_bound(_vocabulary.begin(),
      _vocabulary.end(), word, comp);
  if (i == _vocabulary.end()) return -1;
  if (*i == word) return i - _vocabulary.begin();
  else
  {
    return -1;
  }
}
// _____________________________________________________________________________
signed int VocabularyBuilder::getWordId(const std::string& word) const
{
  return getWordId(word, std::less<string>());
}
// _____________________________________________________________________________
void VocabularyBuilder::writeVocabularyToOutputFiles(
    const string& outputFileName) const
{
  LOG(INFO) << "Writing vocabulary to file: \"" << outputFileName << "\" ..."
      << std::endl;
  File outputFile(outputFileName.c_str(), "w");
  for (size_t i = 0; i < _vocabulary.size(); ++i)
  {
    outputFile.write(_vocabulary[i].c_str(), _vocabulary[i].size());
    outputFile.write("\n", 1);
  }
  LOG(INFO) << "done." << flush << endl;
}
}
