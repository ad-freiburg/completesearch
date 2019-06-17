// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "./QueryTriple.h"
#include "./Disjunct.h"
#include "./QueryTreeNode.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
TEST(IsATripleTest, testAsString)
{
  Index index;
  Engine engine;
  QueryExecutionContext qec(index, engine);
  IsATriple objectUnderTest(&qec);
  objectUnderTest.setTargetClass(":e:test:Test");
  ASSERT_EQ("<-- :r:is-a :e:test:Test>", objectUnderTest.asString());
  objectUnderTest.setTargetClass(".");
  ASSERT_EQ("<-- :r:is-a .>", objectUnderTest.asString());
}
TEST(OccursWithTripleTest, testAsString)
{
  Index index;
  Engine engine;
  QueryExecutionContext qec(index, engine);

  IsATriple leaf1(&qec);
  leaf1.setTargetClass(":e:test:Test");
  IsATriple leaf2(&qec);
  leaf2.setTargetClass(":e:a:A");
  Disjunct disj1(&qec);
  disj1.addTriple(leaf1);
  disj1.addTriple(leaf2);
  Disjunct disj2(&qec);
  disj2.addTriple(leaf2);

  QueryTreeNode node1(&qec);
  node1.addDisjunct(disj1);

  QueryTreeNode node2(&qec);
  node2.addDisjunct(disj1);
  node2.addDisjunct(disj2);

  OccursWithTriple objectUnderTest(&qec);

  objectUnderTest.addWord("word");
  ASSERT_EQ("<-- :r:occurs-with (word)>", objectUnderTest.asString());
  objectUnderTest.addWord("ab");
  ASSERT_EQ("<-- :r:occurs-with (ab word)>", objectUnderTest.asString());
  objectUnderTest.addSubtree(node1);
  ASSERT_EQ(
      "<-- :r:occurs-with (ab word <NODE <D <-- :r:is-a :e:a:A> "
      "<-- :r:is-a :e:test:Test>>>)>",
      objectUnderTest.asString());

  objectUnderTest.addSubtree(node2);
    ASSERT_EQ(
        "<-- :r:occurs-with (ab word <NODE <D <-- :r:is-a :e:a:A> "
        "<-- :r:is-a :e:test:Test>> <D <-- :r:is-a :e:a:A>>> "
        "<NODE <D <-- :r:is-a :e:a:A> <-- :r:is-a :e:test:Test>>>)>",
        objectUnderTest.asString());
}
TEST(RelationTripleTest, testAsString)
{
  Index index;
  Engine engine;
  QueryExecutionContext qec(index, engine);
  RelationTriple objectUnderTest(&qec);

  IsATriple leaf1(&qec);
  leaf1.setTargetClass(":e:test:Test");
  OccursWithTriple leaf2(&qec);
  leaf2.addWord("word");
  Disjunct disj1(&qec);
  disj1.addTriple(leaf1);
  disj1.addTriple(leaf2);
  Disjunct disj2(&qec);
  disj2.addTriple(leaf1);

  QueryTreeNode node1(&qec);
  node1.addDisjunct(disj1);

  QueryTreeNode node2(&qec);
  node2.addDisjunct(disj1);
  node2.addDisjunct(disj2);

  objectUnderTest.setRelationName(":r:born-in");
  objectUnderTest.setTarget(node1);
  ASSERT_EQ("<-- :r:born-in <NODE <D <-- :r:is-a :e:test:Test> "
      "<-- :r:occurs-with (word)>>>>", objectUnderTest.asString());
  objectUnderTest.setTarget(node2);
  objectUnderTest.setRelationName(":r:other");
  ASSERT_EQ("<-- :r:other <NODE <D <-- :r:is-a :e:test:Test> "
        "<-- :r:occurs-with (word)>> "
        "<D <-- :r:is-a :e:test:Test>>>>", objectUnderTest.asString());
}
}
