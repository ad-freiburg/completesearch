// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <vector>
#include <string>
#include "./Query.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
// _____________________________________________________________________________
void Query::constructQueryTreeFromTriplesString(const string& triplesString,
    const string& rootNode)
{
  LOG(INFO) << "Constructing query-tree from triples string" << endl;
  LOG(INFO) << "Triples string: \"" << triplesString << "\"" << endl;
  LOG(INFO) << "Root: \"" << rootNode << "\"" << endl;

  delete _queryTree;
  _originalRequest = triplesString + ", root: " + rootNode;
  // This method may throw exceptions later on.
  // Hence pointer _queryTree would still point to already deleted memory,
  // so we set it to NULL for safety.
  _queryTree = NULL;

  // -----------------------------------------------
  // I. Split the string into several triples.
  // -----------------------------------------------
  vector<string> triples;
  size_t indexFrom = 0;
  for (size_t i = 0; i < triplesString.size(); ++i)
  {
    if (triplesString[i] == ';')
    {
      triples.push_back(triplesString.substr(indexFrom, (i - indexFrom)));
      indexFrom = i + 1;
      if (triples.back().size() == 0)
      {
        AD_THROW(Exception::BAD_QUERY, "Found empty triple in query string.");
      }
    }
  }
  if (indexFrom < triplesString.size() - 1)
  {
    triples.push_back(triplesString.substr(indexFrom));
  }

  // -----------------------------------------------
  // II. Get the triples as a map.
  // -----------------------------------------------
  ad_utility::HashMap<string, vector<ParsedQueryTriple> > variableTriplesMap;
  // For each triple:
  for (size_t tripleNum = 0; tripleNum < triples.size(); ++tripleNum)
  {
    // Split the triple into three parts.
    size_t indexOfFirstNonSpace = triples[tripleNum].find_first_not_of(" ");
    size_t indexOfFirstSpace = triples[tripleNum].find(' ',
        indexOfFirstNonSpace);
    size_t indexOfSecondSpace = triples[tripleNum].find(' ',
        indexOfFirstSpace + 1);

    // The first part always has to be a variable.
    string source = triples[tripleNum].substr(indexOfFirstNonSpace,
        indexOfFirstSpace - indexOfFirstNonSpace);
    AD_CHECK(source.size() > 1);
    if (source[0] != QUERY_VARIABLE_START)
    {
      AD_THROW(Exception::BAD_QUERY,
          "All triples have to start with a variable.");
    }

    // The second part is used as relation name.
    string relationName = triples[tripleNum].substr(indexOfFirstSpace + 1,
        indexOfSecondSpace - (indexOfFirstSpace + 1));
    AD_CHECK(relationName.size() > 0);

    // The third part has to be non-empty and may contain 0 or more varibales.
    string dest = triples[tripleNum].substr(indexOfSecondSpace + 1);
    AD_CHECK(dest.size() > 0);

    // Tokenize dest.
    vector<string> destVariables;
    vector<string> destWords;
    bool destIsVariable = dest[0] == QUERY_VARIABLE_START;
    indexFrom = 0;
    for (size_t i = 0; i < dest.size(); ++i)
    {
      if (dest[i] == ' ')
      {
        if (destIsVariable)
        {
          destVariables.push_back(dest.substr(indexFrom, i - indexFrom));
        }
        else
        {
          destWords.push_back(dest.substr(indexFrom, i - indexFrom));
        }
        destIsVariable = (i + 1 < dest.size()
            && dest[i + 1] == QUERY_VARIABLE_START);
        indexFrom = i + 1;
      }
    }
    if (indexFrom + 1 < dest.size())
    {
      if (destIsVariable)
      {
        destVariables.push_back(dest.substr(indexFrom));
      }
      else
      {
        destWords.push_back(dest.substr(indexFrom));
      }
    }

    // Check plausibility of nof variables and words in dest.
    if (relationName == string(RELATION_PREFIX) + IS_A_RELATION)
    {
      if (destWords.size() != 1 || destVariables.size() > 0)
      {
        AD_THROW(Exception::BAD_QUERY,
            "Triples with \"is-a\" need to have exactly "
            "one non-variable third component.")
      }
    }
    else if (relationName == string(RELATION_PREFIX) + EQUALS_RELATION)
    {
      if (destWords.size() != 1 || destVariables.size() > 0)
      {
        AD_THROW(Exception::BAD_QUERY,
            "Triples with \"equals\" need to have exactly "
            "one non-variable third component.")
      }
    }
    else if (relationName == string(RELATION_PREFIX) + OCCURS_WITH_RELATION)
    {
      if (destWords.size() + destVariables.size() == 0)
      {
        AD_THROW(Exception::BAD_QUERY,
            "All triples have to have three parts separated by whitespaces.");
      }
    }
    else
    {
      // Case: normal relation
      if (destVariables.size() != 1 || destWords.size() > 0)
      {
        AD_THROW(
            Exception::BAD_QUERY,
            "Queries with classical relations have to have"
            " exactly one variable as third component. Not true for: "
            + source + ' ' + relationName + ' ' + dest);
      }
    }

    // Store triples with more than one variable in both directions, exceptions:
    // Occurs-with does not need reversed marker (isA should be impossible).
    ParsedQueryTriple currentTriple = {source, relationName, destVariables,
                                       destWords};
    variableTriplesMap[source].push_back(currentTriple);
    for (size_t i = 0; i < destVariables.size(); ++i)
    {
      vector<string> reversedDestVars(destVariables);
      reversedDestVars[i] = source;
      string reversedName = relationName;
      if (relationName != string(RELATION_PREFIX) + OCCURS_WITH_RELATION)
      {
        reversedName += REVERSED_RELATION_SUFFIX;
      }
      ParsedQueryTriple reversed = {destVariables[i], reversedName,
                                    reversedDestVars, destWords};
      variableTriplesMap[destVariables[i]].push_back(reversed);
    }
  }
  // -----------------------------------------------
  // III. Find an ordering of variables.
  // Has to start at the root, be acyclic, contain every variable exactly once.
  // -----------------------------------------------
  vector<string> orderedVariables;
  ad_utility::HashSet<string> variablesVisited;
  ad_utility::HashSet<string> thisVariableAdditions;
  vector<string> variablesToDo;
  ad_utility::HashMap<string, vector<ParsedQueryTriple> >::const_iterator it =
      variableTriplesMap.find(rootNode);
  if (it == variableTriplesMap.end())
  {
    AD_THROW(Exception::BAD_QUERY, string("Root node has to be contained ")
    + "as left-hand-side in at least one of the query triples. "
    + "Couldn't find \"" + rootNode + "\" in \"" + triplesString + "\".");
  }
  orderedVariables.push_back(it->first);
  variablesVisited.insert(it->first);
  // Init variablesToDo with direct target variables.
  for (size_t i = 0; i < it->second.size(); ++i)
  {
    for (size_t j = 0; j < it->second[i]._destVars.size(); ++j)
    {
      if (it->second[i]._destVars[j] == rootNode)
      {
        AD_THROW(Exception::BAD_QUERY, "Cyclic query detected.");
      }
      if (thisVariableAdditions.count(it->second[i]._destVars[j]) == 0)
      {
        variablesToDo.push_back(it->second[i]._destVars[j]);
        thisVariableAdditions.insert(it->second[i]._destVars[j]);
      }
    }
  }

  // Word through variables to do. Processing the to-do vector and appending
  // to it on the fly, relates to breadth-first processing.
  size_t variablesDoneIndex = 0;
  while (variablesDoneIndex < variablesToDo.size())
  {
    thisVariableAdditions.clear();
    string var = variablesToDo[variablesDoneIndex];
    orderedVariables.push_back(var);
    if (variablesVisited.count(var) != 0)
    {
      AD_THROW(Exception::BAD_QUERY, "Cyclic query detected."
      " We have already seen variable: \"" + var + "\".");
    }
    ad_utility::HashMap<string, vector<ParsedQueryTriple> >::const_iterator
    iter = variableTriplesMap.find(var);
    for (size_t i = 0; i < iter->second.size(); ++i)
    {
      for (size_t j = 0; j < iter->second[i]._destVars.size(); ++j)
      {
        if (variablesVisited.count(iter->second[i]._destVars[j]) == 0)
        {
          thisVariableAdditions.insert(iter->second[i]._destVars[j]);
        }
      }
    }
    for (ad_utility::HashSet<string>::iterator itt =
        thisVariableAdditions.begin(); itt != thisVariableAdditions.end();
        ++itt)
    {
      if (variablesVisited.count(*itt) != 0)
      {
        AD_THROW(Exception::BAD_QUERY, "Cyclic query detected. "
        "We have already seen variable: \"" + *itt + "\".");
      }
      variablesToDo.push_back(*itt);
      variablesVisited.insert(*itt);
    }
    ++variablesDoneIndex;
  }

  // -----------------------------------------------
  // IV. Construct the tree bottom up
  // (wrt to the ordering identified previously).
  // -----------------------------------------------
  ad_utility::HashMap<string, QueryTreeNode> nodesByVariable;
  for (size_t i = orderedVariables.size(); i > 0; --i)
  {
    size_t index = i - 1;
    Disjunct disjunct(_executionContext);
    const vector<ParsedQueryTriple>& currentTriples =
        variableTriplesMap[orderedVariables[index]];
    AD_CHECK_GT(currentTriples.size(), 0);
    for (size_t j = 0; j < currentTriples.size(); ++j)
    {
      if (currentTriples[j]._relationName
          == string(RELATION_PREFIX) + OCCURS_WITH_RELATION)
      {
        OccursWithTriple owt(_executionContext);
        bool tripleIsValid = true;
        for (size_t k = 0; k < currentTriples[j]._destWords.size(); ++k)
        {
          owt.addWord(currentTriples[j]._destWords[k]);
        }
        for (size_t k = 0; k < currentTriples[j]._destVars.size(); ++k)
        {
          ad_utility::HashMap<string, QueryTreeNode>::const_iterator
          targetNode = nodesByVariable.find(currentTriples[j]._destVars[k]);
          if (targetNode != nodesByVariable.end())
          {
            owt.addSubtree(targetNode->second);
          }
          else
          {
            tripleIsValid = false;
          }
        }
        if (tripleIsValid)
        {
          disjunct.addTriple(owt);
        }
      }
      else if (currentTriples[j]._relationName
          == string(RELATION_PREFIX) + EQUALS_RELATION)
      {
        disjunct.addTriple(
            EqualsTriple(_executionContext, currentTriples[j]._destWords[0]));
      }
      else if (currentTriples[j]._relationName
          == string(RELATION_PREFIX) + IS_A_RELATION)
      {
        disjunct.addTriple(
            IsATriple(_executionContext, currentTriples[j]._destWords[0]));
      }
      else
      {
        // Case: normal relation.
        ad_utility::HashMap<string, QueryTreeNode>::const_iterator targetNode =
            nodesByVariable.find(currentTriples[j]._destVars[0]);
        if (targetNode != nodesByVariable.end())
        {
          disjunct.addTriple(
              RelationTriple(_executionContext,
                  currentTriples[j]._relationName, targetNode->second));
        }
      }
    }
    QueryTreeNode nodeForThisVariable(_executionContext);
    nodeForThisVariable.addDisjunct(disjunct);
    nodesByVariable[orderedVariables[index]] = nodeForThisVariable;
  }
  _queryTree = new QueryTreeNode(nodesByVariable[rootNode]);
  LOG(INFO) << "Done constructing query-tree from triples string." << endl;
  LOG(DEBUG) << "Query: " << this->asString() << endl;
}

