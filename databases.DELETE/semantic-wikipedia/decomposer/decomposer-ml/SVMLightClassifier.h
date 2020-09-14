// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMLIGHTCLASSIFIER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMLIGHTCLASSIFIER_H_

#include <string>
#include "./FeatureExtractor.h"
#include "./SVMClassifier.h"

namespace ad_decompose
{
struct model;

class SVMLightClassifier: public SVMClassifier
{
  public:
    explicit SVMLightClassifier(std::string const & modelFile);
    ~SVMLightClassifier();
    virtual bool classifyFV(FeatureVector const & fv) const;
  private:
    model * _model;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_SVMLIGHTCLASSIFIER_H_
