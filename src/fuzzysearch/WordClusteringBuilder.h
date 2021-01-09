// Copyright [2010] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_WORDCLUSTERINGBUILDER_H_
#define FUZZYSEARCH_WORDCLUSTERINGBUILDER_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include  <unordered_map>
#include  <unordered_set>
// #include <hash_map>
// #include <hash_set>
// #include <ext/hash_map>
// #include <ext/hash_set>

#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/FastSS.h"
#include "../fuzzysearch/PermutedLexicon.h"
#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{
const char ENCODINGS[2][15] = {"iso-8859-1\0", "utf-8\0"};

template<class T>
class WordClusteringBuilder
{
  public:

  static enum { ISO_8859_1 = 0, UTF_8 = 1 } encoding;

  // build clusters for fuzzy word matching
  void buildWordClusteringOld(const vector<T>& vocabulary,
                           const vector<T>& clusterCenters,
                           vector<int>& frequencies,
                           FuzzySearchAlgorithm<T>& alg,
                           int maxNofClustersPerWord,
                           bool clusterTheCentroids,
                           vector<vector<int> >* clusters,
                           vector<int>* unclusteredWords);

  // build clusters for fuzzy word matching
  void buildWordClustering(const vector<T>& vocabulary,
                           const vector<T>& clusterCenters,
                           vector<int>& frequencies,
                           FuzzySearchAlgorithm<T>& alg,
                           int maxNofClustersPerWord,
                           bool clusterTheCentroids,
                           vector<vector<int> >* clusters,
                           vector<int>* unclusteredWords);

  // build clustering for the frequent words only
  void buildWordClusteringFrequentWords(
                          const vector<T>& vocabulary,
                          vector<int>& frequencies,
                          int maxNofClustersPerWord,
                          int minFrequency,
                          vector<T>* clusterCenters,
                          vector<vector<int> >* clusters,
                          vector<int>* unclusteredWords);

  // build clusters for fuzzy completion matching
  void buildCompletionClustering(
             const vector<T>& vocabulary,
             const vector<T>& clusterCenters,
             vector<int>& frequencies,
             bool clusterFrequentWords,
             int prefixLength,
             int maxNofClustersPerWord,
             int minFrequency,
             double distance,
             vector<vector<int> >* clusters,
             vector<int>* unclusteredWords,
             vector<int>* unclusteredFreq);

  // build clusters for fuzzy completion matching
  void buildCompletionClustering(
             const vector<T>& vocabulary,
             const vector<vector<T> >& clusterCenters,
             vector<int>& frequencies,
             bool clusterFrequentWords,
             int prefixLength,
             int maxNofClustersPerWord,
             int minFrequency,
             double distance,
             vector<vector<int> >* clusters,
             vector<int>* unclusteredWords,
             vector<int>* unclusteredFreq);

  // build clusters for fuzzy completion matching
  void buildCompletionClustering(
                   const vector<T>& vocabulary,
                   const vector<T>& clusterCenters,
                   vector<int>& frequencies,
                   FuzzySearchAlgorithm<T>& fsAlg,
                   int prefixLength,
                   int maxNofClustersPerWord,
                   bool clusterTheCentroids,
                   double maxDistance,
                   int minFrequency,
                   vector<vector<int> >* clusters,
                   vector<int>* unclusteredWords,
                   vector<int>* unclusteredFreq);

    // Read the vocabulary from disk
    static void readVocabularyFromDisk(const string& filename,
                                       bool filterGarbage,
                                       int maxNofWords,
                                       bool sort,
                                       vector<T>* vocabulary);

    // read vocabulary together with a separate file for the frequencies
    static int readVocabularyFromDisk(const string& vocFilename,
                                       bool filterGarbage,
                                       int maxNofWords,
                                       vector<T>* vocabulary,
                                       vector<int>* frequencies);

    // returns true if the fuzzy searcg alg. does completion matching and false
    // if word matching
    bool completionMatching()
    {
      if (_fuzzySearchAlgorithm == NULL)
        return false;
      return _fuzzySearchAlgorithm->completionMatching();
    }

    // Set the character econding (0 - ISO-8859-1, 1 - UTF-8)
    void setEncoding(int encoding) { _encoding = encoding; }

    // Get the character econding (0 - ISO-8859-1, 1 - UTF-8)
    int getEncoding() { return _encoding; }

    // Pick cluster center from the vocabulary of words
    void pickClusterCenters(const vector<T>& vocabulary,
        const vector<int>& correctWords,
        FuzzySearchAlgorithm<T>& fsAlg,
        double coverage, vector<T>*
        clusterCenters);

    // Pick cluster center from a list ordered w.r.t. word freq.
    // in ascending order
    void pickClusterCenters(const vector<T>& vocabulary,
                            FuzzySearchAlgorithm<T>& fsAlg,
                            double coverage,
                            int cutOff,
                            vector<T>* clusterCenters,
                            vector<int>* unclusteredWords);


    // a modification of the above method that should work faster
    // and achieve more or less the same (should)
    void pickClusterCenters(const vector<T>& vocabulary,
                       const vector<int>& frequencies,
                       FuzzySearchAlgorithm<T>& fsAlg,
                       int minFrequency,
                       int cutOff,
                       vector<T>* clusterCenters);

    // Pick cluster center from a list ordered w.r.t. word freq.
    // stop after a limit is reached (do not consider covering)
    void pickClusterCenters(const vector<T>& vocabulary,
                            const vector<int>& frequencies,
                            int minFrequency,
                            vector<T>* clusterCenters,
                            vector<int>* freq);

