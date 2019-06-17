// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "base/CompositeContextDecomposer.h"
#include "util/ContextDecomposerUtil.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::pair;
namespace ad_decompose
{
// ____________________________________________________________________________
template <class Token>
CompositeContextDecomposer<Token>::CompositeContextDecomposer()
:_marker(NULL), _extractor(NULL), _nIgnoredSentences(0), _nTotalSentences(0)
{
}

// ____________________________________________________________________________
template <class Token>
CompositeContextDecomposer<Token>::~CompositeContextDecomposer()
{
  LOG(INFO) << "CompositeDecomposer decomposed " << _nTotalSentences
      << " sentences and"
      << " ignored " << _nIgnoredSentences << "." << std::endl;
}

// ____________________________________________________________________________
template <class Token>
void CompositeContextDecomposer<Token>::setContextMarker(
    ContextMarkerBase<Token> * marker)
{
  _marker = marker;
}

// ____________________________________________________________________________
template <class Token>
void CompositeContextDecomposer<Token>::setContextExtractor(
    ContextRecombinerBase<Token> * extractor)
{
  _extractor = extractor;
}

// ____________________________________________________________________________
template <class Token>
Contexts<Token> const CompositeContextDecomposer<Token>::decomposeToPtr(
    Sentence<Token> & sentence)
{
  assert(_marker != NULL);
  assert(_extractor != NULL);

  if (sentence.getWords().size() <= MAX_DECOMPOSER_SENTENCE_SIZE)
  {
    ++_nTotalSentences;
    _marker->markSentence(&sentence);
    return _extractor->extractContextsPtr(sentence);
  }
  else
  {
    LOG(INFO)
      << "CompositeDecomposer not decomposing sentence with "
      << sentence.getWords().size() << " words.\n";
    ++_nIgnoredSentences;
    Contexts<Token> result;
    Context<Token> resultContext;
    vector <Token *> const & words = sentence.getWords();
    for (size_t i = 0; i < words.size(); ++i)
      resultContext.push_back(words[i]);
    result.addContext(resultContext);
    return result;
  }
}

template class CompositeContextDecomposer<DefaultToken>;
template class CompositeContextDecomposer<DeepToken>;
}

