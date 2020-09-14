// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREEXTRACTOR_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREEXTRACTOR_H_


#include <map>
#include <set>
#include <string>
#include <vector>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "decomposer-ml/FeatureVector.h"
#include "base/ContextDecomposerBase.h"
#include "sentence/Sentence.h"

using std::map;
using std::multimap;
using std::string;
using std::set;

namespace ad_decompose
{
// A class only to store configuration for the feature extractor.
class FeatureExtractorConfig
{
  public:
    // the words to consider, separated by comma, word sequences split by spaces
    string words;
    // word window left
    int wwLeft;
    // word window right
    int wwRight;
    // prefix for above features
    string pre_ww;

    // postag window left
    int pwLeft;
    // postag window right
    int pwRight;
    // prefix for above features
    string pre_pw;

    // chunk window left
    int cwLeft;
    // chunk window right
    int cwRight;
    // prefix for above features
    string pre_cw;

    // the types of br-tags to consider, space separated
    string brTypes;
    // br window left
    int bwLeft;
    // br window right
    int bwRight;
    // prefix for above features
    string pre_bw;

    // the type of dynamic br-tag to consider
    string dynbrType;
    // br window left
    int dynbrLeft;
    // br window right
    int dynbrRight;

    // count word types
    string countWTypes;
    // count pos types
    string countPTypes;
    // count chunk types
    string countCTypes;
    // prefix for above features
    string pre_count;


    // pattern word types
    string patternWTypes;
    // pattern pos types
    string patternPTypes;
    // pattern chunk types
    string patternCTypes;
    // prefix for pattern features
    string pre_pattern;
};

// A map mapping from a string to an integer with the ability
// to write to / load from a file.
class FeatureMap
{
  public:
    // Provide the backing file and whether the feature map shall
    // be generated. If it is generated (genFeatureMap is true)
    // nothing will be read from file.
    // If lockFeatureMap is true the map will only be loaded from the
    // provided. Else if the file exists it will be loaded and the
    // map will possibly be extended.
    FeatureMap(const string & fmFileName, bool lockFeatureMap);
    // Write the map to the file given at instantiation. If the map
    // is locked nothing is written, else the current contents are
    // written.
    void writeFeatureMap();
    // Lookup the index of the feature given as parameter. If no such
    // feature was found or could be added return -1, else return the
    // feature id. Will add a new feature if genFeatureMap is true.
    int getFeatureId(string const & featureString);
    std::map<int, std::string> const getReverseMap() const;
  private:
    // If the map is locked we do not add new entries.
    bool _lockFeatureMap;
    // Load the feature map from the provided file
    void loadFeatureMap();
    // The next index we assign to a feature.
    mutable int _nextIndex;
    // Filename of the feature map.
    string _fmFileName;
    // Feature map, mapping from a string to an index in the feature vector.
    typedef ad_utility::HashMap<string, int> dense_fmap;
    mutable dense_fmap _fMap;
};


// Extract a feature vector from a sentence
class FeatureExtractor
{
  public:
    // If seesAllBrTags is true all the currently set br tags will be
    // considered. Else the provided filters in the configuration
    // will be used.
    FeatureExtractor(FeatureMap * _fMap,
        FeatureExtractorConfig const & fexConfig,
        bool seesAllBrTags = false, bool processWordCounts = false);
    // Extract features for the word at pos.
    // You must call newSentence to set the sentence beforehand!!
    FeatureVector extractWordFeatures(size_t pos) const;
    // Extract features between start and end
    FeatureVector extractClauseFeatures(size_t start, size_t end) const;
    // Set the next sentence to extract features from. This will do some
    // preprocessing.
    void newSentence(Sentence<DefaultToken> const & sentence);
    // Update the newly classified brTag of a token at pos.
    // Also update the counts so that no new recomputation is necessary.
    void appendBrTag(size_t pos, string const & brTag);
  private:
    // A configuration object defining what features to extract
    FeatureExtractorConfig _fexConfig;
    // Add the word features to the passed feature vector.
    void extractWordFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add the pos features to the passed feature vector.
    void extractPosFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add the chunk features to the passed feature vector.
    void extractChunkFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add the br features to the passed feature vector.
    void extractBrFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add the dynamic features to the passed feature vector.
    void extractDynBrFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add the count features to the passed feature vector.
    void extractCountFeatures(FeatureVector * fv, size_t pos,
        string const & prefix = "") const;
    // Add a feature to the given vector. Does the lookup and
    // concatenation of prefix etc.
    void addFeatureToVector(string const & prefix, int pos, string ident,
        FeatureVector * fv) const;
    // Extract features of a pattern between start and end (including both)
    // The pattern uses | (pipe-symbol) as a delimiter between consecutive
    // tokens.
    void extractPatternFeatures(FeatureVector * fv, size_t start, size_t end,
        string const & prefix = "") const;

    // The FeatureMap
    FeatureMap * _fMap;

    // The current sentence we process
    Sentence<DefaultToken> const * _currentSentence;

    // A number of sets, that contain the elements to be counted for a feature.
    set<string> _countCTypes;
    set<string> _countPTags;
    set<string> _countWForms;
    set<string> _patternCTypes;
    set<string> _patternPTags;
    set<string> _patternWForms;
    set<string> _brTypes;
    multimap<string, vector<string> > _words;

    // A vector containing a map for each word, mapping from
    // an item to its current count:
    // "VP" -> 2
    // "," -> 0
    // etc. Indicating how often the item occured from the beginning
    // up to (and including) the current word.
    vector<map<string, int> > _countMaps;

    bool _seesAllBrTags;

    bool _processWordCounts;
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_FEATUREEXTRACTOR_H_
