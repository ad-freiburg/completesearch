// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stxxl/vector>
#include <stxxl/algorithm>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "../server/Identifiers.h"
#include "./IndexBuilderBase.h"
#include "./IndexBuilderComparators.h"
#include "./Postings.h"
#include "./Extractors.h"
#include "./PlainSerializer.h"
#include "./PlainOntologySerializer.h"
#include "./Facts.h"

using std::string;
using std::vector;
using std::endl;
using std::flush;

using ad_utility::File;

namespace ad_semsearch
{
// _____________________________________________________________________________
void IndexBuilderBase::setOntologyVocabulary(
    const string& ontologyVocabularyFilename)
{
  File vocFile(ontologyVocabularyFilename.c_str(), "r");
  string line;
  char buf[BUFFER_SIZE_ONTOLOGY_WORD];
  Id id = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  while (vocFile.readLine(&line, buf, BUFFER_SIZE_ONTOLOGY_WORD))
  {
    _ontologyVocabulary[line] = id;
    ++id;
  }
  // Make sure no overflow occurred.
  AD_CHECK(isIdOfType(id - 1, IdType::ONTOLOGY_ELEMENT_ID));
}
// _____________________________________________________________________________
template<class Extractor, class Posting, class Comparator, class Serializer>
void IndexBuilderBase::doBuildFulltextIndex(const string& asciiFilename,
    const string& vocabularyFilename,
    const string& fulltextBlockBoundariesFilename,
    const string& ontologyBlockBoundariesFilename,
    const string& outputFilename) const
{
  stxxl::vector<Posting> postings;
  Comparator comparator;

  // Read the vocabulary
  HashMap<string, Id> vocabulary;
  readFulltextVocabulary(vocabularyFilename, &vocabulary);

  // Read the boundary files.
  vector<string> ontologyBlockBoundaries;
  char buf[BUFFER_SIZE_WORD];
  File ontologyBlockBoundariesFile(ontologyBlockBoundariesFilename.c_str(),
      "r");
  ontologyBlockBoundariesFile.readIntoVector(&ontologyBlockBoundaries, buf,
      BUFFER_SIZE_WORD);
  vector<string> fulltextBlockBoundaries;
  File fulltextBlockBoundariesFile(fulltextBlockBoundariesFilename.c_str(),
      "r");
  fulltextBlockBoundariesFile.readIntoVector(&fulltextBlockBoundaries, buf,
      BUFFER_SIZE_WORD);
  std::sort(fulltextBlockBoundaries.begin(), fulltextBlockBoundaries.end());

  // Extract from the ASCII file.
  LOG(INFO)
      << "Initializing the extractor with vocabularies and block boundaries ..."
      << endl;
  Extractor extractor;
  extractor.setOntologyVocabulary(&_ontologyVocabulary);
  extractor.setTextVocabulary(&vocabulary);
  extractor.setOntologyBlockBoundaries(ontologyBlockBoundaries);
  extractor.setTextBlockBoundaries(fulltextBlockBoundaries);
  LOG(INFO) << "Initialization done." << endl << flush;
  LOG(INFO) << "Extracting from the ASCII words-file "
      << "and converting to binary postings ..." << endl << flush;

  _extractionTimer.start();
  _multiplyTimer.reset();
  extractPostingsFromAsciiFulltextFile(asciiFilename, extractor, &postings,
      comparator);
  _extractionTimer.stop();
  LOG(INFO) << "done in " << _extractionTimer.secs()
      << " seconds. Whereas multiplying entity postings took "
      << _multiplyTimer.secs() << " seconds." << endl << flush;

  // Sort the index.
  LOG(INFO) << "Sorting the index (stxxl file-based sort)... " << endl << flush;

  _sortTimer.start();
  stxxl::sort(postings.begin(), postings.end(), comparator,
      STXXL_MEMORY_TO_USE);
  _sortTimer.stop();
  LOG(INFO) << "done in " << _sortTimer.secs() << " seconds." << endl << flush;

  // Write to file.
  LOG(INFO) << "Serializing the index to file... " << endl << flush;
  _serializationTimer.start();
  Serializer serializer;
  serializer.serialize(postings, outputFilename);
  _serializationTimer.stop();
  LOG(INFO) << "done in " << _serializationTimer.secs() << " seconds." << endl
      << flush;
}
// _____________________________________________________________________________
template<class Extractor, class Fact, class Serializer>
void IndexBuilderBase::doBuildOntologyIndex(const string& asciiFilename,
    const vector<string>& relationsToBeSplitIntoBlocksByLhs,
    const string& instancesCountsFile, const string& outputFilename) const
{
  // Read the Ontology ASCII file line by line.
  // This fills the actual triples in relations and meta data concerning
  // the relations.
  stxxl::vector<Fact> triples;
  vector<RelationMetaData> relationMetaData;

  // Extract from the ASCII file.
  LOG(INFO) << "Extracting from the ASCII ontology file "
      << "and converting to binary postings ..." << endl;
  Extractor extractor;
  extractor.setOntologyVocabulary(&_ontologyVocabulary);
  _extractionTimer.start();
  extractor.extract(asciiFilename, &triples, &relationMetaData);
  _extractionTimer.stop();
  LOG(INFO) << "done. Took " << _extractionTimer.secs() << " secs." << endl;

  // Create the special relation entity -> relation "has relations".
  LOG(INFO)
    << "Creating the special relation has-relations..." << endl << flush;
  _hasRelationsTimer.start();
  RelationMetaData hasRelationsMetaData = resolveMetaDataForHasRelations();
  relationMetaData.push_back(hasRelationsMetaData);
  createHasRelations(&triples);
  _hasRelationsTimer.stop();
  LOG(INFO) << "done. Took " << _hasRelationsTimer.secs() << " secs." << endl;

  // Create the special relation class -> #instances "has instances".
  if (instancesCountsFile.size() > 0)
  {
    LOG(INFO)
    << "Creating the special relation has-instances..." << endl << flush;
    _hasInstancesTimer.start();
    File classInstancesCounts(instancesCountsFile.c_str(), "r");
    RelationMetaData hasInstancesMetaData = resolveMetaDataForHasInstances();
    relationMetaData.push_back(hasInstancesMetaData);
    createHasInstances(&classInstancesCounts, &triples);
    _hasInstancesTimer.stop();
    LOG(INFO)
    << "done. Took " << _hasInstancesTimer.secs() << " secs." << endl;
  }
  else
  {
    LOG(WARN)
    << "No file with class-instances counts supplied. "
        << "Not creating has-instances relation! " << endl << flush;
  }

  // Sort the relations
  FactsComp<Fact> comp;
  RelationMetaDataComp rmdComp;
  LOG(INFO) << "Sorting the relations..." << endl << flush;
  _sortTimer.start();
  stxxl::sort(triples.begin(), triples.end(), comp, STXXL_MEMORY_TO_USE);
  std::sort(relationMetaData.begin(), relationMetaData.end(), rmdComp);
  _sortTimer.stop();
  LOG(INFO) << "done. Took " << _sortTimer.secs() << " secs." << endl;

  // Determine Ids of relations that should be split into blocks.
  ad_utility::HashSet<Id> relationsToBeSplitIntoBlocks;
  for (size_t i = 0; i < relationsToBeSplitIntoBlocksByLhs.size(); ++i)
  {
    try
    {
      relationsToBeSplitIntoBlocks.insert(
          getOntologyElementId(relationsToBeSplitIntoBlocksByLhs[i]));
    }
    catch(const Exception& e)
    {
      LOG(ERROR) << e.getFullErrorMessage() << endl;
    }
  }

  // Serialize the whole thing to an index file.
  LOG(INFO) << "Serializing relations to file \"" << outputFilename << "\"..."
      << endl << flush;
  Serializer serializer;
  _serializationTimer.start();
  serializer.serialize(triples, relationMetaData,
      relationsToBeSplitIntoBlocks, outputFilename);
  _serializationTimer.stop();
  LOG(INFO) << "done. Took " << _serializationTimer.secs() << " secs." << endl;
}
// _____________________________________________________________________________
template<class Extractor, class Posting, class PostingComparator>
void IndexBuilderBase::extractPostingsFromAsciiFulltextFile(
    const string& asciiFileName, const Extractor& extractor,
    stxxl::vector<Posting>* postings, const PostingComparator& comp) const
{
  File asciiFile(asciiFileName.c_str(), "r");
  size_t nofLinesInAscii = 0;
  char buf[BUFFER_SIZE_ASCII_LINE];
  string line;

  // Data that is required longer than only for one line of the ASCII file.
  Id lastContextId(~Id(0));
  vector<Posting> wordPostings;
  vector<Posting> entityPostings;
  bool firstLine = true;

  // Read the file line-wise.
  while (asciiFile.readLine(&line, buf, BUFFER_SIZE_ASCII_LINE))
  {
    ++nofLinesInAscii;
    Posting posting;
    extractor.extract(line, &posting);
    if (!firstLine && lastContextId != posting._contextId)
    {
      // Change of the contextId!
      // Do the multiplication for the last context.
      _multiplyTimer.cont();
      multiply(wordPostings, entityPostings, postings, comp);
      _multiplyTimer.stop();
      // Reset what just has been processed.
      wordPostings.clear();
      entityPostings.clear();
    }
    firstLine = false;
    // Assign the information extracted form this line for further
    // usage.
    if (isIdOfType(posting._wordId, IdType::ONTOLOGY_ELEMENT_ID))
    {
      entityPostings.push_back(posting);
    }
    else
    {
      wordPostings.push_back(posting);
    }
    // Update the last context ID for the next round.
    lastContextId = posting._contextId;
  }
  // Do the multiplication for the last context.
  multiply(wordPostings, entityPostings, postings, comp);
  LOG(INFO) << "Read " << nofLinesInAscii << " lines / postings from ASCII file"
      << endl;
  LOG(INFO) << "After entity x word mulitplications, there are now "
      << postings->size() << " postings" << endl;
  LOG(INFO) << "Blowup factor: " << static_cast<double_t> (postings->size())
      / nofLinesInAscii << endl << flush;
}
// _____________________________________________________________________________
template<class Posting, class PostingComparator>
void IndexBuilderBase::multiply(const vector<Posting>& wordPostings,
    const vector<Posting>& entityPostings,
    stxxl::vector<Posting>* postings, const PostingComparator& comp) const
{
  // First insert all entities once, because their block information
  // may be corrupted later on.
  for (size_t i = 0; i < entityPostings.size(); ++i)
  {
    postings->push_back(entityPostings[i]);
  }

  typedef vector<Posting> PostingVector;
  PostingVector tempPostings;
  for (size_t i = 0; i < wordPostings.size(); ++i)
  {
    // Insert the each word posting.
    postings->push_back(wordPostings[i]);
    // Build the product with entity postings in the same context.
    for (size_t j = 0; j < entityPostings.size(); ++j)
    {
      tempPostings.push_back(entityPostings[j]);
      tempPostings.back()._blockId = wordPostings[i]._blockId;
    }
  }
  // Eliminate duplicates. The same entity may have been inserted
  // into the same block twice or more due to co-occurrence
  // With different words that are located in the same block.
  std::sort(tempPostings.begin(), tempPostings.end(), comp);
  tempPostings.resize(
      std::unique(tempPostings.begin(), tempPostings.end())
          - tempPostings.begin());
  for (size_t i = 0; i < tempPostings.size(); ++i)
  {
    postings->push_back(tempPostings[i]);
  }
}
// _____________________________________________________________________________
void IndexBuilderBase::readFulltextVocabulary(const string& filename,
    HashMap<string, Id>* vocabulary) const
{
  File vocFile(filename.c_str(), "r");
  string line;
  char buf[BUFFER_SIZE_WORD];
  Id id = getFirstId(IdType::WORD_ID);
  while (vocFile.readLine(&line, buf, BUFFER_SIZE_WORD))
  {
    (*vocabulary)[line] = id;
    ++id;
  }
  // Make sure no overflow occurred.
  AD_CHECK(isIdOfType(id - 1, IdType::WORD_ID));
}
// _____________________________________________________________________________
RelationMetaData IndexBuilderBase::resolveMetaDataForHasRelations() const
{
  try
  {
    Id relationId = getOntologyElementId(
        string(RELATION_PREFIX) + HAS_RELATIONS_RELATION);
    Id lhsTypeId = getOntologyElementId(string(TYPE_PREFIX) + "entity");
    Id rhsTypeId = getOntologyElementId(string(TYPE_PREFIX) + "relation");

    return RelationMetaData(relationId, lhsTypeId, rhsTypeId);
  }
  catch(const Exception& e)
  {
    LOG(FATAL) << e.getFullErrorMessage() << endl;
    exit(1);
  }
  // Unreachable code so that eclipse won't complain.
  // TODO(buchholb): Remove as soon as eclipse is able to handle this.
  return RelationMetaData();
}
// _____________________________________________________________________________
template<class Fact>
void IndexBuilderBase::createHasRelations(
    stxxl::vector<Fact>* allFactsSoFar) const
{
  if (allFactsSoFar->size() == 0) return;

  try
  {
    // Resolve relation Ids
    string hasRealtionsRelName = string(RELATION_PREFIX)
        + HAS_RELATIONS_RELATION;
    Id hasRelationsRelId = getOntologyElementId(hasRealtionsRelName);

    Id isARelId = ~Id(0);
    Id isAReversedRelId = ~Id(0);
    try
    {
      string isARelName = string(RELATION_PREFIX) + IS_A_RELATION;
      isARelId = getOntologyElementId(isARelName);
      string isAReversedRelName = isARelName + REVERSED_RELATION_SUFFIX;
      isAReversedRelId = getOntologyElementId(isAReversedRelName);
    } catch(const Exception& e)
    {
      LOG(WARN) << "No is-a relation found in the ontology vocabulary! "
          << "This should only happen in test-cases etc." << std::endl;
    }


    // Sort the facts.
    OrderFactsByLhsComparator<Fact> comp;
    stxxl::sort(allFactsSoFar->begin(), allFactsSoFar->end(), comp,
        STXXL_MEMORY_TO_USE);

    // Go through the sorted facts and collect all relations
    // for each entity. Append them to the facts list.
    Id lastEntityId = allFactsSoFar->begin()->_lhs;
    vector<Id> relationsForThisId;
    size_t originalSize = allFactsSoFar->size();
    for (size_t i = 0; i < originalSize; ++i)
    {
      Id currentLhs = allFactsSoFar->operator[](i)._lhs;
      if (currentLhs == lastEntityId)
      {
        if (allFactsSoFar->operator[](i)._relationId != isARelId
            && allFactsSoFar->operator[](i)._relationId != isAReversedRelId)
        {
          relationsForThisId.push_back(
              allFactsSoFar->operator[](i)._relationId);
        }
      }
      if (currentLhs != lastEntityId)
      {
        // Make the relations for the entity unique and append them.
        std::sort(relationsForThisId.begin(), relationsForThisId.end());
        relationsForThisId.resize(
            std::unique(relationsForThisId.begin(), relationsForThisId.end())
                - relationsForThisId.begin());
        for (size_t j = 0; j < relationsForThisId.size(); ++j)
        {
          allFactsSoFar->push_back(
              Fact(hasRelationsRelId, lastEntityId, relationsForThisId[j]));
        }
        lastEntityId = currentLhs;
        relationsForThisId.clear();
        if (allFactsSoFar->operator[](i)._relationId != isARelId
            && allFactsSoFar->operator[](i)._relationId != isAReversedRelId)
        {
          relationsForThisId.push_back(
              allFactsSoFar->operator[](i)._relationId);
        }
      }
    }
    // Process data for the very last LHS
    std::sort(relationsForThisId.begin(), relationsForThisId.end());
    relationsForThisId.resize(
        std::unique(relationsForThisId.begin(), relationsForThisId.end())
            - relationsForThisId.begin());
    for (size_t j = 0; j < relationsForThisId.size(); ++j)
    {
      allFactsSoFar->push_back(
          Fact(hasRelationsRelId, lastEntityId, relationsForThisId[j]));
    }
  }
  catch(const Exception& e)
  {
    LOG(FATAL) << e.getFullErrorMessage() << endl;
    exit(1);
  }
}
// _____________________________________________________________________________
RelationMetaData IndexBuilderBase::resolveMetaDataForHasInstances() const
{
  try
  {
    Id relationId = getOntologyElementId(
        string(RELATION_PREFIX) + HAS_INSTANCES_RELATION);
    Id lhsTypeId = getOntologyElementId(string(TYPE_PREFIX) + "class");
    Id rhsTypeId = getOntologyElementId(string(TYPE_PREFIX) + "count");

    return RelationMetaData(relationId, lhsTypeId, rhsTypeId);
  }
  catch(const Exception& e)
  {
    LOG(FATAL) << e.getFullErrorMessage() << endl;
    exit(1);
  }
  // Unreachable code so that eclipse won't complain.
  // TODO(buchholb): Remove as soon as eclipse is able to handle this.
  return RelationMetaData();
}
// _____________________________________________________________________________
template<class Fact>
void IndexBuilderBase::createHasInstances(File* classInstanceCounts,
    stxxl::vector<Fact>* allFactsSoFar) const
{
  try
  {
    Id hasInstancesRelationId = getOntologyElementId(
        string(RELATION_PREFIX) + HAS_INSTANCES_RELATION);

    string line;
    char buf[BUFFER_SIZE_ONTOLOGY_LINE];
    while (classInstanceCounts->readLine(&line, buf, BUFFER_SIZE_ONTOLOGY_LINE))
    {
      size_t indexOfTab = line.find('\t');
      std::istringstream iss(line.substr(0, indexOfTab));
      Id count;
      iss >> count;
      Id classId = getOntologyElementId(line.substr(indexOfTab + 1));
      allFactsSoFar->push_back(Fact(hasInstancesRelationId, classId, count));
    }
  }
  catch(const Exception& e)
  {
    LOG(FATAL) << e.getFullErrorMessage() << endl;
    exit(1);
  }
}
// _____________________________________________________________________________
Id IndexBuilderBase::getOntologyElementId(const string& element) const
{
  HashMap<string, Id>::const_iterator it = _ontologyVocabulary.find(element);
  if (it == _ontologyVocabulary.end())
  {
    AD_THROW(Exception::VOCABULARY_MISS,
        string("Error getting a wordId for ontology word \"")
        + element + "\"!");
  }
  return it->second;
}
//! EXPLICIT INSTANTIATIONS
template void
IndexBuilderBase::multiply(const vector<BasicPosting>& wordPostings,
    const vector<BasicPosting>& entityPostingsPtr,
    stxxl::vector<BasicPosting>* postings,
    const PostingComparatorEntitiesMixedIn<BasicPosting>& comp) const;

template void
IndexBuilderBase::extractPostingsFromAsciiFulltextFile(
    const string& asciiFileName, const BasicExtractor& extractor,
    stxxl::vector<BasicPosting>* postings,
    const PostingComparatorEntitiesMixedIn<BasicPosting>& comp) const;

template void
    IndexBuilderBase::multiply(const vector<Posting>& wordPostings,
        const vector<Posting>& entityPostingsPtr,
        stxxl::vector<Posting>* postings,
        const DefaultPostingComparator& comp) const;

template void
IndexBuilderBase::extractPostingsFromAsciiFulltextFile(
    const string& asciiFileName, const DefaultExtractor& extractor,
    stxxl::vector<Posting>* postings,
    const DefaultPostingComparator& comp) const;

template void
IndexBuilderBase::doBuildFulltextIndex<DefaultExtractor, Posting,
  DefaultPostingComparator, PlainSerializer>(
    const string& asciiFilename, const string& vocabularyFilename,
    const string& ontologyBlockBoundariesFilename,
    const string& fulltextBlockBoundariesFilename,
    const string& outputFilename) const;

template void
IndexBuilderBase::doBuildOntologyIndex<OntologyTripleExtractor,
  RelationFact, PlainOntologySerializer>(const string & asciiFilename,
    const vector<string>& relationsToBeSplitIntoBlocksByLhs,
    const string& instancesCountsFile,
    const string& outputfilename) const;

template void IndexBuilderBase::createHasRelations<RelationFact>(
    stxxl::vector<RelationFact>* allFactsSoFar) const;

template void IndexBuilderBase::createHasInstances(File* classInstanceCounts,
    stxxl::vector<RelationFact>* allFactsSoFar) const;
}
