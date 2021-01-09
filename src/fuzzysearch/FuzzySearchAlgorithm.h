// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_FUZZYSEARCHALGORITHM_H_
#define FUZZYSEARCH_FUZZYSEARCHALGORITHM_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>

namespace FuzzySearch
{
using std::vector;
using std::string;
using std::wstring;

// const char ENCODINGS[2][15] = {"iso-8859-1\0", "utf-8\0"};

// abstract class for FuzzySearchAlgorithm which can work in two modes - word
// matching and completion matching (if possible)
template <class T>
class FuzzySearchAlgorithm
{
  public:
    // default constructor required!
    FuzzySearchAlgorithm() {}

    // Get k words that are close to the given query word, which itself may be,
    // but does not have to be in the lexicon.   // The k words are not
    // necessarily the k words with the smallest edit distance to the given
    // query word, but they are close to this set. The k words are returned as
    // a vector of word ids. Also returned is a flag saying whether the query
    // word was in the lexicon or not.
    virtual void findClosestWords(const T& query,
                          const vector<T>& vocabulary,
                          const vector<T>& indexedWords,
                          bool* queryIsInLexicon,
                          vector<int>* closestWordsIds,
                          vector<double>* distances) = 0;

    // build the fuzzy search index
    virtual void buildIndex(const vector<T>& vocabulary, bool reserved) = 0;

    // save the vocabulary and the index to disk
    virtual void saveDataStructureToFile(const string& filename) = 0;

    // load the vocabulary and the index from disk
    virtual void loadDataStructureFromFile(const string& filename,
                                   vector<T>* vocabulary) = 0;

    // get the type of the distance used.
    // error                                    -> -1
    // for edit distance that is not normalized -> 0
    // for normalized edit distance             -> 1
    // for extension distance that is not norm. -> 2
    // for normalized extension distance        -> 3
    // ...
    virtual int getDistanceType() = 0;

    // returns the threshold
    virtual double getThreshold() = 0;

    // checks if a word is in a vocabulary by binary search
    // note: vocabulary must be sorted!
    static bool inVocabulary(const T& word, const vector<T>& vocabulary)
    {
      if (vocabulary.size() == 0)
        return false;
      if (vocabulary.size() == 1)
        return (vocabulary[0] == word);
      int low  = 0;
      int high = vocabulary.size() - 1;
      int mid  = 0;

      if (vocabulary[0] < vocabulary[1])
      {
        // vocabulary in increasing order
        while (low <= high && vocabulary[mid] != word)
        {
          mid = (low + high) / 2;
          if (vocabulary[mid] < word)
            low = mid + 1;
          else
            high = mid - 1;
        }
        return vocabulary[mid] == word;
      }
      else
      {
        // vocabulary in decreasing order
        while (low <= high && vocabulary[mid] != word)
        {
          mid = (low + high) / 2;
          if (vocabulary[mid] > word)
            low = mid + 1;
          else
            high = mid - 1;
        }
        return vocabulary[mid] == word;
      }
    }

    // returns the mode used
    int getMode() { return _mode; }

    // returns true if the data structure does completion matching and false
    // if word matching
    bool completionMatching() { return _isCompletionDistanceUsed; }

    // virtual destructor
    virtual ~FuzzySearchAlgorithm() {}

  protected:
    // what kind of distance will ne used - plain edit distance, normalized
    // edit distance, extension distance etc.
    int _mode;

    // indicates whether the data structure is used for completion matching
    // or not (it's related to _mode!)
    bool _isCompletionDistanceUsed;
};
}

#endif  // FUZZYSEARCH_FUZZYSEARCHALGORITHM_H_
