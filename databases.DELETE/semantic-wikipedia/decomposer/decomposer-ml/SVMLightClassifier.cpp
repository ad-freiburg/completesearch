// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <string>
#include <map>
#include "./SVMLightClassifier.h"
#include "./FeatureVector.h"
namespace ad_decompose
{
extern "C"
{
#include "svm_common.h" // NOLINT
#include "svm_learn.h" // NOLINT
}


SVMLightClassifier::SVMLightClassifier(std::string const & modelFile)
{
  std::cout << "Classifier using: " << modelFile << std::endl;
  _model = read_model(const_cast<char *> ((modelFile).c_str()));
}

SVMLightClassifier::~SVMLightClassifier()
{
  free_model(_model, 1);
}

// ____________________________________________________________________________
bool SVMLightClassifier::classifyFV(FeatureVector const & fv) const
{
  size_t size = fv.getFeatureVector().size();
  WORD * words = reinterpret_cast<WORD *>(malloc(sizeof(WORD) * (size+1)));
  std::map<int, double>::const_iterator it;
  int i = 0;
  for (it = fv.begin(); it != fv.end(); ++it)
  {
    words[i].weight = it->second;
    words[i].wnum = it->first;
    i++;
  }
  words[size].wnum = 0;
  char s[] = "";
  DOC * doc = create_example(-1, 0, 0, 0.0, create_svector(words, s, 1.0));
  double d = classify_example(_model, doc);
  free(words);
  free_example(doc, 1);
  if (d >= 0)
  {
    return true;
  }
  else
    return false;
}
}
