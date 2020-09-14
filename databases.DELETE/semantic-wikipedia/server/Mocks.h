// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_MOCKS_H_
#define SEMANTIC_WIKIPEDIA_SERVER_MOCKS_H_

#include <vector>
#include <string>
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./Vocabulary.h"

using std::vector;
using std::string;

namespace ad_semsearch
{
class IndexMock: public Index
{
  public:
    IndexMock()
    {
      Vocabulary vocab;
      vocab.push_back("a");
      vocab.push_back("b");
      vocab.push_back("c");
      _fulltextVocabularies.push_back(vocab);

      _ontologyVocabulary.push_back(":e:a");
      _ontologyVocabulary.push_back(":e:b");
      _ontologyVocabulary.push_back(":e:c");
    }
    void readBlock(const BlockMetaData& blockMetaData,
        PostingList* postingList, size_t numberOfFulltextIndex = 0) const
    {
      Id word0 = getFirstId(IdType::WORD_ID);
      Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
      postingList->push_back(TextPosting(word0, 0, 1, 0));
      postingList->push_back(TextPosting(entity0, 0, 1, 1));
      postingList->push_back(TextPosting(word0 + 1, 1, 1, 0));
      postingList->push_back(TextPosting(entity0 + 1, 1, 1, 1));
    }

    void readFullRelation(const RelationMetaData& relMetaData,
        Relation* relationList) const
    {
      Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
      relationList->push_back(RelationEntry(entity0, entity0 + 2));
    }

    //! Reads a Relation Block from disc and appends to relationRead.
    //! Use the registered ontologyIndex as file.
    void readRelationBlock(const RelationBlockMetaData& blockMetaData,
        Relation* relationList) const
    {
      Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
      relationList->push_back(RelationEntry(entity0, entity0 + 2));
    }

    const FulltextMetaData& getFullTextMetaData(size_t numberOfFulltextIndex =
        0) const
    {
      AD_CHECK_GT(_fulltextMetaData.size(), numberOfFulltextIndex);
      return _fulltextMetaData[numberOfFulltextIndex];
    }

    //! Gets an excerpt from the excerpts-file. No highlighting set/done yet.
    string getRawExcerptForContextId(Id contextId,
        size_t numberOfFulltextIndex = 0) const
    {
      return "@raw @mock @excerpt";
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_MOCKS_H_
