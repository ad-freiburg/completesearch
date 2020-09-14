// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <assert.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
#include "./FeatureVector.h"
#include "./FeatureExtractor.h"
#include "./MLExampleWriter.h"

using std::string;
namespace ad_decompose
{
// _____________________________________________________________________________
SVMLightExampleWriter::~SVMLightExampleWriter()
{
}

// _____________________________________________________________________________
void SVMLightExampleWriter::writeExamples(std::string const & outputFileName,
    std::vector<MLExample> examples)
{
  std::ofstream outStream(outputFileName.c_str(), std::ios::trunc);
  assert(outStream.is_open());
  for (size_t i = 0; i < examples.size(); ++i)
  {
    outStream << examples[i].getClass();
    std::map<int, double>::const_iterator it = examples[i].begin();
    outStream << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    while (it != examples[i].end())
    {
      outStream << " " << (*it).first << ":" << (*it).second;
      ++it;
    }
    outStream << std::endl;
    outStream << std::resetiosflags(std::ios::fixed);
  }
  outStream.close();
}

// _____________________________________________________________________________
void SVMLightExampleWriter::writeMappedExamples(
    std::string const & outputFileName, FeatureMap const & fMap, std::vector<
        MLExample> examples)
{
  std::ofstream outStream(outputFileName.c_str(), std::ios::trunc);
  assert(outStream.is_open());
  std::map<int, string> const reverseMap =
      fMap.getReverseMap();
  for (size_t i = 0; i < examples.size(); ++i)
  {
    outStream << examples[i].getClass();
    std::map<int, double>::const_iterator it = examples[i].begin();
    outStream << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    while (it != examples[i].end())
    {
      outStream << " " << (reverseMap.find(it->first))->second << ":"
          << (*it).second;
      ++it;
    }
    outStream << std::endl;
    outStream << std::resetiosflags(std::ios::fixed);
  }
  outStream.close();
}
}
