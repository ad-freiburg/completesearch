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
#include "base/SemanticWikipediaDecomposer.h"
#include "util/ContextDecomposerUtil.h"
#include "decomposer-rule/RobustContextMarker.h"
#include "base/ContextMarkerBase.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"


using std::pair;

// ____________________________________________________________________________
RobustContextMarker::RobustContextMarker(bool verbose)
:_verbose(verbose)
{
}

// ____________________________________________________________________________
RobustContextMarker::~RobustContextMarker()
{
}

// ____________________________________________________________________________
void RobustContextMarker::markSentence(Sentence<DefaultToken> * sentence)
{
  LOG(TRACE)
    << "Rule-marking sentence: " << std::endl <<
    sentence->asStringWithPhrases() << std::endl;
  if (_verbose)
  {
    std::cout << "Rule-marking sentence: " << std::endl;
    std::cout << sentence->asStringWithPhrases();
  }

  // Contains pair of MARKER and Phrase<DefaultToken> index
  _markers.clear();
  // One marker for each syntactic chunk
  // _chunkMarkerVector.clear();
  // Contains a marker for each phrase
  _markerVector.clear();
  _markerVector.insert(_markerVector.begin(), sentence->getPhrases().size(),
      NONE);
  _markerVector.front() = IGNORE;
  _markerVector.back() = IGNORE;

  // Mark brackets as relative clauses.
  markBrackets(*sentence);
  // mark all relative clauses
  markRelativeClauses(*sentence);
  //  printMarkedSentence(*sentence);
  // find all commas, CCs and VPs
  markCommaAndVPs(*sentence);
  //  printMarkedSentence(*sentence);
  // mark them as LIST or SEP
  markSEPandLIST(*sentence);
  if (_verbose)
  {
    std::cout << "After marking: " << std::endl;
    std::cout << internalMarkedSentenceAsString(*sentence) << std::endl;
  }
  LOG(TRACE)
    << "After marking: " << std::endl
    << internalMarkedSentenceAsString(*sentence) << std::endl;
  // mark some of the LIST as REL
  correctLISTtoRELbw(*sentence);
  // mark some of the SEP as REL
  // correctSEPtoREL(sentence);
  //  printMarkedSentence(*sentence);
  markListStarts(sentence);

  if (_verbose)
  {
    std::cout << "After correcting: " << std::endl;
    std::cout << internalMarkedSentenceAsString(*sentence) << std::endl;
  }
  LOG(TRACE)
    << "After correcting: " << std::endl
    << internalMarkedSentenceAsString(*sentence) << std::endl;

  // Mark the final sentence.
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  for (size_t i = 0; i < _markers.size(); ++i)
  {
    size_t phraseIndex = _markers[i].second;
    InternalMark marker = _markers[i].first;

    DefaultToken & word =
        const_cast<DefaultToken &> (phrases[phraseIndex]->getFirstWord());
    switch (marker)
    {
      case RELA:
        word.appendMark(DefaultTokenMarks::RELA_OPEN);
        break;
      case REL:
        word.appendMark(DefaultTokenMarks::REL_OPEN);
        break;
      case SEP:
        word.appendMark(DefaultTokenMarks::SEP);
        break;
      case LIST:
        word.appendMark(DefaultTokenMarks::LIT_OPEN);
        break;
      case LISTV:
        word.appendMark(DefaultTokenMarks::LIT_OPEN);
        break;
      default:
        break;
    }
  }
  markEndings(sentence);

  // Only move the markings to the expected place if we are in evaluation mode.
  // The outcome will not influence the generated contexts.
  if (EVAL_RUN)
    correctMarks(sentence);
  if (_verbose)
  {
    std::cout << "Final rule-marking: " << std::endl;
    std::cout << sentence->asStringWithWordMarks() << std::endl;
  }
  LOG(TRACE)
    << "Final rule-marking: " << std::endl << sentence->asStringWithWordMarks()
    << std::endl;
}

