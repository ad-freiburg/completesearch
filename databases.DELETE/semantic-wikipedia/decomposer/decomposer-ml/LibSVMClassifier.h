// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_LIBSVMCLASSIFIER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_LIBSVMCLASSIFIER_H_

#include <string>
#include "sentence/Sentence.h"
#include "./FeatureExtractor.h"
#include "./SVMClassifier.h"

namespace ad_decompose
{
class svm_model;

class LibSVMClassifier: public SVMClassifier
{
  public:
    explicit LibSVMClassifier(std::string const & modelFile);
    ~LibSVMClassifier();
    virtual bool classifyFV(FeatureVector const & fv) const;
  private:
    svm_model * _model;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_LIBSVMCLASSIFIER_H_
