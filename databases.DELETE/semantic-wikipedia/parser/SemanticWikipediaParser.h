// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Hannah Bast <bast>, Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_PARSER_SEMANTICWIKIPEDIAPARSER_H_
#define SEMANTIC_WIKIPEDIA_PARSER_SEMANTICWIKIPEDIAPARSER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <string>
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/parser/XmlParser.h"
#include "./EntityAwareWriter.h"

#define OUTPUT_SPECIAL_CHARS 1
#define STANDARD  0

using ad_utility::HashMap;
using ad_utility::HashSet;

namespace ad_semsearch
{
// If the parser finds something that appears to be an entity but is longer
// than specified by this constant, it is ignored. This is an easy way to
// get rid of problems caused by errors in the wikipedia markup for
// some page.
static const size_t MAX_ENTITY_LENGTH = 200;

template<unsigned char MODE>
class SemanticWikipediaParser: public XmlParser
{
  public:
    SemanticWikipediaParser(
        std::string::size_type minWordLength, size_t maxHeaderLevel);

    virtual ~SemanticWikipediaParser()
    {
    }

    // Write the finish to the index. At the moment this means writing the
    // translations for example from :person to the long paths E:entity:...:
    void writeFinish(const string& wordsFileAppendixName,
        const string& docsFileAppendixName, const string& yagoPathsFileName);

    void setYagoRelationsFileName(const string& yagoRelationsFileName);

    void setRedirectMap(const string& fileName)
    {
      if (fileName.size() > 0) _writer.readRedirectMap(fileName);
    }

    void setWordNetSynonymsMap(const string& fileName)
    {
      if (fileName.size() > 0) _writer.readSynonymsMap(fileName);
    }

    void setYagoFacts(const string& fileName)
    {
      if (fileName.size() > 0) _writer.readYagoFacts(fileName);
    }

    void setAbstractnessCounts(const string& fileName)
    {
      if (fileName.size() > 0) _writer.readAbstractnessCounts(fileName);
    }

    void setOutputCategories(bool outputCategories)
    {
      _outputCategories = outputCategories;
    }

    void setSectionHeadersToSkip(const string& fileName);

    void setStopWordsFile(const string& fileName)
    {
      _writer.readStopWordsFile(fileName);
    }

    void initializeAvailableEntitiesMap(const std::string& entityListFile,
        const std::string& redirectMapFile)
    {
      _writer.initializeAvailableEntitiesMap(entityListFile, redirectMapFile);
    }

    void setAnaphoraPronouns(const string& fileName)
    {
      if (fileName.size() > 0) _writer.readAnaphoraPronounsFile(fileName);
    }

    void setMarkEntityEnds(bool markEntityEnds)
    {
      _writer.setMarkEntityEnds(markEntityEnds);
    }

    void setPreserveCaseInOutput(bool value)
    {
      _writer.setPreserveCaseInOutput(value);
    }

    void setOnlyWriteEntitiesFromOntology(bool value)
    {
      _writer.setOnlyWriteEntitiesFromOntology(value);
    }

    void setWriteWikiDocIdPosting(bool value)
    {
      _writer.setWriteWikiDocIdPosting(value);
    }

    void setDoNotWritePaths(bool value)
    {
      _writer.setDoNotWritePaths(value);
      if (value) _writer.setEntityPrefix(ENTITY_PREFIX);
    }

    void setEntityPrefix(const std::string& entityPrefix)
    {
      _writer.setEntityPrefix(entityPrefix);
    }

    void setNewsMode(bool value)
    {
      _writer.setNewsMode(value);
    }


  private:
    EntityAwareWriter _writer;
    HashSet<string> _sectionHeadersToSkip;

    // Overload this method from XmlParser.
    void outputDocAndWords();

    // A flag whether or not relations are used. If there aren't
    // any like for the classical SUSI search, and Makefile call sw-parse,
    // no entity contexts have to be written at all.
    bool _usesRelations;

    bool _outputCategories;

    // Produce output for doc and words file in a string.
    // not static since the result will depend on the state of
    // the associated EntityRepository later on.
    FRIEND_TEST(SemanticWikipediaParserTest, parseText);
    FRIEND_TEST(SemanticWikipediaParserTest, testPrechunkMode);
    void parseText(const string& text, const string& title,
        const string& docUrl, string* wordsFileOutput, string* docsFileOutput);

    void parseText(const string& text, const string& title,
        string* wordsFileOutput, string* docsFileOutput)
    {
      parseText(text, title, "", wordsFileOutput, docsFileOutput);
    }

    // Checks if some character is a normal word character.
    bool isWordChar(const char c)
    {
      // NEW (björn, 18Mar11): Removed keeping utf-8 characters
      // NEW 14Sep11 (buchholb): Uses prechunk map now which keeps chars like
      // ' in it's and - in ta-da was word characters.
      return ad_utility::PRECHUNK_W_CHAR_MAP[static_cast<uint8_t> (c)] == 'w';
    }

    // Checks if some character is a word separator.
    // TODO(buchholb): Handle multi-byte separators!
    bool isWordSeparator(const char c)
    {
      return ad_utility::S_CHAR_MAP[static_cast<uint8_t> (c)] == 'y';
    }

    // Checks if a comment starts in the text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, commentStarts);
    bool commentStarts(const string& text, const std::string::size_type& pos);

    // Determines the size of a comment in text starting at pos.
    FRIEND_TEST(SemanticWikipediaParserTest, commentSize);
    int commentSize(const string& text, const std::string::size_type& pos);

    // Checks if a template starts in the text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, templateStarts);
    bool templateStarts(const string& text, const std::string::size_type& pos);

    // Determines the size of a template in text starting at pos.
    FRIEND_TEST(SemanticWikipediaParserTest, templateSize);
    int templateSize(const string& text, const std::string::size_type& pos);

    // Checks if a math section starts in the text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, mathStarts);
    bool mathStarts(const string& text, const std::string::size_type& pos);

    // Determines the size of a math section in text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, mathSize);
    int mathSize(const string& text, const std::string::size_type& pos);

    // Checks if a math section starts in the text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, refStarts);
    bool refStarts(const string& text, const std::string::size_type& pos);

    // Determines the size of a math section in text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, refSize);
    int refSize(const string& text, const std::string::size_type& pos);

    // Determines the size of a random tag in the text at position pos.
    FRIEND_TEST(SemanticWikipediaParserTest, tagSize);
    int tagSize(const string& text, const std::string::size_type& pos);

    // Handles link occurrences in the text.
    FRIEND_TEST(SemanticWikipediaParserTest, handleLink);
    int handleLink(const string& text, const std::string::size_type& pos);

    // Checks if a section starts
    FRIEND_TEST(SemanticWikipediaParserTest, sectionStarts);
    bool sectionStarts(const string& text, const std::string::size_type& pos);

    // Handles section starts the the text
    FRIEND_TEST(SemanticWikipediaParserTest, handleSection);
    int handleSection(const string& text, const std::string::size_type& pos);
};
}
#endif  // SEMANTIC_WIKIPEDIA_PARSER_SEMANTICWIKIPEDIAPARSER_H_