// ____________________________________________________________________________
void RobustContextMarker::correctMarks(Sentence<DefaultToken> * sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  size_t MAX_OFFSET = 2;
  for (size_t i = 0; i < phrases.size(); ++i)
  {
    DefaultToken & firstWord =
        const_cast<DefaultToken &> (phrases[i]->getFirstWord());
    DefaultToken & lastWord =
        const_cast<DefaultToken &> (phrases[i]->getLastWord());
    // Only if the current phrase-type is O and not
    // starts and ends are marked at the same time
    // (firstWord.marks is only for beginnings of constituents
    // and lastWord.marks for ends).
    if (phrases[i]->getType() == "O")
    {
      if (!firstWord.marks.empty() && Token<DefaultTokenMarks>::opensClause(
          firstWord.marks))  //  && lastWord.marks.empty())
      {
        for (size_t j = 1; j < MAX_OFFSET; ++j)
        {
          if (i + j < phrases.size() && phrases[i + j]->getType() != "O")
          {
            // std::cout << "MOVING\n";
            for (size_t m = 0; m < firstWord.marks.size(); ++m)
              const_cast<DefaultToken &> (phrases[i + j]->getFirstWord())
                .prependMark(firstWord.marks[m]);
            firstWord.clearBrTags();
            break;
          }
        }
      }
      else if (!lastWord.marks.empty()
          && Token<DefaultTokenMarks>::closesClause(
          lastWord.marks))  // && firstWord.marks.empty())
      {
        for (size_t j = 1; j < MAX_OFFSET; ++j)
        {
          if (i - j >= 0 && phrases[i - j]->getType() != "O")
          {
            // std::cout << "MOVING\n";
            for (size_t m = 0; m < lastWord.marks.size(); ++m)
              const_cast<DefaultToken &>(phrases[i - j]->getLastWord())
                .appendMark(lastWord.marks[m]);
            lastWord.clearBrTags();
            break;
          }
        }
      }
    }
  }

  vector<DefaultToken *> const & words = sentence->getWords();

  // Move SEPs to the rightmost possibility.
  for (size_t i = 0; i < words.size(); ++i)
  {
    // We marked an O-Phrase<DefaultToken> with a SEP.
    if (words[i]->marks.size() > 0 && words[i]->marks[0] ==
        DefaultTokenMarks::SEP)
    {
      // The next Phrase<DefaultToken> is of type O, IN, or RB
      if (words.size() > i + 1 && (words[i + 1]->posTag == "RB"
          || words[i + 1]->posTag == "IN" || words[i + 1]->posTag == "CC"
          || words[i + 1]->posTag == "O"))
      {
        // Erase the SEP. This assumes the only mark was the SEP.
        words[i]->clearBrTags();
        // words[i]->marks.erase(words[i]->marks.begin());
        words[i + 1]->prependMark(DefaultTokenMarks::SEP);
      }
    }
  }
}

// ____________________________________________________________________________
void RobustContextMarker::markListStarts(Sentence<DefaultToken> * sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();
  InternalMark previousMark = IGNORE;
  size_t previousPhraseIndex = 0;
  for (std::vector<std::pair<InternalMark, size_t> >::iterator it =
      _markers.begin(); it != _markers.end(); ++it)
  {
    size_t phraseIndex = it->second;
    InternalMark mark = it->first;

    switch (mark)
    {
      case RELA:
        previousPhraseIndex = phraseIndex;
        previousMark = mark;
        break;
      case REL:
        previousPhraseIndex = phraseIndex;
        previousMark = mark;
        break;
      case SEP:
        previousPhraseIndex = phraseIndex;
        previousMark = mark;
        break;
      case LIST:
        if (previousMark != LIST)
        {
          // std::cout << "LIST at :" << phraseIndex << "\n";
          size_t np = previousPhraseIndex + 1;

          // There is nothing inbetween we could use.
          if (np == phraseIndex)
          {
            previousMark = LIST;
            previousPhraseIndex = phraseIndex;
            break;
          }

          for (size_t i = np; i < phraseIndex; ++i)
          {
            if (phrases[i]->getType() == "NP")
            {
              np = i;
            }
          }
          // Avoid putting a LIST directly behind a SEP - it causes
          // problems for extraction.
          if ((previousMark == SEP && previousPhraseIndex == np - 1)
              || _markerVector[np] == IGNORE)
          {
            previousMark = LIST;
            previousPhraseIndex = phraseIndex;
            break;
          }
          // std::cout << "adding list at :" << np << "\n";
          it = _markers.insert(it, std::make_pair(LIST, np));
          _markerVector[np] = LIST;
        }
        previousMark = LIST;
        previousPhraseIndex = phraseIndex;
        break;
      case LISTV:
        if (previousMark != LISTV)
        {
          size_t vp = previousPhraseIndex + 1;

          // There is nothing inbetween we could use.
          if (vp == phraseIndex)
          {
            previousMark = LISTV;
            previousPhraseIndex = phraseIndex;
            break;
          }

          for (size_t i = vp; i < phraseIndex; ++i)
          {
            // Experimentation shows using first VP gives
            // better results than using last VP
            if (phrases[i]->getType() == "VP")
            {
              vp = i;
              break;
            }
          }
          // Avoid putting a LISTV directly behind a SEP - it causes
          // problems for extraction.
          if ((previousMark == SEP && previousPhraseIndex == vp - 1)
              || _markerVector[vp] == IGNORE)
          {
            previousMark = LISTV;
            previousPhraseIndex = phraseIndex;
            break;
          }
          // std::cout << "VP at :" << vp << "\n";
          it = _markers.insert(it, std::make_pair(LISTV, vp));
          _markerVector[vp] = LISTV;
        }
        previousMark = LISTV;
        previousPhraseIndex = phraseIndex;
        break;
      default:
        break;
    }
  }
}

