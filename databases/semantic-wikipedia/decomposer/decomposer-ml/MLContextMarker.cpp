// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <ext/hash_set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "util/ContextDecomposerUtil.h"
#include "./MLContextMarker.h"
#include "./MLInference.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"

// #include "boost/date_time/posix_time/posix_time.hpp"
// include all types plus i/o
// using namespace boost::posix_time;


using std::pair;
using std::string;
using std::vector;
using std::make_pair;

namespace ad_decompose
{
// ____________________________________________________________________________
MLContextMarker::MLContextMarker(bool writeToStdOut, bool onlyPhraseBoundaries,
    string wordClassifiers, string wordOpenClassifiers,
    string wordCloseClassifiers, string clauseClassifiers,
    string wordFeatureMapFile, string clauseFeatureMapFile,
    string featureConfig) :
  _writeToStdOut(writeToStdOut), _onlyPhraseBoundaries(onlyPhraseBoundaries)
{
  std::vector<string> wordClassifiersSplit;
  boost::split(wordClassifiersSplit, wordClassifiers, boost::is_any_of(","));
  std::vector<string> clauseClassifiersSplit;
  boost::split(clauseClassifiersSplit, clauseClassifiers,
      boost::is_any_of(","));
  std::vector<string> openClassifiersSplit;
  boost::split(openClassifiersSplit, wordOpenClassifiers,
      boost::is_any_of(","));
  std::vector<string> closeClassifiersSplit;
  boost::split(closeClassifiersSplit, wordCloseClassifiers, boost::is_any_of(
      ","));

  // Initialize all classifiers.
  for (size_t i = 0; i < wordClassifiersSplit.size(); ++i)
  {
    SVMClassifier * svm = new LibSVMClassifier(wordClassifiersSplit[i]);
    _wordClassifiers.push_back(make_pair(wordClassifiersSplit[i + 1], svm));
    ++i;
  }
  for (size_t i = 0; i < clauseClassifiersSplit.size(); ++i)
  {
    _clauseClassifiers[clauseClassifiersSplit[i + 1]] = new LibSVMClassifier(
        clauseClassifiersSplit[i]);
    ++i;
  }
  for (size_t i = 0; i < openClassifiersSplit.size(); ++i)
  {
    SVMClassifier * svm = new LibSVMClassifier(openClassifiersSplit[i]);
    _wordOpenClassifiers.push_back(make_pair(openClassifiersSplit[i + 1], svm));
    ++i;
  }
  for (size_t i = 0; i < closeClassifiersSplit.size(); ++i)
  {
    SVMClassifier * svm = new LibSVMClassifier(closeClassifiersSplit[i]);
    _wordCloseClassifiers.push_back(
        make_pair(closeClassifiersSplit[i + 1], svm));
    ++i;
  }

  // Initialize feature config.
  FeatureExtractorConfig config = ContextDecomposerUtil::parseFeatureConfig(
      featureConfig);
  config.dynbrLeft = 0;
  config.dynbrRight = 0;

  // Get the feature maps.
  _mfMap = new FeatureMap(wordFeatureMapFile, true);
  _cfMap = new FeatureMap(clauseFeatureMapFile, true);

  // Initialize the feature extractors.
  _wordFex = new FeatureExtractor(_mfMap, config, true);
  _clauseFex = new FeatureExtractor(_cfMap, config, true);

  if (_writeToStdOut)
  {
    std::cout << "Marking only at phrase boundaries\n";
  }

  // TODO(elmar): We currently only support marking on phrase boundaries.
  // This is still a restriction in the ContextExtractor class where we
  // assume for syntactic chunks that only a phrase start can open a clause
  // and only a phrase end can close a clause.
  assert(onlyPhraseBoundaries);
}

// ____________________________________________________________________________
MLContextMarker::~MLContextMarker()
{
  // Clean up the mess.
  ad_utility::HashMap<string, SVMClassifier *>::iterator it;
  for (it = _clauseClassifiers.begin(); it != _clauseClassifiers.end(); ++it)
    delete it->second;
  for (size_t i = 0; i < _wordClassifiers.size(); ++i)
    delete _wordClassifiers[i].second;
  for (size_t i = 0; i < _wordOpenClassifiers.size(); ++i)
    delete _wordOpenClassifiers[i].second;
  for (size_t i = 0; i < _wordCloseClassifiers.size(); ++i)
    delete _wordCloseClassifiers[i].second;
  delete _wordFex;
  delete _clauseFex;
  delete _mfMap;
  delete _cfMap;
}

// ____________________________________________________________________________
void MLContextMarker::markSentence(Sentence<DefaultToken> * sentence)
{
  _wordFex->newSentence(*sentence);
  _clauseFex->newSentence(*sentence);

  if (_writeToStdOut)
  {
    std::cout << "Before marking: " << std::endl;
    printMarkedWords(*sentence);
  }
  // ptime start(microsec_clock::local_time());
  if (_onlyPhraseBoundaries)
    markSentenceAtPhraseBoundaries(sentence);
  else
    markSentenceAtWords(sentence);
  // ptime mark(microsec_clock::local_time());
  if (_writeToStdOut)
  {
    std::cout << "After marking: " << std::endl;
    printMarkedWords(*sentence);
  }
  correctBracketing(sentence);
  // ptime infer(microsec_clock::local_time());
  // time_duration marking = mark - start;
  // time_duration inferring =  infer - mark;
  // std::cout << "Marking: " << marking.total_microseconds() << " µs.\n";
  // std::cout << "Inferring: " << inferring.total_microseconds() << " µs.\n";

  if (_writeToStdOut)
  {
    std::cout << "After inferring: " << std::endl;
    printMarkedWords(*sentence);
  }
}

// ____________________________________________________________________________
void MLContextMarker::correctBracketing(Sentence<DefaultToken> * sentence) const
{
  vector<DefaultToken *> const & words = sentence->getWords();
  // TODO(elmar): make this generic, and ideally independent of the exact
  // type of brackets used.
  std::vector<size_t> rel_open;
  std::vector<size_t> rel_close;
  std::vector<size_t> lit_open;
  std::vector<size_t> lit_close;
  std::vector<size_t> rela_open;
  std::vector<size_t> rela_close;
  size_t nextStart = 0;
  for (size_t i = 0; i < words.size(); ++i)
  {
    for (size_t j = 0; j < words[i]->marks.size(); ++j)
    {
      switch (words[i]->marks[j])
      {
        case(DefaultTokenMarks::REL_OPEN):
          rel_open.push_back(i);
          break;
        case(DefaultTokenMarks::REL_CLOSE):
          rel_close.push_back(i);
          break;
        case(DefaultTokenMarks::RELA_OPEN):
          rela_open.push_back(i);
          break;
        case(DefaultTokenMarks::RELA_CLOSE):
          rela_close.push_back(i);
          break;
        case(DefaultTokenMarks::LIT_OPEN):
          lit_open.push_back(i);
          break;
        case(DefaultTokenMarks::LIT_CLOSE):
          lit_close.push_back(i);
          break;
          // For each SEP we see perform the inference for the up to now seen
          // suggestions.
        case(DefaultTokenMarks::SEP):
          for (size_t k = nextStart; k < i; ++k)
          {
            sentence->clearBrTag(k);
          }

          // All opened brackets so far can close directly before that
          // if they do not close anyways.
          if (rel_close.empty() || rel_close.back() != (i - 1))
            rel_close.push_back(i - 1);
          if (rela_close.empty() || rela_close.back() != (i - 1))
            rela_close.push_back(i - 1);
          if (lit_close.empty() || lit_close.back() != (i - 1))
            lit_close.push_back(i - 1);

          inferBracketing(sentence, rel_open, rel_close, lit_open, lit_close,
              rela_open, rela_close);
          rel_open.clear();
          rel_close.clear();
          lit_open.clear();
          lit_close.clear();
          rela_open.clear();
          rela_close.clear();
          nextStart = i + 1;
          break;
        default:
          break;
      }
    }
  }
  // For the remaining part perform the inference for the up to now seen
  // suggestions.
  if (nextStart != words.size())
  {
    // Clear up to the current position.
    for (size_t k = nextStart; k < words.size(); ++k)
    {
      sentence->clearBrTag(k);
    }

    // All opened brackets so far can close directly before that
    // if they do not close anyways.
    size_t i = words.size();
    if (rel_close.empty() || rel_close.back() != (i - 1))
      rel_close.push_back(i - 1);
    if (rela_close.empty() || rela_close.back() != (i - 1))
      rela_close.push_back(i - 1);
    if (lit_close.empty() || lit_close.back() != (i - 1))
      lit_close.push_back(i - 1);

    inferBracketing(sentence, rel_open, rel_close, lit_open, lit_close,
        rela_open, rela_close);
  }
}

// ____________________________________________________________________________
void MLContextMarker::inferBracketing(Sentence<DefaultToken> * sentence,
    std::vector<size_t> const & rel_open,
    std::vector<size_t> const & rel_close,
    std::vector<size_t> const & lit_open,
    std::vector<size_t> const & lit_close,
    std::vector<size_t> const & rela_open,
    std::vector<size_t> const & rela_close) const
{
  // Construct all pairs of possible opens and closes of the same type.
  std::vector<MLInference::Vertex> vertices;
  std::vector<MLInference::Vertex> result;

  for (size_t i = 0; i < rel_open.size(); ++i)
  {
    for (size_t j = 0; j < rel_close.size(); ++j)
    {
      // Only if start <= end.
      if (rel_open[i] <= rel_close[j])
      {
        MLInference::Vertex v;
        v.start = rel_open[i];
        v.end = rel_close[j];
        // Match clause.
        if (_clauseClassifiers["REL"]->classifyFV(
            _clauseFex->extractClauseFeatures(v.start, v.end)))
        {
          if (_writeToStdOut)
          {
            std::cout << "REL true for " << v.start << "-" << v.end
                << std::endl;
          }
          v.weight = 2;
        }
        else
          v.weight = 0.1;
        v.type = "REL";
        vertices.push_back(v);
      }
    }
  }
  for (size_t i = 0; i < rela_open.size(); ++i)
  {
    for (size_t j = 0; j < rela_close.size(); ++j)
    {
      if (rela_open[i] <= rela_close[j])
      {
        MLInference::Vertex v;
        v.start = rela_open[i];
        v.end = rela_close[j];
        if (_clauseClassifiers["RELA"]->classifyFV(
            _clauseFex->extractClauseFeatures(v.start, v.end)))
        {
          if (_writeToStdOut)
          {
            std::cout << "RELA true for " << v.start << "-" << v.end
                << std::endl;
          }
          v.weight = 2;
        }
        else
          v.weight = 0.1;
        v.type = "RELA";
        vertices.push_back(v);
      }
    }
  }
  for (size_t i = 0; i < lit_open.size(); ++i)
  {
    for (size_t j = 0; j < lit_close.size(); ++j)
    {
      if (lit_open[i] <= lit_close[j])
      {
        MLInference::Vertex v;
        v.start = lit_open[i];
        v.end = lit_close[j];

        if (_clauseClassifiers["LIT"]->classifyFV(
            _clauseFex->extractClauseFeatures(v.start, v.end)))
        {
          if (_writeToStdOut)
          {
            std::cout << "LIT true for " << v.start << "-" << v.end
                << std::endl;
          }
          v.weight = 2;
        }
        else
          v.weight = 0.1;
        v.type = "LIT";
        vertices.push_back(v);
      }
    }
  }

  if (_writeToStdOut)
    std::cout << "Constructing and solving graph with " << vertices.size()
        << " vertices\n";
  if (vertices.size() <= MAX_VERTICES)
    result = _inference.constructAndSolveGraph(vertices);
  else
  {
    if (_writeToStdOut)
      std::cout << "Ignoring because num_vertices > MAX_VERTICES.\n";
  }
  // std::cout << "After inferring " << result.size() << " brackets remain\n";

  // TODO(elmar): should make sure we obey the order of constituent starts.
  // Hint: Longer constituents start before shorter constituents.
  for (size_t i = 0; i < result.size(); ++i)
  {
    if (result[i].type == "LIT")
    {
      sentence->appendBrTag(result[i].start, "LIT(");
      sentence->appendBrTag(result[i].end, "LIT)");
    }
    else if (result[i].type == "REL")
    {
      sentence->appendBrTag(result[i].start, "REL(");
      sentence->appendBrTag(result[i].end, "REL)");
    }
    else if (result[i].type == "RELA")
    {
      sentence->appendBrTag(result[i].start, "RELA(");
      sentence->appendBrTag(result[i].end, "RELA)");
    }
  }
}

// ____________________________________________________________________________
void MLContextMarker::classifyAllWords(Sentence<DefaultToken> * sentence,
    SVMClassifier const & classifier, string mark) const
{
  vector<DefaultToken *> const & words = sentence->getWords();
  for (size_t i = 0; i < words.size(); ++i)
  {
    // If a SEP is already here, move on.
    if (words[i]->marks.size() > 0
        && words[i]->marks[0]== DefaultTokenMarks::SEP)
      continue;
    if (classifier.classifyFV(_wordFex->extractWordFeatures(i)))
    {
      _wordFex->appendBrTag(i, mark);
      sentence->appendBrTag(i, mark);
    }
  }
}

// ____________________________________________________________________________
void MLContextMarker::classifyPhraseStarts(Sentence<DefaultToken> * sentence,
    SVMClassifier const & classifier, string mark) const
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  for (size_t i = 1; i < phrases.size(); ++i)
  {
    // TODO(elmar): This is unnecessarily complicated.
    // Should implement this differently.
    // Skip the phrase after a SEP marker.
    if (phrases[i - 1]->getFirstWord().marks.size() > 0)
    {
      if (phrases[i - 1]->getFirstWord().marks[0] == DefaultTokenMarks::SEP)
      {
        continue;
      }
    }
    // If a SEP is already here, move on.
    if (phrases[i]->getFirstWord().marks.size() > 0
        && phrases[i]->getFirstWord().marks[0] == DefaultTokenMarks::SEP)
      continue;
    size_t start = phrases[i]->getWordsStartIndex();
    if (classifier.classifyFV(_wordFex->extractWordFeatures(start)))
    {
      _wordFex->appendBrTag(start, mark);
      sentence->appendBrTag(start, mark);
    }
  }
}

