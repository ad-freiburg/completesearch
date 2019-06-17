// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>

#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./Index.h"
#include "./Engine.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
  {"ontology-base", required_argument, NULL, 'o'},
  {"fulltext-base", required_argument, NULL, 't'},
  {NULL, 0, NULL, 0}
};

namespace ad_semsearch
{
class IndexPerformanceTest
{
  public:
    void doTest(Index & index)
    {
      ad_semsearch::Engine engine;
      cout << endl << endl;
      {
        LOG(INFO)
        << "--- RELATIVITY PERSON ---" << endl;
        LOG(INFO)
        << "Attempting to get the PostingList for \"relativity\"." << endl;

        LOG(INFO)
        << "Get the wordId for \"relativity\":" << endl;
        ad_semsearch::Id wordIdRelativity;
        index._fulltextVocabularies[0].getIdForFullTextWord("relativity",
            &wordIdRelativity);
        LOG(INFO) << "Id: " << wordIdRelativity << endl;

        LOG(INFO)
        << "Get the correct block Info for that Id:" << endl;
        const ad_semsearch::BlockMetaData& relBlockMeta =
            index._fulltextMetaData[0].getBlockInfoByWordRange(
                wordIdRelativity, wordIdRelativity);
        LOG(INFO)
        << "The block has - size: " << relBlockMeta._nofPostings
            << " and max word: "
            << index._fulltextVocabularies[0][relBlockMeta._maxWordId] << endl;

        LOG(INFO)
        << "Read the block from disk" << endl;
        ad_semsearch::PostingList postingList;
        index.readBlock(index._fullTextIndexes[0], relBlockMeta, &postingList);
        LOG(INFO)
        << "Result: " << postingList.asString() << endl;

        LOG(INFO)
        << "Filter by WordId-(Range):" << endl;
        PostingList relativityList;
        engine.filterPostingListBySingleWordId(postingList, wordIdRelativity,
            &relativityList);
        LOG(INFO)
        << "Result: " << relativityList.asString() << endl;

        LOG(INFO)
        << "Aggregating the entities from the relativity list" << endl;
        EntityList relativityEntities;
        engine.aggregatePostingList(relativityList, &relativityEntities);
        LOG(INFO)
        << "Result: " << relativityEntities.asString() << endl;

        LOG(INFO)
        << "Attempting to get a list of persons." << endl;

        LOG(INFO)
        << "Get the word Id for the reversed is-a list." << endl;
        Id isARelId;
        index._ontologyVocabulary.getIdForOntologyWord(":r:is-a_(reversed)",
            &isARelId);

        LOG(INFO)
        << "Get the metaData for the reversed is-a list." << endl;
        const RelationMetaData& isaMetaData =
            index._ontologyMetaData.getRelationMetaData(isARelId);

        LOG(INFO)
        << "Get the person word Id." << endl;
        Id personWordId;
        index._ontologyVocabulary.getIdForOntologyWord(
            ":e:person:Person", &personWordId);

        LOG(INFO)
        << "Read the person block from the reversed is-a list from disk."
            << endl;
        Relation isaRelation;
        index.readRelationBlock(isaMetaData.getBlockInfo(personWordId),
            &isaRelation);
        LOG(INFO)
        << "Result: " << isaRelation.asString() << endl;

        LOG(INFO)
        << "Get the person EntityList." << endl;
        EntityList personList;
        engine.getRelationRhsBySingleLhs(isaRelation, personWordId,
            &personList);
        LOG(INFO)
        << "Result: " << personList.asString() << endl;

        LOG(INFO)
        << "Intersecting the entity Lists." << endl;
        EntityList subtreeList;
        engine.intersectEntityLists(relativityEntities, personList,
            &subtreeList);
        LOG(INFO)
        << "Result: " << subtreeList.asString() << endl;

        LOG(INFO)
        << "Ranking the entity list, getting the top 10 entities" << endl;
        EntityList resultList;
        engine.getTopKEntities(subtreeList, 10, &resultList);
        LOG(INFO)
        << "Result: " << resultList.asString() << endl;

        LOG(INFO)
        << "Filtering the intersected postings list by "
          "the result entity list to get postings for highlighting" << endl;
        PostingList filtered;
        engine.filterPostingListByEntityListKeepWordPostings(
            relativityList, resultList, &filtered);
        LOG(INFO)
        << "Result: " << filtered.asString() << endl;

        if (resultList.size() > 0)
        {
          LOG(INFO)
              << "Filtering by top 1 entity: " << index.getWordById(
              resultList[0]._id) << " now." << endl;
          PostingList group1Popstings;
          engine.filterPostingsByEntityId(filtered, resultList[0]._id,
              &group1Popstings);
          LOG(INFO)
          << "Result: " << group1Popstings.asString() << endl;

          LOG(INFO)
          << "Get top-3 contexts with highlight positions: " << endl;
          HitList hits;
          engine.getTopKContextsWithHighlights(group1Popstings, 3, &hits);
          LOG(INFO)
          << "Hits: " << hits.asString() << endl;

          LOG(INFO)
          << "Get excerpts from file and process them (highlight)." << endl;
          for (size_t i = 0; i < hits.size(); ++i)
          {
            hits[i].setRawExcerptString(
                index.getRawExcerptForContextId(hits[i].getContextId()));
            LOG(INFO) << hits[i] << endl;
          }
        }
      }
      cout << endl << endl;
      {
        LOG(INFO)
        << "--- PLANT EDIBLE LEAV* ---" << endl;
        LOG(INFO)
        << "Attempting to get the PostingList for \"edible\"." << endl;

        LOG(INFO)
        << "Get the wordId for \"edible\":" << endl;
        ad_semsearch::Id wordIdEdible;
        index._fulltextVocabularies[0].getIdForFullTextWord("edible",
            &wordIdEdible);

        LOG(INFO)
        << "Get the correct block Info for that Id:" << endl;
        const ad_semsearch::BlockMetaData& ediBlockMeta =
            index._fulltextMetaData[0].getBlockInfoByWordRange(wordIdEdible,
                wordIdEdible);
        LOG(INFO)
        << "The block has - size: " << ediBlockMeta._nofPostings
            << " and max word: "
            << index._fulltextVocabularies[0][ediBlockMeta._maxWordId] << endl;

        LOG(INFO)
        << "Read the block from disk" << endl;
        ad_semsearch::PostingList postingListEdi;
        index.readBlock(index._fullTextIndexes[0], ediBlockMeta,
            &postingListEdi);
        LOG(INFO)
        << "Result: " << postingListEdi.asString() << endl;

        LOG(INFO)
        << "Filter by WordId-(Range):" << endl;
        PostingList postingListEdible;
        engine.filterPostingListBySingleWordId(postingListEdi, wordIdEdible,
            &postingListEdible);
        LOG(INFO)
        << "Result: " << postingListEdible.asString() << endl;

        LOG(INFO)
        << "Get the wordRange for prefix \"leav*\":" << endl;
        ad_semsearch::IdRange rangeLeav;
        index._fulltextVocabularies[0].getIdRangeForFullTextPrefix("leav*",
            &rangeLeav);

        LOG(INFO) << "Get the correct block Info for that Id:" << endl;
        const ad_semsearch::BlockMetaData& leaBlockMeta =
            index._fulltextMetaData[0].getBlockInfoByWordRange(rangeLeav._first,
                rangeLeav._last);
        LOG(INFO)
            << "The block has - size: " << leaBlockMeta._nofPostings
            << " and max word: "
            << index._fulltextVocabularies[0][leaBlockMeta._maxWordId] << endl;

        LOG(INFO)
        << "Read the block from disk" << endl;
        ad_semsearch::PostingList postingListLea;
        index.readBlock(index._fullTextIndexes[0], leaBlockMeta,
            &postingListLea);
        LOG(INFO)
        << "Result: " << postingListLea.asString() << endl;

        LOG(INFO) << "Intersecting edible with lea* list" << endl;
        PostingList postingListEdibleLea;
        engine.joinPostingListsOnContextId(postingListEdible, postingListLea,
            &postingListEdibleLea);
        LOG(INFO) << "Result: " << postingListEdibleLea.asString() << endl;

        LOG(INFO)
        << "Aggregating the entities from that posting list" << endl;
        EntityList edibleLeaEntities;
        engine.aggregatePostingList(postingListEdibleLea, &edibleLeaEntities);
        LOG(INFO)
        << "Result: " << edibleLeaEntities.asString() << endl;

        LOG(INFO)
        << "Attempting to get a list of plants." << endl;

        LOG(INFO)
        << "Get the word Id for the reversed is-a list." << endl;
        Id isARelId;
        index._ontologyVocabulary.getIdForOntologyWord(
            ":r:is-a_(reversed)", &isARelId);

        LOG(INFO)
        << "Get the metaData for the reversed is-a list." << endl;
        const RelationMetaData& isaMetaData =
            index._ontologyMetaData.getRelationMetaData(isARelId);

        LOG(INFO)
        << "Get the plant word Id." << endl;
        Id plantWordId;
        index._ontologyVocabulary.getIdForOntologyWord(
            ":e:plant:Plant", &plantWordId);

        LOG(INFO)
        << "Read the plant block from the reversed is-a list from disk."
            << endl;
        Relation isaRelation;
        index.readRelationBlock(isaMetaData.getBlockInfo(plantWordId),
            &isaRelation);
        LOG(INFO)
        << "Result: " << isaRelation.asString() << endl;

        LOG(INFO)
        << "Get the plant EntityList." << endl;
        EntityList plantList;
        engine.getRelationRhsBySingleLhs(isaRelation, plantWordId, &plantList);
        LOG(INFO)
        << "Result: " << plantList.asString() << endl;

        LOG(INFO)
        << "Intersecting the entity Lists." << endl;
        EntityList subtreeList;
        engine.intersectEntityLists(edibleLeaEntities, plantList, &subtreeList);
        LOG(INFO)
        << "Result: " << subtreeList.asString() << endl;

        LOG(INFO)
        << "Ranking the entity list, getting the top 10 entities" << endl;
        EntityList resultList;
        engine.getTopKEntities(subtreeList, 10, &resultList);
        LOG(INFO)
        << "Result: " << resultList.asString() << endl;

        LOG(INFO)
        << "Filtering the intersected postings list by "
          "the result entity list to get postings for highlighting" << endl;
        PostingList filtered;
        engine.filterPostingListByEntityListKeepWordPostings(
            postingListEdibleLea, resultList, &filtered);
        LOG(INFO)
        << "Result: " << filtered.asString() << endl;

        if (resultList.size() > 0)
        {
          LOG(INFO)
          << "Filtering by top 1 entity: "
          << index.getWordById(resultList[0]._id) << " now." << endl;
          PostingList group1Popstings;
          engine.filterPostingsByEntityId(filtered, resultList[0]._id,
              &group1Popstings);
          LOG(INFO)
          << "Result: " << group1Popstings.asString() << endl;

          LOG(INFO)
          << "Get top-3 contexts with highlight positions: " << endl;
          HitList hits;
          engine.getTopKContextsWithHighlights(group1Popstings, 3, &hits);
          LOG(INFO)
          << "Hits: " << hits.asString() << endl;

          LOG(INFO)
          << "Get excerpts from file and process them (highlight)." << endl;
          for (size_t i = 0; i < hits.size(); ++i)
          {
            hits[i].setRawExcerptString(
                index.getRawExcerptForContextId(hits[i].getContextId()));
            LOG(INFO)
            << hits[i] << endl;
          }
        }

        cout << endl << endl;
      }
    }
};
}
// Main function.
int main(int argc, char** argv)
{
    std::cout << std::endl << EMPH_ON << "IndexPerformanceTest, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string ontologyBase = "";
  string fulltextBase = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "o:t:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'o':
      ontologyBase = optarg;
      break;
    case 't':
      fulltextBase = optarg;
      break;
    default:
      cout << endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  // Start the Program
  if (ontologyBase.size() == 0 || fulltextBase.size() == 0)
  {
    std::cerr << "Usage: " << std::endl
        << "IndexPerformanceTest -o <ontologyBasename> -t <fulltextBasenme>"
        << endl;
    exit(1);
  }
  ad_semsearch::Index index;
  index.registerOntologyIndex(ontologyBase);
  index.registerFulltextIndex(fulltextBase, true);
  ad_semsearch::IndexPerformanceTest ipt;
  ipt.doTest(index);

  return 0;
}