// ____________________________________________________________________________
void RobustContextMarker::markEndings(Sentence<DefaultToken> * sentence)
{
  // boost::ptr_vector <Token> const & words = sentence->getWords();
  vector<Phrase<DefaultToken> *> const & phrases = sentence->getPhrases();

  std::vector<size_t> openRelativeClauses;
  std::vector<size_t> openAppositives;
  std::vector<size_t> openLists;
  DefaultToken * previousWord = const_cast<DefaultToken *>
    (&phrases[0]->getLastWord());

  // We start with the second Phrase - the first Phrase is ignored anyways.
  for (size_t i = 1; i < phrases.size(); ++i)
  {
    DefaultToken & firstWord =  const_cast<DefaultToken &>
      (phrases[i]->getFirstWord());
    size_t currentWordIndex = phrases[i]->getWordsStartIndex();
    previousWord = const_cast<DefaultToken *> (&phrases[i - 1]->getLastWord());

    std::vector<DefaultTokenMarks::TypeEnum> const & marks = firstWord.marks;

    // We encounter a SEP - a SEP always occurs alone.
    // Close all clauses - nothing extends across a SEP
    if (marks.size() == 1 && marks[0] == DefaultTokenMarks::SEP)
    {
      if (i - 1 >= 0)
      {
        for (size_t j = 0; j < openRelativeClauses.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::REL_CLOSE);
        for (size_t j = 0; j < openAppositives.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::RELA_CLOSE);
        for (size_t j = 0; j < openLists.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::LIT_CLOSE);
      }
      openRelativeClauses.clear();
      openAppositives.clear();
      openLists.clear();
      // We can continue because the SEP stands alone.
      continue;
    }

    // This is a special marker to indicate some intermitted clause
    // ends. So we close all up to now existing clauses.
    if (_markerVector[i] == CLOSE)
    {
      if (i - 1 >= 0)
      {
        for (size_t j = 0; j < openRelativeClauses.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::REL_CLOSE);
        for (size_t j = 0; j < openAppositives.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::RELA_CLOSE);
        for (size_t j = 0; j < openLists.size(); ++j)
          previousWord->appendMark(DefaultTokenMarks::LIT_CLOSE);
      }
      openRelativeClauses.clear();
      openAppositives.clear();
      openLists.clear();
    }

    // A closing brackets closes one relativ clause.
    if (phrases[i]->getFirstWord().tokenString == ")")
    {
      if (!openRelativeClauses.empty())
      {
        firstWord.appendMark(DefaultTokenMarks::REL_CLOSE);
        openRelativeClauses.pop_back();
      }
    }

    // TODO(elmar): check if we need this?
    // A relative clause also ends if after the comma a VP follows in
    // a specific form
    //    if ((firstWord.tokenString == ",")
    //        && i + 1 < phrases.size()
    //        && phrases[i + 1]->getFirstWord().marks.size() == 0
    //        && phrases[i + 1]->getType() == "VP"
    //        && !phrases[i + 1]->matchPOSRegex(".*VB(G|N).*"))
    //    {
    //      std::cout << "THIS HAPPENED at"
    //            << phrases[i+1]->getFirstWord().tokenString;
    //      if (firstWord.marks.size()>0)
    //        std::cout << "BUT I WAS MARKED";
    //      for (size_t j = 0; j < openRelativeClauses.size(); ++j)
    //        previousWord.appendMark(DefaultTokenMarks::REL_CLOSE);
    //      for (size_t j = 0; j < openAppositives.size(); ++j)
    //        previousWord.appendMark(DefaultTokenMarks::RELA_CLOSE);
    //      for (size_t j = 0; j < openLists.size(); ++j)
    //        previousWord.appendMark(DefaultTokenMarks::LIT_CLOSE);
    //      openRelativeClauses.clear();
    //      openAppositives.clear();
    //      openLists.clear();
    //    }


    // We encounter one (or more) REL_OPEN
    if (std::find(marks.begin(), marks.end(),
        DefaultTokenMarks::REL_OPEN) != marks.end())
    {
      // A relative clause is already opened.
      if (!openRelativeClauses.empty())
      {
        // A relative clause starts with the POS tags
        // "CC WD(T|P)" attaches to the
        // same noun/noun-phrase.
        // We close th last rel clauses so far and open a new relative clause.
        // TODO(elmar) : use the type of relative clause (who, which) etc.
        // to check agains "and who" vs. "and which" etc.
        if ((i + 1 < phrases.size() && firstWord.posTag == "CC" && phrases[i
            + 1]->matchPOSRegex("W(DT|P).*")))
        {
          for (size_t j = 0; j < openRelativeClauses.size(); ++j)
            previousWord->appendMark(DefaultTokenMarks::REL_CLOSE);
          openRelativeClauses.clear();
        }
      }

      // Open the new relative clauses.
      size_t n = std::count(marks.begin(), marks.end(),
          DefaultTokenMarks::REL_OPEN);
      for (size_t j = 0; j < n; ++j)
        openRelativeClauses.push_back(currentWordIndex);

      // Lists and appositives are done in any case.
      for (size_t j = 0; j < openAppositives.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::RELA_CLOSE);
      for (size_t j = 0; j < openLists.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::LIT_CLOSE);
      openAppositives.clear();
      openLists.clear();
    }

    // We encounter one (or more) LIT_OPEN
    if (std::find(marks.begin(), marks.end(),
        DefaultTokenMarks::LIT_OPEN) != marks.end())
    {
      // Just close previous list items.
      // Lists and appositives are done in any case.
      for (size_t j = 0; j < openAppositives.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::RELA_CLOSE);
      for (size_t j = 0; j < openLists.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::LIT_CLOSE);
      openAppositives.clear();
      openLists.clear();
      // And remember the openend ones.
      size_t n = std::count(marks.begin(), marks.end(),
          DefaultTokenMarks::LIT_OPEN);
      for (size_t j = 0; j < n; ++j)
        openLists.push_back(currentWordIndex);
    }

    // We encounter one (or more) RELA_OPEN
    if (std::find(marks.begin(), marks.end(),
        DefaultTokenMarks::RELA_OPEN) != marks.end())
    {
      // Lists and appositives are done in any case.
      for (size_t j = 0; j < openAppositives.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::RELA_CLOSE);
      for (size_t j = 0; j < openLists.size(); ++j)
        previousWord->appendMark(DefaultTokenMarks::LIT_CLOSE);
      openAppositives.clear();
      openLists.clear();
      // And remember the openend ones.
      size_t n = std::count(marks.begin(), marks.end(),
          DefaultTokenMarks::RELA_OPEN);
      for (size_t j = 0; j < n; ++j)
        openAppositives.push_back(currentWordIndex);
    }
    //  if (phrases[i]->getType() != "O")
    //  previousWord = &phrases[i]->getLastWord();
  }

  // At the end everything is closed.
  DefaultToken & lastWord =  const_cast<DefaultToken &>
      (phrases.back()->getLastWord());

  for (size_t j = 0; j < openRelativeClauses.size(); ++j)
    lastWord.appendMark(DefaultTokenMarks::REL_CLOSE);
  for (size_t j = 0; j < openAppositives.size(); ++j)
    lastWord.appendMark(DefaultTokenMarks::RELA_CLOSE);
  for (size_t j = 0; j < openLists.size(); ++j)
    lastWord.appendMark(DefaultTokenMarks::LIT_CLOSE);
  openRelativeClauses.clear();
  openAppositives.clear();
  openLists.clear();
}

