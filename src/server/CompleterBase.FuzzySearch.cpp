#include <cmath>
#include <queue>
#include "CompleterBase.h"
#include "HYBCompleter.h"
#include "../fuzzysearch/FuzzySearcher.h"
#include "../fuzzysearch/Utils.h"
#include "../fuzzysearch/StringDistances.h"
#include <google/sparse_hash_map>
#include <unordered_map>
#include <unordered_set>
double fuzzySearchEditDistanceThreshold;
extern bool fuzzySearchUseClustering;

// A cluster of completions of a prefix like C:7623:*.
class ClusterOfSimilarWords
{
  public:
   // The query word, for example: ball.
   string queryWord;
   // The similar words from one cluster, for example: C:7623:ballerina.
   // TODO(bast): is the C:7623: part of the word here or not?
   vector<string> similarWords;
   // The edit distances of these strings to the query word.
   vector<double> distances;
   // The number of relevant words in the cluster, where a word is relevant if
   // its edit distance is below the user-defined threshold.
   double nofRelevantWords;
   // holds the fraction nofRelevantWords/size
   double fractionRelevant;
   // TODO(bast): What is this? -important stuff
   int clusterId;
   // Whether words from this cluster will be included in the rewritten query.
   bool includeInRewrittenQuery;
   // number of words in this cluster
   int size;

   // Human-readable string with information about the cluster.
   string DebugString();
};

// Select the smallest number of clusters covering all the given relevant
// words. Uses a simple greedy heuristic explained in the code.
void selectClusters(
    const vector<vector<int> >& wordIdsPerCluster,
    const vector<vector<int> >& clustersPerWordId,
    const vector<int>& clusterIdToClusterIndex,
    vector<ClusterOfSimilarWords>& clusters,
    std::unordered_set<int>& allRelevantWords,
    int heuristic,
    vector<bool>* isClusterSelected,
    vector<double>* qualityOfClusters,
    size_t* numRelevantWordsSelected,
    size_t* numDistinctWordsInSelectedClusters);

// _____________________________________________________________________________
// NOTE(bast): Of all the arguments, the current implementation uses only
// lastPartOfQuery, replaces that with a disjunction (for example: ball*~ ->
// ball*|C:1543:*|C:89:*|C:3485:*), and then calls processBasicQuery again
// (which will then call processOrQuery).
// NOTE(bast): Beware that the modified query does not contain a ! anymore,
// otherwise we will have an infinite loop. As of the time of writing of this
// comment, this was not the case.
//
// OLD NOTES FROM CompleterBase.cpp (where this code was living first).
//
// Assume that the clusters with ids 1526 and 271 are returned. Then replace the
// last part of the query by query*|C:1526:*|C:271:* and call processBasicQuery
// again with that query. Note that the part with query* will be processed a
// second time here, but that will cost no time, since the result will simply be
// taken from the history.
//
// NOTE 1: In a first implementation, it might be easier to *either* execute the
// standard prefix query without the tilda, *or* execute the query for the
// clusters.
//
// NOTE 2: If we only have C:<cluster id>:... words in the collection, it's
// not clear how to execute normal prefix queries, since a prefix cannot be
// uniquely assigned to a single cluster. A simple fix would be to index every
// word xyzy twice, as xyz itself, and as s:<id>:xyz . But that doubles the
// index size, which might be undesirable in applications with very large
// amounts of data.
//

// should print debug output for fuzzysearch ?
// this variable is used to debug fuzzy search without
// displaying other unnecessary info from CompleteSearch
// by using high verbosity
bool showFuzzyDebugOutput = false;

// INTERSECT TWO LISTS in time linear in the sum of the list lengths
// used for query rotation and computing suggestions due to speed
void intersectLists(const QueryResult& list1,
                    const QueryResult& list2,
                    QueryResult* resultList);

//! MERGE TWO LISTS in time linear in the sum of the list lengths
// _____________________________________________________________________________
void mergeTwoLists(QueryResult& list1,
                   QueryResult& list2,
                   QueryResult* resultList);

// _____________________________________________________________________________
void mergeLists(const vector<const QueryResult*>& lists,
                QueryResult* resultList);

// Optimal binary merge algorithm
// _____________________________________________________________________________
void mergeOpt(const vector<QueryResult>& lists,
              const unsigned int k,
              QueryResult* resultList);

// order the fuzzy query suggestion by their counts / score
bool orderByCounts(const pair<const vector<WordId>*, double>& x,
                   const pair<const vector<WordId>*, double>& y)
{
  return x.second > y.second;
}

// hashes 2 32-bit ints -> 1 64-bit int by concatenation
// _____________________________________________________________________________
class TwoIntsHashFunction
{
  public:
    off_t operator()(const vector<WordId>& s) const
    {
      off_t x = s[0];
      x = x << 32;
      return (x | s[1]);
    }
};

// hash function used to hashes n ints into 1 int
// originally used for hashing strings. Any better idea?
// _____________________________________________________________________________
class IntsHashFunction
{
  public:
    off_t operator()(const vector<WordId>& s) const
    {
      off_t x = s[0];
      for (size_t i = 1; i < s.size(); i++)
      {
        x ^= (x << 5) + s[i] + (x >> 7);
      }
      return x;
    }
};

// counted score of the query suggestions (later filtered)
vector<pair<const vector<WordId>*, double> > countedQuerySuggestions;

// hash_map used for counting score for query suggestions (if 2-words)
std::unordered_map<vector<WordId>, int, TwoIntsHashFunction> hashMapCountPairs;

// hash_map used for counting score query suggestions (if n-words)
std::unordered_map<vector<WordId>, int, IntsHashFunction> hashMapCountTuples;

// used by findFuzzySearchQuerySuggestions2() to speed up the computation
vector<vector<WordId> > countsPerWordId;

// needed by findFuzzySearchQuerySuggestions2() for its priority queue
// used to skip considering suggestions for which it's clear that have
// score that is too low
class Compare
{
  public:
    bool operator()(const int& x, const int& y) const
    {
      return x < y;
    }
};

// this are the possible operators I know about
const char queryOperators[] = "*^~\0";

// return only the keyword, excluding operators
// _____________________________________________________________________________
string stripQueryWord(const string& keyword)
{
  int n = strlen(queryOperators);
  int splitPos = -1;
  for (size_t i = 0; i < keyword.length(); i++)
  {
    for (int j = 0; j < n; j++)
      if (queryOperators[j] == keyword[i])
      {
        splitPos = i;
        break;
      }
    if (splitPos > 0)
      return keyword.substr(0, splitPos);
  }
  return keyword;
}

// count tuples in each hash table by using a recursive function
// _____________________________________________________________________________
void countTuples(vector<std::unordered_map<WordId, int> >& hashWords,
           std::unordered_map<vector<WordId>, int, IntsHashFunction>& hashMap,
           vector<WordId> tuple,
           int i,
           int p)
{
  if (i == static_cast<int>(hashWords.size()))
  {
    hashMap[tuple] += p;
    return;
  }
  for (auto it = hashWords[i].begin(); it != hashWords[i].end(); it++)
  {
    tuple[i] = it->first;
    countTuples(hashWords, hashMap, tuple, i + 1, MIN(p, it->second));
  }
}

// special case of counting tuples when the query has 3 words
// due toe speed (recursive counting which works in general case
// above is slower due to copying of tuple)
// _____________________________________________________________________________
void countFZTriples(vector<std::unordered_map<WordId, int> >& hashWords,
                    std::unordered_map<vector<WordId>, int,
		                       IntsHashFunction>& hashMap)
{
  vector<WordId> FZTriple;
  FZTriple.resize(3);
  for (auto it1 = hashWords[0].begin(); it1 != hashWords[0].end(); it1++)
    for (auto it2 = hashWords[1].begin(); it2 != hashWords[1].end(); it2++)
      for (auto it3 = hashWords[2].begin(); it3 != hashWords[2].end(); it3++)
      {
        FZTriple[0] = it1->first;
        FZTriple[1] = it2->first;
        FZTriple[2] = it3->first;
        hashMap[FZTriple]+=MIN(MIN(it1->second, it2->second), it3->second); // (it1->second * it2->second * it3->second);
      }
}

// special case of counting pairs when the query has 2 words
// due toe speed (recursive counting which works in general case
// above is slower due to copying of tuple)
// _____________________________________________________________________________
void countPairs(vector<std::unordered_map<WordId, int> >& hashWords,
                std::unordered_map<vector<WordId>, int,
		                   TwoIntsHashFunction>& hashMap)
{
  vector<WordId> pair;
  pair.resize(2);
  for (auto it1 = hashWords[0].begin(); it1 != hashWords[0].end(); it1++)
    for (auto it2 = hashWords[1].begin(); it2 != hashWords[1].end(); it2++)
    {
      pair[0] = it1->first;
      pair[1] = it2->first;
      hashMap[pair]+=MIN(it1->second, it2->second);
    }
}

bool phrFlag;

// count tuples in each hash table by using a recursive function
// but only cosider phrases (that are counted only once per doc.)
void countPhrases(vector<std::unordered_map<WordId, int> >& hashWords,
                  std::unordered_map<vector<WordId>, int,
		                     IntsHashFunction>& hashMap,
                  vector<WordId> tuple,
                  int i,
                  int p)
{
  if (i == static_cast<int>(hashWords.size()))
  {
    hashMap[tuple] += 1;
    phrFlag = true;
    return;
  }
  for (auto it = hashWords[i].begin(); it != hashWords[i].end(); it++)
  {
    if (it->second - p <= 2 || p == -1)
    {
      tuple[i] = it->first;
      countPhrases(hashWords, hashMap, tuple, i + 1, it->second);
    }
  }
}

