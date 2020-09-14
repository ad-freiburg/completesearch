// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMCLASSIFIER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMCLASSIFIER_H_

#include <string>
#include "sentence/Sentence.h"
#include "./FeatureExtractor.h"
namespace ad_decompose
{
class SVMClassifier
{
  public:
    virtual ~SVMClassifier()
    {
    }
    virtual bool classifyFV(FeatureVector const & fv) const = 0;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMCLASSIFIER_H_
