// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "base/ContextDecomposerBase.h"
#include "evaluation/GroundTruthReader.h"

using std::string;
using std::endl;
using std::flush;
using std::cout;
using ad_decompose::GroundTruthReader;
using ad_decompose::GroundTruthSentence;


// Available options.
struct option options[] =
{
{ "verbose", no_argument, NULL, 'v' },
{ NULL, 0, NULL, 0 } };

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./SVMStructClassifierInputMain"
      << "<groundTruthFile> <outputFile>" << "\t Required options:\n" << endl
      << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "SVMStruct Classifier Input, version "
      << __DATE__ << " " << __TIME__ << ad_utility::EMPH_OFF << std::endl;
  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      default:
        printUsage();
        exit(1);
    }
  }

  if (optind + 2 > argc)
  {
    printUsage();
    exit(1);
  }

  // File names.
  std::string indexFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(indexFile.size() > 0);
  assert(outputFile.size() > 0);

  // Start the classification
  cout << "Starting generation... " << endl << flush;
  std::ofstream outFile(outputFile.c_str(), std::ios::out);
  GroundTruthReader<DefaultToken> gtReader;
  boost::ptr_vector<GroundTruthSentence<DefaultToken> > sentences;
  gtReader.readGroundTruthsFromFile(&sentences, indexFile, true, false);
  string currentNpTag = "";
  Phrase<DefaultToken> const * previousPhrase = NULL;
  for (size_t i = 0; i < sentences.size(); ++i)
  {
    int open = 0;
    int close = 0;
    currentNpTag = "";
    previousPhrase = NULL;
    outFile << "( (S \n";
    vector<DefaultToken *> const & words = sentences[i].sentence.getWords();
    for (size_t j = 0; j < words.size(); ++j)
    {
      // if (words[j]._phrase->getType() != currentNpTag)
      if (words[j]->_phrase != previousPhrase)
      {
        if (previousPhrase != NULL)
        {
          outFile << " )";
          if (open < 0)
            std::cout << sentences[i].sentence.asString();
        }
      }

      if (words[j]->brTag != "*")
      {
        std::vector<string> brTags;
        boost::split(brTags, words[j]->brTag, boost::is_any_of(","));
        for (size_t b = 0; b < brTags.size(); ++b)
        {
          std::string::size_type indexO = brTags[b].find('(');
          if (indexO != std::string::npos)
          {
            outFile << "(" << brTags[b].substr(0, indexO) << " ";
            open++;
          }
        }
      }
      // if (words[j]._phrase->getType() != currentNpTag)
      if (words[j]->_phrase != previousPhrase)
      {
        previousPhrase = words[j]->_phrase;
        outFile << "(" << previousPhrase->getType();
        open++;
      }
      if (words[j]->tokenString == "(")
      {
        outFile << " (" << "[" << " " << "["
            << ")";
      }
      else if (words[j]->tokenString == ")")
      {
        outFile << " (" << "]" << " " << "]"
            << ")";
      }
      else
      {
        outFile << " (" << words[j]->posTag << " " << words[j]->tokenString
                   << ")";
      }
      if (words[j]->brTag != "*")
      {
        std::vector<string> brTags;
        boost::split(brTags, words[j]->brTag, boost::is_any_of(","));
        for (size_t b = 0; b < brTags.size(); ++b)
        {
          std::string::size_type indexE = brTags[b].find(')');
          if (indexE != std::string::npos)
          {
            outFile << " )";
            open--;
            if (open < 0)
              std::cout << sentences[i].sentence.asString();
          }
        }
      }
    }
    outFile << " )\n ))" << "\n";
    if (close != 0)
    {
      std::cout << close << "!=" << 0 << "\n";
      std::cout << sentences[i].sentence.asString();
    }
  }
}