// compute fuzzy suggestions for the current query by using query rotation
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::findFuzzySearchQuerySuggestions(
    Query& query,
    int k,
    bool suggestOnlyPhrases,
    vector<string>* suggestions,
    vector<double>* scores)
{
  CS_ASSERT(suggestions != NULL);
  CS_ASSERT(scores != NULL);
  vector<QueryResult*> lists;
  Query first = query;
  Query last;
  Separator separator;
  if (query.getQueryString().size() == 0)
    return;
  size_t keywordCount = 0;
  first.setQueryString(first.getQueryString() + " t");

  // 1. Get list for each query prefix from cache

  // Check if the whole query has been processed by checking if it is in
  // the history;
  QueryResult* resultListFromHistory = history->isContained(
      query.getQueryString() + "&hf=" + getFlagForHistory());
  if (resultListFromHistory == NULL)
  {
    log << "! FUZZY: Query not in history when it should be! "
        << "Query suggestion canceled." << endl;
    return;
  }
  if (resultListFromHistory->_docIds.size() == 0)
  {
    log << "! FUZZY: Empty result list in history! "
        << "Query suggestion canceled." << endl;
    return;
  }
  while(true)
  {
    Query temp;
    first.splitAtLastSeparator(&temp, &last, &separator);
    first = temp;
    if (first.getQueryString().size() == 0)
      break;
    QueryResult* resultListFromHistory = history->isContained(
        first.getQueryString() + "&hf=" + getFlagForHistory());
    if (resultListFromHistory == NULL)
    {
      log << "! FUZZY: ERROR: query part " << first.getQueryString()
          << " not in history. Query suggestion canceled!" << endl;
      return;
    }
    else
      if (resultListFromHistory->_docIds.size() == 0)
        return;
    lists.resize(lists.size() + 1);
    lists[keywordCount] = resultListFromHistory;
    keywordCount++;
  }
  if (keywordCount <= 1)
  {
    suggestions->clear();
    scores->clear();
    return;
  }

  // 2. Compute the inverted list for each keyword using query rotation
  vector<QueryResult> keywordLists;
  keywordLists.resize(lists.size());
  keywordLists[0] = *lists[0];
  for (size_t i = 1; i < lists.size(); i++)
    intersectLists(*lists[0], *lists[i], &keywordLists[i]);

  // TODO(celikik): there is a bug in findFuzzySearchQuerySuggestions2
  // if two-word query, call special faster function and quit.
  // if (keywordCount == 2 && !suggestOnlyPhrases)
  // {
  //  findFuzzySearchQuerySuggestions2(keywordLists[1], keywordLists[0], k,
  //      suggestions, scores);
  //  return;
  // }

  // 3. Count n-tuples of words
  vector<std::unordered_map<WordId, int> > hashWords;
  vector<size_t> listCounters;
  vector<WordId> tempTuple;
  DocId currentDocId;
  hashWords.resize(keywordCount);
  listCounters.resize(keywordCount);
  hashMapCountPairs.clear();
  hashMapCountTuples.clear();
  bool repeat = true;
  tempTuple.resize(hashWords.size());
  vector<DocId> phrDocIds;
  while(repeat)
  {
    for (size_t i = 0; i < hashWords.size(); i++)
      hashWords[i].clear();
    currentDocId = keywordLists[0]._docIds[listCounters[0]];
    phrFlag = false;
    for (size_t i = 0; i < hashWords.size(); i++)
    {
      while(keywordLists[i]._docIds[listCounters[i]] == currentDocId)
      {
        WordId id = keywordLists[i]._wordIdsOriginal[listCounters[i]];
        if (id >= 0)
        {
          if (suggestOnlyPhrases)
          {
            if (hashWords[i].count(id) <= 0)
              hashWords[i][id] = keywordLists[i]._positions[listCounters[i]];
          }
          else
            hashWords[i][id]++;
        }
        listCounters[i]++;
        if (listCounters[i] == keywordLists[i]._docIds.size())
        {
          repeat = false;
          break;
        }
      }
    }
    if (!suggestOnlyPhrases)
    {
      switch(hashWords.size())
      {
        case 3: countFZTriples(hashWords, hashMapCountTuples); break;
        case 2: countPairs(hashWords, hashMapCountPairs); break;
        default: countTuples(hashWords, hashMapCountTuples, tempTuple, 0, 1); break;
      }
    }
    else
    {
      countPhrases(hashWords, hashMapCountTuples, tempTuple, 0, -1);
      if (phrFlag)
        phrDocIds.push_back(currentDocId);
    }
  }

  // 3. Sort tuples of words by their count and return the top-k
  k = MAX(1, k);
  countedQuerySuggestions.clear();
  // Extract each query word from the full query (without operators)
  vector<string> queryWords;
  first = query;
  while(first.getQueryString().size() > 0)
  {
    Query temp;
    first.splitAtLastSeparator(&temp, &last, &separator);
    first = temp;
    queryWords.push_back(Query::normalizeQueryPart(stripQueryWord(last.getQueryString())));  // NOLINT
  }
  FuzzySearch::ExtensionEditDistance extDist;
  if (hashWords.size() == 2 && !suggestOnlyPhrases)
    for (auto iter = hashMapCountPairs.begin(); iter != hashMapCountPairs.end(); iter++)
    {
      countedQuerySuggestions.resize(countedQuerySuggestions.size() + 1);
      countedQuerySuggestions.back().first = &(iter->first);
      double distance = 0;
      for (int j = countedQuerySuggestions.back().first->size() - 1; j >= 0; j--)
      {
        string suggKeyWord;
        suggKeyWord = (*_vocabulary)[_vocabulary->mappedWordId(countedQuerySuggestions.back().first->at(j))];
        distance += 1.0 * extDist.calculate(queryWords[j], suggKeyWord, MAX_ED) /
            MY_MIN(queryWords[j].length(), suggKeyWord.length());
      }
      distance = (distance > 1.0 ? 1 : distance);
      double sim = 1 - distance;
      countedQuerySuggestions.back().second = static_cast<int>(sim * iter->second);
    }
  else
    for (auto iter = hashMapCountTuples.begin(); iter != hashMapCountTuples.end(); iter++)
    {
      countedQuerySuggestions.resize(countedQuerySuggestions.size() + 1);
      countedQuerySuggestions.back().first  = &(iter->first);
      double distance = 0;
      for (int j = countedQuerySuggestions.back().first->size() - 1; j >= 0; j--)
      {
        string suggKeyWord;
        suggKeyWord = (*_vocabulary)[_vocabulary->mappedWordId(countedQuerySuggestions.back().first->at(j))];
        distance += 1.0 * extDist.calculate(queryWords[j], suggKeyWord, MAX_ED) /
            MY_MIN(queryWords[j].length(), suggKeyWord.length());
      }
      distance = (distance > 1.0 ? 1 : distance);
      double sim = 1 - distance;
      countedQuerySuggestions.back().second = static_cast<int>(sim * iter->second);
    }
  sort(countedQuerySuggestions.begin(), countedQuerySuggestions.end(), orderByCounts);
  suggestions->clear();
  scores->clear();
  std::unordered_map<string, bool, StringHashFunction> suggHashMap;
  for (int i = 0; i < MY_MIN(k, static_cast<int>(countedQuerySuggestions.size())); i++)
  {
    string suggestion;
    for (int j = countedQuerySuggestions[i].first->size() - 1; j >= 0; j--)
    {
      suggestion += (*_vocabulary)[_vocabulary->mappedWordId(countedQuerySuggestions[i].first->at(j))];
      if (j > 0)
        suggestion += " ";
    }
    if (suggHashMap.count(suggestion) < 1)
    {
      suggestions->push_back(suggestion);
      scores->push_back(countedQuerySuggestions[i].second);
      suggHashMap[suggestion] = true;
      if (showFuzzyDebugOutput)
        log << i+1 << "-th ranked suggestion: " << suggestion << ", with "
            << countedQuerySuggestions[i].second << " occ." << endl;
    }
    else
      k++;
  }
}

// compute fuzzy suggestion for the case when the number of query words is 2
// this function is faster than the function for the case when the query
// contains n-keywords
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::findFuzzySearchQuerySuggestions2(
    QueryResult& hitsPerQueryWord1,
    QueryResult& hitsPerQueryWord2,
    int k,
    vector<string>* suggestions,
    vector<double>* scores)
{
  CS_ASSERT(suggestions != NULL);
  CS_ASSERT(scores != NULL);

  // 1. find matching q_i*~ by "query rotation"
  if (hitsPerQueryWord1._docIds.size() == 0)
    return;
  if (hitsPerQueryWord2._docIds.size() == 0)
    return;
  hitsPerQueryWord1._docIds.push_back(INT_MAX);

  // 2. count frequencies of matching pairs of words
  // case: 2 query words
  std::unordered_set<WordId> hashSet;
  std::unordered_set<WordId> hashSet1;
  DocId currentDocId;
  vector<size_t> listCounters;
  vector<vector<WordId> > listWords;
  vector<std::unordered_set<WordId> > hashWords;
  vector<std::unordered_map<WordId, int> > hashWords1;
  listCounters.resize(2);
  listWords.resize(2);
  hashWords.resize(2);
  hashWords1.resize(2);
  vector<WordId> tuple;
  tuple.resize(2);
  hashMapCountPairs.clear();
  hashMapCountPairs.reserve(100000);  // NOTE(bast): was hash_map.resize
  if (countsPerWordId.size() != _vocabulary->size())
  {
    fuzzySearchComputeQuerySuggestionsTimer.stop();
    fuzzySearchTotalTimer.stop();
    fuzzySearchComputeQuerySuggestionsTimer.stop();
    countsPerWordId.resize(_vocabulary->size());
    for (size_t i = 0; i < countsPerWordId.size(); i++)
      countsPerWordId[i].push_back(0);
    fuzzySearchComputeQuerySuggestionsTimer.cont();
    fuzzySearchComputeQuerySuggestionsTimer.cont();
    fuzzySearchTotalTimer.cont();
  }
  size_t oldHashSetSize = 0;
  size_t oldHashSetSize1 = 0;
  size_t counter = 0;
  priority_queue<int, vector<int>, Compare > pq;
  bool repeat = true;
  while(repeat)
  {
    if (counter < 100)
    {
      for (size_t i = 0; i < listWords[0].size(); i++)
      {
        hashSet.insert(listWords[0][i]);
        hashSet1.insert(countsPerWordId[listWords[0][i]].back());
      }
      if (hashSet.size() == oldHashSetSize && hashSet1.size() == oldHashSetSize1)
        counter++;
      else
      {
        counter = 0;
        oldHashSetSize = hashSet.size();
        oldHashSetSize1 = hashSet1.size();
      }
    }
    listWords[0].clear();
    listWords[1].clear();
    currentDocId = hitsPerQueryWord1._docIds[listCounters[0]];
    while(hitsPerQueryWord1._docIds[listCounters[0]] == currentDocId)
    {
      WordId id = hitsPerQueryWord1._wordIdsOriginal[listCounters[0]];
      if (id >= 0)
        listWords[0].push_back(id);
      listCounters[0]++;
    }
    if (currentDocId == INT_MAX)
      repeat = false;
    while(hitsPerQueryWord2._docIds[listCounters[1]] == currentDocId)
    {
      WordId id = hitsPerQueryWord2._wordIdsOriginal[listCounters[1]];
      if (id >= 0)
        for (size_t i = 0; i < listWords[0].size(); i++)
          countsPerWordId[listWords[0][i]].push_back(id);
      listCounters[1]++;
    }
  }
  size_t min = 5;
  for (auto it1 = hashSet.begin(); it1 != hashSet.end(); it1++)
  {
    if (countsPerWordId[*it1].size() > min)
    {
      for (size_t i = 1; i < countsPerWordId[*it1].size(); i++)
        countsPerWordId[countsPerWordId[*it1][i]][0]++;
      tuple[0] = *it1;
      for (auto it2 = hashSet1.begin(); it2 != hashSet1.end(); it2++)
      {
        tuple[1] = *it2;
        if (countsPerWordId[*it2][0] > min)
        {
          hashMapCountPairs[tuple] = countsPerWordId[*it2][0];
          if (pq.size() < 10)
            pq.push(countsPerWordId[*it2][0]);
          else
          {
            if (pq.top() < countsPerWordId[*it2][0])
            {
              pq.pop();
              pq.push(countsPerWordId[*it2][0]);
            }
            min = pq.top();
          }
        }
        countsPerWordId[*it2][0] = 0;
      }
    }
  }
  countedQuerySuggestions.clear();
  countedQuerySuggestions.reserve(hashMapCountPairs.size());

  // 3. sort the counted pairs of words by frequency
  k = MAX(1, k);
  for (auto iter = hashMapCountPairs.begin(); iter != hashMapCountPairs.end(); iter++)
  {
    countedQuerySuggestions.resize(countedQuerySuggestions.size() + 1);
    countedQuerySuggestions.back().first  = &(iter->first);
    countedQuerySuggestions.back().second = iter->second;
  }
  sort(countedQuerySuggestions.begin(), countedQuerySuggestions.end(), orderByCounts);
  suggestions->clear();
  scores->clear();
  std::unordered_map<string, bool, StringHashFunction> suggHashMap;
  for (int i = 0; i < MY_MIN(k, static_cast<int>(countedQuerySuggestions.size())); i++)
  {
    string suggestion;
    for (int j = countedQuerySuggestions[i].first->size() - 1; j >= 0; j--)
    {
      suggestion += (*_vocabulary)[_vocabulary->mappedWordId(countedQuerySuggestions[i].first->at(j))];
      if (j > 0)
        suggestion += " ";
    }
    if (suggHashMap.count(suggestion) < 1)
    {
      suggestions->push_back(suggestion);
      scores->push_back(countedQuerySuggestions[i].second);
      suggHashMap[suggestion] = true;
      if (showFuzzyDebugOutput)
        log << i+1 << "-th ranked suggestion: " << suggestion << ", with "
            << countedQuerySuggestions[i].second << " occ." << endl;
    }
    else
      k++;
    if (showFuzzyDebugOutput)
      log << i+1 << "-th ranked suggestion: " << suggestion << ", with "
          << countedQuerySuggestions[i].second << " occ." << endl;
  }
}

