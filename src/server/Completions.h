#ifndef __COMPLETIONS_H__
#define __COMPLETIONS_H__

#include "Globals.h"
#include "WordList.h"
#include "Vector.h"

using namespace std;

//! Class for storing the top-ranked words (completions) + scores for a query;
//! TODO: occ / doc counts are still part of completion string
class Completions : public vector<pair<string, Score> >
{
 public:

  //! Dump object to string.
  std::string asString(void) const;

  //! Set from list given word ids and vocabulary and scores; TODO: not timed
  template <typename vS, typename vC, typename vO> 
  void set(const Vocabulary& vocabulary, const WordList& wordIds, const vS& scores, const vC& docCounts, const vO& occCounts);

  //! Set to empty list (no completions)
  void clear() { vector<pair<string, Score> >::clear(); }

  //! Get the scores of the top-ranked words
  Vector<Score> getCompletionScores() const;
  
  //! True iff list is empty
  bool isEmpty() const { return size()==0; }

  //! Total number of bytes consumed (strings + scores); TODO: not up to date,
  //! e.g., occurrence counts are missing
  size_t sizeInBytes() const;
  
  //! Comparison for sort; for same score, lexicographical order decides
  class SortByScore;

  //! Sort by score; if \c k > 0, top-k only
  void sortByScore(unsigned int k = 0);

  //! Show completions with occurrence / document counts and scores
  void show(unsigned int maxNumItemsToShowPerList = 10) const;

  //    typedef pair<string, string> Completion;
  // vector<Completion> _completions;

    /*
    //! return i-th completion as string of the form  mehlhorn  (179)
    string operator[](unsigned int i) { assert( i < _completions.size()); return _completions[i].first + " (" + _completions[i].second + ")"; }

    //! return word of i-th completion
    string wordAt(unsigned int i) { assert( i < _completions.size() ); return _completions[i].first; }
    
    //! return frequency of i-th completion
    unsigned int freqAt(unsigned int i) { assert( i < _completions.size() ); return atoi(_completions[i].second.c_str()); }

    //! number of completions
    unsigned int size() const { return _completions.size(); }

    //! clear 
    void clear() { _completions.clear(); }

    //! push word and frequency
    void push_back(string word, unsigned int freq) { ostringstream os; os << freq; _completions.push_back( Completion(word, os.str() ) ); }

    //! push word and some other string (needed in NaiveCompleter::mergeCompletions)
    void push_back(string word, string info) { _completions.push_back( Completion(word, info) ); }

    //! remove last completion (needed in NaiveCompleter::completionsForPrefix)
    void pop_back() { _completions.pop_back(); }

    //! append completions 
    void append(Completions& completions) { _completions.insert(_completions.end(), completions._completions.begin(), completions._completions.end()); }

    //! sort completions by frequency 
    //  NOTE: fancy second strings like the 12|34|12 produced by NaiveCompleter::completionsForQueryEnhanced are treated as 0 by this sort
    class CompareByFreq 
    { 
      public: bool operator()(Completion& x, Completion& y) 
              { 
                static unsigned int xFreq, yFreq;
                xFreq = atoi(x.second.c_str());
                yFreq = atoi(y.second.c_str()); 
                return xFreq > yFreq || (xFreq == yFreq && x.first.compare(y.first));
              }
    };
    void sortByFreq(unsigned int k = UINT_MAX) 
    { 
      unsigned int kk = k < _completions.size() ? k : _completions.size();
      partial_sort( _completions.begin(), _completions.begin() + kk, _completions.end(), CompareByFreq() ); 
    }
    */

}; // end of class Completions

#endif