// ____________________________________________________________________________
// void RobustContextMarker::correctSEPtoREL(Sentence<DefaultToken> * sentence)
// {
//  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
//  for (std::vector<std::pair<Mark, size_t> >::iterator it =
//   _markers.begin(); it
//      != _markers.end(); ++it)
//  {
//    // find all SEPs and if they are directly followed by a VP in VBN or VBG
//    // form set them to REL
//    // this is only good for comma separators nothing else
//    if ((*it).first == SEP && phrases[(*it).second]->getFirstWord().posTag
//        == ","
//    // make sure the "," is really followed by something
//        && (*it).second < phrases.size() - 1
//        && (phrases[(*it).second + 1]->getFirstWord().posTag == "VBN"
//            || phrases[(*it).second + 1]->getFirstWord().posTag == "VBG"))
//    {
//      (*it).first = REL;
//      _markerVector[(*it).second] = REL;
//      // std::cout <<  "MARKSEPTOREL";
//    }
//  }
// }

// ____________________________________________________________________________
void RobustContextMarker::correctLISTtoRELbw(
    Sentence<DefaultToken> const & sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  bool listOpen = false;
  // is this a list of verbs
  bool isVerbListing = false;
  // move through all markers from the back
  for (std::vector<std::pair<InternalMark, size_t> >::iterator it =
      _markers.end() - 1; it != _markers.begin() - 1; --it)
  {
    size_t phraseIndex = (*it).second;
    InternalMark & marker = (*it).first;

    // we encounter a LIST and the Phrase behind does not start with CC
    // or is a CONJP
    if (!listOpen && marker == LIST
        && phrases[phraseIndex]->getFirstWord().posTag != "CC"
        && phrases[phraseIndex]->getType() != "CONJP")
    {
      // It may be classified as LIST because a VerbPhrase followed,
      // however it can be the continuation of a clause - then we classify as
      // CLOSE to recognize the end of the intermitted clause.
      if (phraseIndex + 1 < phrases.size()
          && (phrases[phraseIndex + 1]->getType() == "VP"
              && !phrases[phraseIndex + 1]->matchPOSRegex("VB(G|N).*")))
      {
        // std::cout << "Marked sep here";
        marker = CLOSE;
        _markerVector[phraseIndex] = CLOSE;
      }
      else
      {
        // std::cout << "REVERTED to RELA\n";
        marker = RELA;
        _markerVector[phraseIndex] = RELA;
      }
    }
    // we encounter a LIST and the Phrase<DefaultToken>
    // behind starts with CC or is a CONJP
    // -> this is a correct beginning of a list
    else if (!listOpen && marker == LIST
        && (phrases[phraseIndex]->getFirstWord().posTag == "CC"
            || phrases[phraseIndex]->getType() != "CONJP"))
    {
      if (phraseIndex + 1 < phrases.size()
          && phrases[phraseIndex + 1]->getType() == "VP")
      {
        marker = LISTV;
        _markerVector[phraseIndex] = LISTV;
        isVerbListing = true;
      }
      listOpen = true;
    }
    // we encounter a non-LIST, end the list
    // except we are in a verb listing, and look at the verb directly
    // following a list separator ( LISTV VIO....LISTV VIO...)
    else if (listOpen && marker != LIST && !(isVerbListing && marker == VIO
        && it - 1 != _markers.end() && ((*(it - 1)).first == LIST
        || (*(it - 1)).first == VIO)))
    {
      listOpen = false;
      isVerbListing = false;
    }
    // If we encounter a LIST but are inside a verb list correct it.
    else if (listOpen && marker == LIST && isVerbListing)
    {
      marker = LISTV;
      _markerVector[phraseIndex] = LISTV;
    }
    // If we encounter a LIST consume it - do nothing.
  }
}