// used to compute fuzzy suggestions by filtering when the query is typed
// letter by letter (much faster but it can still be improved)
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::filterQuerySuggestions(
    Query& query,
    int k,
    vector<string>* suggestions,
    vector<double>* scores)
{
  CS_ASSERT(suggestions != NULL);
  CS_ASSERT(scores != NULL);
  vector<string> queryWords;
  Query first = query;
  Query last;
  Separator separator;
  CS_ASSERT_GT(query.getQueryString().size(), 0);
  CS_ASSERT(suggestions != NULL);

  // 1. Extract each query word from the full query (without operators)
  while(first.getQueryString().size() > 0)
  {
    Query temp;
    first.splitAtLastSeparator(&temp, &last, &separator);
    first = temp;
    queryWords.push_back(Query::normalizeQueryPart(stripQueryWord(last.getQueryString())));  // NOLINT
  }

  // 2. Do the actual filtering by checking whether all words of the current query
  // are within distance threshold with the next top suggestion
  // TODO(celikik): since the prefix of the query does not change, why do I compute
  // the distance to all keywords and not only to the last?
  int counter = 0;
  string completion;
  suggestions->clear();
  scores->clear();
  k = MAX(k, 1);
  for (size_t i = 0; i < countedQuerySuggestions.size(); i++)
  {
    bool similar = true;
    CS_ASSERT_EQ(queryWords.size(), (*(countedQuerySuggestions[i].first)).size());
    for (size_t j = 0; j < (*(countedQuerySuggestions[i].first)).size(); j++)
    {
      double threshold = 1;
      if (queryWords[j].length() >= 6)
        threshold = 2;
      const string& complFromVoc = (*_vocabulary)[(*(countedQuerySuggestions[i].first))[j]];
      size_t pos = complFromVoc.rfind(wordPartSep);
      if (pos != string::npos)
        completion = complFromVoc.substr(pos + 1);
      else
        completion = complFromVoc;
      double dist = _fuzzySearcher->getDistance(queryWords[j], completion);
      if (dist > threshold)
      {
        similar = false;
        break;
      }
    }
    if (similar)
    {
      string suggestion;
      for (int j = (*(countedQuerySuggestions[i].first)).size() - 1; j >= 0; j--)
      {
        suggestion += (*_vocabulary)[_vocabulary->mappedWordId((*(countedQuerySuggestions[i].first))[j])];
        if (j > 0)
          suggestion += " ";
      }
      suggestions->push_back(suggestion);
      scores->push_back(countedQuerySuggestions[i].second);
      if (showFuzzyDebugOutput)
      {
        log << counter + 1 << "-th ranked suggestion: " << suggestion << ", ";
        log << countedQuerySuggestions[i].second << " occ." << endl;
      }
      ++counter;
      if (counter == k)
        break;
    }
  }
}

// given a set of word ids of fuzzy search, compute the equivalent
// (hash)set of word ids of the completer
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::computeWordIds(const vector<int>& closestWordIds,
                                         const string& query,
                                         std::unordered_set<int>* hashSet)
{
  // 1. Collect all word-ids, including on those words that have a cluster
  // as a prefix, e.g. C:101:ballerina
  // 1.a. Collect the words as strings first (fuzzysearch has no knowledge
  // of word-ids in the vocabulary)
  vector<string> words;
  for (size_t j = 0; j < closestWordIds.size(); j++)
  {
    words.push_back(_fuzzySearcher->getClusterCentroid(closestWordIds[j]));
    for (size_t k = 0; k < _fuzzySearcher->clusterIdsPerClusterCenter[closestWordIds[j]].size(); k++)  // NOLINT
    {
      ostringstream word;
      int clusterId = _fuzzySearcher->clusterIdsPerClusterCenter[closestWordIds[j]][k];  // NOLINT
      word << "C" << wordPartSep << clusterId << wordPartSep;
      word << _fuzzySearcher->getClusterCentroid(closestWordIds[j]);
      words.push_back(word.str());
    }
  }

  // 1.b. Convert word -> word-ids
  // FuzzySearch::IntHashSet hashSet;
  CS_ASSERT(hashSet != NULL);
  for (size_t i = 0; i < words.size(); i++)
  {
    unsigned int wordId = _vocabulary->findWord(words[i]);
    hashSet->insert(wordId);
  }
}

// filter only those posting from the old result list that have word ids in
// hashSet to produce the new result list
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::fuzzySearchFilterList(
                                         const QueryResult& oldResultList,
                                         const std::unordered_set<int>& hashSet,
                                         QueryResult* newResultList)
{
  newResultList->_docIds.reserve(oldResultList._docIds.size());
  newResultList->_positions.reserve(oldResultList._positions.size());
  newResultList->_wordIdsOriginal.reserve(oldResultList._wordIdsOriginal.size());
  newResultList->_scores.reserve(oldResultList._scores.size());
  for (size_t i = 0; i < oldResultList._docIds.size(); i++)
  {
    if (hashSet.find(oldResultList._wordIdsOriginal[i]) != hashSet.end())
    {
      newResultList->_docIds.push_back(oldResultList._docIds[i]);
      newResultList->_positions.push_back(oldResultList._positions[i]);
      newResultList->_wordIdsOriginal.push_back(oldResultList._wordIdsOriginal[i]);
      newResultList->_scores.push_back(oldResultList._scores[i]);
    }
  }
}

// process the fuzzy search query by filtering when the query is typed
// letter by letter
// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::fuzzySearchFilterResultList(
                           const QueryResult& oldResultList,
                           const vector<int>& closestWordIds,
                           const string& query,
                           QueryResult* newResultList)
{

  // compute a set of word ids of the completer from closestWordIds needed
  // for the filtering
  std::unordered_set<int> hashSet;
  computeWordIds(closestWordIds, query, &hashSet);

  // 1.c. This should be executed if fuzzy search is in word mode and the query
  // is a prefix query, e.g. memori*~. Includes all words in the vocabulary
  // that are strict prefixes of the query
  // comment: unfortunately when using fuzzy search in word mode using
  // the history does not make sense since the old result list is not always
  // a superset of the new result list :-(
  // if (query.length() > 0)
  // {
  //  unsigned int wordId = _vocabulary->findWord(query);
  //  while (wordId < _vocabulary->size())
  //  {
  //    if (FuzzySearch::isStrictPrefix(query, (*_vocabulary)[wordId]))
  //      hashSet.insert(wordId);
  //    else
  //      if (query != (*_vocabulary)[wordId])
  //        break;
  //    wordId++;
  //  }
  // }

  // 2. Filter the old result list by performing a hash-look-up of the current
  // word-id in hashSet
  fuzzySearchFilterList(oldResultList, hashSet, newResultList);
}

// *** global funs. and variables required by processFuzzySearchQuery ***

// order clusters by number of relevant words contained
// _____________________________________________________________________________
static bool orderByNofRelWords(const ClusterOfSimilarWords& x,
                               const ClusterOfSimilarWords& y)
{
  return x.nofRelevantWords > y.nofRelevantWords;
}

// order clusters by fraction of relevant words in the cluster
// _____________________________________________________________________________
static bool orderByFracRelWords(const ClusterOfSimilarWords& x,
                               const ClusterOfSimilarWords& y)
{
  return x.fractionRelevant > y.fractionRelevant;
}

// used to sort lists by decreasing order (used for debugging only)
// _____________________________________________________________________________
bool decreasingOrder(const int& x, const int& y)
{
  return x > y;
}

// used for sorting Queries before reading from disk
// _____________________________________________________________________________
bool orderQuery(const Query& x, const Query& y)
{
  return x.getQueryString() < y.getQueryString();
}

// map id of a cluster to its array index
vector<int> clusterId2ClusterIndex;

// some global counter variable (used for debugging)
size_t numClustersSelected = 0;
size_t totalVolumeProcessed = 0;

// some general params
bool usePremerging              = true;
bool useHistory                 = true;
bool filterClustersGlobal       = true;
bool skipUnrelatedFrequentWords = false;
bool processRareWords           = true;
bool processFrequentWords       = true;
bool useFuzzySearchGlobal       = true;
bool computeQuerySuggestions    = false;
bool useOptimalBinaryMerge      = true;

bool useBaseline                = false;

// params of the processing algorithm
double factor         = 2;
double fraction       = 0.5;
double minClusterPrec = 1.0;
size_t nofLists       = 30;
const double recall   = 1.0;

// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::processFuzzySearchQuery
                           (const QueryResult& inputList,
                            const Query&       firstPartOfQuery,
                            const Query&       lastPartOfQuery,
                            const Separator&   separator,
                                  QueryResult& result)
{
  bool useFuzzySearch = useFuzzySearchGlobal;
  // comment: lines below needed only for the experiments
  if (useBaseline)
  {
    useOptimalBinaryMerge = false;
    usePremerging = false;
    filterClustersGlobal = false;
    fuzzySearchUseClustering = false;
  }
  else
  {
    useOptimalBinaryMerge = true;
    usePremerging = true;
    filterClustersGlobal = true;
    fuzzySearchUseClustering = true;
  }

  fuzzySearchTotalTimer.cont();
  fuzzySearchEditDistanceThreshold = _fuzzySearcher->getThreshold();

  if (showFuzzyDebugOutput)
  {
    log << "! FUZZY: fuzzy search query; last part of query is \""
        << lastPartOfQuery << "\"" << endl;
    log << "! FUZZY: fuzzy searcher has "
        << _fuzzySearcher->getNofClusterCentroids() << " cluster-centroids" << endl;
  }
  Query newLastPartOfQuery = lastPartOfQuery;
  newLastPartOfQuery.removeLastCharacter();
  bool isSynonymQuery = newLastPartOfQuery.getLastCharacter() == '^';
  if (isSynonymQuery) newLastPartOfQuery.removeLastCharacter();
  bool isPrefixQuery = newLastPartOfQuery.getLastCharacter() == '*';
  if (isPrefixQuery) newLastPartOfQuery.removeLastCharacter();
  // Remove everything until the last : and remember it.
  string queryStringTmp = newLastPartOfQuery.getQueryString();
  string lastPartOfQueryUpToLastColon;
  size_t pos = queryStringTmp.rfind(wordPartSep);
  if (pos != string::npos)
  {
    lastPartOfQueryUpToLastColon = queryStringTmp.substr(0, pos + 1);
    queryStringTmp = queryStringTmp.substr(pos + 1);
  }
  newLastPartOfQuery.setQueryString(queryStringTmp);
  string queryWord = newLastPartOfQuery.getQueryString();
  queryWord = Query::normalizeQueryPart(queryWord);

  // disable fuzzy search on words that are too short
  if (queryWord.length() < MIN_WORD_LEN)
    useFuzzySearch = false;

  bool filterClusters = filterClustersGlobal;

  // 1. Find words similar to query word, within given edit distance.
  vector<int> closestWordIds;
  vector<double> closestWordDistances;
  bool isInLexicon;
  if (showFuzzyDebugOutput)
  {
    log << "! FUZZY: finding closest words for \"" << queryWord << "\"" << endl;
    if (_fuzzySearcher->completionMatching())
      log << "! FUZZY: *** prefix search mode is used" << endl;
    else
      log << "! FUZZY: *** word search mode is used" << endl;
  }
  Timer timer;
  timer.start();
  fuzzySearchDistanceComputationsTimer.cont();
  if (useFuzzySearch)
  {
    _fuzzySearcher->findClosestWords(
        _fuzzySearcher->usingNormalization() ? _fuzzySearcher->normalizeWord(queryWord) : queryWord,
        false,
        &isInLexicon,
        &closestWordIds,
        &closestWordDistances);

    // if the query is a frequent word and if we want to skip words with
    // relatively similar frequency, that probably have different meaning
    // than the query word
    if (!_fuzzySearcher->completionMatching() &&
        skipUnrelatedFrequentWords &&
        closestWordIds.size() > 0)
    {
      string queryWord1 = _fuzzySearcher->usingNormalization()
          ? _fuzzySearcher->normalizeWord(queryWord) : queryWord;
      if (_fuzzySearcher->getFrequency(queryWord1) != 0)
      {
        if (_fuzzySearcher->getClusterCentroid(closestWordIds[0]) == queryWord1)
        {
          vector<int> temp1;
          vector<double> temp2;
          temp1.push_back(closestWordIds[0]);
          temp2.push_back(closestWordDistances[0]);
          int q_tf = _fuzzySearcher->getFrequency(queryWord1);
          if (showFuzzyDebugOutput)
            log << "! FUZZY: Query keyword has frequency: " << q_tf << endl;
          for (size_t i = 1; i < closestWordIds.size(); i++)
          {
            string word = _fuzzySearcher->getClusterCentroid(closestWordIds[i]);
            // if the word has considerably smaller or considerably larger
            // frequency, include it
            if (_fuzzySearcher->getFrequency(word) != 0)
            {
              int w_tf = _fuzzySearcher->getFrequency(word);
              if (q_tf <= 100 || (q_tf > 100 && w_tf <= 100))
              {
                temp1.push_back(closestWordIds[i]);
                temp2.push_back(closestWordDistances[i]);
              }
              else
              {
                if (showFuzzyDebugOutput)
                  log << "! FUZZY: Skipping frequent but probably "
                      "unrelated word \"" << word << "\" with frequency: "
                      << w_tf << endl;
              }
            }
            else
            {
              temp1.push_back(closestWordIds[i]);
              temp2.push_back(closestWordDistances[i]);
            }
          }
          closestWordIds = temp1;
          closestWordDistances = temp2;
        }
        else
        {
          log << "! FUZZY: Warning: queryWord != closestWords[0], although "
              "queryWord is a frequent word! ("
              << _fuzzySearcher->getClusterCentroid(closestWordIds[0])
              << ", " << queryWord1 << ")" << endl;
        }
      }
    }
  }

  // a hack due to a bug in buildIndex
  // relevant only when in "INV mode"
  vector<int> temp1 = closestWordIds;
  vector<double> temp2 = closestWordDistances;
  closestWordIds.clear();
  closestWordDistances.clear();
  for (size_t i = 0; i < temp1.size(); i++)
  {
    char chr = _fuzzySearcher->getClusterCentroid(temp1[i])[0];
    if (chr != 'x' && chr != 'y' && chr != 'z' &&
        !(_fuzzySearcher->getClusterCentroid(temp1[i])[0] == 'w' &&
            _fuzzySearcher->getClusterCentroid(temp1[i])[1] == 'y'))
    {
      closestWordIds.push_back(temp1[i]);
      closestWordDistances.push_back(temp2[i]);
    }
  }

  // if completion matching is used, then don't allow
  // more than 1 error in the first 4 letters
  // hack: also, if completion matching is used but query is no prefix,
  // filter out long completions.
  if (_fuzzySearcher->completionMatching() && queryWord.length() >= 4)
  {
    FuzzySearch::ExtensionEditDistance exDist;
    vector<int> temp1 = closestWordIds;
    vector<double> temp2 = closestWordDistances;
    closestWordIds.clear();
    closestWordDistances.clear();
    for (size_t i = 0; i < temp1.size(); i++)
    {
      string completion = _fuzzySearcher->getClusterCentroid(temp1[i]);
      if (temp2[i] > 1)
      {
        string completion4 = completion.substr(0, MY_MIN(4, completion.length()));
        if (exDist.calculate(queryWord, completion4, false) <= 1)
        {
          if (isPrefixQuery
              || (completion.length() - queryWord.length() <= MAX_ED))
          {
            closestWordIds.push_back(temp1[i]);
            closestWordDistances.push_back(temp2[i]);
          }
        }
      }
      else
        if (isPrefixQuery
            || (completion.length() - queryWord.length() <= MAX_ED))
        {
          closestWordIds.push_back(temp1[i]);
          closestWordDistances.push_back(temp2[i]);
        }
    }
  }

  if (_fuzzySearcher->completionMatching() /* && queryWord.length() >= 4 */)
  {
    FuzzySearch::ExtensionEditDistance exDist;
    vector<int> temp1 = closestWordIds;
    vector<double> temp2 = closestWordDistances;
    closestWordIds.clear();
    closestWordDistances.clear();
    for (size_t i = 0; i < temp1.size(); i++)
    {
      string completion = _fuzzySearcher->getClusterCentroid(temp1[i]);
      if (isPrefixQuery
          || (static_cast<int>(completion.length()) - static_cast<int>(queryWord.length()) <= MAX_ED))
      {
        closestWordIds.push_back(temp1[i]);
        closestWordDistances.push_back(temp2[i]);
      }
    }
  }

  /*
  // needed only for simulation / experiments
  if (_fuzzySearcher->completionMatching())
  {
    FuzzySearch::ExtensionEditDistance exDist;
    vector<int> temp1 = closestWordIds;
    vector<double> temp2 = closestWordDistances;
    closestWordIds.clear();
    closestWordDistances.clear();
    for (size_t i = 0; i < temp1.size(); i++)
      if (temp2[i] <= 0)
      {
        closestWordIds.push_back(temp1[i]);
        closestWordDistances.push_back(temp2[i]);
      }
  }
  */

  fuzzySearchDistanceComputationsTimer.stop();
  timer.stop();

  std::unordered_set<int> relevantWords;
  std::unordered_set<int> relevantWordsFW;
  std::unordered_set<int> relevantWordsIW;
  vector<string> closestWords(closestWordIds.size());
  fuzzySearchNumSimilarWords = closestWords.size();

  for(size_t i = 0; i < closestWordIds.size(); i++)
  {
    relevantWords.insert(closestWordIds[i]);
    closestWords[i] = _fuzzySearcher->getClusterCentroid(closestWordIds[i]);
  }
  CS_ASSERT_EQ(closestWordIds.size(), closestWordDistances.size());
  if (showFuzzyDebugOutput)
  {
    log << "! FUZZY: Finding closest words time: " << timer << endl;
    log << "! FUZZY: found "
        << numberAndNoun(closestWordIds.size(), "closest word", "closest words")
        << " in " << timer << endl;
    log << endl;
    log << "! FUZZY: is original word in lexicon? "
        << (isInLexicon ? "YES" : "NO") << endl;
  }

  // 1b. Set word id of closest match as best match id. Note that the ids
  // returned by findClosestWords have nothing to do with the word ids from
  // the _vocabulary. First check whether the original word itself is in the
  // vocabulary, if yes, that is the closest word. 
  // Note that we use the un-normalized query word here in any case (otherwise
  // we can get strange effects, like a word X that is in the vocabulary being
  // normalized, but it's normalization norm(X) is not in the vocabulary, and the
  // vocabulary word closest to N(X) is some word different from X).
  string closestWord;
  unsigned int closestWordId;
  {
    closestWord = queryWord;
    closestWordId = _vocabulary->findWord(queryWord);
    if (closestWordId >= _vocabulary->size() ||
        (*_vocabulary)[closestWordId] != closestWord)
    {
      closestWordId = UINT_MAX;
      if (closestWordIds.size() > 0)
      {
        closestWord = _fuzzySearcher->getClusterCentroid(closestWordIds[0]);
        closestWordId = _vocabulary->findWord(closestWord);
        if (closestWordId >= _vocabulary->size() ||
            (*_vocabulary)[closestWordId] != closestWord)
          closestWordId = UINT_MAX;
      }
    }
  }
  if (closestWordId != UINT_MAX && showFuzzyDebugOutput)
    log << "! FUZZY: closest word is \"" << closestWord << "\", setting"
        << " _lastBestMatchWordId to its word id " << closestWordId << endl;
  _lastBestMatchWordId = closestWordId != UINT_MAX ? closestWordId : -1;

  // 1c. Check if "previous" query is in history and if so, process it by
  // filtering
  // TODO(celikik): recomputing the query from scratch should happen whenever:
  // len. old query <= 5,  len. new query > 5;
  // len. old query <= 10, len. new query > 10;
  // the condition below is only a hack.
  bool computedFromHistory = false;
  if (useHistory
      && _fuzzySearcher->completionMatching()
      && queryWord.length() > MIN_WORD_LEN && queryWord.length() != 6
      && queryWord.length() != 11)
  {
    Timer fuzzySearchFilterTimer;
    fuzzySearchFilterTimer.cont();
    string fullQuery = firstPartOfQuery.getQueryString() + separator.getSeparatorString() +
        queryWord;
    int minLength = firstPartOfQuery.getQueryString().length() + 1 + MY_MIN(lastPartOfQuery.length(), 4);
    if (firstPartOfQuery.getQueryString().length() == 0)
      minLength = MY_MIN(lastPartOfQuery.length(), 4);
    string star;
    if (isPrefixQuery)
      star = "*";
    for (int i = fullQuery.size() - 1; i >= minLength; i--)
    {
      string prefixQuery = fullQuery.substr(0, i) + star + "~" + "&hf=" + getFlagForHistory();
      QueryResult* resultListFromHistory = history->isContained(prefixQuery);
      if (resultListFromHistory != NULL)
      {
        computedFromHistory = true;
        fuzzySearchFilterResultList(*resultListFromHistory,
            closestWordIds,
            // (!_fuzzySearcher->completionMatching() && isPrefixQuery) ? queryWord : "",
            "",
            &result);
        fuzzySearchFilterTimer.stop();
        fuzzySearchComputeQuerySuggestionsTimer.start();
        if (computeQuerySuggestions)
        {
          filterQuerySuggestions(
              _query,
              10,
              &result._topFuzzySearchQuerySuggestions,
              &result._topFuzzySearchQuerySuggestionScores);
        }
        fuzzySearchComputeQuerySuggestionsTimer.stop();
        fuzzySearchTotalTimer.stop();
        if (showFuzzyDebugOutput)
        {
          log << "! FUZZY: previous query \""
              << prefixQuery << "\" found in history." << endl;
          log << "! FUZZY: filtering time               : "
              << fuzzySearchFilterTimer << endl;
          log << "! FUZZY: Total fuzzy search time      : "
              << fuzzySearchTotalTimer << endl;
          log << "! FUZZY: Total ranking time           : "
              << fuzzySearchComputeQuerySuggestionsTimer << " / ";
          log << 100 * fuzzySearchComputeQuerySuggestionsTimer.usecs()
              / fuzzySearchTotalTimer.usecs() << "%" << endl;
        }
        return;
      }
    }
    if (!computedFromHistory && showFuzzyDebugOutput)
      log << "! FUZZY: no prefix of query found in history." << endl;
  }

  // 2. Compute the OR of the original word and the words closest to it.  If the
  // original word had a * appended, append a * to all closest words as well.
  // Also compute the set of cluster ids.
  // NOTE(bast): this OR is just for the output to the log, so that we can see
  // all the similar words, and how much we save by taking the OR of cluster id
  // prefixes instead.
  string orOfClosestWords;
  orOfClosestWords = queryWord;
  if (isPrefixQuery) orOfClosestWords += "*";
  // NEW(hagn, 01Aug11): If the original word had a ^ appended, append a ^
  // to all closest words as well.
  if (isSynonymQuery) orOfClosestWords += "^";

  Timer getClusterIdsTimer;
  google::sparse_hash_map<int, int> clusterIds;
  google::sparse_hash_map<int, int> clusterIdsIW;
  for (size_t i = 0; i < closestWordIds.size(); ++i)
  {
    int closestWordId = closestWordIds[i];
    CS_ASSERT_LE(0, closestWordId);
    CS_ASSERT_GT(_fuzzySearcher->getNofClusterCentroids(), (size_t)(closestWordId));
    fuzzySearchAuxTimer.cont();
    // TODO(celikik): why did I comment this out?
    // if (closestWords[i] != queryWord)
    // {
      orOfClosestWords += "|" + closestWords[i];
      if (isPrefixQuery) orOfClosestWords += "*";
      // NEW(hagn, 01Aug11): If the original word had a ^ appended, append a ^
      // to all closest words as well.
      if (isSynonymQuery) orOfClosestWords += "^";
    // }
    fuzzySearchAuxTimer.stop();
    ostringstream os;
    if (showFuzzyDebugOutput)
    {
      os << "! FUZZY: closest word #" << i + 1 << " is \"" << closestWords[i] << "\""
         << " (" << closestWordDistances[i] << "); cluster ids are: ";
    }

    // Add the cluster ids of this word to clusterIdsForAllWords.
    if (_fuzzySearcher->clusterIdsPerClusterCenter.size() >
        (size_t)(closestWordId))
    {
      fuzzySearchDistanceComputationsTimer.cont();
      getClusterIdsTimer.cont();
      const vector<int>& clusterIdsOfThisWord =
          _fuzzySearcher->clusterIdsPerClusterCenter[closestWordId];
      for (size_t j = 0; j < clusterIdsOfThisWord.size(); ++j)
      {
        if (_fuzzySearcher->isFrequentWordClusterId(clusterIdsOfThisWord[j]))
        {
          clusterIds[clusterIdsOfThisWord[j]]++;
        }
        else
          clusterIdsIW[clusterIdsOfThisWord[j]]++;
        if (showFuzzyDebugOutput)
          os << (j > 0 ? ", " : "") << clusterIdsOfThisWord[j];
      }
    }
    if (_fuzzySearcher->isClusterCentroid[closestWordId])
    {
      if (processFrequentWords)
        relevantWordsFW.insert(closestWordId);
    }
    else
    {
      if (processRareWords)
        relevantWordsIW.insert(closestWordId);
    }
    fuzzySearchDistanceComputationsTimer.stop();
    getClusterIdsTimer.stop();
    if (showFuzzyDebugOutput)
      log << IF_VERBOSITY_HIGH << os.str() << endl;
  }

  if (showFuzzyDebugOutput)
  {
    log << IF_VERBOSITY_HIGH << "! FUZZY: disjunction (OR) of closest "
        "words is \"" << orOfClosestWords << "\"" << endl;
    log << "! FUZZY: get cluster ids time: " << getClusterIdsTimer << endl;
  }

  // 3a. For each cluster id, get the words matching the respective query and
  // put them all into a ClusterOfSimilarWords object.
  vector<ClusterOfSimilarWords> clusters;
  vector<ClusterOfSimilarWords> clustersIW;
  std::unordered_set<string, StringHashFunction> allWordsInClusters;
  Timer buildClustersTimer;
  buildClustersTimer.start();
  fuzzySearchDistanceComputationsTimer.cont();
  bool trivialClustering = true;
  if (fuzzySearchUseClustering)
  {
    clusters.resize(clusterIds.size());
    int count = 0;
    for (google::sparse_hash_map<int, int>::iterator it = clusterIds.begin();
         it != clusterIds.end(); ++it)
    {
      clusters[count].clusterId = it->first;
      clusters[count].size = _fuzzySearcher->wordIdsPerCluster[it->first].size();
      clusters[count].nofRelevantWords = 1.0 * it->second;
      clusters[count].fractionRelevant = 1.0 * it->second / clusters[count].size;
      if (clusters[count].size != 1)
        trivialClustering = false;
      count++;
    }
    clustersIW.resize(clusterIdsIW.size());
    count = 0;
    for (google::sparse_hash_map<int, int>::iterator it = clusterIdsIW.begin();
         it != clusterIdsIW.end(); ++it)
    {
      clustersIW[count].clusterId = it->first;
      clustersIW[count].size = _fuzzySearcher->wordIdsPerCluster[it->first].size();
      clustersIW[count].nofRelevantWords = 1.0 * it->second;
      clustersIW[count].fractionRelevant = 1.0 * it->second / clustersIW[count].size;
      if (clustersIW[count].size != 1)
        trivialClustering = false;
      count++;
    }
    buildClustersTimer.stop();
    Timer sortTimer;
    sortTimer.start();
    sort(clusters.begin(), clusters.end(), orderByNofRelWords);
    sort(clustersIW.begin(), clustersIW.end(), orderByFracRelWords);
    sortTimer.stop();
    fuzzySearchDistanceComputationsTimer.stop();

    if (showFuzzyDebugOutput)
    {
      log << "! FUZZY: Cluster building time: " << buildClustersTimer << endl;
      log << "! FUZZY: Sorting time: " << sortTimer << endl;
      log << "! FUZZY: "
          << numberAndNoun(clusterIds.size() + clusterIdsIW.size(), "cluster", "clusters") << endl;
          // << " with " << numberAndNoun(numDistinctWordsInClusters, "word", "words")
          // << endl;
          // << ", of which " << numberAndNoun(relevantWords.size(), "is", "are")
          // << " relevant" << endl;
    }
  }

  // 3b. Select from the above computed clusters that cover the similar words
  // for this query with Marjan's greedy heuristic.
  vector<bool> isClusterSelected;
  vector<bool> isClusterSelectedIW;
  vector<double> qualityOfClusters;
  vector<double> qualityOfClustersIW;
  size_t numRelevantWordsSelected = 0;
  size_t numRelevantWordsSelectedIW = 0;
  size_t numDistinctWordsInSelectedClusters;
  size_t numRelevantWords = relevantWords.size();
  Timer selectClustersTimer;
  selectClustersTimer.start();
  fuzzySearchDistanceComputationsTimer.cont();
  if (!fuzzySearchUseClustering || trivialClustering)
    filterClusters = false;
  if (fuzzySearchUseClustering)
  {
    if (trivialClustering)
    {
      if (showFuzzyDebugOutput)
        log << "! FUZZY: Using trivial clustering !" << endl;
      isClusterSelected.resize(clusters.size());
      for (size_t i = 0; i < isClusterSelected.size(); i++)
        isClusterSelected[i] = true;
      isClusterSelectedIW.resize(clustersIW.size());
      for (size_t i = 0; i < isClusterSelectedIW.size(); i++)
        isClusterSelectedIW[i] = true;
      numRelevantWordsSelected = clusters.size() + clustersIW.size();
      numDistinctWordsInSelectedClusters = numRelevantWordsSelected;
      qualityOfClusters.resize(clusters.size());
      qualityOfClustersIW.resize(clustersIW.size());
    }
    else
    {
      // select clusters for frequent words
      clusterId2ClusterIndex.resize(_fuzzySearcher->wordIdsPerCluster.size());
      for (size_t j = 0; j < clusters.size(); j++)
        clusterId2ClusterIndex[clusters[j].clusterId] = j;
      selectClusters(_fuzzySearcher->wordIdsPerCluster,
                      _fuzzySearcher->clusterIdsPerClusterCenter,
                      clusterId2ClusterIndex,
                      clusters,
                      relevantWordsFW,
                      4,
                      &isClusterSelected,
                      &qualityOfClusters,
                      &numRelevantWordsSelected,
                      &numDistinctWordsInSelectedClusters);
      size_t numDistinctWordsInSelectedClustersIW = 0;
      for (size_t j = 0; j < clustersIW.size(); j++)
        clusterId2ClusterIndex[clustersIW[j].clusterId] = j;

      // select clusters for infrequent words
      // isClusterSelectedIW.resize(clustersIW.size());
      selectClusters(_fuzzySearcher->wordIdsPerCluster,
                      _fuzzySearcher->clusterIdsPerClusterCenter,
                      clusterId2ClusterIndex,
                      clustersIW,
                      relevantWordsIW,
                      1,
                      &isClusterSelectedIW,
                      &qualityOfClustersIW,
                      &numRelevantWordsSelectedIW,
                      &numDistinctWordsInSelectedClustersIW);
      numRelevantWordsSelected += numRelevantWordsSelectedIW;
      numDistinctWordsInSelectedClusters += numDistinctWordsInSelectedClustersIW;
    }
  }
  else
  {
    numRelevantWordsSelected = relevantWords.size();
    numDistinctWordsInSelectedClusters = relevantWords.size();
    isClusterSelected.resize(clusters.size());
    for (size_t i = 0; i < isClusterSelected.size(); i++)
      isClusterSelected[i] = false;
  }
  fuzzySearchDistanceComputationsTimer.stop();
  selectClustersTimer.stop();
  if (showFuzzyDebugOutput)
    log << "! FUZZY: Clusters selected in " << selectClustersTimer << endl;

  // 3c. Show which clusters have been selected + precision and recall.
  size_t numClustersSelected = 0;
  if (fuzzySearchUseClustering)
  {
    for (size_t i = 0; i < clusters.size(); ++i)
    {
      if (isClusterSelected[i]) ++numClustersSelected;
      if (showFuzzyDebugOutput)
        log << IF_VERBOSITY_HIGH
            << "! FUZZY: " << (isClusterSelected[i] ? GREEN : RED)
            << clusters[i].DebugString() << "  "
            << (isClusterSelected[i] ? "[KEEP]" : "[DISCARD]")
            << BLACK << endl;
    }
    for (size_t i = 0; i < clustersIW.size(); ++i)
    {
      if (isClusterSelectedIW[i]) ++numClustersSelected;
      if (showFuzzyDebugOutput)
        log << IF_VERBOSITY_HIGH
            << "! FUZZY: " << (isClusterSelectedIW[i] ? GREEN : RED)
            << clustersIW[i].DebugString() << "  "
            << (isClusterSelectedIW[i] ? "[KEEP]" : "[DISCARD]")
            << BLACK << endl;
    }
  }
  else
    numClustersSelected = relevantWords.size();
  if (numRelevantWords > 0 && numDistinctWordsInSelectedClusters > 0)
  {
    if (showFuzzyDebugOutput)
      log << "! FUZZY: selected "
          << numberAndNoun(numClustersSelected, "cluster", "clusters")
          << ", recall is "
          << (100 * numRelevantWordsSelected / numRelevantWords)
          << "%, precision is "
          << (100 * numRelevantWordsSelected / numDistinctWordsInSelectedClusters)
          << "%, smoothed precision is [TODO]" << endl;
  }

  // 4a. Compute disjunction (OR) of the original word and the prefixes
  // corresponding to the selected cluster ids (for example:
  // ball*|C:234:*|C:1203:*).
  // Also remember query parts separately, so that we can call
  // the new QueryResult::mergeResultLists taking a vector<QueryResult>.
  ostringstream orOfClusterIdPrefixes;
  if (showFuzzyDebugOutput)
    orOfClusterIdPrefixes << lastPartOfQueryUpToLastColon << queryWord;
  if (isPrefixQuery) orOfClusterIdPrefixes << "*";
  // NEW(hagn, 01Aug11): If the original word had a ^ appended, append a ^
  // to the prefixes as well.
  if (isSynonymQuery) orOfClusterIdPrefixes << "^";
  ostringstream firstSeparateQueryLastPart;
  firstSeparateQueryLastPart << lastPartOfQueryUpToLastColon << queryWord;
  if (isPrefixQuery) firstSeparateQueryLastPart << "*";
  // NEW(hagn, 01Aug11): If the original word had a ^ appended, append a ^
  // to the prefixes as well.
  if (isSynonymQuery) orOfClusterIdPrefixes << "^";

  vector<Query> separateQueriesLastPart;
  if ((!processFrequentWords && processRareWords) || !useFuzzySearch
      || (!isInLexicon && !_fuzzySearcher->completionMatching()))
  {
    // Comment: In the original ver. this always takes place i.e. adding the
    // query as the first thing in the disj. but I see it this is necessary only
    // when mode2 of fuzzysearch is used.
    // Comment 1: I am not sure if one should always add the word if it is not
    // in the vocabulary.
    // Comment 2: If it is in word mode, the query is always added as the first
    // thing in the disj. This results in double matches, but if I removed results
    // in not finding completions of the query word.
    separateQueriesLastPart.resize(1);
    separateQueriesLastPart[0].setQueryString(firstSeparateQueryLastPart.str());
  }

  // If the closest word differs from the original query word, add the closest
  // word to the OR query. Only that way we get the BEST_MATCH_BONUS in this
  // case. For example, bethoven*|beethoven*|C:2132:*|...
  if (closestWord != queryWord)
  {
    orOfClusterIdPrefixes << "|" << lastPartOfQueryUpToLastColon << closestWord;
    if (isPrefixQuery) orOfClusterIdPrefixes << "*";
    // NEW(hagn, 01Aug11):  If the original word had a ^ appended, append a ^
    // to closestWord as well.
    if (isSynonymQuery) orOfClusterIdPrefixes << "^";
  }
  unsigned int n;
  // experimental
  vector<double> qualityOfClustersAll;
  qualityOfClustersAll.resize(separateQueriesLastPart.size());
  if (fuzzySearchUseClustering)
  {
    // add frequent words in clusters
    n = clusters.size();
    CS_ASSERT_EQ(clusters.size(), qualityOfClusters.size());
    for (size_t i = 0; i < n; ++i)
    {
      if (isClusterSelected[i] == false) continue;
      ostringstream os;
      int clusterId = clusters[i].clusterId;
      os << lastPartOfQueryUpToLastColon << "C" << wordPartSep << clusterId
         << wordPartSep << "*";
      if (showFuzzyDebugOutput)
        orOfClusterIdPrefixes << "|" << os.str();
      separateQueriesLastPart.resize(separateQueriesLastPart.size() + 1);
      separateQueriesLastPart.back().setQueryString(os.str());
      qualityOfClustersAll.push_back(1 + qualityOfClusters[i]);
    }
    // add frequent words not in clusters
    if (relevantWordsFW.size() > 0 && 1.0 * numRelevantWordsSelected / closestWordIds.size() < recall)
    {
      for (auto iter = relevantWordsFW.begin(); iter != relevantWordsFW.end(); iter++)
      {
        string word = _fuzzySearcher->getClusterCentroid(*iter);
        ostringstream os;
        os << lastPartOfQueryUpToLastColon << word;
        if (showFuzzyDebugOutput)
          orOfClusterIdPrefixes << "|" << os.str();
        separateQueriesLastPart.resize(separateQueriesLastPart.size() + 1);
        separateQueriesLastPart.back().setQueryString(os.str());
        qualityOfClustersAll.push_back(1 + 1.0);
        ++numClustersSelected;
        ++numRelevantWordsSelected;
        ++numDistinctWordsInSelectedClusters;
        if (1.0 * numRelevantWordsSelected / closestWordIds.size() >= recall)
          break;
      }
    }
    // add infrequent words in clusters
    n = clustersIW.size();
    CS_ASSERT_EQ(clustersIW.size(), qualityOfClustersIW.size());
    for (size_t i = 0; i < n; ++i)
    {
      ostringstream os;
      if (isClusterSelectedIW[i] == false) continue;
      int clusterId = clustersIW[i].clusterId;
      os << lastPartOfQueryUpToLastColon << "C" << wordPartSep << clusterId << wordPartSep << "*";
      if (showFuzzyDebugOutput)
        orOfClusterIdPrefixes << "|" << os.str();
      separateQueriesLastPart.resize(separateQueriesLastPart.size() + 1);
      separateQueriesLastPart.back().setQueryString(os.str());
      qualityOfClustersAll.push_back(qualityOfClustersIW[i]);
    }
    // add infrequent words not in clusters
    if (relevantWordsIW.size() > 0 && 1.0 * numRelevantWordsSelected / closestWordIds.size() < recall)
    {
      for (auto iter = relevantWordsIW.begin(); iter != relevantWordsIW.end(); iter++)
      {
        string word = _fuzzySearcher->getClusterCentroid(*iter);
        ostringstream os;
        os << lastPartOfQueryUpToLastColon << word;
        if (showFuzzyDebugOutput)
          orOfClusterIdPrefixes << "|" << os.str();
        separateQueriesLastPart.resize(separateQueriesLastPart.size() + 1);
        separateQueriesLastPart.back().setQueryString(os.str());
        qualityOfClustersAll.push_back(1.0);
       ++numClustersSelected;
        ++numRelevantWordsSelected;
        ++numDistinctWordsInSelectedClusters;
        if (1.0 * numRelevantWordsSelected / closestWordIds.size() >= recall)
          break;
      }
    }
  }
  else
  {
    // comment: // TODO(celikik): what's this? is it relevant?
    // if (closestWords[i] != queryWord)
    n = closestWords.size();
    for (size_t i = 0; i < n; ++i)
    {
      ostringstream os;
      if (_fuzzySearcher->isClusterCentroid[closestWordIds[i]] || processRareWords)
      {
        os << lastPartOfQueryUpToLastColon << closestWords[i];
        // comment: in my version this was commented
        // if (isPrefixQuery && !_fuzzySearcher->completionMatching())
        //  os << "*";
        // NEW(hagn, 01Aug11): If the original word had a ^ appended, append a ^
        // to closestWord as well.
        if (isSynonymQuery) os << "^";
        separateQueriesLastPart.resize(separateQueriesLastPart.size() + 1);
        separateQueriesLastPart.back().setQueryString(os.str());
      }
    }
  }

  // orOfClusterIdPrefixes << os.str();
  // if (i < clusters.size() - 1)
  //  orOfClusterIdPrefixes << "|";
  if (fuzzySearchUseClustering && showFuzzyDebugOutput)
    log << "! FUZZY: or of cluster id prefixes is \""
        << orOfClusterIdPrefixes.str() << "\"" << endl;
  
  // 4b. Show the list of words that will be found by the disjunction
  // computednumDistinctWordsInSelectedClusters
  // in step 4a, as described above.
  // {
  //   ostringstream os;
  //   os << "! FUZZY: this effectively does \"" << queryWord;
  //   for (hash_set<string, StringHashFunction>::iterator
  //        it = wordsThatWillBeFound.begin(); it != wordsThatWillBeFound.end();
  //        ++it) os << "|" << (*it);
  //   os << "\"";
  //   log << os.str() << endl;
  // }

  // 4c. Now replace the last part of the query by this disjunction.
  if (fuzzySearchUseClustering)
    newLastPartOfQuery.setQueryString(orOfClusterIdPrefixes.str());
  else
    newLastPartOfQuery.setQueryString(orOfClosestWords);

  // log << "! FUZZY: replacing \"" << lastPartOfQuery.getQueryString() << "\""
  //    << " by \"" << newLastPartOfQuery.getQueryString() << "\"" << endl;

  // 4d. Also replace this in _queryRewrittenForHighlighting.
  fuzzySearchAuxTimer.cont();
  {
    string tmp = _queryRewrittenForHighlighting.getQueryString();
    size_t pos =tmp.find(lastPartOfQuery.getQueryString());
    if (pos != string::npos)
    {
      tmp.replace(pos, lastPartOfQuery.length(),
          newLastPartOfQuery.getQueryString());
    }
    _queryRewrittenForHighlighting.setQueryString(tmp);
  }
  fuzzySearchAuxTimer.stop();

  // 5. Finally process the query (reading, merging, intersecting etc.)
  // 5a. Read the inverted lists (query results) for the first part of the query
  QueryResult fullResult(true);
  Separator fullSeparator = Separator("",pair<signed int, signed int>(-1,-1),FULL);
  Query emptyQuery;
  size_t totalVolume = 0;
  size_t shortListVolume = 0;
  vector<QueryResult> separateResults(separateQueriesLastPart.size());
  vector<int> listSizes;
  fuzzySearchFetchListsTimer.cont();

  // Filtering clusters that contain too much non-relevant
  // hits. Still experimental.
  vector<QueryResult> separateResultsUnfiltered;
  std::unordered_set<int> Sq;
  if (filterClusters)
  {
    separateResultsUnfiltered.resize(separateQueriesLastPart.size());
    computeWordIds(closestWordIds, "", &Sq);
  }

  // Sort queries in lexicographic order for faster disk access. Not sure how
  // faster though, if at all.
  // sort(separateQueriesLastPart.begin(), separateQueriesLastPart.end(), orderQuery);
  size_t volumeReadSoFar = 0;
  for (size_t i = 0; i < separateResults.size(); ++i)
  {
    bool blockCleaned = false;
    Timer tmpTimer3;
    tmpTimer3.start();
    if (separateQueriesLastPart[i].getQueryString() == "*")
      CS_THROW(Exception::SINGLE_STAR_NOT_ALLOWED, "");
    if (filterClusters && ((qualityOfClustersAll[i] < minClusterPrec)))
      // || (qualityOfClustersAll[i] > 1.0 && qualityOfClustersAll[i] < 1.9)))
    {
      processBasicQuery(fullResult,
                        emptyQuery,
                        separateQueriesLastPart[i],
                        fullSeparator,
                        separateResultsUnfiltered[i]);
      fuzzySearchFilterList(separateResultsUnfiltered[i], Sq, &separateResults[i]);
      blockCleaned = true;
    }
    else
    {
      processBasicQuery(fullResult,
                        emptyQuery,
                        separateQueriesLastPart[i],
                        fullSeparator,
                        separateResults[i]);
    }
    totalVolume += separateResults[i]._docIds.size();
    // usleep(1000);
    tmpTimer3.stop();
    if (showFuzzyDebugOutput)
    {
      log << "! FUZZY: \""
          << separateQueriesLastPart[i] << "\" read";
      if (blockCleaned)
        log << " and cleaned";
      log << " (took " << tmpTimer3 << ", list size: "
          << separateResults[i]._docIds.size() << " items / "
          << commaStr(volumeReadFromFile - volumeReadSoFar) << " B)" << endl;
      volumeReadSoFar = volumeReadFromFile;
    }
    listSizes.push_back(separateResults[i]._docIds.size());
  }
  fuzzySearchFetchListsTimer.stop();
  totalVolumeProcessed += volumeReadFromFile;

  if (showFuzzyDebugOutput)
  {
    log << "! FUZZY: Number of lists                : " << separateResults.size() << endl;
    log << "! FUZZY: Total time to read             : " << fuzzySearchFetchListsTimer << endl;
    log << "! FUZZY: Total time spent on disk seeks : " << _indexStructureFile.diskSeekTimer << endl;
    log << "! FUZZY: Total volume read (B)          : " << commaStr(volumeReadFromFile) << endl;
    log << "! FUZZY: Total volume to process        : " << commaStr(totalVolume) << endl;
    log << "! FUZZY: inputList size                 : " << commaStr(inputList._docIds.size()) << endl;
    sort(listSizes.begin(), listSizes.end(), decreasingOrder);
    log << "! FUZZY: ";
    for (size_t j = 0; j < listSizes.size(); j++)
      log << listSizes[j] << "(" << 1.0 * listSizes[j] / totalVolume * 100 << "%) ";
    log << endl;
  }

  // 5.b. Pre-merging: merge short lists in the disjunction separately by using
  // group-merge
  fuzzySearchProcessQueryTimer.cont();
  size_t mergeCount = 0;
  if (usePremerging)
  {
    vector<const QueryResult*> tempSeparateResults;
    vector<bool> shortList;
    shortList.resize(separateResults.size());
    QueryResult tempMergeResult;
    for (size_t j = 0; j < separateResults.size(); j++)
    {
      // if (100.0 * separateResults[j]._docIds.size() / totalVolume <= fraction)
      if (separateResults[j]._docIds.size() <= 100)
      {
        tempSeparateResults.push_back(&separateResults[j]);
        shortList[j] = true;
        mergeCount++;
        shortListVolume += separateResults[j]._docIds.size();
      }
    }
    if (mergeCount >= 2)
    {
      fuzzySearchMergeTimer.cont();
      fuzzySearchPreMergeTimer.cont();
      Timer preMergeTime;
      preMergeTime.start();
      QueryResult::mergeResultLists(tempSeparateResults, &tempMergeResult);
      // QueryResult::groupMerge(tempSeparateResults, 1.5, &tempMergeResult);
      for (size_t k = 0; k < separateResults.size(); k++)
        if (shortList[k])
          separateResults[k].clear();
      separateResults.push_back(tempMergeResult);
      Query tmpQuery;
      tmpQuery.setQueryString("*groupMerge*");
      separateQueriesLastPart.push_back(tmpQuery);
      fuzzySearchPreMergeTimer.stop();
      // totalPremergingTime += preMergeTimer.usecs();
      preMergeTime.stop();
      fuzzySearchMergeTimer.stop();
      if (showFuzzyDebugOutput)
      {
        log << "! FUZZY: Short list volume to process: "
            << commaStr(shortListVolume) << endl;
        log << "! FUZZY: " << mergeCount << " short lists have been "
            "pre-merged in " << preMergeTime << ". Number of lists is now: "
            << separateResults.size() - mergeCount << endl;
      }
    }
    else
      mergeCount = 0;
  }
  fuzzySearchProcessQueryTimer.stop();

  // 5.c. decide which query processing strategy to use (Variant 1 or Variant 2)
  bool mergeAfterIntersect = false;
  //  if ((inputList._docIds.size() > 0) &&
  //     (separateResults.size() - mergeCount <= nofLists) &&
  //     (1.0 * inputList._docIds.size() / totalVolume <= factor))
  //    mergeAfterIntersect = true;
  int n_l = separateResults.size() - mergeCount + 1;
  if ((inputList._docIds.size() > 0) &&
     (n_l <= 1 + (1.0 * totalVolume / (inputList._docIds.size()) * 1.5 * std::log(1.0 * n_l))))
    mergeAfterIntersect = true;

  // Process the query using Variant 2: first full merge, then intersect
  if (!mergeAfterIntersect)
  {
    if (showFuzzyDebugOutput)
      log << "! FUZZY: Using Variant 2 / UI of query processing" << endl;
    fuzzySearchProcessQueryTimer.cont();
    fuzzySearchMergeTimer.cont();
    QueryResult mergeResult;
    if (useOptimalBinaryMerge)
    {
      if (!firstPartOfQuery.getQueryString().empty())
        mergeOpt(separateResults, 2, &mergeResult);
      else
        mergeOpt(separateResults, 2, &result);
    }
    else
    {
      if (!firstPartOfQuery.getQueryString().empty())
        QueryResult::mergeResultLists(separateResults, &mergeResult);
      else
        QueryResult::mergeResultLists(separateResults, &result);
    }
    fuzzySearchMergeTimer.stop();
    Timer timerInt;
    timerInt.start();
    if (!firstPartOfQuery.getQueryString().empty())
    {
      intersectionTimer.cont();
      intersectLists(inputList, mergeResult, &result);
      intersectionTimer.stop();
      // intersectTwoPostingLists(inputList, mergeResult, result);
      if (showFuzzyDebugOutput)
        log << "! FUZZY: intersecting 2 lists with sizes "
            << inputList._docIds.size() << " and "
            << mergeResult._docIds.size() << endl;
    }
    timerInt.stop();
    fuzzySearchProcessQueryTimer.stop();
    if (showFuzzyDebugOutput)
    {
      log << "! FUZZY: intersection took: " << timerInt << endl;
      log << "! FUZZY: merged results from separate queries (took " << fuzzySearchMergeTimer << ")" << endl;
      log << "! FUZZY: processing query with new merge gave result list of size "
          << commaStr(result._docIds.size()) << endl;
    }
  }

  // Process the query using Variant 1: first intersect, then merge
  if (mergeAfterIntersect)
  {
    if (showFuzzyDebugOutput)
      log << "! FUZZY: Using Variant 1 / IU for query processing" << endl;
    vector<QueryResult> separateResults1;
    Timer tmpTimer2;
    tmpTimer2.start();
    fuzzySearchProcessQueryTimer.cont();
    // fuzzySearchAuxTimer.cont();
    Separator fullSeparator = Separator("",pair<signed int, signed int>(-1,-1),FULL);
    QueryResult fullResult(true);
    Query emptyQuery;
    for (size_t i = 0; i < separateResults.size(); ++i)
    {
      Timer tmpTimer3;
      tmpTimer3.start();
      if (!firstPartOfQuery.getQueryString().empty())
      {
        if (separateResults[i]._docIds.size() > 0)
        {
          intersectionTimer.cont();
          separateResults1.resize(separateResults1.size() + 1);
          intersectLists(inputList, separateResults[i], &separateResults1[separateResults1.size() - 1]);
          // intersectTwoPostingLists(inputList, separateResults[i], separateResults1[separateResults1.size() - 1]);
          intersectionTimer.stop();
        }
        tmpTimer3.stop();
        if (showFuzzyDebugOutput)
        {
          fuzzySearchProcessQueryTimer.stop();
          if (separateResults[i]._docIds.size() > 0 && showFuzzyDebugOutput)
          {
            log << "! FUZZY: launched separate query \""
                << firstPartOfQuery << separator._separatorString
                << separateQueriesLastPart[i] << "\", lengths: "
                << inputList._docIds.size() << ", "
                << separateResults[i]._docIds.size() << ", "
                << (separateResults1.size() == 0 ? 0 : separateResults1.back()._docIds.size())
                << " (took " << tmpTimer3 << ")" << endl;
            fuzzySearchProcessQueryTimer.cont();
          }
        }
      }
    }
    // fuzzySearchAuxTimer.stop();
    fuzzySearchMergeTimer.cont();
    if (useOptimalBinaryMerge)
    {
      if (!firstPartOfQuery.getQueryString().empty())
      {
        mergeOpt(separateResults1, 2, &result);
      }
      else
        mergeOpt(separateResults, 2, &result);
    }
    else
    {
      if (!firstPartOfQuery.getQueryString().empty())
        QueryResult::mergeResultLists(separateResults1, &result);
      else
        QueryResult::mergeResultLists(separateResults, &result);
    }
    fuzzySearchMergeTimer.stop();
    fuzzySearchProcessQueryTimer.stop();
    tmpTimer2.stop();
    if (showFuzzyDebugOutput)
    {
      log << "! FUZZY: merged results from separate queries (took " << fuzzySearchMergeTimer << ")" << endl;
      log << "! FUZZY: processing query with new merge took " << fuzzySearchProcessQueryTimer << endl;
      log << "! FUZZY: processing query with new merge gave result list of size "
          << commaStr(result._docIds.size()) << endl;
    }
  }

  // additional things of the result object that have to be set
  result._prefixCompleted = queryWord;

  // global counters for fuzzy search
  fuzzySearchCoverIndex                         = separateResults.size() - mergeCount;
  fuzzySearchNumRelevantWordsSelected           = (numRelevantWordsSelected /*- mergeCount*/);
  fuzzySearchNumRelevantWords                   = (numRelevantWords /*- mergeCount*/);
  fuzzySearchNumDistinctWordsInSelectedClusters =
      numDistinctWordsInSelectedClusters;

  fuzzySearchComputeQuerySuggestionsTimer.start();
  if (computeQuerySuggestions)
    findFuzzySearchQuerySuggestions(
        _query,
        10,
        true,
        &result._topFuzzySearchQuerySuggestions,
        &result._topFuzzySearchQuerySuggestionScores);
  fuzzySearchComputeQuerySuggestionsTimer.stop();
  fuzzySearchTotalTimer.stop();

  if (showFuzzyDebugOutput)
  {
    log << "Computed query suggestions   : " << result._topFuzzySearchQuerySuggestions.size() << endl;
    log << "Total fuzzy search time      : " << fuzzySearchTotalTimer << endl;
    log << "Total query processing time  : " << fuzzySearchProcessQueryTimer << " / ";
    log << 100 * fuzzySearchProcessQueryTimer.usecs() / fuzzySearchTotalTimer.usecs() << "%" << endl;
    log << "Total disk time and decompr. : " << fuzzySearchFetchListsTimer << " / ";
    log << 100 * fuzzySearchFetchListsTimer.usecs() / fuzzySearchTotalTimer.usecs() << "%" << endl;
    log << "Total cluster selection time : " << fuzzySearchDistanceComputationsTimer << " / ";
    log << 100 * fuzzySearchDistanceComputationsTimer.usecs() / fuzzySearchTotalTimer.usecs() << "%" << endl;
    log << "Total ranking time           : " << fuzzySearchComputeQuerySuggestionsTimer << " / ";
    log << 100 * fuzzySearchComputeQuerySuggestionsTimer.usecs() / fuzzySearchTotalTimer.usecs() << "%" << endl;
    log << endl;
  }
}

