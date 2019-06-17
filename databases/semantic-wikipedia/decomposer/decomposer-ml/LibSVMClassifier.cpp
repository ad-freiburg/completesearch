// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <string>
#include <map>
#include "base/ContextDecomposerBase.h"
#include "sentence/Sentence.h"
#include "./FeatureVector.h"
#include "./LibSVMClassifier.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

// #include "boost/date_time/posix_time/posix_time.hpp"
// include all types plus i/o
// using namespace boost::posix_time;
namespace ad_decompose
{
#include "svm.h" // NOLINT

LibSVMClassifier::LibSVMClassifier(std::string const & modelFile)
{
  LOG(INFO) << "LibSVMClassifier using: " << modelFile << std::endl;
  _model = svm_load_model((modelFile).c_str());
}

LibSVMClassifier::~LibSVMClassifier()
{
  svm_free_and_destroy_model(&_model);
}

// ____________________________________________________________________________
bool LibSVMClassifier::classifyFV(FeatureVector const & fv) const
{
  // ptime start(microsec_clock::local_time());
  LOG(TRACE) << "Using LibSVM to classify feature vector.\n";
  size_t size = fv.getFeatureVector().size();
  struct svm_node * x = reinterpret_cast<svm_node *> (malloc(sizeof(svm_node)
      * (size + 1)));
  std::map<int, double>::const_iterator it;
  int i = 0;
  LOG(TRACE) << "Feature vector size: " << fv.getSize() << "\n";
  for (it = fv.begin(); it != fv.end(); ++it)
  {
    x[i].value = it->second;
    x[i].index = it->first;
    LOG(TRACE)
      << "Feature vector index " << x[i].index << " value " << x[i].value
          << "\n";
    i++;
  }
  x[size].index = -1;
  double d = svm_predict(_model, x);
  LOG(TRACE)
    << "LibSVM returned " << d << "\n";

  free(x);

  // ptime end(microsec_clock::local_time());

  // time_duration classifying = end - start;
  // std::cout << "Classify: " << classifying.total_microseconds() << " µs.\n";

  if (d >= 0)
  {
    return true;
  }
  else
    return false;
}
}
