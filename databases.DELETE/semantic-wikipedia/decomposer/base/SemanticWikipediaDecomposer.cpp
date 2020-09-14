// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Hannah Bast <bast>, Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "base/SemanticWikipediaDecomposer.h"
#include "base/SemanticWikipediaWriter.h"
#include "util/ContextDecomposerUtil.h"
#include "base/SemanticWikipediaReader.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;

namespace ad_decompose
{
// ____________________________________________________________________________
template <class Token>
SemanticWikipediaDecomposer<Token>::SemanticWikipediaDecomposer(
    bool writeToStdOut, ContextDecomposerBase<Token> & decomposer,
    ContextWriterBase<Token> const & writer,
    SemanticWikipediaReader<Token> & reader)
: _verbose(writeToStdOut),
  _decomposer(decomposer), _writer(writer), _reader(reader)
{
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaDecomposer<Token>::parseAndChunk()
{
  LOG(INFO) << "Decomposing..." << std::endl;
  std::string line;
  Sentence<Token> * sentence = new Sentence<Token>();
  while (_reader.parseNextSentence(sentence))
  {
    // If the sentence is empty continue. This can
    // happen because in the index a sentence can consist
    // only of ct: words which we ignore for now.
    if (sentence->getWords().size() == 0)
    // No need to delete the sentence - it can be reused.
    continue;
    if (sentence->getWords().size() > MAX_SENTENCE_SIZE)
    {
      LOG(INFO)
      << "SemanticWikipediaDecomposer skipping long sentence with "
          << sentence->getWords().size() << " words.\n";
      _writer.writeLongSentence(*sentence);
      delete sentence;
      sentence = new Sentence<Token>();
      continue;
    }
    if (_verbose)
    {
      LOG(INFO)
      << sentence->asStringWithPhrases();
    }
    Contexts<Token> contexts = _decomposer.decomposeToPtr(*sentence);
    if (_verbose)
    {
      LOG(INFO)
      << "Resulting contexts: " << std::endl
          << contexts.asString() << std::endl;
    }
    _writer.writeContexts(contexts, *sentence);
    delete sentence;
    sentence = new Sentence<Token>();
  }
  // Delete the very last sentence.
  delete sentence;
}

template class SemanticWikipediaDecomposer<DefaultToken>;
template class SemanticWikipediaDecomposer<DeepToken>;
}