class CompareLists
{
  public:
    bool operator()(const QueryResult* x, const QueryResult* y) const
    {
      return x->_docIds.size() > y->_docIds.size();
    }
};

inline bool getNext2e(const QueryResult& list1, const QueryResult& list2, int i, int j)
{
  if (list1._docIds[i] < list2._docIds[j])
    return true;
  else
  {
    return (list1._docIds[i] == list2._docIds[j] &&
        list1._positions[i] <= list2._positions[j]);
  }
}

inline bool getNext2n(const QueryResult& list1, const QueryResult& list2, int i, int j)
{
  if (list1._docIds[i] < list2._docIds[j])
    return true;
  else
  {
    return (list1._docIds[i] == list2._docIds[j] &&
        list1._positions[i] < list2._positions[j]);
  }
}

// _____________________________________________________________________________
void mergeLists(const vector<const QueryResult*>& lists,
                QueryResult* resultList)
{
  CS_ASSERT(lists.size() > 1);
  if (lists.size() >= 3)
    QueryResult::mergeResultLists(lists, 0, lists.size(), resultList);
  else
  if (lists.size() == 2)
    QueryResult::mergeResultLists(*lists[0], *lists[1], *resultList);
}

// _____________________________________________________________________________
void mergeOpt(const vector<QueryResult>& lists,
              const unsigned int k,
              QueryResult* resultList)
{
  CS_ASSERT(resultList != NULL);
  if (lists.size() == 0)
  {
    return;
  }
  if (lists.size() == 1)
  {
    *resultList = lists[0];
    return;
  }
  std::priority_queue<const QueryResult*, vector<const QueryResult*>, CompareLists > pq;
  vector<vector<int>* > listPointers;
  listPointers.resize(lists.size());
  for (size_t i = 0; i < lists.size(); i++)
    pq.push(&lists[i]);
  int nofLists = lists.size();
  vector<const QueryResult*> listsToBeMerged;
  vector<QueryResult*> tempLists;
  QueryResult* tempList;
  while (nofLists > 1)
  {
    listsToBeMerged.clear();
    for (size_t i = 0; i < k && pq.size() > 0; i++)
    {
      listsToBeMerged.push_back(pq.top());
      pq.pop();
    }
    nofLists = nofLists - listsToBeMerged.size() + 1;
    if (nofLists == 1)
      mergeLists(listsToBeMerged, resultList);
    else
    {
      tempList = new QueryResult();
      tempLists.push_back(tempList);
      mergeLists(listsToBeMerged, tempList);
      pq.push(tempList);
    }
  }
  for (size_t i = 0; i < tempLists.size(); i++)
    delete tempLists[i];
}

