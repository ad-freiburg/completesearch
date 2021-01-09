// Copyright 2013, Albert-Ludwigs-Unoversität
// Lehrstuhl für Algorithmen und Datenstrukturen
//
// Author: Ina Baumgarten  <baumgari>

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "./XmlToJson.h"

TEST(XmlToJsonTest, attributes2json)
{
  XmlToJson xtj;

  string xml;
  string value;
  string attributes;

  // String with very strange positioned spaces.
  xml = "title    ee = \"banana and apple \""
        "a =\" fruits \" lala=   \"blub \" ";
  attributes = xtj.attributes2json(xml, &value);
  EXPECT_EQ("title", value);
  EXPECT_EQ("\"@ee\":\"banana and apple \",\"@a\":\""
            " fruits \",\"@lala\":\"blub \"", attributes);

  // Take care of special characters.
  xml = "a href=\"http://example.com/hagu_jatc\\kagz?’+'kajsh\"";
  attributes = xtj.attributes2json(xml, &value);
  EXPECT_EQ("a", value);
  EXPECT_EQ("\"@href\":\"http://example.com/hagu_jatc\\kagz?’+'kajsh\"",
            attributes);

  // No attributes.
  xml = "test";
  attributes = xtj.attributes2json(xml, &value);
  EXPECT_EQ("test", value);
  ASSERT_TRUE(attributes.empty());

  // Misplaced space.
  xml = "blub ";
  attributes = xtj.attributes2json(xml, &value);
  EXPECT_EQ("blub", value);
  ASSERT_TRUE(attributes.empty());

  // A case which failed earlier. Due to the empty url.
  xml = "dblp:venue url=\"\" journal=\" Uni Trier\"";
  attributes = xtj.attributes2json(xml, &value);
  EXPECT_EQ("dblp:venue", value);
  EXPECT_EQ("\"@url\":\"\",\"@journal\":\" Uni Trier\"", attributes);
}

