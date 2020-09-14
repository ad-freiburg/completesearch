// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLEXAMPLEWRITER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLEXAMPLEWRITER_H_

#include <string>
#include <vector>
#include "./FeatureVector.h"
#include "./FeatureExtractor.h"

namespace ad_decompose
{
// Write provided features to a file.
class MLExampleWriterBase
{
  public:
    virtual ~MLExampleWriterBase()
    {
    }
    // Write all examples to outputFile.
    virtual void writeExamples(std::string const & outputFileName, std::vector<
        MLExample> examples) = 0;
  private:
};

// Write provided features to a file.
class SVMLightExampleWriter: MLExampleWriterBase
{
  public:
    virtual ~SVMLightExampleWriter();
    // Write all examples to outputFile.
    virtual void writeExamples(std::string const & outputFileName, std::vector<
        MLExample> examples);

    void writeMappedExamples(std::string const & outputFileName,
        FeatureMap const & fMap, std::vector<MLExample> examples);

  private:
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLEXAMPLEWRITER_H_
