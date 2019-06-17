// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_CONTEXTDECOMPOSERUTIL_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_CONTEXTDECOMPOSERUTIL_H_

#include <gtest/gtest.h>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <set>
#include <hash_set>
#include "../decomposer-ml/FeatureExtractor.h"
#include "../sentence/Sentence.h"
#include "sentence/Context.h"

using std::pair;

namespace ad_decompose
{
class ContextDecomposerUtil
{
  public:
    // Print contexts to stdout.
    template<class Token>
    static void printContexts(Contexts<Token> const & contexts);

//    // Convert a vector of tokens to a string.
//    static std::string tokenVectorToString(
//        std::vector<DefaultToken const *> const & tokens);
//
//    // A version of above for non-const tokens.
//    static std::string tokenVectorToString(std::vector<DefaultToken *>
    //  const & tokens);
//
//    // A version of above for non-pointer tokens.
//    static std::string tokenVectorToString(std::vector<DefaultToken>
    //  const & tokens);
//
//    // Transform from a vector of pointers to a vector of instances.
//    static std::vector<Context> contextPtrToContext(
//        std::vector<ContextPtr> const & contextsPtr);

    // Tokenize a string at whitespaces.
    static std::vector<string> tokenizeString(string const & tokenizeString);

    // Parse a single line into default-tokens.
    // The line's items are tab-separated.
    static DefaultToken * parseTokenLine(std::string const & line,
        DefaultToken * token);

    // Parse a single line into deep-tokens. The line's items are tab-separated.
    static DeepToken * parseTokenLine(std::string const & line,
        DeepToken * token);

    static std::map<string, string>
    readPropertyMap(string const & propertyFile);

    static FeatureExtractorConfig getDefaultFexConfig();

    static FeatureExtractorConfig parseFeatureConfig(
        std::string const & configFile);

    // Compute the precision given # of true and # of false positives.
    static float computePrecision(float nTrue, float nFalsePos);

    // Compute the recall given # of true and # of false negatives.
    static float computeRecall(float nTrue, float nFalseNeg);

    // Compute F-Measure given precision and recall;
    static float computeFMeasure(float precision, float recall);

    // Compute F-Measure given # of true, false negatives and false positives.
    static float computeFMeasure(float nTrue, float nFalsePos,
        float nFalseNeg);

    // Read file line by line and put each string into the set.
    static void
    readFileToContainer(std::set<string> * set, string const & file);

    // A helper function to lookup a key in a map. Puts the found value
    // in the parameter and returns true if it exists. Else returns false.
    template<class K, class V>
    static bool mapLookup(map<K, V> const & map, K lookup, V * value)
    {
      typename std::map<K, V>::const_iterator it;
      if ((it = map.find(lookup)) != map.end())
      {
        *value = it->second;
        return true;
      }
      else
        return false;
    }

  private:
    static string getStringPropertyFromMap(string const & property,
        map<string, string> const & map);

    static pair<int, int> getIntPairPropertyFromMap(string const & property,
        map<string, string> const & map);
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_CONTEXTDECOMPOSERUTIL_H_
