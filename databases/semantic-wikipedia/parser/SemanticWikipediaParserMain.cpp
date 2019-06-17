// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Hannah Bast <bast>, Björn Buchhold <buchholb>

#include <getopt.h>
#include <string>
#include <iomanip>
#include "./SemanticWikipediaParser.h"

// Available options.
struct option options[] =
    { {"yago-facts", required_argument, NULL, 'y'},
      {"yago-paths", required_argument, NULL, 'P'},
      {"yago-relations", required_argument, NULL, 'R'},
      {"min-word-length", required_argument, NULL, 'm'},
      {"max-header-level", required_argument, NULL, 'h'},
      {"redirect-map", required_argument, NULL, 'r'},
      {"pronouns-file-name", required_argument, NULL, 'p'},
      {"section-headers-to-skip", required_argument, NULL, 's'},
      {"stop-words", required_argument, NULL, 'S'},
      {"prechunk-mode", no_argument, NULL, 'c'},
      {"mark-entity-ends", no_argument, NULL, 'e'},
      {"preserve-case-in-output", no_argument, NULL, 'C'},
      {"only-write-entities-from-ontology", no_argument, NULL, 'E'},
      {"do-not-write-paths", no_argument, NULL, 'X'},
      {"wordnet-synonyms", required_argument, NULL, 'w'},
      {"news", no_argument, NULL, 'N'},
      {"no-translations", no_argument, NULL, 'T'},
      {"ontology-entity-list", required_argument, NULL, 'O'},
      {"abstractness-counts", required_argument, NULL, 'a'},
      {"output-categories", no_argument, NULL, 'K'},
      {NULL, 0, NULL, 0}
    };

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON << "Wikipedia parser, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  bool outputSpecialChars = false;
  bool markEntityEnds = false;
  bool prechunkMode = false;
  bool onlyWriteEntitiesFromOntolgoy = false;
  bool doNotWritePaths = false;
  bool preserveCase = false;
  bool newsMode = false;
  bool noTranslations = false;
  bool outputCategories = false;
  int maxHeaderLevel = 1;
  int minWordLength = 1;
  std::string redirectMapFileName = "";
  std::string sectionHeadersToSkipFileName = "";
  std::string anaphora_pronous_file = "";
  std::string yagoFactsFileName = "";
  std::string yagoPathsFileName = "";
  std::string yagoRelationsFileName = "";
  std::string wordNetSynonymsFileName = "";
  std::string stopWordFileName = "";
  std::string ontologyEntityListFileName = "";
  std::string abstractnessCountsFileName = "";


  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "y:m:h:p:r:s:R:P:w:ceCNTS:O:a:K",
        options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'y':
      yagoFactsFileName = optarg;
      break;
    case 'P':
      yagoPathsFileName = optarg;
      break;
    case 'R':
      yagoRelationsFileName = optarg;
      break;
    case 'm':
      minWordLength = atoi(optarg);
      break;
    case 'h':
      maxHeaderLevel = atoi(optarg);
      break;
    case 'r':
      redirectMapFileName = optarg;
      break;
    case 'p':
      anaphora_pronous_file = optarg;
      break;
    case 'w':
      wordNetSynonymsFileName = optarg;
      break;
    case 's':
      sectionHeadersToSkipFileName = optarg;
      break;
    case 'S':
      stopWordFileName = optarg;
      break;
    case 'e':
      markEntityEnds = true;
      break;
    case 'E':
      onlyWriteEntitiesFromOntolgoy = true;
      break;
    case 'O':
      ontologyEntityListFileName = optarg;
      break;
    case 'c':
      prechunkMode = true;
      outputSpecialChars = true;
      break;
    case 'C':
      preserveCase = true;
      break;
    case 'N':
      newsMode = true;
      break;
    case 'X':
      doNotWritePaths = true;
      break;
    case 'T':
       noTranslations = true;
       break;
    case 'a':
       abstractnessCountsFileName = optarg;
       break;
    case 'K':
       outputCategories = true;
       break;
    default:
      std::cout << std::endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << std::endl << std::endl;
      exit(1);
    }
  }

  // File names.
  std::string dbName = optind < argc ? argv[optind++]
      : "semantic-wikipedia-test";
  std::string xmlFileName = dbName + ".xml";
  std::string docsFileName = dbName
      + (doNotWritePaths ? "-full.docs-by-contexts.ascii" : ".docs-unsorted");
  std::string wordsFileName =
      dbName + (prechunkMode ? ".words-unsorted.prechunk.ascii"
          : (doNotWritePaths ? "-full.words-by-contexts.ascii" :
              ".words-unsorted.ascii"));
  std::string vocabularyFileName = dbName + ".vocabulary";
  std::string entitiesFileName = dbName + ".words-unsorted.entities";
  std::string logFileName = dbName + ".parse-log";

  // Parse the xml file.
  if (outputSpecialChars)
  {
    ad_semsearch::SemanticWikipediaParser<OUTPUT_SPECIAL_CHARS> xp(
        minWordLength, maxHeaderLevel);
    if (ontologyEntityListFileName.size() > 0)
    {
      xp.initializeAvailableEntitiesMap(ontologyEntityListFileName,
          redirectMapFileName);
    }
    else
    {
      xp.setRedirectMap(redirectMapFileName);
      xp.setWordNetSynonymsMap(wordNetSynonymsFileName);
      xp.setYagoFacts(yagoFactsFileName);
    }

    xp.setSectionHeadersToSkip(sectionHeadersToSkipFileName);
    xp.setAnaphoraPronouns(anaphora_pronous_file);


    // Check for relations feature
    if (yagoRelationsFileName.length() > 0)
    {
      xp.setYagoRelationsFileName(yagoRelationsFileName);
    }

    if (stopWordFileName.size() > 0)
    {
      xp.setStopWordsFile(stopWordFileName);
    }

    xp.setMarkEntityEnds(markEntityEnds);
    xp.setPreserveCaseInOutput(preserveCase);
    xp.setNewsMode(newsMode);
    xp.setOnlyWriteEntitiesFromOntology(onlyWriteEntitiesFromOntolgoy);
    xp.setDoNotWritePaths(doNotWritePaths);
    xp.setAbstractnessCounts(abstractnessCountsFileName);
    xp.setOutputCategories(outputCategories);

    xp.parse(xmlFileName, docsFileName, wordsFileName, logFileName);
    if (noTranslations && yagoRelationsFileName.size() == 0)
    {
      return 0;
    }
    if (doNotWritePaths) return 0;

    std::cout << "Writing appendix files (:t: words and ontology stuff)..."
        << std::flush;
    xp.writeFinish(wordsFileName + ".appendix", docsFileName + ".appendix",
        yagoPathsFileName);
    std::cout << "done." << std::endl;
  }
  else
  {
    ad_semsearch::SemanticWikipediaParser<STANDARD> xp(minWordLength,
        maxHeaderLevel);

    if (ontologyEntityListFileName.size() > 0)
    {
      xp.initializeAvailableEntitiesMap(ontologyEntityListFileName,
          redirectMapFileName);
    }
    else
    {
      xp.setRedirectMap(redirectMapFileName);
      xp.setWordNetSynonymsMap(wordNetSynonymsFileName);
      xp.setYagoFacts(yagoFactsFileName);
    }

    xp.setSectionHeadersToSkip(sectionHeadersToSkipFileName);
    xp.setAnaphoraPronouns(anaphora_pronous_file);

    // Check for relations feature
    if (yagoRelationsFileName.length() > 0)
    {
      xp.setYagoRelationsFileName(yagoRelationsFileName);
    }

    if (stopWordFileName.size() > 0)
    {
      xp.setStopWordsFile(stopWordFileName);
    }

    xp.setMarkEntityEnds(markEntityEnds);
    xp.setPreserveCaseInOutput(preserveCase);
    xp.setNewsMode(newsMode);
    xp.setDoNotWritePaths(doNotWritePaths);
    xp.setOnlyWriteEntitiesFromOntology(onlyWriteEntitiesFromOntolgoy);
    xp.setAbstractnessCounts(abstractnessCountsFileName);
    xp.setOutputCategories(outputCategories);

    xp.parse(xmlFileName, docsFileName, wordsFileName, logFileName);

    if (noTranslations && yagoRelationsFileName.size() == 0)
    {
      return 0;
    }

    if (doNotWritePaths) return 0;

    std::cout << "Writing appendix files (:t: words and ontology stuff)..."
        << std::flush;
    xp.writeFinish(wordsFileName + ".appendix", docsFileName + ".appendix",
        yagoPathsFileName);
    std::cout << "done." << std::endl;
  }
  return 0;
}
