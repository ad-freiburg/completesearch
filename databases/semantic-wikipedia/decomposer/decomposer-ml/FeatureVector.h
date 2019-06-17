// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREVECTOR_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREVECTOR_H_

#include <vector>
#include <map>

using std::map;
namespace ad_decompose
{
// A feature vector for machine learning tasks.
class FeatureVector
{
  public:
    void addFeature(int fIndex, double value);
    map<int, double> const & getFeatureVector() const;
    size_t getSize() const;
    map<int, double>::const_iterator begin() const {return _fv.begin();}
    map<int, double>::const_iterator end() const {return _fv.end();}
  private:
    map<int, double> _fv;
};

// A machine learning example: a class associated with
// a feature vector.
class MLExample
{
  public:
    MLExample(double dClass, FeatureVector fv) :
      _class(dClass), _fv(fv)
    {};
    // Return the class.
    double getClass() {return _class;}
    // Return the feature fector.
    FeatureVector getFeatureVector() {return _fv;}
    // Return a const_iterator of the feature vector.
    map<int, double>::const_iterator begin() {return _fv.begin();}
    // Return a const_iterator of the feature vector.
    map<int, double>::const_iterator end() {return _fv.end();}
  private:
    double _class;
    FeatureVector _fv;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREVECTOR_H_