    // Pick cluster center from a list ordered w.r.t. word freq.
    // stop after a limit is reached (do not consider covering)
    // used for fuzzycompletions, experimental
    void pickClusterCentersCompl(
                            const vector<T>& vocabulary,
                            const vector<int>& frequencies,
                            int minFrequency,
                            int prefixLen,
                            bool prefixLenStrict,
                            vector<T>* clusterCenters);

    // Pick cluster center from a list ordered w.r.t. word freq.
    // Cluster centroids are words of the kind al*gor, i.e.
    // prefixes with don't cares
    void pickClusterCentersComplStar(
                            const vector<T>& vocabulary,
                            const vector<int>& frequencies,
                            size_t minFrequency,
                            size_t prefixLength,
                            int nofErrors,
                            bool flag,
                            vector<vector<T> >* clusterCenters,
                            vector<T>* newVoca = NULL,
                            vector<int>* newFreq = NULL);

    // Pick cluster center from a list ordered w.r.t. word freq.
    // Cluster centroids are words of the kind al*gor, i.e.
    // prefixes with don't cares
    void chopWords(const vector<T>& vocabulary,
                   size_t prefixLength,
                   int nofErrors,
                   vector<T>* choppedWords);

    // experimental
    void processUnclusteredCompl(
                            const vector<T>& vocabulary,
                            FuzzySearchAlgorithm<T>& fsAlg,
                            int prefixLen,
                            vector<T>* clusterCenters,
                            vector<vector<int> >* clustersRareWords);

    // sort the two arrays in parallel
    static void sortParallel(vector<T>* vocabulary,
                             vector<int>* freq,
                             int col,
                             bool reverse = false);

    // sort the two arrays in parallel
    static void sortParallel(vector<int>* vocabulary,
                             vector<int>* freq);

    // normalize the given word (e.g. ck -> k, y -> i etc.)
    static T normalizeWord(const T& word);

    // normalize each word in the vocabulary
    static void normalizeVocabulary(const vector<T>& vocabulary,
                                    vector<T>* normalizedVocabulary);

    // returns true if normalization is used and false otherwise
    bool usingNormalization()
    {
      return _useNormalization;
    }

    // setter function for word normalization
    void useNormalization(bool use)
    {
      _useNormalization = use;
    }

    // write the produced clusters to a given file
    static void writeClustersToFile(const vector<vector<int> >& clusters,
                             const vector<T>& vocabulary,
                             bool writeSingletons,
                             const string& filename);

    // write the produced clusters to a given file
    static void writeClustersToFile(const vector<T>& clusteredWords,
                             const vector<int>& clusterIds,
                             const string& filename);

    // if more clusters are to be added later, append to an
    // existing file
    static void appendClustersToFile(const vector<vector<int> >& clusters,
                              const vector<T>& vocabulary,
                              bool writeSingletons,
                              int startingId,
                              const string& filename);

    // just write vocabulary to file (used for debug.)
    static void writeVocabularyToFile(const vector<T>& vocabulary,
                                      const vector<int>& freq,
                                      const string& filename);

    // return true if the wordId corresp. to word that is centroid
    bool isClusterCentroid(size_t wordId)
    {
      if (wordId >= _isClusterCenter.size())
        return false;
      return _isClusterCenter[wordId];
    }

    // read correct words needed for clustering when frequencies are not
    // available
    void getCorrectWordIds(const string& dictFileName,
        const vector<T>& vocabulary,
        vector<int>* correctWordIds);

    // read one string from a ifstream
    static void readLine(fstream* f, T* line, bool toLower);

    // read one line/string from a FILE*
    static void readLine(FILE* f, T* line, bool toLower);

    // print a string/wstring on the screen
    static void printStr(const T& str);

    // convert string/wstring to string
    static void toString(const T& strx, string* str);

    // the sum of the frequencies of all the words in the voca.
    // TODO(celikik): there will be a problem if multiple instances are used
    // in the same program (the program will display wrong space overhead)
    static size_t totalNumberOfOccurences;

  private:

    // populate the stop-words
    static void getStopWords(
        std::unordered_map<string, bool, StringHashFunction>* hashMap);

    // true if word seems to be garbled and false otherwise
    static bool isWordGarbage(const char* word);

    // Returns true if the character is vowel (english) and false otherwise
    static bool isVowel(unsigned char c)
    {
      if (c > 127)
        return true;
      static const unsigned char v[] = {'a', 'e', 'i', 'o', 'u', 'h', 'y', 'r'};
      for (uint32_t i = 0; i < sizeof(v); i++)
        if (c == v[i])
          return true;
      return false;
    }

    // add stars recursively (needed for cluster centroids of the form
    // al*gor, alg*or, ...
    void addBlocksSubs(const T& str,
                       uint16_t beg,
                       int depth,
                       int maxDepth,
                       std::unordered_set<T, StringHash<T> >& h,
                       vector<T>* centroids);

    // the used algorithm for fuzzy search
    FuzzySearchAlgorithm<T>* _fuzzySearchAlgorithm;

    // true if the i-th word from the vocabulary is a correct word
    vector<bool> _isClusterCenter;

    // the current encoding (iso/utf8)
    int _encoding;

    // indicates whether the clusters should be built over normalized vocabulary
    // or not
    bool _useNormalization;

    // needed for speeding up buildClusters - if pickClusters is used then no
    // need to search words that have been not picked at all
    vector<bool> _pickedWords;

    // needed for buildClusters - min freq. to be considered for the frequency
    // grpups in buildClusters; TODO(celikik): is this really needed?
    int _minimumFrequency;
};
}

#endif  // FUZZYSEARCH_WORDCLUSTERINGBUILDER_H_