// ____________________________________________________________________________
void RobustContextMarker::markSEPandLIST(
    Sentence<DefaultToken> const & sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();

  for (size_t i = 0; i < _markerVector.size(); ++i)
  {
    if (_markerVector[i] != NONE && _markerVector[i] != IGNORE)
      _markers.push_back(std::make_pair(_markerVector[i], i));
  }

  for (size_t i = 0; i < _markers.size(); ++i)
  {
    if (_markers[i].first == RED)
    {
      // SEP wenn nächste Farbe VIO, aber nicht direkt danach
      if (i < (_markers.size() - 1) && _markers[i + 1].first == VIO
          && _markers[i + 1].second != _markers[i].second + 1)
      {
        // An ADVP is no reason to separate...
        if (phrases[_markers[i + 1].second]->getType() == "ADVP")
        {
          _markers[i].first = CLOSE;
          _markerVector[_markers[i].second] = CLOSE;
        }
        else
        {
        // std::cout << "Marked sep";
          _markers[i].first = SEP;
          _markerVector[_markers[i].second] = SEP;
        }
      }
      // A ; is always a SEP
      else if (phrases[_markers[i].second]->getFirstWord().tokenString  == ";")
      {
        _markers[i].first = SEP;
        _markerVector[_markers[i].second] = SEP;
      }
      // sonst LIST
      else
      {
        _markers[i].first = LIST;
        _markerVector[_markers[i].second] = LIST;
      }
    }
  }
}