TEST(XmlToJsonTest, xmlToJson)
{
  // Check that CDATA is handled correctly.
  {
    // If just CDATA is given, erase the tags.
    XmlToJson xtj;
    string xml = "<![CDATA[dub di dub <test> <hans> <!peter]] or blub]]>";
    EXPECT_EQ("\"dub di dub <test> <hans> <!peter]] or blub\"",
              xtj.xmlToJson(xml));
  }

  {
    // Simple CDATA field with letters to be escaped..
    XmlToJson xtj;
    string xml          = "<show field=\"test\">"
                            "<![CDATA[\\\\<test>\\\\ \"<hans>\\\" <!peter]] blub\\\"]]>"
                          "</show>";
    string expectedJson = "{"
                             "\"show\":{"
                               "\"@field\":\"test\","
                               "\"text\":\"\\\\<test>\\\\ \\\"<hans>\\\" <!peter]] blub\\\"\"}}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Simple CDATA field with letters to be escaped..
    XmlToJson xtj;
    string xml          = "<show field=\"test\">"
                            "<![CDATA[\\\"<test>\\\\ \"<hans>\\\" <!peter]] blub\\\\]]>"
                          "</show>";
    string expectedJson = "{"
                             "\"show\":{"
                               "\"@field\":\"test\","
                               "\"text\":\"\\\"<test>\\\\ \\\"<hans>\\\" <!peter]] blub\\\\\"}}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Empty CDATA field.
    XmlToJson xtj;
    string xml          = "<show field=\"test\">"
                            "<![CDATA[]]>"
                          "</show>";
    string expectedJson = "{"
                             "\"show\":{"
                               "\"@field\":\"test\","
                               "\"text\":\"\"}}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Multiple CDATA fields. Attention: Nested CDATA doesn't make sense and
    // cannot be handled!
    XmlToJson xtj;
    string xml          = "<show field=\"test\">"
                            "<title ee=\"hello\">"
                              "<![CDATA[ <hans> <!peter]] or blub]]>"
                            "</title>"
                            "<author>"
                              "<![CDATA[nanananana nana\"lala\"]]>"
                            "</author>"
                          "</show>";
    string expectedJson = "{"
                             "\"show\":{"
                               "\"@field\":\"test\","
                               "\"title\":{"
                                 "\"@ee\":\"hello\","
                                 "\"text\":\" <hans> <!peter]] or blub\"},"
                               "\"author\":\"nanananana nana\\\"lala\\\"\"}}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  // Ensure, that everything is escaped properly.
  {
    XmlToJson xtj;
    string xml = "<url b=\"<does><this><lead><to><problems>\">"
                 "http://example.com/hagu_jatc\\kagz?’+'\"kajsh</url>";
    string expectedJson = "{\"url\":{\"@b\":\"<does><this><lead><to><problems>\","
                          "\"text\":\"http://example.com/hagu_jatc\\\\kagz?"
                          "’+'\\\"kajsh\"}}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
    xml                 = "\\\"I am a very \"simple\" string\\\"";
    expectedJson        = "\"\\\"I am a very \\\"simple\\\" string\\\"\"";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  // Ensure, that misplaced spaces to not lead to a wrong output.
  {
    XmlToJson xtj;
    string xml          = "<a>b</a> <a>c</a>";
    string expectedJson = "{\"a\":[\"b\",\"c\"]}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  // Ensure that mixed content is working.
  // Ensure that empty tags are no problem.
  {
    // Simple empty tag.
    XmlToJson xtj;
    string xml = "<dubida></dubida>";
    EXPECT_EQ("{\"dubida\":\"\"}", xtj.xmlToJson(xml));
  }

  {
    // Empty tag with attribute.
    XmlToJson xtj;
    string xml = "<dubida z=\"a\"></dubida>";
    EXPECT_EQ("{\"dubida\":{\"@z\":\"a\"}}", xtj.xmlToJson(xml));
  }

  {
    // Empty tag with context.
    XmlToJson xtj;
    string xml = "<hello a=\"b\"><dubida></dubida></hello>";
    EXPECT_EQ("{\"hello\":{\"@a\":\"b\",\"dubida\":\"\"}}", xtj.xmlToJson(xml));
  }

  {
    // Empty tag with attribute with context.
    XmlToJson xtj;
    string xml = "<hello a=\"b\"><dubida z=\"a\"></dubida></hello>";
    EXPECT_EQ("{\"hello\":{\"@a\":\"b\",\"dubida\":{\"@z\":\"a\"}}}", xtj.xmlToJson(xml));
  }

  // Ensure that lists are working.
  {
    // Simple empty tag as multiple.
    vector<string> multiples = {"dubida"};
    XmlToJson xtj(multiples);
    string xml = "<dubida></dubida>";
    EXPECT_EQ("{\"dubida\":[]}", xtj.xmlToJson(xml));
  }

  {
    // Simple empty tag with attribute as multiple.
    vector<string> multiples = {"dubida"};
    XmlToJson xtj(multiples);
    string xml = "<dubida z=\"a\"></dubida>";
    EXPECT_EQ("{\"dubida\":[{\"@z\":\"a\"}]}", xtj.xmlToJson(xml));
  }

  {
    // Simple empty tag with context as multiple.
    vector<string> multiples = {"dubida"};
    XmlToJson xtj(multiples);
    string xml = "<hello a=\"b\"><dubida></dubida></hello>";
    EXPECT_EQ("{\"hello\":{\"@a\":\"b\",\"dubida\":[]}}", xtj.xmlToJson(xml));
  }

  {
    // Simple empty tag with attribute and context as multiple.
    vector<string> multiples = {"dubida"};
    XmlToJson xtj(multiples);
    string xml = "<hello a=\"b\"><dubida z=\"a\"></dubida></hello>";
    EXPECT_EQ("{\"hello\":{\"@a\":\"b\",\"dubida\":[{\"@z\":\"a\"}]}}", xtj.xmlToJson(xml));
  }

  {
    // Multiple list, without defining as multiple.
    XmlToJson xtj;
    string xml          = "<a>a</a><a>b</a><a>c</a>";
    string expectedJson = "{\"a\":[\"a\",\"b\",\"c\"]}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Multiple lists in context.
    XmlToJson xtj;
    string xml          = "<z>hj</z><z>z</z><a>a</a><a>b</a><a>c</a><y>y</y>";
    string expectedJson = "{\"z\":[\"hj\",\"z\"],\"a\":[\"a\",\"b\",\"c\"],\"y\":\"y\"}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // No problem if not all multiples occur.
    vector<string> multiples = {"a", "b", "c"};
    XmlToJson xtj(multiples);
    string xml          = "<a>a</a><a>b</a><a>c</a>";
    string expectedJson = "{\"a\":[\"a\",\"b\",\"c\"]}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Multiple multiples + single element.
    vector<string> multiples = {"a", "b"};
    XmlToJson xtj(multiples);
    string xml          = "<a>a</a><b>b</b><c>c</c>";
    string expectedJson = "{\"a\":[\"a\"],\"b\":[\"b\"],\"c\":\"c\"}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }

  {
    // Multiple with attribute and content + context.
    vector<string> multiples = {"a", "b"};
    XmlToJson xtj(multiples);
    string xml          = "<a b=\"g\">a</a><b>b</b><c>c</c>";
    string expectedJson = "{\"a\":[{\"@b\":\"g\",\"text\":\"a\"}],\"b\":[\"b\"],\"c\":\"c\"}";
    EXPECT_EQ(expectedJson, xtj.xmlToJson(xml));
  }
}

TEST(XmlToJsonTest, escapeInvalidChars)
{
  XmlToJson xtj;

  // Nothing to escape.
  EXPECT_EQ("lala banana apple", xtj.escapeInvalidChars("lala banana apple"));

  // Escape non-escaped invalid characters, but not already escaped ones.
  string toEscape = "\"lala\" \"banana\" \\apple\\";
  EXPECT_EQ("\\\"lala\\\" \\\"banana\\\" \\\\apple\\\\",
            xtj.escapeInvalidChars(toEscape));
}
