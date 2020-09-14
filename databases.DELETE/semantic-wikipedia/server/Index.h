// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_INDEX_H_
#define SEMANTIC_WIKIPEDIA_SERVER_INDEX_H_

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

#include "./PostingList.h"
#include "./Relation.h"
#include "./EntityList.h"
#include "./Identifiers.h"
#include "./Vocabulary.h"
#include "./IndexMetaData.h"
#include "./ExcerptOffsetCollection.h"

using ad_utility::File;
using std::string;
using std::vector;

namespace ad_semsearch
{
//! The search-index behind everything.
//! Provides access to different indexed lists and operations
//! on those lists.
class Index
{
  public:
    Index();
    virtual ~Index();

    //! Reads a block from disc and into a posting list.
    //! Assumes a file layout that matches the info in the metaData.
    void readBlock(File& file, const BlockMetaData& blockMetaData,
        PostingList* postingList) const;

    //! Reads a block from disc and into a posting list.
    //! Assumes a file layout that matches the info in the metaData.
    void readBlock(const BlockMetaData& blockMetaData,
        PostingList* postingList, size_t numberOfFulltextIndex = 0) const
    {
      AD_CHECK_GT(_fullTextIndexes.size(), numberOfFulltextIndex);
      AD_CHECK(_fullTextIndexes[numberOfFulltextIndex].isOpen());
      readBlock(_fullTextIndexes[numberOfFulltextIndex], blockMetaData,
          postingList);
    }

    //! Reads a relation from disc.
    void readFullRelation(File* file, const RelationMetaData& relMetaData,
        Relation* relationList) const;

    //! Reads a Relation Block from disc and appends to relationRead.
    void readRelationBlock(File* file,
        const RelationBlockMetaData& blockMetaData,
        Relation* relationList) const;

    //! Get the special has-relations relation.
    const Relation& getHasRelationsRelation() const
    {
      AD_CHECK(_initialized);
      return _hasRelationsRelation;
    }

    //! Get the special available classes EntityList.
    const EntityList& getAvailableClasses() const
    {
      AD_CHECK(_initialized);
      return _availableClasses;
    }


    //! Reads a relation from disc. Use the registered ontologyIndex
    //! as file.
    void readFullRelation(const RelationMetaData& relMetaData,
        Relation* relationList) const
    {
      AD_CHECK(_ontologyIndex.isOpen());
      readFullRelation(&_ontologyIndex, relMetaData, relationList);
    }

    //! Reads a Relation Block from disc and appends to relationRead.
    //! Use the registered ontologyIndex as file.
    void readRelationBlock(const RelationBlockMetaData& blockMetaData,
        Relation* relationList) const
    {
      AD_CHECK(_ontologyIndex.isOpen());
      readRelationBlock(&_ontologyIndex, blockMetaData, relationList);
    }

    const FulltextMetaData& getFullTextMetaData(
        size_t numberOfFulltextIndex = 0) const
    {
      AD_CHECK_GT(_fulltextMetaData.size(), numberOfFulltextIndex);
      return _fulltextMetaData[numberOfFulltextIndex];
    }

    //! Gets an excerpt from the excerpts-file. No highlighting set/done yet.
    string getRawExcerptForContextId(Id contextId,
        size_t numberOfFulltextIndex = 0) const;

    //! Registers a full-text index file. Reads meta data
    //! and the corresponding vocabulary.
    //! Note that this will register the index as additional
    //! index and not replace any existing index.
    //! ContextId are assumed to be disjunct.
    void registerFulltextIndex(const string& baseName,
        bool alsoRegisterExcerptsFile);

    //! Clears the list of registered full-text indexes
    void clearRegisteredFulltextIndexes()
    {
      _fullTextIndexes.clear();
      _fulltextMetaData.clear();
      _fulltextVocabularies.clear();
    }

    //! Tries to get the word represented by an arbitrary ID.
    //! Take care: If id is a full-text-word-id, numberOfFulltextIndex
    //! has to be passed correctly to avoid throwing an Exception.
    //! In general, callers should prefer calling getFulltextWordById and
    //! getOntologyWordById depending on what is needed.
    const string& getWordById(Id id, size_t numberOfFulltextIndex = 0) const
    {
      if (isIdOfType(id, IdType::ONTOLOGY_ELEMENT_ID))
      {
        return getOntologyWordById(id);
      }
      else
      {
        return getFulltextWordById(id, numberOfFulltextIndex);
      }
    }

    //! Get an Id from the vocabulary for some ontology word.
    bool getIdForOntologyWord(const string& word, Id* id) const
    {
      return _ontologyVocabulary.getIdForOntologyWord(word, id);
    }