// ____________________________________________________________________________
void RobustContextMarker::markCommaAndVPs(
    Sentence<DefaultToken> const & sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    // we only consider phrases of type O if they are not already marked as
    // relative clause
    if (phrases[i]->getType() == "O" && _markerVector[i] == NONE)
    {
      // we find "and" / "or"...
      if (phrases[i]->getFirstWord().posTag == "CC" || phrases[i]->getType()
          == "CONJP")
      {
        _markerVector[i] = RED;
      }
      // we find ","
      else if (phrases[i]->getFirstWord().tokenString == "," && i != 0)
      {
        // it is followed by a CC, mark the CC ignore comma
        if (i < phrases.size() - 1 && (phrases[i + 1]->getFirstWord().posTag
            == "CC" || phrases[i + 1]->getType() == "CONJP"))
        {
          _markerVector[i + 1] = RED;
          _markerVector[i] = IGNORE;
        }
        // it is not followed by a CC, just mark the comma
        else
          _markerVector[i] = RED;
      }
      // Mark ; as RED
      else if (phrases[i]->getFirstWord().tokenString == ";" && i != 0
          && i < phrases.size() - 1 )
      {
        // Only mark, if ; not directly follows.
        if (phrases[i + 1]->getFirstWord().tokenString != ";" &&
            phrases[i + 1]->getFirstWord().posTag != "CC" &&
            phrases[i + 1]->getFirstWord().tokenString != ",")
        {
          _markerVector[i] = RED;
        }
      }
    }
    else if (phrases[i]->getType() == "VP" && _markerVector[i] == NONE)
    {
      _markerVector[i] = VIO;
    }
  }
}

void RobustContextMarker::markBrackets(Sentence<DefaultToken> const & sentence)
{
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  // an opening bracket is assumed to start a relative clause.
  vector<pair<size_t, size_t> > brackets;
  vector<size_t> startPositions;
  bool correctBracketing = true;
  // Collect opening and closing brackets.
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if (phrases[i]->getFirstWord().tokenString == "(" && _markerVector[i]
        != IGNORE)
    {
      startPositions.push_back(i);
    }
    else if (phrases[i]->getFirstWord().tokenString == ")" && _markerVector[i]
        != IGNORE)
    {
      if (!startPositions.empty())
      {
        brackets.push_back(std::make_pair(startPositions.back(), i));
        startPositions.pop_back();
      }
      else
      {
        correctBracketing = false;
        break;
      }
    }
  }

  // All opening brackets must have been closed.
  if (correctBracketing && !startPositions.empty())
    correctBracketing = false;

  if (correctBracketing)
  {
    // Mark the ignores.
    for (size_t i = 0; i < brackets.size(); ++i)
    {
      // Find the largest spanning bracket.
      while ( i + 1 < brackets.size()
          && brackets[i+1].first > brackets[i].first)
      {
        ++i;
      }
      for (size_t s = brackets[i].first; s <= brackets[i].second; ++s)
        _markerVector[s] = IGNORE;
    }
    // Mark the starts.
    for (size_t i = 0; i < brackets.size(); ++i)
    {
      _markerVector[brackets[i].first] = REL;
    }
  }
  // Fall back to a simple marking of brackets by matching
  // each opening bracket to the next, closest closing bracket.
  else
  {
    for (size_t i = 0; i < phrases.size(); i++)
    {
      if (phrases[i]->getFirstWord().tokenString == "("
          && _markerVector[i]!= IGNORE)
      {
        size_t endingBracket = 0;
        // Check if we find a matching closing bracket.
        for (size_t j = i + 1; j < phrases.size(); ++j)
        {
          if (phrases[j]->getFirstWord().tokenString == ")"
              && _markerVector[i]!= IGNORE)
          {
            endingBracket = j;
            break;
          }
        }
        // If we found matching brackets.
        if (endingBracket != 0)
        {
          // Mark the start.
          _markerVector[i] = REL;
          // Ignore whatever is inside the brackets.
          for (size_t j = i + 1; j < endingBracket; ++j)
            _markerVector[j] = IGNORE;
        }
      }
    }
  }
}


