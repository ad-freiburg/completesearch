// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <iostream>
#include "./SVMLightClassifier.h"
#include "base/SemanticWikipediaReader.h"
#include "base/SemanticWikipediaWriter.h"
#include "./FeatureExtractor.h"
#include "./MLExampleWriter.h"

using std::string;
using std::endl;
using std::flush;
using std::cout;
using ad_decompose::SemanticWikipediaReader;
using ad_decompose::SemanticWikipediaWriter;
using ad_decompose::FeatureExtractorConfig;
using ad_decompose::FeatureMap;
using ad_decompose::SVMLightClassifier;
using ad_decompose::FeatureExtractor;





// Available options.
struct option options[] =
{
    { "verbose", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
};

FeatureExtractorConfig getDefaultFexConfig()
{
  FeatureExtractorConfig fexConfig;
  fexConfig.wwLeft = 3;
  fexConfig.wwRight = 3;
  fexConfig.pre_ww = "ww:";

  fexConfig.cwLeft = 3;
  fexConfig.cwRight = 3;
  fexConfig.pre_cw = "cw:";

  fexConfig.pwLeft = 3;
  fexConfig.pwRight = 3;
  fexConfig.pre_pw = "pw:";

  fexConfig.bwLeft = 3;
  fexConfig.bwRight = 0;
  fexConfig.pre_bw = "bw:";
  fexConfig.brTypes = "REL( REL) LIT( LIT) RELA( RELA) SEP";

  fexConfig.countWTypes = "that";
  fexConfig.countPTypes = "( ) , \" . : ``";
  fexConfig.countCTypes = "VP WP WP$";
  fexConfig.pre_count = "count:";

  return fexConfig;
}

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./SVMLightClassifierMain "
      << "<options> <featureMapFile> <modelFile> <brTag> <indexFile> "
      << "<outputFile>\n"
      << "\t Required options:\n" << endl
      << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "SVMLight Classifier, version " << __DATE__ << " "
      << __TIME__ << ad_utility::EMPH_OFF << std::endl;
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

  if (optind + 5 > argc)
  {
    printUsage();
    exit(1);
  }

  // File names.
  std::string featureMapFile = argv[optind++];
  std::string modelFile = argv[optind++];
  std::string brTag = argv[optind++];
  std::string indexFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(indexFile.size() > 0);
  assert(featureMapFile.size() > 0);
  assert(modelFile.size() > 0);
  assert(brTag.size() > 0);
  assert(outputFile.size() > 0);

  // Start the classification
  cout << "Starting classification... " << endl << flush;

  FeatureMap mfMap(featureMapFile, true);
  FeatureExtractor fex(&mfMap, getDefaultFexConfig());
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputFilename(indexFile);
  SemanticWikipediaWriter<DONT_WRITE_DOCS_FILE, DefaultToken> sww(outputFile
      , outputFile, outputFile+".very-long-sentences", 2);
  SVMLightClassifier svm(modelFile);
  Sentence<DefaultToken> * s = new Sentence<DefaultToken>();
  while (swr.parseNextSentence(s))
  {
    fex.newSentence(*s);
    size_t i = 0;
    for (i = 0; i < s->getWords().size(); ++i)
    {
      if (svm.classifyFV(fex.extractWordFeatures(i)))
      {
        fex.appendBrTag(i, brTag);
        s->appendBrTag(i, brTag);
      }
    }
    // sww.writeTaggedSentence(*s);
    delete s;
    s = new Sentence<DefaultToken>();
  }
}