// ____________________________________________________________________________
void MLContextMarker::classifyPhraseEnds(Sentence<DefaultToken> * sentence,
    SVMClassifier const & classifier, string mark) const
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  for (size_t i = 1; i < phrases.size(); ++i)
  {
    // TODO(elmar): This is unnecessarily complicated.
    // Should implement this differently.
    // Skip the phrase after a SEP marker.
    if (phrases[i - 1]->getLastWord().marks.size() > 0)
    {
      if (phrases[i - 1]->getLastWord().marks[0] == DefaultTokenMarks::SEP)
      {
        continue;
      }
    }
    // If a SEP is already here, move on.
    if (phrases[i]->getLastWord().marks.size() > 0
        && phrases[i]->getLastWord().marks[0] == DefaultTokenMarks::SEP)
      continue;
    size_t end = phrases[i]->getWordsEndIndex();
    if (classifier.classifyFV(_wordFex->extractWordFeatures(end)))
    {
      _wordFex->appendBrTag(end, mark);
      sentence->appendBrTag(end, mark);
    }
  }
}

// ____________________________________________________________________________
void MLContextMarker::classifyPhraseStartAndEnds(
    Sentence<DefaultToken> * sentence,
    SVMClassifier const & classifier, string mark) const
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  for (size_t i = 0; i < phrases.size(); ++i)
  {
    size_t start = phrases[i]->getWordsStartIndex();
    size_t end = phrases[i]->getWordsEndIndex();

    if (classifier.classifyFV(_wordFex->extractWordFeatures(start)))
    {
      // std::cout << "Classified as " << mark << std::endl;
      _wordFex->appendBrTag(start, mark);
      sentence->appendBrTag(start, mark);
    }
    if (start != end)
    {
      if (classifier.classifyFV(_wordFex->extractWordFeatures(end)))
      {
        _wordFex->appendBrTag(end, mark);
        sentence->appendBrTag(end, mark);
      }
    }
  }
}