    //! Get an IdRange from the vocabulary for some full-text word-prefix.
    //! If the word is no prefix first and last ID in the range will be equal.
    //! The return value signals if the word / prefix have been found at all.
    bool getIdRangeForFullTextWordOrPrefix(const string& word,
        IdRange* range) const
    {
      AD_CHECK_GT(word.size(), 0);
      if (word[word.size() - 1] == PREFIX_CHAR)
      {
        return _fulltextVocabularies[0].getIdRangeForFullTextPrefix(word,
            range);
      }
      else
      {
        Id wordId;
        bool success = _fulltextVocabularies[0].getIdForFullTextWord(word,
            &wordId);
        range->_first = wordId;
        range->_last = wordId;
        return success;
      }
    }

    //! Get an IdRange from the vocabulary for some ontology word-prefix.
    //! If the word is no prefix first and last ID in the range will be equal.
    //! The return value signals if the word / prefix have been found at all.
    bool getIdRangeForOntologyWordOrPrefix(const string& word,
        IdRange* range) const
    {
      AD_CHECK_GT(word.size(), 0);
      if (word[word.size() - 1] == PREFIX_CHAR)
      {
        return _ontologyVocabulary.getIdRangeForOntologyPrefix(word,
            range);
      }
      else
      {
        Id id;
        bool success = _ontologyVocabulary.getIdForOntologyWord(word,
            &id);
        range->_first = id;
        range->_last = id;
        return success;
      }
    }

    //! Gets block meta data for a given range of full-text word ids.
    const BlockMetaData& getBlockInfoByWordRange(IdRange idRange) const
    {
      return _fulltextMetaData[0].getBlockInfoByWordRange(idRange._first,
          idRange._last);
    }

    //! Gets block meta data for a single full-text word id.
    const BlockMetaData& getBlockInfoByFulltextWordId(Id wordId) const
    {
      return _fulltextMetaData[0].getBlockInfoByWordRange(wordId, wordId);
    }

    //! Gets a word from the full-text vocabulary represented by that particular
    //! Id. Take care that the correct full-text index is used or else
    //! an exception will be thrown.
    const string& getFulltextWordById(Id wordId,
        size_t numberOfFulltextIndex = 0) const
    {
      AD_CHECK_LT(numberOfFulltextIndex, _fulltextVocabularies.size());
      AD_CHECK_LT(wordId, _fulltextVocabularies[numberOfFulltextIndex].size());
      return _fulltextVocabularies[numberOfFulltextIndex][wordId];
    }

    //! Gets the ontology word represented by the ID passed.
    const string& getOntologyWordById(Id ontologyWordId) const
    {
      AD_CHECK_LT(getPureValue(ontologyWordId), _ontologyVocabulary.size());
      return _ontologyVocabulary[getPureValue(ontologyWordId)];
    }


    //! Get the meta data for a given relation by relationId.
    const RelationMetaData& getRelationMetaData(Id relationId) const
    {
      return _ontologyMetaData.getRelationMetaData(relationId);
    }

    //! Registers the ontology-index and vocabulary.
    //! Note that only one ontology-index is supported at a time
    //! and registering an index will remove any old one.
    void registerOntologyIndex(const string& baseName);

    //! Initialize this object properly, i.e. read the has-relations relation.
    void initInMemoryRelations();

    // Friend tests (most tests don't need friendship so list is short):
    friend class IndexPerformanceTest;
    FRIEND_TEST(IndexTest, registerFulltextIndexTest);
    FRIEND_TEST(IndexTest, registerOntologyIndexTest);
    FRIEND_TEST(IndexTest, readFullRelationTest);
    FRIEND_TEST(IndexTest, readRelationBlockTest);

  private:

    bool _initialized;

    // The single registered ontology index and related members.
    mutable File _ontologyIndex;
    Vocabulary _ontologyVocabulary;
    OntologyMetaData _ontologyMetaData;

    // A list of registered full-text indexes and associated information.
    mutable vector<File> _fullTextIndexes;
    vector<Vocabulary> _fulltextVocabularies;
    vector<FulltextMetaData> _fulltextMetaData;

    mutable vector<File> _excerptFiles;
    vector<ExcerptOffsetCollection> _excerptOffsetCollections;

    //! Always keep in memory:
    Relation _hasRelationsRelation;
    EntityList _availableClasses;

    //! Reads the has-instances relation from disc and drives an
    //! EntityList of available classes.
    //! Use the registered ontologyIndex as file.
    void readAvailableClasses(File* ontolgyIndex,
        const RelationMetaData& relMetaData,
        EntityList* availableClassesList) const;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_INDEX_H_
