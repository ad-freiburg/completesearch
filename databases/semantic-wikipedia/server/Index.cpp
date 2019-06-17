// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>

#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "./Index.h"
#include "./Identifiers.h"
#include "./Relation.h"
#include "./Excerpt.h"

using std::string;
using std::endl;

using ad_utility::File;

namespace ad_semsearch
{
// _____________________________________________________________________________
void Index::readBlock(File& file, const BlockMetaData& blockMetaData,
    PostingList* postingList) const
{
  LOG(DEBUG) << "Reading block from disk." << endl;
  size_t nofElements = blockMetaData._nofPostings;

  Id* words = new Id[nofElements];
  Id* contexts = new Id[nofElements];
  Score* scores = new Score[nofElements];
  Position* positions = new Position[nofElements];

  size_t ret = file.read(words, nofElements * sizeof(Id),
      blockMetaData._startOfWordList);
  AD_CHECK_EQ(ret, nofElements * sizeof(Id));
  ret = file.read(contexts, nofElements * sizeof(Id),
      blockMetaData._startOfContextList);
  AD_CHECK_EQ(ret, nofElements * sizeof(Id));
  ret = file.read(scores, nofElements * sizeof(Score),
      blockMetaData._startOfScoreList);
  AD_CHECK_EQ(ret, nofElements * sizeof(Score));
  ret = file.read(positions, nofElements * sizeof(Position),
      blockMetaData._startOfPositionList);
  AD_CHECK_EQ(ret, nofElements * sizeof(Position));

  for (size_t i = 0; i < nofElements; ++i)
  {
    postingList->push_back(
        TextPosting(words[i], contexts[i], scores[i], positions[i]));
  }

  delete[] words;
  delete[] contexts;
  delete[] scores;
  delete[] positions;

  LOG(DEBUG) << "Done reading block from disk. Size: " << postingList->size()
      << endl;
}
// _____________________________________________________________________________
void Index::registerFulltextIndex(const string& baseName,
    bool alsoRegisterExcerptsFile)
{
  LOG(INFO) << "Registering Fulltext-Index with basename: " << baseName
      << endl;
  string indexFileName = baseName + INDEX_FILE_EXTENSION;
  string vocabFileName = baseName + VOCABULARY_FILE_EXTENSION;
  string excerptsFileName = baseName + EXCERPTS_FILE_EXTENSION;
  string excerptsOffsetsFileName = baseName + ".docs-offsets";
  _fullTextIndexes.push_back(File(indexFileName.c_str(), "r"));
  _fulltextVocabularies.push_back(Vocabulary());
  _fulltextVocabularies.back().readFromFile(vocabFileName);
  FulltextMetaData meta;
  meta.initFromFile(&_fullTextIndexes.back());
  _fulltextMetaData.push_back(meta);
  AD_CHECK_EQ(_fullTextIndexes.size(), _fulltextMetaData.size());
  AD_CHECK_EQ(_fullTextIndexes.size(), _fulltextVocabularies.size());
  if (alsoRegisterExcerptsFile)
  {
    _excerptFiles.resize(_fullTextIndexes.size() - 1);
    _excerptFiles.push_back(File(excerptsFileName.c_str(), "r"));
    _excerptOffsetCollections.resize(_fullTextIndexes.size());
    ExcerptOffsetCollection& offsets = _excerptOffsetCollections.back();
    offsets.readFromFile(excerptsOffsetsFileName);
  }
  LOG(INFO)
      << "Registration of Fulltext-Index complete. There are "
      << meta.getBlockCount() << " blocks." << endl;
  LOG(DEBUG)
      << "The registered Fulltext-Index has "
      << meta.calculateTotalNumberOfPostings() << " postings in total."
      << endl;
}
// _____________________________________________________________________________
void Index::registerOntologyIndex(const string& baseName)
{
  LOG(INFO) << "Registering Ontology-Index with basename: " << baseName
      << endl;
  string indexFileName = baseName + INDEX_FILE_EXTENSION;
  string vocabFileName = baseName + VOCABULARY_FILE_EXTENSION;
  _ontologyIndex.open(indexFileName.c_str(), "r");
  _ontologyVocabulary.readFromFile(vocabFileName);
  _ontologyMetaData.initFromFile(&_ontologyIndex);
  LOG(INFO)
      << "Registration of Ontology-Index complete. There are "
      << _ontologyMetaData.getRelationCount() << " relations." << endl;
}
// _____________________________________________________________________________
void Index::readFullRelation(File* file, const RelationMetaData& relMetaData,
    Relation* relationList) const
{
  LOG(DEBUG) << "Reading full relation from disk." << endl;
  AD_CHECK(relationList);
  AD_CHECK_EQ(relationList->size(), 0);
  for (size_t i = 0; i < relMetaData._blockInfo.size(); ++i)
  {
    readRelationBlock(file, relMetaData._blockInfo[i], relationList);
  }
  LOG(DEBUG) << "Done reading relation. Size: " << relationList->size() << endl;
}
// _____________________________________________________________________________
void Index::readRelationBlock(File* file,
    const RelationBlockMetaData& blockMetaData, Relation* relationList) const
{
  LOG(DEBUG) << "Reading relation-block from disk." << endl;
  AD_CHECK(relationList);
  size_t nofElements = blockMetaData._nofElements;

  Id* content = new Id[2* nofElements];

  size_t ret = file->read(content, nofElements * sizeof(Id) * 2,
      blockMetaData._startOfLhsData);
  AD_CHECK_EQ(ret, nofElements * sizeof(Id) * 2);

  for (size_t i = 0; i < nofElements; ++i)
  {
    RelationEntry entry(content[i], content[i + nofElements]);
    relationList->push_back(entry);
  }

  delete[] content;

  LOG(DEBUG) << "Done reading block. " << "Current size of target list "
      << "(not necessarily everything from this block): "
      << relationList->size() << endl;
}
// _____________________________________________________________________________
string Index::getRawExcerptForContextId(
    Id contextId, size_t numberOfFulltextIndex) const
{
  LOG(DEBUG) << "Trying to read excerpt for context Id " << contextId << endl;
  off_t offset = _excerptOffsetCollections[numberOfFulltextIndex]
                  .getExcerptOffsetForContextId(contextId);
  string excerpt;
  File& excerptsFile = _excerptFiles[numberOfFulltextIndex];
  excerptsFile.seek(offset, 0);
  char buf[BUFFER_SIZE_DOCSFILE_LINE];
  excerptsFile.readLine(&excerpt, buf, BUFFER_SIZE_DOCSFILE_LINE);
  LOG(DEBUG)
  << "Done reading excerpt. It's " << excerpt.size() << " bytes long" << endl;
  return excerpt;
}
// _____________________________________________________________________________
void Index::initInMemoryRelations()
{
  AD_CHECK(!_initialized)
  AD_CHECK_GT(_ontologyVocabulary.size(), 0);

  // The has-relations relation.
  {
    _hasRelationsRelation.clear();
    Id relId;
    getIdForOntologyWord(string(RELATION_PREFIX) + HAS_RELATIONS_RELATION,
        &relId);
    const RelationMetaData& rmd = getRelationMetaData(relId);
    readFullRelation(rmd, &_hasRelationsRelation);
  }

  // The available classes list
  {
    _availableClasses.clear();
    Id relId;
    getIdForOntologyWord(string(RELATION_PREFIX) + HAS_INSTANCES_RELATION,
        &relId);
    const RelationMetaData& rmd = getRelationMetaData(relId);
    AD_CHECK(_ontologyIndex.isOpen());
    readAvailableClasses(&_ontologyIndex, rmd, &_availableClasses);
  }
  _initialized = true;
}
// _____________________________________________________________________________
void Index::readAvailableClasses(File* ontolgyIndex,
    const RelationMetaData& relMetaData,
    EntityList* availableClassesList) const
{
  LOG(DEBUG) << "Reading available classes from disk." << endl;
  AD_CHECK(availableClassesList);
  AD_CHECK_EQ(availableClassesList->size(), 0);

  AD_CHECK_EQ(relMetaData._blockInfo.size(), 1);
  const RelationBlockMetaData& blockMetaData = relMetaData._blockInfo[0];
  size_t nofElements = blockMetaData._nofElements;

  Id* content = new Id[2 * nofElements];

  size_t ret = ontolgyIndex->read(content, nofElements * sizeof(Id) * 2,
      blockMetaData._startOfLhsData);
  AD_CHECK_EQ(ret, nofElements * sizeof(Id) * 2);

  for (size_t i = 0; i < nofElements; ++i)
  {
    EntityWithScore entry(content[i],
        static_cast<AggregatedScore>(content[i + nofElements]));
    availableClassesList->push_back(entry);
  }

  delete[] content;

  LOG(DEBUG)
  << "Done reading available classes. Read " << availableClassesList->size()
      << " classes," << endl;
}
// _____________________________________________________________________________
Index::Index()
: _initialized(false), _ontologyIndex(), _ontologyVocabulary(),
  _ontologyMetaData(), _fullTextIndexes(),
  _fulltextVocabularies(), _fulltextMetaData(), _hasRelationsRelation()
{
}

// _____________________________________________________________________________
Index::~Index()
{
}
}
