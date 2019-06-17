// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <ostream>
#include <string>
#include <vector>
#include "./SemanticWikipediaQuery.h"

using std::string;
using std::vector;
using std::ostringstream;

// Test the construction of queries.
TEST(SemanticWikipediaQueryTest, getFullQueryString)
{
  SemanticWikipediaQuery query("test ce:entity:*", "host", 1);
  ASSERT_EQ("http://host:1/?q=test ce:entity:*&h=0&c=100000&en=0&er=0",
      query.getFullQueryString());

  query.setNumberOfCompletionsParam(10);
  ASSERT_EQ("http://host:1/?q=test ce:entity:*&h=0&c=10&en=0&er=0",
      query.getFullQueryString());

  query.setNumberOfExcerptsParam(10);
  ASSERT_EQ("http://host:1/?q=test ce:entity:*&h=0&c=10&en=10&er=0",
      query.getFullQueryString());

  query.setNumberOfHitsParam(10);
  ASSERT_EQ("http://host:1/?q=test ce:entity:*&h=10&c=10&en=10&er=0",
      query.getFullQueryString());

  query.setSizeOfExcerptsParam(10);
  ASSERT_EQ("http://host:1/?q=test ce:entity:*&h=10&c=10&en=10&er=10",
      query.getFullQueryString());
}

// Test the result processing. Query execution itself
// is not tested since it comprises much more than
// only this unit's scope.
TEST(SemanticWikipediaQueryTest, processResult)
{
  ostringstream inOs;
  inOs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" << "<result>\n"
      << "<query>suicide ce:entity:physical_entity:object:whole:"
      << "living_thing:organism:person:scientist*</query>\n"
      << "<status code=\"12\">OK</status>\n"
      << "<time unit=\"msecs\">168.29</time>\n"
      << "<completions total=\"5\" computed=\"5\" sent=\"5\">\n"
      << "<c sc=\"0\" dc=\"26\" oc=\"38\" id=\"12115579\">"
      << "ce:entity:physical_entity:" << "object:"
      << "whole:living_thing:organism:person:scientist:biologist:"
      << "microbiologist:"
      << "davidkellyweaponsexpert:david_kelly_(weapons_expert)</c>\n"
      << "<c sc=\"0\" dc=\"26\" oc=\"38\" id=\"12121518\">ce:entity:"
      << "physical_entity:object:"
      << "whole:living_thing:organism:person:scientist:"
      << "davidkellyweaponsexpert:" << "david_kelly_(weapons_expert)</c>\n"
      << "<c sc=\"0\" dc=\"17\" oc=\"18\" id=\"12139485\">ce:entity:"
      << "physical_entity:object:"
      << "whole:living_thing:organism:person:scientist:"
      << "research_worker:arthurkoestler:" << "arthur_koestler</c>\n"
      << "</completions>\n"
      << "<hits total=\"887\" computed=\"100\" sent=\"0\" first=\"0\">\n"
      << "</hits>\n" << "</result>\n";

  SemanticWikipediaQuery query("test", "host", 1);
  vector<string> output;
  query.processResult(inOs.str(), &output);

  ASSERT_EQ("david_kelly_(weapons_expert)" , output[0]);
  ASSERT_EQ("david_kelly_(weapons_expert)" , output[1]);
  ASSERT_EQ("arthur_koestler" , output[2]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
