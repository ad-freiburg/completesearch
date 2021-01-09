#include "QueryParameters.h"
#include <gtest/gtest.h>

TEST(QueryParametersTest, extractFromRequestStringHttp)
{
  // Test all options
  {
    QueryParameters q;
    string query =
        string("?q=test")  // Query
      + "&c=10"    // Num of completions
      + "&h=500"   // Num of hits
      + "&f=2"     // Number of the first hit
      + "&en=3"    // Num of excerpts per hit
      + "&er=4"    // Num of excerpts per hit
      + "&rd=2d"   // How to rank docs
      + "&s=smbs"  // Score aggregation 
      + "&fd=0.2"  // fuzzy damping
      + "&rw=3a"   // How to rank words
      + "&p=3"     // Index of the part within the title.
      + "&format=xml";  // Format of the result

    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ("test", q.query);
    EXPECT_EQ(10u,     q.nofCompletionsToSend);
    EXPECT_EQ(500u,    q.nofHitsToSend);
    EXPECT_EQ(2u,      q.firstHitToSend);
    EXPECT_EQ(3u,      q.nofExcerptsPerHit);
    EXPECT_EQ(4u,      q.excerptRadius);
    EXPECT_EQ(3u,      q.titleIndex);
    EXPECT_FLOAT_EQ(0.2,    q.fuzzyDamping);
    EXPECT_EQ(QueryParameters::NORMAL,                q.queryType);
    EXPECT_EQ(QueryParameters::XML,                   q.format);
    EXPECT_EQ(QueryParameters::RANK_DOCS_BY_WORD_ID,  q.howToRankDocs);
    EXPECT_EQ(QueryParameters::RANK_WORDS_BY_WORD_ID, q.howToRankWords);
    EXPECT_EQ(SORT_ORDER_DESCENDING, q.sortOrderDocs);
    EXPECT_EQ(SORT_ORDER_ASCENDING,  q.sortOrderWords);
    EXPECT_EQ(SCORE_AGG_SUM,         q.docScoreAggSameCompletion);
    EXPECT_EQ(SCORE_AGG_SUM,         q.wordScoreAggDifferentDocuments);
    EXPECT_EQ(SCORE_AGG_MAX,         q.docScoreAggDifferentCompletions);
    EXPECT_EQ(SCORE_AGG_SUM_WITH_BONUS, q.wordScoreAggSameDocument);
  }

  
  {
    QueryParameters q;
    string query = "?exe=kokos&callback=banane";

    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ("banane", q.callback);
    EXPECT_EQ(QueryParameters::EXE,                   q.queryType);
    EXPECT_EQ(QueryParameters::JSONP,                 q.format);
  }
  
  // Malformed queries.
  // We want to ignore bad param-value pairs, but parse the rest correctly.
  {
    QueryParameters q;
    // An empty query should not throw an error.
    string query = "";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));

    // Missing question mark. We do expect a question mark.
    // This is necessary to differ between query and page request.
    query="q=test&c=10";
    EXPECT_FALSE(q.extractFromRequestStringHttp(query));
 
    // Double amps should lead to an unknown parameter, but not to an error. 
    query = "?q=test&&c=10";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ("test", q.query);
    EXPECT_EQ(10u,     q.nofCompletionsToSend);
    
    // Double equality sign: the second one belongs to the value.
    query = "?q==test";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ("=test", q.query);
    
    // Missing param should be recognized as unknown options.
    // Examples:
    // ?=test has param "=test"
    // ?c=10&=test has param "c" with value 10 and param "=test"
    query = "?=test";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    query = "?c=20&=test&h=5";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ(20u, q.nofCompletionsToSend);
    EXPECT_EQ(5u,  q.nofHitsToSend);

    // Missing value, means empty value:
    query = "?q=";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    query = "?h=3?q=&c=30";
    EXPECT_TRUE(q.extractFromRequestStringHttp(query));
    EXPECT_EQ("", q.query);
    EXPECT_EQ(30u, q.nofCompletionsToSend);
    EXPECT_EQ(3u,  q.nofHitsToSend);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
