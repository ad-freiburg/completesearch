// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast@informatik.uni-freiburg.de>, Johannes Schwenk
// <johannes.schwenk@gmail.com>.

#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include "codebase/parser/XmlParserNew.h"

using std::vector;
using std::string;

// Subclass XmlParserNew.
class UniFreiburgParser : public XmlParserNew
{
  void outputDocAndWords();
};

// Here the actual parsing of a document is done.
void UniFreiburgParser::outputDocAndWords()
{
  ostringstream os;
  os << _docId;
  outputWords(os.str(), "docid", ":", NULL);

  // !! get the contents of a particular tag like this
  string title   = getItem("title");
  string content = getItem("content");
  string description = getItem("description");
  string url = getItem("url");
  string portal_type = getItem("portal_type");
  string language = getItem("language");
  vector<const char*> sources = getItems("source");
  vector<const char*> tags = getItems("tag");

  // !! output all words in a string like this
  string sep = " ";
  outputWords(title + sep + description + sep + content + sep + url);
  outputWords(portal_type, "ptype", "__:", NULL);
  outputWords(language, "language", "__:", NULL);
  for (size_t i = 0; i < sources.size(); ++i)
  {
    std::string str = sources[i];
    str.erase(::remove_if(str.begin(), str.end(), ::isspace), str.end());
    outputWords(str, "src", "__:", NULL);
  }
  // Output tags, equip with higher score
  for (size_t i = 0; i < tags.size(); ++i)
  {
    std::string str = tags[i];
    str.erase(::remove_if(str.begin(), str.end(), ::isspace), str.end());
    outputWords(str, NO_DOC_ID, 200, 0);
  }

  // !! for each document, specify a title, url, and text
  // !! Note: the document id is generated automatically (in
  // !! consecutive order, starting from 1)
  // string url = string("file:///") + source;
  // string title = string("File: ") + source;
  outputDocument(url, title, content);
}

// main
int main(int argc, char** argv)
{
  // First parse general parser options.
  UniFreiburgParser ufp;
  ufp.parseCommandLineOptions(argc, argv);

  // Now parse options specific for this parser.
  opterr = 0;
  optind = 1;
  static struct option longOptions[] =
  {
    {"my-option-without-argument" , 0, NULL, 'o'},
    {"my-option-with-argument"    , 1, NULL, 'w'},
    {NULL                         , 0, NULL,  0 }
  };
  while (true)
  {
    int c = getopt_long(argc, argv, "ow:", longOptions, NULL);
    if (c == -1) break;
    switch (c)
    {
      case 'o': break;
      case 'w': break;
    }
  }

  // Parse.
  ufp.parse();
}
