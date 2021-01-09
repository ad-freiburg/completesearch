// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#ifndef SERVER_CUSTOMSCORER_H_
#define SERVER_CUSTOMSCORER_H_

#include "server/Globals.h"
#include "server/Vocabulary.h"
#include <unordered_map>

// Class for changing scores of a subset of postings at runtime, that is,
// without the need for rebuilding the index from scratch.
// NOTE: This is used in the following way. After a block is read from disk, a
// scan over the block is made, and the scores of all postings in the mentioned
// subset will be overwritten. This will not affect efficiency as long as the
// subset is reasonably small.
class CustomScorer
{
 public:
  // Constructor. Needs vocabulary, so that it can assign words to word ids.
  CustomScorer(const Vocabulary& vocabulary);

  // Read substitute scores from given file with lines in the following format:
  // <type><TAB><word or doc id><TAB><new score>
  // where <type> is one of W or D (saying whether the line is about a word or
  // about a document), <word or doc id> is the word or the doc id for which we
  // want to change the score, and <new score> is the new score for all matching
  // postings.
  // TODO(bast): Defined what it means that a posting "matches".
  // TODO(bast): Currently does not actually read the file but stores scores for
  // a hard-coded set of words.
  void readCustomScores(const string& fileName);

  // For given word id, either return custom score or the given disk score. This
  // is implemented below, so that we can inline this function.
  inline Score getScore(WordId wordId, DiskScore);

 private:
  // Reference to vocabulary;
  const Vocabulary& _vocabulary;

  // New Scores for a (small) subset of word ids.
  std::unordered_map<WordId, Score> _newWordScores;
};

// _____________________________________________________________________________
Score CustomScorer::getScore(WordId wordId, DiskScore diskScore)
{
  auto it = _newWordScores.find(wordId);
  return it == _newWordScores.end() ? diskScore : diskScore * it->second;
}

// Whoever includes this should be able to use the global CustomScorer object
// declared in the .cpp file.
extern CustomScorer* globalCustomScorer;

#endif  // SERVER_CUSTOMSCORER_H_
