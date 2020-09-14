// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <set>
#include <algorithm>
#include "./QueryTriple.h"
#include "./QueryTreeNode.h"
#include "./QueryExecutionContext.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"

using std::string;
using std::set;

using ad_utility::AsStringComparatorLt;
using ad_utility::AsStringPtrComparatorLt;

namespace ad_semsearch
{
// _____________________________________________________________________________
OccursWithTriple::OccursWithTriple()
: ExecutableResultProvider()
{
}

// _____________________________________________________________________________
OccursWithTriple::OccursWithTriple(QueryExecutionContext* executionContext)
: ExecutableResultProvider(executionContext)
{
}

// _____________________________________________________________________________
OccursWithTriple::OccursWithTriple(const OccursWithTriple& other)
: ExecutableResultProvider(other), _words(other._words)
{
  // deep-copy subtrees.
  for (set<QueryTreeNode*, AsStringPtrComparatorLt>::const_iterator it =
      other._subtrees.begin(); it != other._subtrees.end(); ++it)
  {
    _subtrees.insert(new QueryTreeNode(**it));
  }
}

// _____________________________________________________________________________
OccursWithTriple::~OccursWithTriple()
{
  // Delete all the subtrees.
  for (set<QueryTreeNode*, AsStringPtrComparatorLt>::const_iterator it =
      _subtrees.begin(); it != _subtrees.end(); ++it)
  {
    delete *it;
  }
}

// _____________________________________________________________________________
OccursWithTriple& OccursWithTriple::operator=(const OccursWithTriple& rhs)
{
  _executionContext = rhs._executionContext;
  _words = rhs._words;

  // Delete existing subtrees.
  for (set<QueryTreeNode*, AsStringPtrComparatorLt>::const_iterator it =
      _subtrees.begin(); it != _subtrees.end(); ++it)
  {
    delete *it;
  }
  _subtrees.clear();

  // Deep-copy subtrees.
  for (set<QueryTreeNode*, AsStringPtrComparatorLt>::const_iterator it =
      rhs._subtrees.begin(); it != rhs._subtrees.end(); ++it)
  {
    _subtrees.insert(new QueryTreeNode(**it));
  }
  return *this;
}

// _____________________________________________________________________________
void OccursWithTriple::addSubtree(const QueryTreeNode& subtree)
{
  _subtrees.insert(new QueryTreeNode(subtree));
}

// _____________________________________________________________________________
void OccursWithTriple::addWord(const string& word)
{
  _words.insert(word);
}

// _____________________________________________________________________________
string OccursWithTriple::asString() const
{
  std::ostringstream os;
  os << "<-- :r:occurs-with (";
  for (set<string>::const_iterator it = _words.begin(); it != _words.end(); )
  {
    os << *it;
    if (++it != _words.end())
    {
      os << " ";
    }
  }
  if (_subtrees.size() > 0)
  {
    os << " ";
  }
  for (set<QueryTreeNode*, AsStringPtrComparatorLt>::const_iterator it =
      _subtrees.begin(); it != _subtrees.end();)
  {
    os << (*it)->asString();
    if (++it !=  _subtrees.end())
    {
      os << " ";
    }
  }
  os << ")>";
  return os.str();
}

// _____________________________________________________________________________
void OccursWithTriple::computeResult(IntermediateQueryResult* result) const
{
  LOG(DEBUG) << "Processing subtree " << asString() << endl;

  // TODO(buchholb): Add this feature.
  if (_words.size() == 0)
  {
    AD_THROW(Exception::NOT_YET_IMPLEMENTED,
        "Cannot process occurs-with triples without words, yet.");
  }

  // Process the words-part.
  for (set<string>::iterator it = _words.begin(); it != _words.end(); ++it)
  {
    IdRange wordIdRange;
    bool wordFound = getIndex().getIdRangeForFullTextWordOrPrefix(*it,
        &wordIdRange);

    // If one of the words isn't even in the vocabulary,
    // the result will be empty.
    if (!wordFound)
    {
      result->_entities.clear();
      result->_postings.clear();
      result->_status = IntermediateQueryResult::FINISHED;
      return;
    }

    BlockMetaData blockData = getIndex().getBlockInfoByWordRange(wordIdRange);
    PostingList rawPostingList;
    getIndex().readBlock(blockData, &rawPostingList);
    PostingList tmp1;
    getEngine().filterPostingListByWordRange(rawPostingList,
        wordIdRange._first, wordIdRange._last, &tmp1);

    if (it == _words.begin())
    {
      result->_postings = tmp1;
    }
    else
    {
      PostingList tmp2;
      getEngine().joinPostingListsOnContextId(result->_postings, tmp1, &tmp2);
      result->_postings = tmp2;
    }
  }

  // Filter with the results of all available subtrees.
  for (set<QueryTreeNode*, ad_utility::AsStringPtrComparatorLt>::iterator it =
      _subtrees.begin(); it != _subtrees.end(); ++it)
  {
    PostingList tmp;
    getEngine().filterPostingListByEntityList(result->_postings,
        (*it)->getResult()._entities, &tmp);
    result->_postings = tmp;
  }

  // Obtain the EntityList from the PostingsList
  getEngine().aggregatePostingList(result->_postings, &result->_entities);
  result->_status = IntermediateQueryResult::FINISHED;

  LOG(DEBUG) << "Creating HashSet of all subtree result entities." << endl;
  // Filter with the results of all available subtrees.
  for (set<QueryTreeNode*, ad_utility::AsStringPtrComparatorLt>::iterator it =
      _subtrees.begin(); it != _subtrees.end(); ++it)
  {
    const EntityList& entities = (*it)->getResult()._entities;
    for (size_t i = 0; i < entities.size(); ++i)
    {
      result->_subtreeEntities.insert(entities[i]._id);
    }
  }
  LOG(DEBUG) << "Done creating HashSet of all subtree result entities." << endl;

  LOG(DEBUG) << "Done processing occurs-with triple." << endl;
}

// _____________________________________________________________________________
void OccursWithTriple::getHitsForEntity(Id entityId, HitList* result) const
{
  const IntermediateQueryResult& iqr = getResult();
  PostingList postingsForEntity;
  getEngine().filterPostingsByEntityId(iqr._postings, entityId,
      iqr._subtreeEntities, &postingsForEntity);
  HitList thisLevelResult;
  if (_subtrees.size() > 0)
  {
    LOG(WARN)
    << "Getting evidence for o/w subtrees, watch the time." << endl;
    getEngine().getTopKContextsWithHighlightsAndEntities(postingsForEntity, 1,
        &thisLevelResult);
    for (size_t i = 0; i < thisLevelResult.size(); ++i)
    {
      // Create the evidence for this hit.
      thisLevelResult[i].setRawExcerptString(
          getIndex().getRawExcerptForContextId(
              thisLevelResult[i].getContextId()));
    }
    if (thisLevelResult.size() > 0)
    {
      // Create evidence for each subtree.
      // Match entities and subtrees and get hits for them.
      for (set<QueryTreeNode*,
          ad_utility::AsStringPtrComparatorLt>::iterator it =
          _subtrees.begin();
          it != _subtrees.end();
          ++it)
      {
        // Find an entity that is highlighted in this hit and in the
        // result of the subtree. There always should be one.
        Id entityId = getEngine().getAnyMatchingEntitiy(
            (*it)->getResult()._entities,
            thisLevelResult[0].getMatchedEntities());
        (*it)->getHitsForEntity(entityId, &thisLevelResult);
      }
    }
    LOG(WARN) << "Done getting evidence for o/w subtrees." << endl;
  }
  else
  {
    getEngine().getTopKContextsWithHighlights(postingsForEntity, 1,
        &thisLevelResult);
    for (size_t i = 0; i < thisLevelResult.size(); ++i)
    {
      thisLevelResult[i].setRawExcerptString(
          getIndex().getRawExcerptForContextId(
              thisLevelResult[i].getContextId()));
    }
  }
  for (size_t i = 0; i < thisLevelResult.size(); ++i)
  {
    result->push_back(thisLevelResult[i]);
  }
}
// _____________________________________________________________________________
RelationTriple::RelationTriple()
  : ExecutableResultProvider(), _relationName(), _target(NULL)
{
}

// _____________________________________________________________________________
RelationTriple::RelationTriple(
    QueryExecutionContext* executionContext)
: ExecutableResultProvider(executionContext), _relationName(), _target(NULL)
{
}

// _____________________________________________________________________________
RelationTriple::RelationTriple(QueryExecutionContext* executionContext,
    const string& relationName, const QueryTreeNode& target)
: ExecutableResultProvider(executionContext), _relationName(relationName),
  _target(new QueryTreeNode(target))
{
}

// _____________________________________________________________________________
RelationTriple::RelationTriple(const RelationTriple& other)
: ExecutableResultProvider(other._executionContext),
  _relationName(other._relationName),
  _target(new QueryTreeNode(*other._target))
{
}

// _____________________________________________________________________________
RelationTriple::~RelationTriple()
{
  delete _target;
}

// _____________________________________________________________________________
RelationTriple& RelationTriple::operator=(const RelationTriple& rhs)
{
  delete _target;
  _executionContext = rhs._executionContext;
  _relationName = rhs._relationName;
  _target = new QueryTreeNode(*rhs._target);
  return *this;
}

// _____________________________________________________________________________
void RelationTriple::setRelationName(const string& relationName)
{
  _relationName = relationName;
}

// _____________________________________________________________________________
void RelationTriple::setTarget(const QueryTreeNode& target)
{
  delete _target;
  _target = new QueryTreeNode(target);
}

// _____________________________________________________________________________
string RelationTriple::asString() const
{
  AD_CHECK(_target);
  std::ostringstream os;
  os << "<-- " << _relationName << ' ' << _target->asString() << '>';
  return os.str();
}

// _____________________________________________________________________________
void RelationTriple::computeResult(IntermediateQueryResult* result) const
{
  AD_CHECK(_target);
  LOG(DEBUG) << "Processing subtree " << asString() << endl;

  // We need to get use the opposite direction of the
  // relation w.r.t reversal, because of bottom up processing.
  Id relId;
  bool relationExists = getIndex().getIdForOntologyWord(
      reverseRelation(_relationName), &relId);
  if (!relationExists)
  {
    AD_THROW(Exception::BAD_QUERY,
        string("Relation ") + _relationName + " does not exists.");
  }
  const RelationMetaData& relationMetaData = getIndex().getRelationMetaData(
      relId);
  // Read the whole relation from disk.
  Relation relation;
  getIndex().readFullRelation(relationMetaData, &relation);
  // Access the relation with the subtree result. This produces the
  // result of the current subtree.
  getEngine().getRelationRhsByEntityListLhs(relation,
      _target->getResult()._entities, &result->_matchingRelationEntries,
      &result->_entities);

  result->_status = IntermediateQueryResult::FINISHED;
  LOG(DEBUG)
  << "Done processing relation triple." << endl;
}

// _____________________________________________________________________________
void RelationTriple::getHitsForEntity(Id entityId, HitList* result) const
{
  const IntermediateQueryResult& iqr = getResult();
  Relation::const_iterator it = std::lower_bound(
      iqr._matchingRelationEntries.begin(), iqr._matchingRelationEntries.end(),
      RelationEntry(entityId, entityId), ad_utility::CompareRhsComparator());
  AD_CHECK(it != iqr._matchingRelationEntries.end());
  Excerpt excerpt(
      ad_utility::getLastPartOfString(getIndex().getOntologyWordById(entityId),
          ':'),
      ad_utility::getLastPartOfString(_relationName, ':'),
      ad_utility::getLastPartOfString(getIndex().getOntologyWordById(it->_lhs),
          ':'));
  result->push_back(
      Hit(YAGO_CONTEXT_ID, excerpt, ENTITY_FROM_RELATION_SCORE));
  _target->getHitsForEntity(it->_lhs, result);
}
// _____________________________________________________________________________
string IsATriple::asString() const
{
  return string("<-- :r:is-a ") + _targetClass + ">";
}

// _____________________________________________________________________________
void IsATriple::computeResult(IntermediateQueryResult* result) const
{
  LOG(DEBUG) << "Processing subtree " << asString() << endl;

  // We need to use the REVERSED is-a relation because
  // we want to go from class to instances
  Id reversedIsARelId;
  bool relationExists = getIndex().getIdForOntologyWord(
      string(RELATION_PREFIX) + IS_A_RELATION + REVERSED_RELATION_SUFFIX,
      &reversedIsARelId);
  AD_CHECK(relationExists);
  Id classId;
  bool classExists = getIndex().getIdForOntologyWord(_targetClass, &classId);
  if (classExists)
  {
    const RelationMetaData& revIsaMetaData = getIndex().getRelationMetaData(
        reversedIsARelId);
    // Read the correct block from the relation.
    Relation revIsARelationSubset;
    getIndex().readRelationBlock(revIsaMetaData.getBlockInfo(classId),
        &revIsARelationSubset);
    // Transform it to an entity list, i.e. write into the result entity list.
    getEngine().getRelationRhsBySingleLhs(revIsARelationSubset, classId,
        &result->_entities);
  }
  result->_status = IntermediateQueryResult::FINISHED;
  LOG(DEBUG)
  << "Done processing is-a triple." << endl;
}

// _____________________________________________________________________________
void IsATriple::getHitsForEntity(Id entityId, HitList* result) const
{
  Excerpt excerpt(
      ad_utility::getLastPartOfString(getIndex().getOntologyWordById(entityId),
          ':'), " is a ", ad_utility::getLastPartOfString(_targetClass, ':'));
  result->push_back(
      Hit(YAGO_CONTEXT_ID, excerpt, ENTITY_FROM_RELATION_SCORE));
}

// _____________________________________________________________________________
string EqualsTriple::asString() const
{
  return string("<-- :r:equals ") + _entity + ">";
}

// _____________________________________________________________________________
void EqualsTriple::computeResult(IntermediateQueryResult* result) const
{
  LOG(DEBUG) << "Processing subtree " << asString() << endl;
  LOG(DEBUG)
      << "Checking if there is a word \"" << _entity
      << "\" in the ontology vocabulary." << endl;
  Id entityId;
  bool entityExists = getIndex().getIdForOntologyWord(_entity, &entityId);
  if (entityExists)
  {
    result->_entities.push_back(EntityWithScore(entityId, ENTITY_EQUALS_SCORE));
  }
  result->_status = IntermediateQueryResult::FINISHED;
  LOG(DEBUG)
      << "Done processing equals triple. Result has "
      << result->_entities.size()
      << " elements." << endl;
}
}

