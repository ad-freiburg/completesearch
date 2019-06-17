// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/server/Globals.h"
#include "../util/ContextDecomposerUtil.h"
#include "sentence/Context.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "decomposer-rule-deep/DeepContextMarker.h"

namespace ad_decompose
{
// ____________________________________________________________________________
template <class Token>
void ContextDecomposerUtil::printContexts(Contexts<Token> const & contexts)
{
  std::cout << "Contexts :" << std::endl << std::endl;
  for (size_t i = 0; i < contexts.size(); ++i)
  {
    Context<Token> const & c = contexts[i];
    std::cout << "Context " << i << ": " << c.asString() << std::endl;
  }
  std::cout << std::endl;
}

// Explicitly instantiate function templates.
template void ContextDecomposerUtil::printContexts<DefaultToken>(
    Contexts<DefaultToken> const & contexts);
template void ContextDecomposerUtil::printContexts<DeepToken>(
    Contexts<DeepToken> const & contexts);

// ____________________________________________________________________________
FeatureExtractorConfig ContextDecomposerUtil::getDefaultFexConfig()
{
  FeatureExtractorConfig fexConfig;
  fexConfig.words = "including,such as,as well as,and,or,but,thus,however";
  fexConfig.wwLeft = -4;
  fexConfig.wwRight = 4;
  fexConfig.pre_ww = "ww:";

  fexConfig.cwLeft = -4;
  fexConfig.cwRight = 4;
  fexConfig.pre_cw = "cw:";

  fexConfig.pwLeft = -4;
  fexConfig.pwRight = 4;
  fexConfig.pre_pw = "pw:";

  fexConfig.brTypes = "REL( REL) LIT( LIT) RELA( RELA) SEP";
  fexConfig.bwLeft = -6;
  fexConfig.bwRight = 6;
  fexConfig.pre_bw = "bw:";

  fexConfig.dynbrType = "";
  fexConfig.dynbrLeft = -3;
  fexConfig.dynbrRight = -1;

  fexConfig.countWTypes = "that however before after";
  fexConfig.countPTypes = "( ) , \" . : ``";
  fexConfig.countCTypes = "VP NP PP";
  fexConfig.pre_count = "count:";

  return fexConfig;
}

//   ________________________________________________________________________
//  void ContextDecomposerUtil::printContexts(
//    std::vector<ContextPtr> const & contexts)
//  {
//  std::cout << "Contexts :" << std::endl << std::endl;
//  for (size_t i = 0; i < contexts.size(); ++i)
//  {
//    std::string contextString = tokenVectorToString(contexts[i]);
//    std::cout << "Context " << i << ": " << contextString << std::endl;
//  }
//  std::cout << std::endl;
//  }
//
//  // ________________________________________________________________________
//  std::string ContextDecomposerUtil::tokenVectorToString(
//    std::vector<DefaultToken const *> const & tokens)
// {
//  string result = "";
//  if (tokens.size() == 0)
//    return result;
//  for (size_t i = 0; i < tokens.size() - 1; ++i)
//  {
//    result += tokens[i]->tokenString + " ";
//  }
//  result += tokens[tokens.size() - 1]->tokenString;
//  return result;
// }
//
// ____________________________________________________________________________
// std::string ContextDecomposerUtil::tokenVectorToString(
//    std::vector<DefaultToken *> const & tokens)
// {
// std::vector<DefaultToken const *> const * ct = reinterpret_cast<
// std::vector <
//      DefaultToken const *> const *> (&tokens);
//  return tokenVectorToString(*ct);
// }
//
//  ____________________________________________________________________________
// std::string ContextDecomposerUtil::tokenVectorToString(Context const &
// context)
// {
//  string result = "";
//  if (context.size() == 0)
//    return result;
//  for (size_t i = 0; i < context.size() - 1; ++i)
//  {
//    result += context[i].tokenString + " ";
//  }
//  result += context[context.size() - 1].tokenString;
//  return result;
// }
//
//// _________________________________________________________________________
// std::vector<Context> ContextDecomposerUtil::contextPtrToContext(
//    std::vector<ContextPtr> const & contextsPtr)
// {
//  std::vector<Context> contexts;
//  for (size_t i = 0; i < contextsPtr.size(); ++i)
//  {
//    Context newContext;
//    for (size_t j = 0; j < contextsPtr[i].size(); ++j)
//    {
//      newContext.push_back(*contextsPtr[i][j]);
//    }
//    contexts.push_back(newContext);
//  }
//  return contexts;
// }

// _____________________________________________________________________________
std::vector<string> ContextDecomposerUtil::tokenizeString(
    string const & tokenizeString)
{
  int pos = 0;
  std::string::size_type findPos = 0;
  std::vector<string> result;
  // Split along whitespace.
  while ((findPos = tokenizeString.find(' ', pos)) != std::string::npos)
  {
    string word = tokenizeString.substr(pos, findPos - pos);
    result.push_back(word);
    pos = findPos + 1;
  }
  // Add the last token.
  string word = tokenizeString.substr(pos, tokenizeString.length() - pos + 1);
  result.push_back(word);
  return result;
}

// ____________________________________________________________________________
DefaultToken * ContextDecomposerUtil::parseTokenLine(std::string const & line,
    DefaultToken * token)
{
  // split along tabs
  std::string::size_type indexOfTab1 = line.find('\t');
  std::string::size_type indexOfTab2 = line.find('\t', indexOfTab1 + 1);
  std::string::size_type indexOfTab3 = line.find('\t', indexOfTab2 + 1);
  std::string::size_type indexOfTab4 = line.find('\t', indexOfTab3 + 1);
  std::string::size_type indexOfTab5 = line.find('\t', indexOfTab4 + 1);
  std::string::size_type indexOfTab6 = line.find('\t', indexOfTab5 + 1);
  std::string::size_type indexOfTab7 = line.find('\t', indexOfTab6 + 1);

  assert(indexOfTab1 != std::string::npos);
  assert(indexOfTab2 != std::string::npos);
  assert(indexOfTab3 != std::string::npos);
  assert(indexOfTab4 != std::string::npos);
  assert(indexOfTab5 != std::string::npos);
  assert(indexOfTab6 != std::string::npos);
  assert(indexOfTab7 != std::string::npos);

  // extract the values in the columns
  std::string wordStr = line.substr(0, indexOfTab1);
  std::string docIdStr = line.substr(indexOfTab1 + 1,
      indexOfTab2 - (indexOfTab1 + 1));
  std::string scoreStr = line.substr(indexOfTab2 + 1,
      indexOfTab3 - (indexOfTab2 + 1));
  std::string positionStr = line.substr(indexOfTab3 + 1,
      indexOfTab4 - (indexOfTab3 + 1));
  std::string posTagStr = line.substr(indexOfTab4 + 1,
      indexOfTab5 - (indexOfTab4 + 1));
  std::string npTagStr = line.substr(indexOfTab5 + 1,
      indexOfTab6 - (indexOfTab5 + 1));
  std::string cTagStr = line.substr(indexOfTab6 + 1,
      indexOfTab7 - (indexOfTab6 + 1));
  std::string brTagStr = line.substr(indexOfTab7 + 1);

  DocId docId = atoi(docIdStr.c_str());
  Score score = atoi(scoreStr.c_str());
  Position position = atoi(positionStr.c_str());

  // Set the values on the token.
  token->completeSearchScore = score;
  token->completeSearchPosition = position;
  token->completeSearchDocId = docId;
  token->tokenString = wordStr;
  token->posTag = posTagStr;
  token->npTag = npTagStr;
  token->cTag = cTagStr;
  token->setBrTags(brTagStr);

  return token;
}



// ____________________________________________________________________________
DeepToken * ContextDecomposerUtil::parseTokenLine(std::string const & line,
    DeepToken * token)
{
  // split along tabs
  std::string::size_type indexOfTab1 = line.find('\t');
  std::string::size_type indexOfTab2 = line.find('\t', indexOfTab1 + 1);
  std::string::size_type indexOfTab3 = line.find('\t', indexOfTab2 + 1);
  std::string::size_type indexOfTab4 = line.find('\t', indexOfTab3 + 1);
  std::string::size_type indexOfTab5 = line.find('\t', indexOfTab4 + 1);
  std::string::size_type indexOfTab6 = line.find('\t', indexOfTab5 + 1);
  std::string::size_type indexOfTab7 = line.find('\t', indexOfTab6 + 1);

  assert(indexOfTab1 != std::string::npos);
  assert(indexOfTab2 != std::string::npos);
  assert(indexOfTab3 != std::string::npos);
  assert(indexOfTab4 != std::string::npos);
  assert(indexOfTab5 != std::string::npos);
  assert(indexOfTab6 != std::string::npos);
  assert(indexOfTab7 != std::string::npos);

  // extract the values in the columns
  std::string wordStr = line.substr(0, indexOfTab1);
  std::string docIdStr = line.substr(indexOfTab1 + 1,
      indexOfTab2 - (indexOfTab1 + 1));
  std::string scoreStr = line.substr(indexOfTab2 + 1,
      indexOfTab3 - (indexOfTab2 + 1));
  std::string positionStr = line.substr(indexOfTab3 + 1,
      indexOfTab4 - (indexOfTab3 + 1));
  std::string posTagStr = line.substr(indexOfTab4 + 1,
      indexOfTab5 - (indexOfTab4 + 1));
  std::string npTagStr = line.substr(indexOfTab5 + 1,
      indexOfTab6 - (indexOfTab5 + 1));
  std::string cTagStr = line.substr(indexOfTab6 + 1,
      indexOfTab7 - (indexOfTab6 + 1));
  std::string brTagStr = line.substr(indexOfTab7 + 1);

  DocId docId = atoi(docIdStr.c_str());
  Score score = atoi(scoreStr.c_str());
  Position position = atoi(positionStr.c_str());

  // Set the values on the token.
  token->completeSearchScore = score;
  token->completeSearchPosition = position;
  token->completeSearchDocId = docId;
  token->tokenString = wordStr;
  token->posTag = posTagStr;
  token->npTag = npTagStr;
  token->cTag = cTagStr;
  token->setBrTags(brTagStr);

  return token;
}

// ____________________________________________________________________________
std::map<string, string> ContextDecomposerUtil::readPropertyMap(
    std::string const & propertyFile)
{
  std::ifstream inputS(propertyFile.c_str(), std::ios::in);
  std::map<string, string> properties;
  boost::regex commentLine("^( )*#.*");
  boost::regex propertyLine("(.*) = (.*)");
  boost::regex emptyLine("^( )*$");
  boost::cmatch what;
  if (!inputS.good())
  {
    LOG(ERROR) << "Error opening " << propertyFile << std::endl;
  }
  std::string line;
  while (std::getline(inputS, line))
  {
    // std::cout << line << std::endl;
    // Ignore comment lines.
    if (boost::regex_match(line.c_str(), what, commentLine))
      continue;
    else if (boost::regex_match(line.c_str(), what, propertyLine))
    {
      string property(what[1].first, what[1].second);
      boost::trim(property);
      string value(what[2].first, what[2].second);
      boost::trim(value);
      properties.insert(std::make_pair<string, string>(property, value));
      // std::cout << "Added:" << property << " == " << value << std::endl;
    }
    else if (boost::regex_match(line.c_str(), what, emptyLine))
      continue;
    else
    {
      LOG(ERROR) << "Wrong format in property line:\n";
      exit(1);
    }
  }
  inputS.close();
  return properties;
}

// ____________________________________________________________________________
FeatureExtractorConfig ContextDecomposerUtil::parseFeatureConfig(
    std::string const & configFile)
{
  std::map<string, string> properties = readPropertyMap(configFile);
  FeatureExtractorConfig config;

  std::pair<int, int> intPair;

  intPair = getIntPairPropertyFromMap("ww", properties);
  assert(intPair.first < intPair.second);
  config.wwLeft = intPair.first;
  config.wwRight = intPair.second;
  config.pre_ww = getStringPropertyFromMap("pre_ww", properties);

  intPair = getIntPairPropertyFromMap("cw", properties);
  assert(intPair.first < intPair.second);
  config.cwLeft = intPair.first;
  config.cwRight = intPair.second;
  config.pre_cw = getStringPropertyFromMap("pre_cw", properties);

  intPair = getIntPairPropertyFromMap("pw", properties);
  assert(intPair.first < intPair.second);
  config.pwLeft = intPair.first;
  config.pwRight = intPair.second;
  config.pre_pw = getStringPropertyFromMap("pre_pw", properties);

  intPair = getIntPairPropertyFromMap("bw", properties);
  assert(intPair.first < intPair.second);
  config.bwLeft = intPair.first;
  config.bwRight = intPair.second;
  config.pre_bw = getStringPropertyFromMap("pre_bw", properties);

  intPair = getIntPairPropertyFromMap("dynbrw", properties);
  assert(intPair.first < intPair.second);
  config.dynbrLeft = intPair.first;
  config.dynbrRight = intPair.second;

  config.brTypes = getStringPropertyFromMap("brTypes", properties);

  config.words = getStringPropertyFromMap("words", properties);

  config.countWTypes = getStringPropertyFromMap("countWTypes", properties);
  config.countPTypes = getStringPropertyFromMap("countPTypes", properties);
  config.countCTypes = getStringPropertyFromMap("countCTypes", properties);
  config.pre_count = getStringPropertyFromMap("pre_count", properties);

  config.patternWTypes = getStringPropertyFromMap("patternWTypes", properties);
  config.patternPTypes = getStringPropertyFromMap("patternPTypes", properties);
  config.patternCTypes = getStringPropertyFromMap("patternCTypes", properties);
  config.pre_pattern = getStringPropertyFromMap("pre_pattern", properties);

  return config;
}

// ____________________________________________________________________________
pair<int, int> ContextDecomposerUtil::getIntPairPropertyFromMap(
    string const & property, map<string, string> const & map)
{
  std::map<string, string>::const_iterator it;
  if ((it = map.find(property)) != map.end())
  {
    std::string value = it->second;
    std::string::size_type indexOfTab1 = value.find(':');
    assert(indexOfTab1 != std::string::npos);
    std::string startStr = value.substr(0, indexOfTab1);
    std::string endStr = value.substr(indexOfTab1 + 1);
    int start = std::atoi(startStr.c_str());
    int end = std::atoi(endStr.c_str());
    return std::make_pair<int, int>(start, end);
  }
  else
  {
    std::cerr << "Property not found: " << property << ".\n";
    exit(1);
  }
}

// ____________________________________________________________________________
std::string ContextDecomposerUtil::getStringPropertyFromMap(
    string const & property, map<string, string> const & map)
{
  std::map<string, string>::const_iterator it;
  if ((it = map.find(property)) != map.end())
  {
    return it->second;
  }
  else
  {
    std::cerr << "Property not found: " << property << ".\n";
    exit(1);
  }
}


// ____________________________________________________________________________
float ContextDecomposerUtil::computePrecision(float nTrue, float nFalsePos)
{
  return nTrue / (nTrue + nFalsePos);
}

// ____________________________________________________________________________
float ContextDecomposerUtil::computeRecall(float nTrue, float nFalseNeg)
{
  return nTrue / (nTrue + nFalseNeg);
}

// ____________________________________________________________________________
float ContextDecomposerUtil::computeFMeasure(float precision, float recall)
{
  return (2 * precision * recall / (precision + recall));
}

// ____________________________________________________________________________
float ContextDecomposerUtil::computeFMeasure(float nTrue, float nFalsePos,
    float nFalseNeg)
{
  float precision = computePrecision(nTrue, nFalsePos);
  float recall = computeRecall(nTrue, nFalseNeg);
  return (2 * precision * recall / (precision + recall));
}

// ____________________________________________________________________________
void ContextDecomposerUtil::readFileToContainer(std::set<string> * set,
    string const & fileName)
{
  if (fileName.length() == 0)
    return;
  ad_utility::File file(fileName.c_str(), "r");
  char buffer[200];
  string line;
  while (file.readLine(&line, buffer, 200))
  {
    set->insert(line);
  }
}
}