// ____________________________________________________________________________
void MLContextMarker::markSentenceAtWords(
    Sentence<DefaultToken> * sentence) const
{
  assert(_wordOpenClassifiers.size() == _wordCloseClassifiers.size());
  // First use the wordClassifiers.
  for (size_t k = 0; k < _wordClassifiers.size(); ++k)
  {
    classifyAllWords(sentence, *_wordClassifiers[k].second,
        _wordClassifiers[k].first);
  }
  // Next use the wordOpen and wordClose classifiers.
  for (size_t k = 0; k < _wordOpenClassifiers.size(); ++k)
  {
    classifyAllWords(sentence, *_wordOpenClassifiers[k].second,
        _wordOpenClassifiers[k].first);
    classifyAllWords(sentence, *_wordCloseClassifiers[k].second,
        _wordCloseClassifiers[k].first);
  }
}

// ____________________________________________________________________________
void MLContextMarker::markSentenceAtPhraseBoundaries(
    Sentence<DefaultToken> * sentence) const
{
  assert(_wordOpenClassifiers.size() == _wordCloseClassifiers.size());
  // First use the wordClassifiers.
  for (size_t k = 0; k < _wordClassifiers.size(); ++k)
  {
    classifyPhraseStartAndEnds(sentence, *_wordClassifiers[k].second,
        _wordClassifiers[k].first);
  }
  // Next use the wordOpen and wordClose classifiers.
  for (size_t k = 0; k < _wordOpenClassifiers.size(); ++k)
  {
    classifyPhraseStarts(sentence, *_wordOpenClassifiers[k].second,
        _wordOpenClassifiers[k].first);
    classifyPhraseEnds(sentence, *_wordCloseClassifiers[k].second,
        _wordCloseClassifiers[k].first);
  }
}

