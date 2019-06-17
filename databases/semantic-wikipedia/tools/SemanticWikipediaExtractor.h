// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TOOLS_SEMANTICWIKIPEDIAEXTRACTOR_H_
#define SEMANTIC_WIKIPEDIA_TOOLS_SEMANTICWIKIPEDIAEXTRACTOR_H_

#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

const unsigned char MATCH_ALL_MODE = 1;
const unsigned char MATCH_ANY_MODE = 2;

template<unsigned char MODE>
class SemanticWikipediaExtractor
{
  public:
  explicit SemanticWikipediaExtractor(const std::string& inTextPattern,
      const std::string& regex, const std::string& titlesFile);
  virtual ~SemanticWikipediaExtractor();

  // Extract from the xml file and write it to the output file.
  // Rules for extraction should be present as class members
  void extract(const std::string& xmlFileName, const std::string& outputFile);
  void doHandleCharacter(const char* s, int len);

  void doHandleStart(const char *el, const char **attr);
  void doHandleEnd(const char *el);

  private:
  typedef ad_utility::HashSet<std::string> TitleSet;

  bool shouldBeKept(const std::string& content);

  TitleSet _titles;
  std::string _inTextPattern;
  std::string _regexS;
  boost::regex _regex;
  FILE* _outputFile;
  char* _buf;
  char* _titleBuf;
  int _pos;
  int _titlePos;
  bool _titleOpen;
};

#endif  // SEMANTIC_WIKIPEDIA_TOOLS_SEMANTICWIKIPEDIAEXTRACTOR_H_
