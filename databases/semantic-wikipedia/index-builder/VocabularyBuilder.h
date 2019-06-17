// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_VOCABULARYBUILDER_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_VOCABULARYBUILDER_H_

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../codebase/semantic-wikipedia-utils/Comparators.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
class VocabularyBuilder
{
  public:

    static const size_t ASCII_LINE_BUFFER_SIZE = 2048;
    explicit VocabularyBuilder();
    ~VocabularyBuilder()
    {
    }

    //! Builds a vocabulary from the Words file (full text parser output)
    //! specified as @param inputFileName.
    //! The vocabulary is constructed and kept in memory. No writing
    //! of files is triggered by this method.
    //! Calls the templated version with the NoEntitiesWordExtractor.
    //! This extractor assumes words are always located at the beginning
    //! of a line, terminated by a tab, and ignores entity words.
    //!
    //! The method also uses the default sort order which
    //! sorts words lexicographically.
    void constructVocabularyFromASCIIWordsFile(const string& inputFileName);

    //! Builds a vocabulary for the given ontology (i.e. ID for entites,
    //! relations, classes, etc).
    //! Calls the templated version with the OntologyWordExtractor.
    //! The method also uses the default sort order which
    //! sorts words lexicographically.
    void constructVocabularyFromOntology(const string& inputFileName);

    //! Builds a vocabulary from the ASCII file specified.
    //! The vocabulary is constructed and kept in memory. No writing
    //! of files is triggered by this method.
    //!
    //! @param inputFiles:
    //!       List of ASCII input files whose union is treated as input.
    //! @param additions:
    //!       Possible additions that should be in the vocabulary but
    //!       are not in the input.
    //!       Example: name of the has-realtions relations.
    //! @param wordExtractor:
    //!       Class that takes care of extracting the actual word from a line
    //!       in the input ascii file. (Used to be the first column always).
    //! @param comp:
    //!       Comparator that determines the sort-order of the words in the
    //!       vocabulary. This allows defining rules such as
    //!       "Entities always after normal words" and whatever may be
    //!       useful in the future.
    template<class WordExtractor, class DesiredSortOrderComparator>
    void constructVocabularyFromASCIIFile(const string& inputFileName,
        const vector<string>& additions,
        const WordExtractor& wordExtractor,
        const DesiredSortOrderComparator& comp);

    //! Writes the current vocabulary (has to be constructed previously)
    //! to the specified output file.
    //! @param outputFileName:
    //!       The name of the output file to be written.
    void writeVocabularyToOutputFiles(const string& outputFileName) const;

    // Friend tests
    FRIEND_TEST(VocabularyBuilderTest, constructVocabularyWordsfile);
    FRIEND_TEST(VocabularyBuilderTest, constructVocabularyOntology);
    FRIEND_TEST(VocabularyBuilderTest, getWordId);

  private:
    // The actual vocabulary, stored as sorted vector.
    vector<string> _vocabulary;

    // Get the word_id for a certain word from the sorted vocabulary.
    // Returns -1 if the word is not found at all.
    template<class SortOrderComparator>
    signed int
    getWordId(const std::string& word, const SortOrderComparator& comp) const;

    //! Gets the ID for a given word.
    signed int getWordId(const std::string& word) const;

    // Default WordExtractor that assumes that words are always
    // located in the beginning of a line and terminated by a tab.
    class DefaultWordExtractor
    {
      public:
        vector<string> operator()(const string& line) const
        {
          size_t posOfFirstTab = line.find('\t');
          vector<string> extracted(1);
          extracted[0] = line.substr(0, posOfFirstTab);
          return extracted;
        }
    };

    //! OntologyWordExtractor. Takes an ASCII ontology with 5 columns:
    //! realtion <tab> sryType <tab> destType <tab> srcVal <tab> destVal
    //! and extracts the words from all five columns
    class OntologyWordExtractor
    {
      public:
        void operator()(const string& line, vector<string>* extracted) const
        {
          size_t posOfFirstTab = line.find('\t');
          size_t posOfSecondTab = line.find('\t', posOfFirstTab + 1);
          size_t posOfThirdTab = line.find('\t', posOfSecondTab + 1);
          size_t posOfFourthTab = line.find('\t', posOfThirdTab + 1);
          extracted->push_back(line.substr(0, posOfFirstTab));
          // Also account for reversed relations.
          // This means we "extract" a word that is not actually there
          // so we can have it in the vocabulary. While it may be a little
          // bit error-prone, it is definitely more practical than writing
          // both directions of each and every relation to the ASCII file.
          extracted->push_back(
              line.substr(0, posOfFirstTab) + REVERSED_RELATION_SUFFIX);
          extracted->push_back(
              line.substr(posOfFirstTab + 1,
                  posOfSecondTab - (posOfFirstTab + 1)));
          extracted->push_back(
              line.substr(posOfSecondTab + 1,
                  posOfThirdTab - (posOfSecondTab + 1)));
          extracted->push_back(
              line.substr(posOfThirdTab + 1,
                  posOfFourthTab - (posOfThirdTab + 1)));
          extracted->push_back(
              line.substr(posOfFourthTab + 1,
                  line.size() - (posOfFourthTab + 1)));
        }
    };

    // WordExtractor that assumes that words are always
    // located in the beginning of a line and terminated by a tab
    // but ignores entities completely and extracts nothing for them.
    class DefaultWordExtractorNoEntities
    {
      public:
        void operator()(const string& line, vector<string>* extracted) const
        {
          size_t posOfFirstTab = line.find('\t');
          string word = line.substr(0, posOfFirstTab);

          if (!ad_utility::startsWith(word, ENTITY_PREFIX))
          {
            (*extracted).push_back(word);
          }
        }
    };
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_VOCABULARYBUILDER_H_
