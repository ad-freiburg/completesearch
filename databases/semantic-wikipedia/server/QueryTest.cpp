// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "./Query.h"

#include "../codebase/semantic-wikipedia-utils/StringUtils.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
TEST(QueryTest, testAsString)
{
  Query query;
  query.setPrefix("pre");
  ASSERT_EQ("Query without triples for prefix: \"pre\"", query.asString());
}
TEST(OccursWithTripleTest, testConstructFromString)
{
  Query query;
  query.setPrefix("pre");
  try
  {
    // Really simple.
    query.constructQueryTreeFromTriplesString(
        "$1 :r:is-a :e:scientist:Scientist", "$1");
    ASSERT_EQ("Semantic query with prefix: \"pre\""
    ": { <NODE <D <-- :r:is-a :e:scientist:Scientist>>> }", query.asString());

    // A bit more complex
    query.constructQueryTreeFromTriplesString(
        "$2 :r:is-a :e:location:Location; "
        "$3 :r:is-a :e:city:City; "
        "$3 :r:located-in $2",
        "$3");
    ASSERT_EQ("Semantic query with prefix: \"pre\": { "
        "<NODE <D <-- :r:is-a :e:city:City> "
        "<-- :r:located-in <NODE "
        "<D <-- :r:is-a :e:location:Location>>>>>> }", query.asString());

    // Same query with different root.
    query.constructQueryTreeFromTriplesString(
        "$2 :r:is-a :e:location:Location; "
        "$3 :r:is-a :e:city:City; "
        "$3 :r:located-in $2",
        "$2");
    ASSERT_EQ("Semantic query with prefix: \"pre\": { "
        "<NODE <D <-- :r:is-a :e:location:Location> "
        "<-- :r:located-in_(reversed) <NODE "
        "<D <-- :r:is-a :e:city:City>>>>>> }", query.asString());

    // With complex co-occurrence
    query.constructQueryTreeFromTriplesString(
        "$1 :r:is-a :e:person:Person; "
        "$1 :r:occurs-with discovered $2 cure; "
        "$2 :r:is-a :e:disease:Disease",
        "$1");
    ASSERT_EQ("Semantic query with prefix: \"pre\": "
        "{ <NODE <D <-- :r:is-a :e:person:Person> "
        "<-- :r:occurs-with (cure discovered <NODE "
        "<D <-- :r:is-a :e:disease:Disease>>>)>>> }", query.asString());

    // Same thing with different root
    query.constructQueryTreeFromTriplesString(
        "$1 :r:is-a :e:person:Person; "
        "$1 :r:occurs-with discovered $2 cure; "
        "$2 :r:is-a :e:disease:Disease",
        "$2");
    ASSERT_EQ("Semantic query with prefix: \"pre\": "
        "{ <NODE <D <-- :r:is-a :e:disease:Disease> "
        "<-- :r:occurs-with (cure discovered <NODE "
        "<D <-- :r:is-a :e:person:Person>>>)>>> }", query.asString());

    // Long query like from averbis slides
    query.constructQueryTreeFromTriplesString(
            "$1 :r:is-a :e:vegetarian:Vegetarian; "
            "$1 :r:directed $2; $2 :r:is-a :e:movie; "
            "$2 :r:occurs-with world war; "
            "$1 :r:occurs-with married $3; "
            "$3 :r:is-a :e:actor:Actor", "$1");
    ASSERT_EQ("Semantic query with prefix: \"pre\": "
        "{ <NODE <D <-- :r:is-a :e:vegetarian:Vegetarian> "
        "<-- :r:occurs-with (married <NODE "
        "<D <-- :r:is-a :e:actor:Actor>>>)> "
        "<-- :r:directed <NODE "
        "<D <-- :r:is-a :e:movie> "
        "<-- :r:occurs-with (war world)>>>>>> }", query.asString());

    // Same thing with different order of triples.
    query.constructQueryTreeFromTriplesString(
        "$1 :r:directed $2; $2 :r:is-a :e:movie; "
        "$3 :r:is-a :e:actor:Actor;"
        "$2 :r:occurs-with world war; "
        "$1 :r:is-a :e:vegetarian:Vegetarian; "
        "$1 :r:occurs-with married $3", "$1");
    ASSERT_EQ("Semantic query with prefix: \"pre\": "
        "{ <NODE <D <-- :r:is-a :e:vegetarian:Vegetarian> "
        "<-- :r:occurs-with (married <NODE "
        "<D <-- :r:is-a :e:actor:Actor>>>)> "
        "<-- :r:directed <NODE "
        "<D <-- :r:is-a :e:movie> "
        "<-- :r:occurs-with (war world)>>>>>> }", query.asString());
  }
  catch(const Exception& e)
  {
    std::cout << e.getFullErrorMessage() << std::endl;
    ASSERT_TRUE(false);
  }

  // Test detection of cyclic queries
  try
  {
    query.constructQueryTreeFromTriplesString(
        "$1 :r:a $2; $2 :r:b $3; $3 :r:c $1", "$1");
    ASSERT_TRUE(false);
  }
  catch(const Exception& e)
  {
    ASSERT_TRUE(ad_utility::startsWith(e.getFullErrorMessage(),
        "BAD QUERY FORMAT (Cyclic query detected."));
  }
}
}
