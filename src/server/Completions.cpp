#include "Completions.h"
#include "ConcurrentLog.h"

//! Set from list given word ids and vocabulary and scores; TODO: not timed
template <typename vS, typename vC, typename vO> 
void Completions::set(const Vocabulary&     vocabulary, 
                      const WordList&       wordIds, 
                      const vS&             scores, 
                      const vC&             docCounts,  
                      const vO&             occCounts)
{
  //LOG << AT_BEGINNING_OF_METHOD << "; number of completions is " << wordIds.size() << endl;
  CS_ASSERT_EQ(wordIds.size(), scores.size());
  CS_ASSERT_EQ(wordIds.size(), docCounts.size());
  CS_ASSERT_EQ(wordIds.size(), occCounts.size());
  resize(wordIds.size());
  WordId currentWordId;
  for(unsigned int i = 0; i < wordIds.size(); ++i)
  {
    ostringstream os; 
      //assert(occCounts[i] >= docCounts[i]);
    currentWordId = wordIds[i];
    CS_ASSERT_GE(currentWordId, 0);
    CS_ASSERT_LT(currentWordId, (int)(vocabulary.size()));
    os << vocabulary[wordIds[i]] << " (" << occCounts[i] << "/" << docCounts[i] << ")";
    operator[](i).first = os.str();
    operator[](i).second = scores[i];
  }
  //LOG << AT_END_OF_METHOD << endl;
}

//! Total number of bytes consumed (strings + scores); TODO: not up to date,
//! e.g., occurrence counts are missing
size_t Completions::sizeInBytes() const
{
  size_t sizeOfStrings = 0;
  for(unsigned int i = 0; i < size(); ++i) 
  {
    sizeOfStrings += operator[](i).first.size();
  }
  return (sizeOfStrings + size()*sizeof(Score));
}


//! Get the scores of the top-ranked words
Vector<Score> Completions::getCompletionScores() const
{
  Vector<Score> scores;
  scores.resize(size());
  for(unsigned long i=0; i<size(); i++)
  {
    scores[i] = operator[](i).second;
    assert(scores[i] > 0);
  }
  return scores;
}


//! Comparison for sort; for same score, lexicographical order decides; TODO: needed anywhere?
/*
class Completions::SortByScore
{
  public: 
    bool operator()(const pair<string,Score>& x, const pair<string,Score>& y) const
    { 
      return x.second > y.second || (x.second == y.second && x.first < y.first); 
    }
};
*/


//! Sort by score; if \c k > 0, top-k only; TODO: needed anywhere?
/*
void Completions::sortByScore(unsigned int k)
{
  if((size() == 0)||(size() == 1)) return;
  if (k == 0 or k > size()) k = size();
  partial_sort(begin(), begin()+k, end(), SortByScore());
}
*/

//! Show completions with occurrence / document counts and scores
void Completions::show(unsigned int maxNumItemsToShowPerList) const
{
  for(unsigned long i = 0; i < maxNumItemsToShowPerList && i < size(); i++)
  {
    cout << operator[](i).first << " [" << operator[](i).second << "] ";
  }
};

std::string Completions::asString(void) const
{
  std::stringstream ss;
  vector<pair<string, Score> >::const_iterator it;
  
  ss << "[";
  for (it = this->begin(); it != this->end(); ++it)
  {
    ss << "(" << (*it).first << "," << (*it).second << "), ";
  }
  ss << "]";
  return ss.str();
}

// EXPLICIT INSTANTIATIONS
template void Completions::set< Vector<Score>, Vector<unsigned int>, Vector<unsigned int> >
(const Vocabulary&           vocabulary, 
 const WordList&             wordIds, 
 const Vector<Score>&        scores, 
 const Vector<unsigned int>& docCounts, 
 const Vector<unsigned int>& occCounts);

