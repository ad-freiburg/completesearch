// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

// Simple example parser derived from the new XmlParserNew. Assumes that each
// top-level tag of the given XML file contains an element <url>...</url>, and
// element <title>...</title>, and an element <author>...</author>. Will write
// the corresponding information to the docs file and the indexed words from
// <author>...</author> in the words file.

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "codebase/parser/XmlParserNew.h"


extern const char* XP_ENTITY_PREFIX;


// Subclass XmlParserNew.
class HomeonetParser : public XmlParserNew
{
  void outputDocAndWords();
};

// Say what to do for each record (= top-level item) of the given XML file.
void HomeonetParser::outputDocAndWords()
{
  // Get the contents of a particular tag like this.
  string url = getItem("url");
  string title = getItem("title");
  string author = getItem("author");
  string text = getItem("text");

  // Some authors have different names, take care of that.
  if (author == "T.Galic" || author == "Galic")
    author = "Tjado Galic";
  else if (author == "Bier")
    author = "Ilse Hübler-Bier";
  else if (author == "Sielmann Gabriele")
    author = "Gabriele Sielmann";
  else if (author == "Karinrieper" || author == "Rieper")
    author = "Karin Rieper";
  else if (author == "G.Strelau")
    author = "Gabriele Strelau";
  else if (author == "Manuelpadrutt")
    author = "Manuel Padrutt";
  else if (author == "Claudia.Schwinghammer")
    author = "Claudia Schwinghammer";
  else if (author == "Wüstholz Eva-Maria")
    author = "Eva-Maria Wüstholz";

  // Tokenise a string an output the individual words to the words file like
  // this.
  XmlParserNew::outputWords(title);
  XmlParserNew::outputWords(text);
  XmlParserNew::outputWords(author);
  XmlParserNew::outputWords(author, "author", "ct:", "cn:");

  // Output a line to the docs file like this. The doc id is maintained
  // automatically, starting from 1 for the first record and incremented by one
  // for each record.
  XmlParserNew::outputDocument(url, title, text);
}

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Main function. Parses command line arguments and then starts the parsing.
int main(int argc, char** argv)
{
  cout << endl
       << EMPH_ON << "EXAMPLE XML PARSER (NEW), last compiled on "
       << __DATE__ << " " << __TIME__ << EMPH_OFF << endl
       << endl;

  XP_ENTITY_PREFIX = "ce:";

  // Create an instance of the example parser class defined above.
  HomeonetParser xp;

  // Let the XML parser process its command line arguments.
  xp.parseCommandLineOptions(argc, argv);

  // Parse remaining command line options particular for this example parser.
  // NOTE: must not forget to reset optind here, otherwise command line options
  // might get lost.
  // TODO(bast): explain why must also reset opterr = 0.
  opterr = 0;
  optind = 1;
  static struct option longOptions[] =
  {
    {"option-a", 0, NULL, 'a'},
    {NULL      , 0, NULL,   0}
  };
  while (true)
  {
    int c = getopt_long(argc, argv, "a:", longOptions, NULL);
    if (c == -1) break;
    switch (c)
    {
      case 'a': printf("Option a = %s\n", optarg); break;
      // NOTE: Must not exit on default here, because there are other options to
      // be parsed by XmlParserNew and ParserBase.
    }
  }

  // Now do the actual parsing. Note that the base file name is a required
  // command-line option, and all file name are derived from it.
  xp.parse();
  cout << endl;
}