// _____________________________________________________________________________
void mergeTwoLists(QueryResult& list1,
                   QueryResult& list2,
                   QueryResult* resultList)
{
  (*resultList).clear();
  if (list1._docIds.empty())
  {
    (*resultList) = list2;
    return;
  }
  if (list2._docIds.empty())
  {
    (*resultList) = list1;
    return;
  }
  size_t n = list1._docIds.size() + list2._docIds.size() + 2;
  (*resultList)._docIds.resize(n);
  (*resultList)._wordIdsOriginal.resize(n);
  (*resultList)._positions.resize(n);
  (*resultList)._scores.resize(n);

  int max = MY_MAX(list1._docIds.back(), list2._docIds.back());

  list1._docIds.push_back(max + 1);
  list1._wordIdsOriginal.push_back(1);
  list1._positions.push_back(1);
  list1._scores.push_back(1);
  list1._docIds.push_back(max + 2);
  list1._wordIdsOriginal.push_back(2);
  list1._positions.push_back(2);
  list1._scores.push_back(2);

  list2._docIds.push_back(max + 1);
  list2._wordIdsOriginal.push_back(1);
  list2._positions.push_back(1);
  list2._scores.push_back(1);
  list2._docIds.push_back(max + 2);
  list2._wordIdsOriginal.push_back(2);
  list2._positions.push_back(2);
  list2._scores.push_back(2);

  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int k = 0;
  while (k < n)
  {
    while (getNext2e(list1, list2, i, j))
    {
      (*resultList)._docIds[k] = list1._docIds[i];
      (*resultList)._wordIdsOriginal[k] = list1._wordIdsOriginal[i];
      (*resultList)._positions[k] = list1._positions[i];
      (*resultList)._scores[k] = list1._scores[i];
      k++;
      i++;
    }
    while (getNext2n(list2, list1, j, i))
    {
      (*resultList)._docIds[k] = list2._docIds[j];
      (*resultList)._wordIdsOriginal[k] = list2._wordIdsOriginal[j];
      (*resultList)._positions[k] = list2._positions[j];
      (*resultList)._scores[k] = list2._scores[j];
      j++;
      k++;
    }
  }
  list1._docIds.resize(list1._docIds.size() - 2);
  list1._wordIdsOriginal.resize(list1._wordIdsOriginal.size() - 2);
  list1._positions.resize(list1._positions.size() - 2);
  list1._scores.resize(list1._scores.size() - 2);
  list2._docIds.resize(list2._docIds.size() - 2);
  list2._wordIdsOriginal.resize(list2._wordIdsOriginal.size() - 2);
  list2._positions.resize(list2._positions.size() - 2);
  list2._scores.resize(list2._scores.size() - 2);
  resultList->_docIds.resize(resultList->_docIds.size() - 2);
  resultList->_wordIdsOriginal.resize(resultList->_wordIdsOriginal.size() - 2);
  resultList->_positions.resize(resultList->_positions.size() - 2);
  resultList->_scores.resize(resultList->_scores.size() - 2);
}