// ____________________________________________________________________________
void RobustContextMarker::markRelativeClauses(
    Sentence<DefaultToken> const & sentence)
{
  // only mark non-restrictive relative clauses: if they are introduced by a
  // comma
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    // We ignore the first and last token. Accept this.
    if (_markerVector[i] == IGNORE)
      continue;
    if (phrases[i]->getType() == "O")
    {
      // For a comma check for relative clauses.
      if (phrases[i]->getFirstWord().tokenString == ",")
      {
        // if the relative clause is preceeded by a comma
        // mark the relative clause
        // and ignore the comma in future
        if (i + 1 < phrases.size() && phrases[i + 1]->startsRelativeClause())
        {
          // If the relative clause does not start with "who", "which" etc.
          // it is an appositive.
          if (!phrases[i + 1]->matchPOSRegex("W(DT|P).*"))
            _markerVector[i] = RELA;
          else
            _markerVector[i] = REL;
        }
        // There can be a coordinating conjunction
        // between the comma and the start.
        // as in " , and who " or ", with whom"
        else if (i + 2 < phrases.size()
            && (phrases[i + 1]->getFirstWord().posTag == "CC"
                || phrases[i + 1]->getFirstWord().posTag == "IN") && phrases[i
            + 2]->startsRelativeClause())
        {
          // If the relative clause does not start with "who", "which" etc.
          // it is an appositive.
          if (!phrases[i + 2]->matchPOSRegex("W(DT|P).*"))
            _markerVector[i] = RELA;
          else
            _markerVector[i] = REL;
        }
        // A comma followed by a Verb in form
        // VBG or VBN starts a relative clause as well
        else if (i + 1 < phrases.size()
            && (phrases[i + 1]->getFirstWord().posTag == "VBG"
                || phrases[i + 1]->getFirstWord().posTag == "VBN"))
        {
          _markerVector[i] = REL;
        }
        else
        {
          // _markerVector[i] = REL;
        }
      }
    }
  }
}

// ____________________________________________________________________________
string RobustContextMarker::internalMarkedSentenceAsString(
    Sentence<DefaultToken> const & sentence)
{
  std::ostringstream s;
  s << std::endl << "MARK\tPHRASE" << std::endl;
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if (_markerVector[i] != NONE)
    {
      s << internalMarkToString(_markerVector[i]);
    }
    s << "\t" << phrases[i]->asString() << std::endl;
  }
  return s.str();
}

// ____________________________________________________________________________
string RobustContextMarker::internalMarkToString(InternalMark mark)
{
  switch (mark)
  {
    case NONE:
      return "NONE";
    case LIST:
      return "LIST";
    case LISTV:
      return "LISTV";
    case SEP:
      return "SEP";
    case REL:
      return "REL";
    case RELA:
      return "RELA";
    case CLOSE:
      return "CLOSE";
    case RED:
      return "RED";
    case VIO:
      return "VIO";
    case IGNORE:
      return "IGNORE";
    default:
      std::cerr << "Invalid MARK: " << mark << std::endl;
      exit(1);
  }
}
