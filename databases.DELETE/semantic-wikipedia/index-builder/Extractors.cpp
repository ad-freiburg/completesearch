// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stxxl/vector>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "./Extractors.h"
#include "./Postings.h"
#include "./Facts.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;
using std::vector;
using std::endl;
using std::flush;

using ad_utility::HashMap;

namespace ad_semsearch
{
// _____________________________________________________________________________
Id ExtractorBase::getOntologyElementId(const string& element) const
{
  HashMap<string, Id>::const_iterator it =
      (*_ontologyVocabulary).find(element);
  if (it == (*_ontologyVocabulary).end())
  {
    AD_THROW(Exception::VOCABULARY_MISS,
        string("Error getting a wordId for ontology word \"")
        + element + "\"!");
  }
  return it->second;
}
// _____________________________________________________________________________
void WordsFileExtractorBase::setBlockBoundaries(
    const vector<string>& boundaries, bool setForOntology)
{
  LOG(DEBUG)
      << "Setting boundaries file for " << (setForOntology ? "ontology"
      : "fulltext") << " in the extractor" << endl;
  LOG(DEBUG)
        << "Length of boundaries list is " << boundaries.size()
        << " lines." << endl;

  vector<Id>* blockIdMap = (setForOntology ? &_blockIdMapForOntology
      : &_blockIdMapForText);
  const HashMap<string, Id>* associatedVocabulary =
      setForOntology ? _ontologyVocabulary : _textVocabulary;

  assert(associatedVocabulary->size() > 0);
  blockIdMap->reserve(associatedVocabulary->size());
  blockIdMap->clear();
  if (boundaries.size() == 0)
  {
    LOG(INFO)
         << "special case: same blockId for every word." << endl;
    // Special case: No boundaries, assign the same
    // blockId for every word.
    for (size_t i = 0; i < associatedVocabulary->size(); ++i)
    {
      blockIdMap->push_back(0);
    }
  }
  else
  {
    // Get a vector version of the vocabulary
    vector<string> vocabulary;
    vocabulary.reserve(associatedVocabulary->size());
    for (HashMap<string, Id>::const_iterator it =
        associatedVocabulary->begin(); it
        != associatedVocabulary->end(); ++it)
    {
      vocabulary.push_back(it->first);
    }
    std::sort(vocabulary.begin(), vocabulary.end());

    // Read boundaries and vocab in parallel and assign
    // the current block Id (starts at 0) for each word.
    // Increment the Id and read the next boundary
    // whenever the word is greater or equal to the current boundary.
    assert(vocabulary.size() == associatedVocabulary->size());
    Id currentBlockId = 0;
    const string* boundary = &(boundaries[currentBlockId]);
    size_t i = 0;

    while (i < vocabulary.size())
    {
      if (vocabulary[i] < *boundary)
      {
        blockIdMap->push_back(currentBlockId);
        ++i;
      }
      else if (vocabulary[i] == *boundary)
      {
        ++i;
        ++currentBlockId;
        blockIdMap->push_back(currentBlockId);
        if (boundaries.size() == currentBlockId)
        {
          // Last Boundary reached, fill the rest
          while (i < vocabulary.size())
          {
            blockIdMap->push_back(currentBlockId);
            ++i;
          }
        }
        else
        {
          boundary = &(boundaries[currentBlockId]);
        }
      }
      else
      {
        // Case word > boundary
        ++currentBlockId;
        blockIdMap->push_back(currentBlockId);
        ++i;
        if (boundaries.size() == currentBlockId)
        {
          // Last Boundary reached, fill the rest
          while (i < vocabulary.size())
          {
            blockIdMap->push_back(currentBlockId);
            ++i;
          }
        }
        else
        {
          boundary = &(boundaries[currentBlockId]);
        }
      }
    }
    LOG(DEBUG)
        << "Set " << currentBlockId << " different Ids for "
        << associatedVocabulary->size() << " different words." << endl;
  }
  assert(associatedVocabulary->size() == blockIdMap->size());
}
// _____________________________________________________________________________
bool WordsFileExtractorBase::isProperlyInitialized() const
{
  if (_ontologyVocabulary->size() + _textVocabulary->size() == 0)
  {
    LOG(ERROR)
        << "Not properly initialized because both vocabularies are empty."
        << endl;
    return false;
  }
  bool retVal = (_blockIdMapForOntology.size()
      == _ontologyVocabulary->size() && _blockIdMapForText.size()
      == _textVocabulary->size());
  if (!retVal)
  {
    LOG(ERROR)
        << "Mismatch in vocabulary and block boundary sizes! " << endl
        << "Text-Vocabulary size: " << _textVocabulary->size()
        << " vs blockId-map size : " << _blockIdMapForText.size() << endl
        << "Ontology-Vocabulary size: " << _ontologyVocabulary->size()
        << " vs blockId-map size : " << _blockIdMapForOntology.size()
        << endl;
  }
  return retVal;
}
// _____________________________________________________________________________
void BasicExtractor::extract(const string& line, BasicPosting* posting) const
{
  try
  {
    assert(isProperlyInitialized());
    size_t indexOfTab1 = line.find('\t', 0);
    size_t indexOfTab2 = line.find('\t', indexOfTab1 + 1);

    string word = line.substr(0, indexOfTab1);
    string contextIdString = line.substr(indexOfTab1 + 1,
        indexOfTab2 - (indexOfTab1 + 1));

    // Words starting with a colon are from the ontology.
    // They get different Ids.
    if (word[0] == ':' && !ad_utility::startsWith(word, WIKI_DOC_WORD_PREFIX))
    {
      posting->_wordId = getOntologyElementId(word);
    }
    else
    {
      HashMap<string, Id>::const_iterator it = (*_textVocabulary).find(word);
      if (it == (*_textVocabulary).end())
      {
        LOG(ERROR)
        << "Error getting a wordId for text word \"" << word << "\"!" << endl
            << "The word is not the the text vocabulary (size: "
            << (*_textVocabulary).size() << ")." << endl;
        exit(1);
      }
      else
      {
        posting->_wordId = it->second;
      }
    }
    // The block Id depends on the id.
    posting->_blockId = getBlockId(posting->_wordId);
    // The context Id is always the same
    std::istringstream iss(contextIdString);
    iss >> posting->_contextId;
  }
  catch(const Exception& e)
  {
    LOG(ERROR) << e.getFullErrorMessage() << endl;
    LOG(ERROR) << "Ignoring line: " << line << endl;
  }
}
// _____________________________________________________________________________
void DefaultExtractor::extract(const string& line, Posting* posting) const
{
  try
  {
    assert(isProperlyInitialized());
    size_t indexOfTab1 = line.find('\t', 0);
    size_t indexOfTab2 = line.find('\t', indexOfTab1 + 1);
    size_t indexOfTab3 = line.find('\t', indexOfTab2 + 1);

    string word = line.substr(0, indexOfTab1);
    string contextIdString = line.substr(indexOfTab1 + 1,
        indexOfTab2 - (indexOfTab1 + 1));
    string scoreString = line.substr(indexOfTab2 + 1,
        indexOfTab3 - (indexOfTab2 + 1));
    string posString = line.substr(indexOfTab3 + 1);

    // Words starting with a colon are from the ontology.
    // They get different Ids.
    if (word[0] == ':' && !ad_utility::startsWith(word, WIKI_DOC_WORD_PREFIX))
    {
      posting->_wordId = getOntologyElementId(word);
    }
    else
    {
      HashMap<string, Id>::const_iterator it = (*_textVocabulary).find(word);
      if (it == (*_textVocabulary).end())
      {
        AD_THROW(Exception::VOCABULARY_MISS,
            string("Error getting a wordId for text word \"") + word + "\"!");
      }
      else
      {
        posting->_wordId = it->second;
      }
    }
    // The block Id depends on the id.
    posting->_blockId = getBlockId(posting->_wordId);
    // The context Id and so on are currently always
    // the same as they are in the ASCII file.
    posting->_contextId = atol(contextIdString.c_str());
    posting->_score = atoi(scoreString.c_str());
    posting->_pos = atoi(posString.c_str());
  }
  catch(const Exception& e)
  {
    LOG(ERROR) << e.getFullErrorMessage() << endl;
    LOG(ERROR) << "Ignoring line: " << line << endl;
  }
}
// _____________________________________________________________________________
void OntologyTripleExtractor::extract(const string& asciiFilename,
    stxxl::vector<RelationFact>* triples,
    vector<RelationMetaData>* relationMetaData) const
{
  // We use that the ASCII ontology is sorted by relation and
  // collect relation meta data for every new relation.
  ad_utility::File ontologyAscii(asciiFilename.c_str(), "r");
  string line;
  char buf[BUFFER_SIZE_ONTOLOGY_LINE];
  Id lastRelId = ~Id(0);
  Id relIdReversed = ~Id(0);
  while (ontologyAscii.readLine(&line, buf, BUFFER_SIZE_ONTOLOGY_LINE))
  {
    size_t indexOfTab1 = line.find('\t');
    size_t indexOfTab2 = line.find('\t', indexOfTab1 + 1);
    size_t indexOfTab3 = line.find('\t', indexOfTab2 + 1);
    size_t indexOfTab4 = line.find('\t', indexOfTab3 + 1);
    string relation = line.substr(0, indexOfTab1);

    Id relId = getOntologyElementId(relation);
    if (relId != lastRelId)
    {
      // New relation, get the meta data.
      Id lhsType = getOntologyElementId(
          line.substr(indexOfTab1 + 1, indexOfTab2 - (indexOfTab1 + 1)));
      Id rhsType = getOntologyElementId(
          line.substr(indexOfTab2 + 1, indexOfTab3 - (indexOfTab2 + 1)));
      relIdReversed
                = getOntologyElementId(relation + REVERSED_RELATION_SUFFIX);

      relationMetaData->push_back(RelationMetaData(relId, lhsType, rhsType));
      relationMetaData->push_back(
          RelationMetaData(relIdReversed, rhsType, lhsType));
      lastRelId = relId;
    }

    Id lhsId = getOntologyElementId(
        line.substr(indexOfTab3 + 1, indexOfTab4 - (indexOfTab3 + 1)));
    Id rhsId = getOntologyElementId(
        line.substr(indexOfTab4 + 1, line.size() - (indexOfTab4 + 1)));
    triples->push_back(RelationFact(relId, lhsId, rhsId));
    triples->push_back(RelationFact(relIdReversed, rhsId, lhsId));
  }

  // The unique shouldn't be necessary but it allows two things:
  // 1. Making sure the IndexBuilder will still work if the ascii ontology is
  // not sorted as expected.
  // 2. Output a warning in case of problems /
  // let the use be sure that the ontology was ordered properly.
  size_t metaSize = relationMetaData->size();
  relationMetaData->resize(
      std::unique(relationMetaData->begin(), relationMetaData->end())
          - relationMetaData->begin());
  if (relationMetaData->size() != metaSize)
  {
    LOG(WARN)
        << "The ontology ASCII is supposed to be ordered by relation. "
        << "Detected duplicate relations while extracting relation by relation"
        << endl << flush;
  }
}
}
