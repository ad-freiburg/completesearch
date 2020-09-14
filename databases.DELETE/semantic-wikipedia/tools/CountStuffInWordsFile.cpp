// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>


// IMPORTANT NOTE: THIS IS PLAYGROUND CODE
// REALLY POOR QUALITY. DONE QUICK N DIRTY

#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include "../utils/File.h"
#include "../utils/HashMap.h"
#include "../utils/HashSet.h"

using std::string;
using std::vector;
using std::ostringstream;

static const int PREFIX_SIZE = 4;

int main(int argc, char** argv)
{
  // global counters
  size_t totalPostings = 0;
  size_t entities = 0;
  size_t entityCoOccurrences = 0;
  size_t contexts = 0;
  char buf[2048];
  semsearch::HashMap<string, size_t> prefixNofWordsMap;
  semsearch::HashMap<string, size_t> prefixNofEntitiesMap;
  semsearch::HashMap<string, size_t> prefixNofPostingsMap;

  // Local data
  semsearch::HashMap<string, semsearch::HashSet<string> > prefixEntitiesMap;
  semsearch::HashSet<string> localEntities;
  vector<string> localWords;
  ostringstream os;

  semsearch::File wordsFile(argv[1], "r");
  string line;
  size_t lastContext = 0;

  while (wordsFile.readLine(&line, buf, static_cast<size_t> (2048)))
  {
    size_t posOfFirstTab = line.find('\t');
    assert(posOfFirstTab != string::npos);
    size_t posOfSecondTab = line.find('\t', posOfFirstTab + 1);
    assert(posOfSecondTab != string::npos);
    string word = line.substr(0, posOfFirstTab);
    while (word.size() < PREFIX_SIZE)
    {
      word += "_";
    }
    size_t context =
        atol(
            line.substr(posOfFirstTab + 1,
                posOfSecondTab - (posOfFirstTab + 1)).c_str());
    if (context != lastContext)
    {
      contexts++;
      // Finish this context
      entityCoOccurrences += ((localEntities.size() * localEntities.size())
          - localEntities.size());

      for (size_t i = 0; i < localWords.size(); ++i)
      {
        for (semsearch::HashSet<string>::iterator it = localEntities.begin(); it
            != localEntities.end(); ++it)
        {
          assert(localWords[i].size() >= PREFIX_SIZE);
          string prefix = localWords[i].substr(0, PREFIX_SIZE);
          if (prefixEntitiesMap[prefix].count(*it) == 0)
          {
            prefixEntitiesMap[prefix].insert(*it);
            ++prefixNofEntitiesMap[prefix];
            ++prefixNofPostingsMap[prefix];
            ++totalPostings;
          }
        }
      }

      // Clear local stuff
      lastContext = context;
      localEntities.clear();
      prefixEntitiesMap.clear();
      localWords.clear();
    }

    // Process this line
    ++totalPostings;
    if (word.substr(0, 3) == ":ee")
    {
      localEntities.insert(word);
      ++entities;
      ++prefixNofEntitiesMap[word];
    }
    else
    {
      localWords.push_back(word);
      ++prefixNofWordsMap[word.substr(0, PREFIX_SIZE)];
      ++prefixNofPostingsMap[word.substr(0, PREFIX_SIZE)];
    }

    // Stop when the translation words start
    if (line.substr(0, 3) == ":t:") break;
  }

  string largestSingleList;
  size_t largestSingleListSize = 0;
  string largestBlock;
  size_t largestBlockSize = 0;

  // Determine the max list size.
  // Of the single lists.
  typedef semsearch::HashMap<string, size_t>::const_iterator MapIter;
  for (MapIter it = prefixNofEntitiesMap.begin(); it
      != prefixNofEntitiesMap.end(); ++it)
  {
    if (it->second > largestSingleListSize)
    {
      largestSingleList = it->first + "*-entities";
      largestSingleListSize = it->second;
    }
  }

  for (MapIter it = prefixNofWordsMap.begin();
      it != prefixNofWordsMap.end(); ++it)
  {
    if (it->second > largestSingleListSize)
    {
      largestSingleList = it->first + "*-words";
      largestSingleListSize = it->second;
    }
  }

  // Of the whole blocks.
  for (MapIter it = prefixNofPostingsMap.begin(); it
      != prefixNofPostingsMap.end(); ++it)
  {
    if (it->second > largestBlockSize)
    {
      largestBlock = it->first + "*";
      largestBlockSize = it->second;
    }
  }

  printf("Contexts: %lu\n", contexts);
  printf(
      "Total expected lines in the new index (without ontology part): %lu\n",
      totalPostings);
  printf("Entity occurrences: %lu\n", entities);
  printf("Entity co-occurrences: %lu\n", entityCoOccurrences);
  printf("Largest single block-list: %s\t size: %lu postings\n",
      largestSingleList.c_str(), largestSingleListSize);
  printf("Largest combined block: %s\t size: %lu postings\n",
      largestBlock.c_str(), largestBlockSize);
}