// _____________________________________________________________________________
void intersectLists(const QueryResult& list1,
                    const QueryResult& list2,
                    QueryResult* resultList)
{
  resultList->_docIds.clear();
  resultList->_wordIdsOriginal.clear();
  resultList->_wordIdsMapped.clear();
  resultList->_positions.clear();
  resultList->_scores.clear();
  if (list1._docIds.empty() || list2._docIds.empty())
    return;
  size_t res = (list1._docIds.size() < list2._docIds.size()
      ? list1._docIds.size() : list2._docIds.size());
  resultList->_docIds.reserve(res);
  resultList->_wordIdsOriginal.reserve(res);
  resultList->_positions.reserve(res);
  resultList->_scores.reserve(res);
  size_t i = 0;
  size_t j = 0;
  while (i < list1._docIds.size() && j < list2._docIds.size())
  {
    if (list1._docIds[i] == list2._docIds[j])
    {
      resultList->_docIds.push_back(list2._docIds[j]);
      resultList->_wordIdsOriginal.push_back(list2._wordIdsOriginal[j]);
      resultList->_positions.push_back(list2._positions[j]);
      resultList->_scores.push_back(list2._scores[j]);
      ++i;
      ++j;
    }
    else
      if (list1._docIds[i] < list2._docIds[j])
        i++;
      else
        j++;
  }
}

