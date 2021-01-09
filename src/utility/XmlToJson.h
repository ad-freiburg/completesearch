// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Ina Baumgarten <baumgari>.

#ifndef UTILITY_XMLTOJSON_H_
#define UTILITY_XMLTOJSON_H_

#include <expat.h>
#include <gtest/gtest_prod.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>
#include <map>

using namespace std; // NOLINT

// Class to convert XML patterns into json.
class XmlToJson
{
 public:
  XmlToJson(const vector<string>& multiples = vector<string>());

  // Converts xml to json.
  string xmlToJson(const string& xml);

  // Check for characters, which should be escaped (backslash and quote).
  static string escapeInvalidChars(const string& text);
  FRIEND_TEST(XmlToJsonTest, escapeInvalidChars);

 private:
  // Struct which stores the tree given by the xml.
  struct Node
  {
    string name;
    string text;
    string attributes;
    vector<Node> children;
  };

  Node getXmlTreeIterative(const string& xml);

  // There are some letters, which impedes the parsing of the xml, like
  // escaped quotes and escaped backslashes and cdata. We mask them at the
  // beginning.
  string maskXml(const string& xml);

  // After building the json string we demask the masked letters / strings.
  void unmaskJson(string* json) const;

  // Helper for doXmlToJsonRecursive, which splits the attribute part of an
  // element into its parts.
  // PRECONDITIONS: <element> does not contain escaped quotes and does not start
  // with < and does not end with >.
  static string attributes2json(const string& element, string* value);
  FRIEND_TEST(XmlToJsonTest, attributes2json);

  // Converts the parsed node to json.
  string convertNodeToJson(const Node &node);

  // Prints the tree - for debugging.
  void printTree(const Node &node, int indent = 0) const;

  // Some fields can be filled by one or more elements. We want those fields to
  // be handled the same in either case: as list.
  map<string, bool> _multiples;

  // Helpers for (de-)masking the text.
  // Each "bad" character or text part is replaced by the _replacementChar.
  static const char _replacementChar = '~';
  // The replaced text is stored concatenated in _replacements.
  string _replacements;
  // _replacementOffsets stores the start of each replacement within in
  // _replacements.
  vector<size_t> _replacementOffsets;
};

#endif  // UTILITY_XMLTOJSON_H_