// ____________________________________________________________________________
void MLContextMarker::printMarkedWords(
    Sentence<DefaultToken> const & sentence) const
{
  std::cout << std::endl << "Word\tMarks" << std::endl;
  vector<DefaultToken *> const & words = sentence.getWords();
  for (size_t i = 0; i < words.size(); i++)
  {
    std::cout << words[i]->tokenString << "\t" << marksAsString(words[i]->marks)
        << std::endl;
  }
  std::cout << std::endl << std::endl;
}

// ____________________________________________________________________________
std::string MLContextMarker::marksAsString(
    std::vector<DefaultTokenMarks::TypeEnum> & marks)
  const
{
  std::string s = "";
  for (size_t i = 0; i < marks.size(); ++i)
  {
    switch (marks[i])
    {
      case DefaultTokenMarks::RELA_OPEN:
        s += "RELA( ";
        break;
      case DefaultTokenMarks::REL_OPEN:
        s += "REL( ";
        break;
      case DefaultTokenMarks::LIT_OPEN:
        s += "LIT( ";
        break;
      case DefaultTokenMarks::RELA_CLOSE:
        s += "RELA) ";
        break;
      case DefaultTokenMarks::REL_CLOSE:
        s += "REL) ";
        break;
      case DefaultTokenMarks::LIT_CLOSE:
        s += "LIT) ";
        break;
      case DefaultTokenMarks::SEP:
        s += "SEP ";
        break;
      default:
        break;
    }
  }
  return s;
}
}