// _____________________________________________________________________________
string Query::asString() const
{
  if (_queryTree)
  {
    return string("Semantic query with prefix: \"") + _prefix + "\": { "
        + _queryTree->asString() + " }";
  }
  else
  {
    return string("Query without triples for prefix: \"") + _prefix + "\"";
  }
}

// _____________________________________________________________________________
void Query::createQueryResult(QueryResult* result) const
{
  if (!_queryTree)
  {
    if (_prefix.size() > 0)
    {
      createQueryResultForQueryWithoutTriples(result);
    }
    return;
  }
  LOG(DEBUG)
  << "Creating query result (ranking, boxes, etc)..." << endl;

  // Classes box
  if (_parameters._nofClassesToSend > 0 && _prefix.size() > 0)
  {
    fillClasses(result);
  }

  // Only continue, and get the subtree result which might actually
  // trigger computation, if we really need it.
  if (_parameters._nofInstancesToSend + _parameters._nofRelationsToSend
      + _parameters._nofHitGroupsToSend + _parameters._nofWordsToSend > 0)
  {
    const EntityList& subtreeResult = getResultForQueryTree()._entities;

    // Words box.
    if (_parameters._nofWordsToSend > 0
        && _prefix.size() >= MIN_WORD_PREFIX_SIZE)
    {
      fillWords(subtreeResult, result);
    }

    // Instances box.
    if (_parameters._nofInstancesToSend > 0)
    {
      fillInstances(subtreeResult, result);
    }

    // Relations box.
    if (_parameters._nofRelationsToSend > 0)
    {
      fillRelations(subtreeResult, result);
    }

    // Hits.
    if (_parameters._nofHitGroupsToSend > 0)
    {
      fillHits(subtreeResult, result);
    }
  }
  LOG(DEBUG) << "Done creating query result." << endl;
}

