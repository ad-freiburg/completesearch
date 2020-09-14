// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_EXTRACTORS_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_EXTRACTORS_H_

#include <gtest/gtest.h>
#include <stxxl/vector>
#include <string>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../server/Identifiers.h"
#include "./Postings.h"
#include "./Facts.h"

using std::string;
using std::vector;
using std::endl;

using ad_utility::HashMap;

namespace ad_semsearch
{
class ExtractorBase
{
  public:

    ExtractorBase() :
      _ontologyVocabulary(&_emptyVocabulary)
    {
    }

    virtual ~ExtractorBase()
    {
    }

    void setOntologyVocabulary(const HashMap<string, Id>* ontologyVocabulary)
    {
      _ontologyVocabulary = ontologyVocabulary;
    }

  protected:
    const HashMap<string, Id>* _ontologyVocabulary;
    HashMap<string, Id> _emptyVocabulary;

    virtual Id getOntologyElementId(const string& element) const;
};
//! Base-class for Extractors that are supposed to extract
//! some kind of posting from ascii full-text files.
class WordsFileExtractorBase : public ExtractorBase
{
  public:
    WordsFileExtractorBase() :
      ExtractorBase(), _textVocabulary(&_emptyVocabulary)
    {
    }

    virtual ~WordsFileExtractorBase()
    {
    }


    void setTextVocabulary(const HashMap<string, Id>* textVocabulary)
    {
      _textVocabulary = textVocabulary;
    }

    void setTextBlockBoundaries(const vector<string>& blockBoundraries)
    {
      setBlockBoundaries(blockBoundraries, false);
    }

    void setOntologyBlockBoundaries(
        const vector<string>& blockBoundraries)
    {
      setBlockBoundaries(blockBoundraries, true);
    }

    // Get the block Id for a given word or ontology element.
    Id getBlockId(const Id& wordId) const
    {
      if (isIdOfType(wordId, IdType::ONTOLOGY_ELEMENT_ID))
      {
        return (getFirstId(IdType::ONTOLOGY_ELEMENT_ID)
            + _blockIdMapForOntology[getPureValue(wordId)]);
      }
      else
      {
        return _blockIdMapForText[getPureValue(wordId)];
      }
    }

    bool isProperlyInitialized() const;

  protected:
    const HashMap<string, Id>* _textVocabulary;
    vector<Id> _blockIdMapForOntology;
    vector<Id> _blockIdMapForText;

  private:
    FRIEND_TEST(WordsFileExtractorBaseTest, setBlockBoundariesTest);
    virtual void setBlockBoundaries(const vector<string>& blockBoundraries,
        bool forOntology);
};

class BasicExtractor: public WordsFileExtractorBase
{
  public:
    virtual void extract(const string& line, BasicPosting* posting) const;
};

class DefaultExtractor: public WordsFileExtractorBase
{
  public:
    virtual void extract(const string& line, Posting* posting) const;
};

class OntologyTripleExtractor : public ExtractorBase
{
  public:
    virtual void extract(const string& asciiFileName,
        stxxl::vector<RelationFact>* facts,
        vector<RelationMetaData>* realtionMetaData) const;
};
}

#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_EXTRACTORS_H_
