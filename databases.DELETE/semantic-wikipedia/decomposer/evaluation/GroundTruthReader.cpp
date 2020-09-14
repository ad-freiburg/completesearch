// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include "base/SemanticWikipediaReader.h"
#include "./GroundTruthReader.h"
#include "util/ContextDecomposerUtil.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"


using std::cerr;
using std::cout;
namespace ad_decompose
{
// _____________________________________________________________________________
template <class Token>
void GroundTruthReader<Token>::readGroundTruthsFromFile(boost::ptr_vector<
    GroundTruthSentence<Token> > * groundTruth, string const & fileName,
    bool tagsRequired, bool ignoreBrTag) const
{
  // std::vector<GroundTruthSentence> groundTruth;
  // Read from the input file.
  SemanticWikipediaReader<Token> swr;
  std::ifstream inputS(fileName.c_str(), std::ios::in);
  if (!inputS.good())
  {
    LOG(FATAL) << "Error opening " << fileName <<".\n";
    exit(1);
  }
  string line;

  uint32_t sentenceNumber = 0;
  GroundTruthSentence<Token> * currentGTSentence =
      new GroundTruthSentence<Token>;
  bool hasPostag = false;
  bool hasNptag = false;
  bool hasContexts = false;
  bool hasbrTag = false;

  while (std::getline(inputS, line))
  {
    // This starts a new ground truth.
    if (boost::starts_with(line, "S"))
    {
      // Check if the previous sentence has all information.
      // If tags are required hasPostag and hasNptag need to be true
      // to continue.
      if (sentenceNumber > 0 && (!(hasContexts) || (tagsRequired && (!hasPostag
          || !hasNptag || !hasbrTag))))
      {
        LOG(FATAL) << "Incorrect input format in line:\n\t" << line << "\n";
        exit(1);
      }
      // Add to the list of ground truths.
      // We are sure it has tags and contexts.
      else if (sentenceNumber != 0)
      {
        // We have collected all words and tags. Build the sentence.
        swr.parseSentenceFromTokens(&(currentGTSentence->sentence),
            currentGTSentence->sentenceTokens);
        // Store the sentence.
        groundTruth->push_back(currentGTSentence);
        // groundTruthSentences.push_back(currentGTSentence);
      }
      currentGTSentence = new GroundTruthSentence<Token>;
      hasPostag = false;
      hasNptag = false;
      hasContexts = false;
      hasbrTag = false;
      // Check if the line has correct format.
      verifyLineFormat(line);
      // Check if the sentence number is correct.
      string::size_type tabPos = line.find('\t');
      uint32_t number = std::atoi(line.substr(1, tabPos - 1).c_str());
      // Check if the sentence number is correct.
      if (number != sentenceNumber + 1 || number != groundTruth->size() + 1)
      {
        LOG(FATAL) << "Incorrect number in line:\n\t" << line << "\n";
        exit(1);
      }
      currentGTSentence->sentenceNumber = number;
      std::vector<string> words = ContextDecomposerUtil::tokenizeString(
          line.substr(tabPos + 1, line.size()));
      for (size_t i = 0; i < words.size(); ++i)
      {
        // Tokens are added to sentence structure later
        // from where they will be deleted.
        Token * token = currentGTSentence->sentence.getNewToken();
        token->tokenString = words[i];
        currentGTSentence->sentenceTokens.push_back(token);
      }
      sentenceNumber = number;
    }
    // This adds POS tags to the started ground truth.
    else if (boost::starts_with(line, "P"))
    {
      // If we are not interested in the tags, just jump over this line.
      if (!tagsRequired)
      {
        continue;
      }
      // Sentence must not have any of POS tag, NP Tag or contexts.
      if (sentenceNumber > 0 && (hasPostag || hasNptag || hasContexts))
      {
        LOG(FATAL) << "Incorrect input format in line:\n\t" << line << "\n";
        exit(1);
      }
      std::vector<string> posTags = getTokensFromLine(line,
          currentGTSentence->sentenceTokens.size(), sentenceNumber);
      for (size_t i = 0; i < posTags.size(); ++i)
      {
        currentGTSentence->sentenceTokens[i]->posTag = posTags[i];
      }
      hasPostag = true;
    }
    // This adds phrase tags to the started ground truth.
    else if (boost::starts_with(line, "N"))
    {
      // If we are not interested in the tags, just jump over this line.
      if (!tagsRequired)
      {
        continue;
      }
      // Sentence must only have POS tag here.
      if (sentenceNumber > 0 && (!hasPostag || hasNptag || hasContexts
          || hasbrTag))
      {
        LOG(FATAL)
          << "Missing previous tag(s) or wrong order in line:\n\t" << line
              << "\n";
        exit(1);
      }
      std::vector<string> npTags = getTokensFromLine(line,
          currentGTSentence->sentenceTokens.size(), sentenceNumber);
      for (size_t i = 0; i < npTags.size(); ++i)
      {
        currentGTSentence->sentenceTokens[i]->npTag = npTags[i];
      }
      hasNptag = true;
    }
    else if (boost::starts_with(line, "B"))
    {
      // If we are not interested in the tags, just jump over this line.
      if (!tagsRequired)
      {
        continue;
      }
      if (ignoreBrTag)
      {
        for (size_t i = 0; i < currentGTSentence->sentenceTokens.size(); ++i)
          currentGTSentence->sentenceTokens[i]->brTag = "*";
        hasbrTag = true;
        continue;
      }
      // Sentence must only have POS and NP tag here.
      if (sentenceNumber > 0 && (!hasPostag || !hasNptag || hasContexts
          || hasbrTag))
      {
        LOG(FATAL)
          << "Missing previous tag(s) or wrong order in line:\n\t" << line
              << "\n";
        exit(1);
      }
      std::vector<string> brTags = getTokensFromLine(line,
          currentGTSentence->sentenceTokens.size(), sentenceNumber);
      for (size_t i = 0; i < brTags.size(); ++i)
      {
        // currentGTSentence->sentenceTokens[i]->brTag = brTags[i];
        currentGTSentence->sentenceTokens[i]->setBrTags(brTags[i]);
      }
      hasbrTag = true;
    }
    // This adds contexts to the started ground truth.
    else if (boost::starts_with(line, "C"))
    {
      // Sentence must only have POS and phrase tag here.
      if (sentenceNumber > 0 && (tagsRequired && (!hasPostag || !hasNptag
          || !hasbrTag)))
      {
        LOG(FATAL)
          << "Missing previous tag(s) or wrong order in line:\n\t" << line
              << "\n";
        exit(1);
      }
      std::vector<string> contextTokens = getTokensFromLine(line,
          0, sentenceNumber);
      Context<Token> newContext;
      std::vector<Token *> tokens;
      for (size_t i = 0; i < contextTokens.size(); ++i)
      {
        // Warning HACK WRONG STOP DELETE
        // This is never deleted and only inserted to allow compilation
        // right now - remove ASAP!!.
        Token * token = new Token;
        token->tokenString = contextTokens[i];
        newContext.push_back(token);
      }
      currentGTSentence->contextsExpected.addContext(newContext);
      hasContexts = true;
    }
  }

  // We shouldn't forget the last sentence.
  if (hasContexts && (!tagsRequired || (hasNptag && hasPostag && hasbrTag)))
  {
    // We have collected all words and tags. Build the sentence.
    swr.parseSentenceFromTokens(&(currentGTSentence->sentence),
        currentGTSentence->sentenceTokens);
    groundTruth->push_back(currentGTSentence);
  }

  // Close input handle.
  inputS.close();

  LOG(INFO) << "Read " << groundTruth->size() << " sentences from ground truth."
      << std::endl;
}

// _____________________________________________________________________________
template <class Token>
void GroundTruthReader<Token>::verifyLineFormat(string const & line) const
{
  string::size_type tabPos = line.find('\t');
  if (tabPos == string::npos || tabPos == 0 || tabPos == line.size())
  {
    LOG(FATAL) << "Incorrect input format in line:\n\t" << line << "\n";
    exit(1);
  }
}

// _____________________________________________________________________________
template <class Token>
void GroundTruthReader<Token>::verifyLineNumber(string const & numberStr,
    uint32_t sentenceNumber) const
{
  uint32_t number = std::atoi(numberStr.c_str());
  // Check if the sentence number is correct.
  if (number != sentenceNumber)
  {
    LOG(FATAL) << "Incorrect number:\t" << numberStr << "\n";
    exit(1);
  }
}

// _____________________________________________________________________________
template <class Token>
std::vector<string> GroundTruthReader<Token>::getTokensFromLine(
    string const & line, uint32_t numTokens, int sentenceNumber) const
{
  verifyLineFormat(line);
  // Check if the sentence number is correct.
  string::size_type tabPos = line.find('\t');
  verifyLineNumber(line.substr(1, tabPos - 1), sentenceNumber);
  std::vector<string> tokens = ContextDecomposerUtil::tokenizeString(
      line.substr(tabPos + 1, line.size()));
  if (numTokens != 0 && numTokens != tokens.size())
  {
    LOG(FATAL) << "Wrong number of tokens in line:\n\t" << line << "\n";
    exit(1);
  }
  return tokens;
}

template class GroundTruthSentence<DefaultToken>;
template class GroundTruthReader<DefaultToken>;
}