// _____________________________________________________________________________
void Query::createQueryResultForQueryWithoutTriples(QueryResult* result) const
{
  LOG(DEBUG)
  << "Processing a query without triples, prefix: \"" << _prefix << "\"."
      << endl;
  // Classes box.
  if (_parameters._nofClassesToSend > 0 && _prefix.size() > 0)
  {
    fillClasses(result);
  }
  else
  {
    AD_THROW(Exception::NOT_YET_IMPLEMENTED,
        "Queries without triples currently only "
        "make sense to fill the classes box.");
  }
  LOG(DEBUG)
      << "Done processing query without triples, prefix: \"" << _prefix << "\"."
      << endl;
}

// _____________________________________________________________________________
void Query::fillInstances(const EntityList& subtreeResult,
    QueryResult* result) const
{
  EntityList entityList;
  if (_prefix.size() == 0)
  {
    result->_totalNofInstances = subtreeResult.size();
    getEngine().getTopKEntities(subtreeResult,
        _parameters._nofInstancesToSend + _parameters._firstInstanceToSend,
        &entityList);
  }
  else
  {
    IdRange idRange;
    getIndex().getIdRangeForOntologyWordOrPrefix(
        string(ENTITY_PREFIX) + _prefix + "*", &idRange);
    EntityList filteredByPrefix;
    getEngine().filterEntityListByIdRange(subtreeResult, idRange,
        &filteredByPrefix);
    result->_totalNofInstances = filteredByPrefix.size();
    getEngine().getTopKEntities(filteredByPrefix,
        _parameters._nofInstancesToSend + _parameters._firstInstanceToSend,
        &entityList);
  }

  result->_firstInstance = _parameters._firstInstanceToSend;

  for (size_t i = _parameters._firstInstanceToSend; i < entityList.size(); ++i)
  {
    string entityAsString = getIndex().getOntologyWordById(entityList[i]._id);
    result->_instances.push_back(
        ItemWithScore(entityAsString, entityList[i]._score));
  }
}

