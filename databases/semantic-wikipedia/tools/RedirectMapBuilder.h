// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TOOLS_REDIRECTMAPBUILDER_H_
#define SEMANTIC_WIKIPEDIA_TOOLS_REDIRECTMAPBUILDER_H_

#include <stdio.h>
#include <string>

#include "../codebase/semantic-wikipedia-utils/HashMap.h"

namespace semsearch
{
class RedirectMapBuilder
{
  public:
  explicit RedirectMapBuilder(const std::string& outputFileName);
  virtual ~RedirectMapBuilder();

  // Process the XML WIkipedia Dump and collect all redirect information.
  // Writes this information line-wise into the output file.
  // If the target of the redirect is a certain section, only the
  // page's title is set as target.
  void process(const std::string& xmlFileName);

  void doHandleCharacter(const char* s, int len);
  void doHandleStart(const char *el, const char **attr);
  void doHandleEnd(const char *el);

  private:
  bool _titleOpen;
  bool _freshText;
  bool _noTextYet;
  bool _inTheMiddeOfText;
  bool _noLinkYet;
  std::string _currentTitle;
  char* _titleBuf;
  char* _redirectsToBuf;
  FILE* _outputFile;
  int _targetPos;
  int _titlePos;
  ad_utility::HashMap<std::string, std::string> _redirectMap;
};
}
#endif  // SEMANTIC_WIKIPEDIA_TOOLS_REDIRECTMAPBUILDER_H_
