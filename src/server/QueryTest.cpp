// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright Jens Hoffmann 2010

#include <gtest/gtest.h>
#include <utility>
#include <vector>
#include "./Separator.h"
#include "./Query.h"
using std::make_pair;

// _____________________________________________________________________________
TEST(QueryTest, constructor)
{
  Query q;
  ASSERT_EQ((unsigned) 0, q._queryString.size());
  Query q1("A Query");
  ASSERT_EQ(q1._queryString , "A Query");
}

// _____________________________________________________________________________
TEST(QueryTest, destructor)
{
  // The test does nothing so far, until there s an implementation for ~Query.
}

// _____________________________________________________________________________
TEST(QueryTest, setQueryString)
{
  Query q;
  q.setQueryString("A query");
  ASSERT_EQ(q._queryString , "A query");
}

TEST(QueryTest, getQueryString)
{
  Query q("A query");
  ASSERT_EQ(q.getQueryString() , "A query");
}

TEST(QueryTest, length)
{
  Query q("A query");
  ASSERT_EQ((unsigned) 7, q.length());
}

TEST(QueryTest, empty)
{
  Query q;
  ASSERT_TRUE(q.empty());
  q.setQueryString("A query");
  ASSERT_FALSE(q.empty());
}

TEST(QueryTest, getLastCharacter)
{
  Query q("A query");
  ASSERT_EQ('y' , q.getLastCharacter());
  q.setQueryString("0");
  ASSERT_EQ('0' , q.getLastCharacter());
}

TEST(QueryTest, removeLastCharacter)
{
  Query q("A query");
  q.removeLastCharacter();
  ASSERT_EQ("A quer" , q.getQueryString());
  q.setQueryString("A");
  q.removeLastCharacter();
  ASSERT_EQ("" , q.getQueryString());
  q.removeLastCharacter();
  ASSERT_EQ("" , q.getQueryString());
}

TEST(QueryTest, append)
{
  Query q("A");
  q.append(" query");
  ASSERT_EQ("A query" , q.getQueryString());
}

TEST(QueryTest, normalize)
{
  Query q("         A query   ");
  q.normalize();
  ASSERT_EQ("A query" , q.getQueryString());
}

TEST(QueryTest, normalizeQueryPart)
{
  Query q1;
  string queryPart;
  // Initialize normalizeWords. Else normalizeQueryPart is just going to return
  // the string, without normalizing.
  extern bool normalizeWords;
  normalizeWords = true;
  // Initialize globalStringConverter. Else there is going to be a segmentation
  // fault since not the initialized globalStringConverter acts on empty maps.
  extern StringConverter globalStringConverter;
  globalStringConverter.init();

  // Check if normalizeWords is really set to true;
  ASSERT_TRUE(normalizeWords);
  
  // Check if nothing gets lost using :filter: of CsvParser
  // Num of queryParts: 1;
  {
    queryPart = "COST*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 2;
  {
    queryPart = ":fil*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 3;
  {
    queryPart = ":filter:NAT*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 4;
  {
    queryPart = ":filter:Nation:COST*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 6;
  {
    queryPart = ":filter:Nation:C:123:*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  
  // Check if nothing gets lost using c?:...:... of XmlParserNew
  // Num of queryParts: 2;
  {
    queryPart = "ce:auth*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 3;
  {
    queryPart = "ce:author:christoph*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 4;
  {
    queryPart = "ce:author:christoph_meinel:*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }

  // Now check if normalization itself works.
  // Num of queryParts: 1;
  {
    queryPart = "renee";
    ASSERT_EQ("renee", q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 2;
  {
    queryPart = ":fil*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 3;
  {
    queryPart = ":filter:NAT*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 4;
  {
    queryPart = ":filter:Nation:COST*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
  // Num of queryParts: 6;
  {
    queryPart = ":filter:Nation:C:123:*";
    ASSERT_EQ(queryPart, q1.normalizeQueryPart(queryPart));
  }
}

TEST(QueryTest, splitAtLastSeparator)
{
  Query q1, first, second;
  Separator separator;

  q1.setQueryString("A B CD");
  q1.splitAtLastSeparator(&first, &second, &separator);
  ASSERT_EQ("A B", first.getQueryString());
  ASSERT_EQ("CD", second.getQueryString());
}

TEST(QueryTest, isRightmostPart)
{
  Query q("word");
  ASSERT_TRUE(q.isRightmostPart());
  q.setQueryString("A query");
  ASSERT_TRUE(!q.isRightmostPart());
}

TEST(QueryTest, matchesTag)
{
  Query q("tag");
  // isCompleteWord (second parameter) = true.
  ASSERT_TRUE(q.matchesTag("<tag>", true));
  ASSERT_TRUE(q.matchesTag("</tag>", true));
  ASSERT_FALSE(q.matchesTag("<<tag>>", true));
  ASSERT_FALSE(q.matchesTag("<tag>>", true));
  ASSERT_FALSE(q.matchesTag("<>tag>", true));
  ASSERT_FALSE(q.matchesTag("<tag/<>", true));
  // isCompleteWord (second parameter) = false.
  q.setQueryString("Pre");
  ASSERT_TRUE(q.matchesTag("<PrefixedTag", false));
  ASSERT_TRUE(q.matchesTag("<PreDefined", false));
  ASSERT_TRUE(q.matchesTag("</Presporok>", false));
}

TEST(QueryTest, splitAtDots)
{
  Query q("A..query....with..dots..");
  std::vector<Query> queries = q.splitAtDots();
  ASSERT_EQ((unsigned) 4, queries.size());
  ASSERT_EQ("A", queries[0].getQueryString());
  ASSERT_EQ("query", queries[1].getQueryString());
  ASSERT_EQ("with", queries[2].getQueryString());
  ASSERT_EQ("dots", queries[3].getQueryString());
  q.setQueryString("A.query.with.dots.");
  queries = q.splitAtDots();
  ASSERT_EQ((unsigned) 1, queries.size());
  ASSERT_EQ("A.query.with.dots.", queries[0].getQueryString());
}

TEST(QueryTest, isKeyword)
{
  // Method not implemented yet.
}

TEST(QueryTest, splitAt)
{
  Query q("A nother query");
  std::vector<Query> queries = q.splitAt("e");
  ASSERT_EQ((unsigned) 3, queries.size());
  ASSERT_EQ("A noth", queries[0].getQueryString());
  ASSERT_EQ("r qu", queries[1].getQueryString());
  ASSERT_EQ("ry", queries[2].getQueryString());
  q.setQueryString("A query");
  queries = q.splitAt("qr");
  ASSERT_EQ((unsigned) 3, queries.size());
  ASSERT_EQ("A ", queries[0].getQueryString());
  ASSERT_EQ("ue", queries[1].getQueryString());
  ASSERT_EQ("y", queries[2].getQueryString());
}

TEST(QueryTest, cleanForHighlighting)
{
  Query q("A_ |qu##e)(((]][r]]y");
  q.cleanForHighlighting();
  ASSERT_EQ("A_ |query", q.getQueryString());
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