// _____________________________________________________________________________
void selectClusters(
    const vector<vector<int> >& wordIdsPerCluster,
    const vector<vector<int> >& clusterIdsPerClusterCenter,
    const vector<int>& clusterId2ClusterIndex,
    vector<ClusterOfSimilarWords>& clusters,
    std::unordered_set<int>& allRelevantWords,
    int heuristic,
    vector<bool>* isClusterSelected,
    vector<double>* qualityOfClusters,
    size_t *numRelevantWordsSelected,
    size_t* numDistinctWordsInSelectedClusters)
{
  CS_ASSERT(isClusterSelected != NULL);
  CS_ASSERT_EQ(0, isClusterSelected->size());
  CS_ASSERT(numRelevantWordsSelected != NULL);
  CS_ASSERT(numDistinctWordsInSelectedClusters != NULL);
  // isClusterSelected->resize(clusters.size());
  // Greedily pick the next best cluster until all relevant words are covered.
  // The next best cluster is the one which covers the largest number of
  // remaining relevant words (called "good" words in the variable names below,
  // for brevity), prefering smaller clusters in the case of ties.
  // TODO(bast): Can't we deal with word ids here instead of with strings? Hmm,
  // maybe not, because different words ids mean the same word (C:1223:ballerina
  // and C:873:ballerina). Marjan: Yes, we can if we take the word-ids from fuzzysearcher instead of
  // the completer!
  std::unordered_set<int>& goodWords = allRelevantWords;
  std::unordered_set<int> distinctWordsInSelectedClusters;
  numClustersSelected = 0;
  *numRelevantWordsSelected = goodWords.size();
  isClusterSelected->resize(clusters.size());
  qualityOfClusters->resize(clusters.size());
  double threshold = 1 - recall;
  while (goodWords.size() > 0
      && 1.0 * goodWords.size() / (*numRelevantWordsSelected) > threshold)
  {
    double maxNumGoodWordsInCluster = -1;
    int minNumAllWordsInSelectedCluster = INT_MAX;
    int bestClusterIndex = -1;
    int numRelWordsInBestCluster = -1;
    int maxNumRelWordsInBestCluster = -1;
    double precision = -1;
    for (size_t i = 0; i < MIN(clusters.size(), INT_MAX); ++i)
    {
      if ((*isClusterSelected)[i])
        continue;
      double numGoodWordsInCluster = 0;
      numGoodWordsInCluster = clusters[i].nofRelevantWords;
      const vector<int>& words = wordIdsPerCluster[clusters[i].clusterId];
      bool cond;
      if (heuristic == 2 || heuristic == 4)
      {
        numRelWordsInBestCluster = numGoodWordsInCluster;
        numGoodWordsInCluster /= words.size();
        cond = (numGoodWordsInCluster > maxNumGoodWordsInCluster ||
          (numGoodWordsInCluster == maxNumGoodWordsInCluster &&
           words.size() > (size_t)minNumAllWordsInSelectedCluster));
        if (cond)
        {
          maxNumGoodWordsInCluster = numGoodWordsInCluster;
          bestClusterIndex = i;
          minNumAllWordsInSelectedCluster = words.size();
          maxNumRelWordsInBestCluster = numRelWordsInBestCluster;
        }
      }
      else
      if (heuristic == 3)
      {
        double currentPrecision = 1.0 * clusters[i].fractionRelevant;
        if (numGoodWordsInCluster > 0)
        {
          cond = (currentPrecision > precision ||
                 (currentPrecision == precision &&
                  numGoodWordsInCluster > maxNumGoodWordsInCluster) ||
                 (precision == currentPrecision &&
                  numGoodWordsInCluster == maxNumGoodWordsInCluster &&
                  words.size() < (size_t)minNumAllWordsInSelectedCluster));
        }
        else
          cond = false;
        if (cond)
        {
          maxNumGoodWordsInCluster = numGoodWordsInCluster;
          bestClusterIndex = i;
          minNumAllWordsInSelectedCluster = words.size();
          maxNumRelWordsInBestCluster = maxNumGoodWordsInCluster;
          precision = currentPrecision;
        }
      }
      else
      {
        if (true)
        {
          cond = (numGoodWordsInCluster > maxNumGoodWordsInCluster ||
              (numGoodWordsInCluster == maxNumGoodWordsInCluster &&
               words.size() < (size_t)minNumAllWordsInSelectedCluster));
        }
        else
          cond = false;
        if (cond)
        {
          maxNumGoodWordsInCluster = numGoodWordsInCluster;
          bestClusterIndex = i;
          minNumAllWordsInSelectedCluster = words.size();
          maxNumRelWordsInBestCluster = maxNumGoodWordsInCluster;
        }
      }
    }
    if (bestClusterIndex == -1)
    {
      if (showFuzzyDebugOutput)
        cout << "No more relevant clusters!" << endl;
      break;
    }

    if (heuristic == 4)
      if (1.0 * maxNumRelWordsInBestCluster / clusters[bestClusterIndex].size < 0.8
          || (maxNumRelWordsInBestCluster <= 1 && clusters[bestClusterIndex].size > 1))  // NOLINT
      {
        threshold = 1;
        break;
      }

    if (heuristic == 1)
      if (1.0 * maxNumRelWordsInBestCluster / clusters[bestClusterIndex].size < 0.1
          || (maxNumRelWordsInBestCluster <= 1 && clusters[bestClusterIndex].size > 1))  // NOLINT
      {
        threshold = 1;
        break;
      }

    if (heuristic == 2)
      if (maxNumRelWordsInBestCluster < 1 ||
          (clusters[bestClusterIndex].fractionRelevant < 1.0)
              || (maxNumRelWordsInBestCluster == 1 && clusters[bestClusterIndex].size > 1))
      {
        threshold = 1;
        break;
      }

    if (showFuzzyDebugOutput)
      cout << "Next best cluster id: C:"
           << clusters[bestClusterIndex].clusterId
           << ", new words covered: "
           << maxNumRelWordsInBestCluster
           << ", relevant words: "
           << clusters[bestClusterIndex].nofRelevantWords
           << " / " << 1.0 * maxNumRelWordsInBestCluster / clusters[bestClusterIndex].size  // NOLINT
           << ", size: " << minNumAllWordsInSelectedCluster << endl;

    // here the code to change the num. of relevant words in each affected
    // cluster from the words in the best chosen cluster
    const vector<int>& wordsInBestCluster = wordIdsPerCluster[clusters[bestClusterIndex].clusterId];  // NOLINT
    for (size_t j = 0; j < wordsInBestCluster.size(); j++)
    {
      if (goodWords.find(wordsInBestCluster[j]) == goodWords.end())
        continue;
      const vector<int>& clusterIdsOfThisWord =
         clusterIdsPerClusterCenter[wordsInBestCluster[j]];
      for (size_t k = 0; k < clusterIdsOfThisWord.size(); k++)
      {
        CS_ASSERT_LT(clusterIdsOfThisWord[k], (int)clusterId2ClusterIndex.size());  // NOLINT
        clusters[clusterId2ClusterIndex[clusterIdsOfThisWord[k]]].nofRelevantWords--;  // NOLINT
      }
    }

    (*isClusterSelected)[bestClusterIndex] = true;
    (*qualityOfClusters)[bestClusterIndex] = 1.0 * maxNumRelWordsInBestCluster / clusters[bestClusterIndex].size;
    ++numClustersSelected;
    const vector<int>& words = wordIdsPerCluster[clusters[bestClusterIndex].clusterId];  // NOLINT
    for(unsigned int i = 0; i < words.size(); i++)
    {
      goodWords.erase(words[i]);
      distinctWordsInSelectedClusters.insert(words[i]);
    }
  }
  *numDistinctWordsInSelectedClusters = distinctWordsInSelectedClusters.size();
  *numRelevantWordsSelected -= goodWords.size();
  if (showFuzzyDebugOutput)
    cout << "-----------------------------------------------------------------"
        "----------------------" << endl;
}

// _____________________________________________________________________________
string ClusterOfSimilarWords::DebugString()
{
  ostringstream osClusterPrefix;
  osClusterPrefix << "C" << wordPartSep << clusterId << wordPartSep << "*";
  string clusterPrefix = osClusterPrefix.str();
  ostringstream os;
  os.setf(ios::fixed);
  os.precision(2);
  os << clusterPrefix << " has " << similarWords.size() 
     << " completions, " << nofRelevantWords << " relevant : ";
  CS_ASSERT_EQ(similarWords.size(), distances.size());
  size_t numWordsWithEditDistanceOk = 0;
  for (size_t i = 0; i < similarWords.size(); ++i)
  {
    bool isEditDistanceOk = distances[i] <= fuzzySearchEditDistanceThreshold;
    os << similarWords[i] << " (" << distances[i]
       << (isEditDistanceOk ? "Y" : "N") << ")"
       << (i + 1 < similarWords.size() ? ", " : "");
    if (isEditDistanceOk) ++numWordsWithEditDistanceOk;
  }
  return os.str();
}



// EXPLICIT INSTANTIATION (so that actual code gets generated).
template void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>
                ::processFuzzySearchQuery
                  (const QueryResult& inputList, 
                   const Query&       firstPartOfQuery, 
                   const Query&       lastPartOfQuery,
                   const Separator&   separator,
                         QueryResult& result);