// _____________________________________________________________________________
void Query::fillClasses(QueryResult* result) const
{
  if (_prefix.size() == 0)
  {
    AD_THROW(Exception::NOT_YET_IMPLEMENTED,
        "Currently not supporting queries for classes without prefix input.");
  }

  const EntityList& availableClasses = getIndex().getAvailableClasses();
  EntityList topClasses;
  IdRange idRange;
  getIndex().getIdRangeForOntologyWordOrPrefix(
      string(ENTITY_PREFIX) + _prefix + "*", &idRange);
  EntityList filteredByPrefix;
  getEngine().filterEntityListByIdRange(availableClasses, idRange,
      &filteredByPrefix);

  result->_totalNofClasses = filteredByPrefix.size();
  result->_firstClass = _parameters._firstClassToSend;

  getEngine().getTopKEntities(filteredByPrefix,
      _parameters._nofClassesToSend + _parameters._firstClassToSend,
      &topClasses);


  for (size_t i = _parameters._firstClassToSend; i < topClasses.size();
      ++i)
  {
    string classAsString = getIndex().getOntologyWordById(
        topClasses[i]._id);
    result->_classes.push_back(
        ItemWithScore(classAsString, topClasses[i]._score));
  }
}

// _____________________________________________________________________________
void Query::fillWords(const EntityList& subtreeResult,
    QueryResult* result) const
{
  IdRange wordIdRange;
  bool wordFound = getIndex().getIdRangeForFullTextWordOrPrefix(_prefix + "*",
      &wordIdRange);

  // If one of the words isn't even in the vocabulary,
  // the result will be empty.
  if (wordFound)
  {
    BlockMetaData blockData = getIndex().getBlockInfoByWordRange(wordIdRange);
    PostingList rawPostingList;
    getIndex().readBlock(blockData, &rawPostingList);
    PostingList filteredByPrefix;
    getEngine().filterPostingListByWordRange(rawPostingList,
        wordIdRange._first, wordIdRange._last, &filteredByPrefix);

    PostingList filteredByPrefixAndEntityList;
    getEngine().filterPostingListByEntityListKeepWordPostings(filteredByPrefix,
        subtreeResult, &filteredByPrefixAndEntityList);

    // Note that from here on, we abuse EntityLists in a way where we actually
    // put non-entity word-Ids in them. However, since operations needed
    // fit perfectly well, this is convenient.
    // Renaming the EntityList class may be a solution.
    // However, since this is the only use-case for this behavior,
    // no renaming is done for now.
    EntityList aggregatedWords;
    getEngine().aggregateWordsInPostingList(filteredByPrefixAndEntityList,
        &aggregatedWords);

    result->_firstWord = _parameters._firstWordToSend;
    result->_totalNofWords = aggregatedWords.size();

    EntityList topWords;
    getEngine().getTopKEntities(aggregatedWords,
        _parameters._nofWordsToSend + _parameters._firstWordToSend, &topWords);

    for (size_t i = _parameters._firstWordToSend; i < topWords.size(); ++i)
    {
      string word = getIndex().getFulltextWordById(topWords[i]._id);
      result->_words.push_back(ItemWithScore(word, topWords[i]._score));
    }
  }
}

// _____________________________________________________________________________
void Query::fillRelations(const EntityList& subtreeResult,
    QueryResult* result) const
{
  const Relation& hasRelationsRelation = getIndex().getHasRelationsRelation();
  EntityList relations;
  Relation dummy;
  getEngine().getRelationRhsByEntityListLhs<
      PlusOneAggregator<AggregatedScore> >(
      hasRelationsRelation, subtreeResult, &dummy, &relations);
  EntityList topRelations;
  if (_prefix.size() == 0)
  {
    result->_totalNofRelations = relations.size();
    getEngine().getTopKEntities(relations,
        _parameters._nofRelationsToSend + _parameters._firstRelationToSend,
        &topRelations);
  }
  else
  {
    IdRange idRange;
    getIndex().getIdRangeForOntologyWordOrPrefix(
        string(RELATION_PREFIX) + _prefix + "*", &idRange);
    EntityList filteredByPrefix;
    getEngine().filterEntityListByIdRange(relations, idRange,
        &filteredByPrefix);
    result->_totalNofRelations = filteredByPrefix.size();
    getEngine().getTopKEntities(filteredByPrefix,
        _parameters._nofRelationsToSend + _parameters._firstRelationToSend,
        &topRelations);
  }

  result->_firstRelation = _parameters._firstRelationToSend;

  for (size_t i = _parameters._firstRelationToSend; i < topRelations.size();
      ++i)
  {
    string relationAsString = getIndex().getOntologyWordById(
        topRelations[i]._id);
    string lhsType = getIndex().getOntologyWordById(
        getIndex().getRelationMetaData(topRelations[i]._id)._lhsType);
    string rhsType = getIndex().getOntologyWordById(
        getIndex().getRelationMetaData(topRelations[i]._id)._rhsType);
    result->_relations.push_back(
        RelationBoxEntry(relationAsString, topRelations[i]._score, lhsType,
            rhsType));
  }
}

// _____________________________________________________________________________
void Query::fillHits(const EntityList& subtreeResult,
    QueryResult* result) const
{
  result->_totalNofHitGroups = subtreeResult.size();
  result->_firstHitGroup = _parameters._firstHitGroupToSend;
  EntityList entityList;
  getEngine().getTopKEntities(subtreeResult,
      _parameters._nofHitGroupsToSend + _parameters._firstHitGroupToSend,
      &entityList);

  for (size_t i = _parameters._firstHitGroupToSend; i < entityList.size(); ++i)
  {
    HitList hitsForThisEntity;
    _queryTree->getHitsForEntity(entityList[i]._id, &hitsForThisEntity);
    string entityAsString = getIndex().getOntologyWordById(entityList[i]._id);
    result->_hitGroups.push_back(
        HitGroup(entityAsString, hitsForThisEntity, entityList[i]._score));
  }
}
}
