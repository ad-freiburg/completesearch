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
#include "decomposer-rule/RobustContextDecomposer.h"
#include "util/ContextDecomposerUtil.h"

using std::pair;

// ____________________________________________________________________________
RobustContextDecomposer::RobustContextDecomposer(bool writeToStdOut)
{
  _writeToStdOut = writeToStdOut;
}

// ____________________________________________________________________________
RobustContextDecomposer::~RobustContextDecomposer()
{
}

// ____________________________________________________________________________
std::vector<ContextPtr> const RobustContextDecomposer::decomposeToPtr(
    Sentence & sentence)
{
  // Contains pair of MARKER and phrase index
  _markers.clear();
  // One marker for each syntactic chunk
  _chunkMarkerVector.clear();
  // Contains a marker for each phrase
  _markerVector.clear();
  _markerVector.insert(_markerVector.begin(), sentence.getPhrases().size(),
      NONE);
  _markerVector.front() = IGNORE;
  _markerVector.back() = IGNORE;
  // mark all relative clauses
  markRelativeClauses(sentence);
  // find all commas, CCs and VPs
  markCommaAndVPs(sentence);
  // mark them as LIST or SEP
  markSEPandLIST(sentence);
  if (_writeToStdOut)
  {
    std::cout << "After Marking: " << std::endl;
    printMarkedSentence(sentence);
  }
  // mark some of the LIST as REL
  correctLISTtoRELbw(sentence);
  // mark some of the SEP as REL
  // correctSEPtoREL(sentence);
  if (_writeToStdOut)
  {
    std::cout << "After Correcting: " << std::endl;
    printMarkedSentence(sentence);
  }
  // create syntactic chunks according to our markings
  createSyntacticChunks(sentence);
  if (_writeToStdOut)
  {
    printChunks(sentence.getSyntacticChunks());
  }
  extractAppositiveRELs(sentence);
  extractRELs(sentence);
  if (_writeToStdOut)
  {
    std::cout << "\nAfter app/-rel extraction \n \n";
    printChunks(sentence.getSyntacticChunks());
  }
  // create semantic chunks from the marked RELs
  // createSemanticRELChunks(sentence);
  // create semantic chunks from the rest
  createSemanticChunks(sentence);
  if (_writeToStdOut)
  {
    printChunks(sentence.getSemanticChunks());
    printChunks(sentence.getSyntacticChunks());
  }

  // Produce the contexts
  std::vector<ContextPtr> contexts = sentence.produceContextsPtr();

  // Output the remaining syntactic chunks
  std::vector<SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    if (chunks[i]->getSemanticChunk()->hasEnumeration())
      continue;
    ContextPtr context = chunks[i]->getWords();
    contexts.push_back(context);
  }
  return contexts;
}

// ____________________________________________________________________________
void RobustContextDecomposer::createSemanticChunks(Sentence & sentence)
{
  std::vector<SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  SemanticChunk * head = chunks[0]->getSemanticChunk();
  // chunks.erase(chunks.begin());
  // _chunkMarkerVector.erase(_chunkMarkerVector.begin());
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    if (_chunkMarkerVector[i] == LIST)
    {
      head->appendSingleEnumerationElement(*chunks[i]);
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      --i;
    }
    else if (_chunkMarkerVector[i] == LISTV)
    {
      head->appendSingleVerbEnumerationElement(*chunks[i]);
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      --i;
    }
    else if (_chunkMarkerVector[i] == SEP)
    {
      head = chunks[i]->getSemanticChunk();
      // chunks.erase(chunks.begin()+i);
      // _chunkMarkerVector.erase(_chunkMarkerVector.begin()+i);
      // --i;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::extractAppositiveRELs(Sentence & sentence)
{
  std::vector<SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  SemanticChunk * head = NULL;
  // See if this is too strict - could also define anything that is
  // not [CC] WD[T|P] as appositive
  for (size_t i = 1; i < chunks.size(); ++i)
  {
    if (_chunkMarkerVector[i] == REL && !(chunks[i]->matchPOSRegex(
        "([CC|IN] *)?WD(T|P).*")))
    {
      head = chunks[i - 1]->getSemanticChunk();
      head->addRelativeChunk(*chunks[i]);
      // append the next chunk to the previous chunk
      if (i + 1 < chunks.size() && i - 1 >= 0)
      {
        if (_chunkMarkerVector[i + 1] == SEP)
        {
          chunks[i - 1]->appendChunk(*chunks[i + 1]);
          _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i + 1);
          chunks.erase(chunks.begin() + i + 1);
        }
      }
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      // Because we deleted this chunk the next one has the same index.
      i--;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::extractRELs(Sentence & sentence)
{
  std::vector<SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  bool relOpen = false;
  SemanticChunk * head = NULL;
  SemanticChunk * listHead = NULL;
  std::vector<pair<size_t, SyntacticChunk *> > openRelativClauses;

  for (size_t i = 1; i < chunks.size(); ++i)
  {
    // A new relative clause opens.
    if (!relOpen && _chunkMarkerVector[i] == REL)
    {
      head = chunks[i - 1]->getSemanticChunk();
      head->addRelativeChunk(*chunks[i]);
      // remember this chunk in case a list starts with the next chunk (a SEP
      // would end the REL)
      pair<size_t, SyntacticChunk *> openRelativClause(i, chunks[i]);
      openRelativClauses.push_back(openRelativClause);
      relOpen = true;
      listHead = NULL;
    }
    // A relative clause opens within an existing one
    else if (relOpen && _chunkMarkerVector[i] == REL)
    {
      // This attaches to the chunk preceeding the last open relative clause.
      if (chunks[i]->matchPOSRegex("CC *WD(T|P).*"))
      {
        pair<size_t, SyntacticChunk *> lastRelClause =
            openRelativClauses.back();
        head = chunks[lastRelClause.first - 1]->getSemanticChunk();
      }
      // Otherwise just attach to the previous chunk
      else
      {
        head = chunks[i - 1]->getSemanticChunk();
      }
      head->addRelativeChunk(*chunks[i]);
      // remember this chunk in case a list starts with the next chunk (a SEP
      // would end the REL)
      pair<size_t, SyntacticChunk *> openRelativClause(i, chunks[i]);
      openRelativClauses.push_back(openRelativClause);
      listHead = NULL;
    }
    else if (relOpen && _chunkMarkerVector[i] == LIST)
    {
      if (listHead == NULL)
      {
        listHead = chunks[i - 1]->getSemanticChunk();
      }
      listHead->appendSingleEnumerationElement(*chunks[i]);
    }
    else if (relOpen && _chunkMarkerVector[i] == LISTV)
    {
      if (listHead == NULL)
      {
        listHead = chunks[i - 1]->getSemanticChunk();
      }
      listHead->appendSingleVerbEnumerationElement(*chunks[i]);
    }
    // We see a SEP - remove the relative clauses and all that was in between
    else if (relOpen && _chunkMarkerVector[i] == SEP)
    {
      chunks[openRelativClauses[0].first - 1]->appendChunk(*chunks[i]);
      // Delete all and including what we currently point to.
      chunks.erase(chunks.begin() + openRelativClauses[0].first, chunks.begin()
          + i + 1);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin()
          + openRelativClauses[0].first, _chunkMarkerVector.begin() + i + 1);
      i = openRelativClauses[0].first;
      openRelativClauses.clear();
      listHead = NULL;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::createSemanticRELChunks(Sentence & sentence)
{
  std::vector<SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  bool relOpen = false;
  SyntacticChunk * previousChunk = NULL;
  SemanticChunk * head = NULL;
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    if (_chunkMarkerVector[i] == REL && i > 0)
    {
      if (previousChunk != NULL)
        head = previousChunk->getSemanticChunk();
      else
        head = chunks[i - 1]->getSemanticChunk();
      head->addRelativeChunk(*chunks[i]);
      // remember this chunk in case a list starts with the next chunk (a SEP
      // would end the REL)
      previousChunk = chunks[i];
      // remove from chunk-list and marker-list
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      i--;
      relOpen = true;
    }
    else if (relOpen && _chunkMarkerVector[i] == LIST)
    {
      head = previousChunk->getSemanticChunk();
      head->appendSingleEnumerationElement(*chunks[i]);
      // std::cout << "RELLIST";
      // remove from chunk-list and marker-list
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      i--;
    }
    else if (relOpen && _chunkMarkerVector[i] == LISTV)
    {
      head = previousChunk->getSemanticChunk();
      head->appendSingleVerbEnumerationElement(*chunks[i]);
      // std::cout << "RELLIST";
      // remove from chunk-list and marker-list
      chunks.erase(chunks.begin() + i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin() + i);
      i--;
    }
    else if (_chunkMarkerVector[i] == SEP)
    {
      relOpen = false;
      previousChunk = NULL;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::createSyntacticChunks(Sentence & sentence)
{
  vector<Phrase *> const & phrases = sentence.getPhrases();
  SyntacticChunk * synChunk = new SyntacticChunk(sentence);
  _chunkMarkerVector.push_back(_markerVector[0]);
  // Iterate through the phrases and create continuous syntactic chunks
  // between our markers.
  for (size_t i = 0; i < _markerVector.size(); ++i)
  {
    // Create a new syntactic chunk for any marker but IGNORE, NONE und VIO.
    if (_markerVector[i] != NONE && _markerVector[i] != IGNORE
        && _markerVector[i] != VIO)
    {
      synChunk = new SyntacticChunk(sentence);
      _chunkMarkerVector.push_back(_markerVector[i]);
      // Only add the first phrase if it has a "non-trivial" type.
      if (phrases[i]->getType() != "O" || i + 1 == _markerVector.size() || (i
          + 1 < _markerVector.size() && _markerVector[i + 1] != NONE
          && _markerVector[i + 1] != IGNORE && _markerVector[i + 1] != VIO))
        synChunk->appendPhrase(*phrases[i]);
    }
    // Add the phrase to the currently opened chunk.
    else
      synChunk->appendPhrase(*phrases[i]);
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::correctSEPtoREL(Sentence & sentence)
{
  vector<Phrase *> const & phrases = sentence.getPhrases();
  for (std::vector<std::pair<Mark, size_t> >::iterator it = _markers.begin(); it
      != _markers.end(); ++it)
  {
    // find all SEPs and if they are directly followed by a VP in VBN or VBG
    // form set them to REL
    // this is only good for comma separators nothing else
    if ((*it).first == SEP && phrases[(*it).second]->getFirstWord().posTag
        == ","
    // make sure the "," is really followed by something
        && (*it).second < phrases.size() - 1
        && (phrases[(*it).second + 1]->getFirstWord().posTag == "VBN"
            || phrases[(*it).second + 1]->getFirstWord().posTag == "VBG"))
    {
      (*it).first = REL;
      _markerVector[(*it).second] = REL;
      // std::cout <<  "MARKSEPTOREL";
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::correctLISTtoRELbw(Sentence & sentence)
{
  vector<Phrase *> const & phrases = sentence.getPhrases();
  bool listOpen = false;
  // is this a list of verbs
  bool isVerbListing = false;
  // move through all markers from the back
  for (std::vector<std::pair<Mark, size_t> >::iterator it = _markers.end() - 1;
      it != _markers.begin() - 1; --it)
  {
    size_t phraseIndex = (*it).second;
    Mark marker = (*it).first;

    // we encounter a LIST and the phrase behind does not start with CC
    // or is a CONJP
    if (!listOpen && marker == LIST
        && phrases[phraseIndex]->getFirstWord().posTag != "CC"
        && phrases[phraseIndex]->getType() != "CONJP")
    {
      // It may be classified as LIST because a Verbphrase followed,
      // however it can be the continuation of a clause - then we classify as
      // SEP to recognize the end of the intermitted clause.
      if (phraseIndex + 1 < phrases.size()
          && (phrases[phraseIndex + 1]->getType() == "VP"
              && !phrases[phraseIndex + 1]->matchPOSRegex("VB(G|N).*")))
      {
        marker = SEP;
        _markerVector[phraseIndex] = SEP;
      }
      else
      {
        marker = REL;
        _markerVector[phraseIndex] = REL;
      }
    }
    // we encounter a LIST and the phrase behind starts with CC or is a CONJP
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
    // following a list separator
    else if (listOpen && marker != LIST && !(isVerbListing && marker == VIO
        && it - 1 != _markers.end() && (*(it - 1)).first == LIST))
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
void RobustContextDecomposer::correctLISTtoREL(Sentence & sentence)
{
  bool listOpen = false;
  vector<Phrase *> const & phrases = sentence.getPhrases();
  std::vector<std::pair<Mark, size_t> *> listEntries;
  for (size_t i = 0; i < _markers.size(); ++i)
  {
    // we are inside or start a list
    if (_markers[i].first == LIST && i < (_markers.size() - 1) && _markers[i
        + 1].first == LIST)
    {
      listOpen = true;
      listEntries.push_back(&_markers[i]);
    }
    // we have ended a list
    else if (_markers[i].first != LIST && listOpen == true)
    {
      // if the last marker is not on "and" or "or" revert the list
      if (phrases[listEntries.back()->second]->getFirstWord().posTag != "CC")
      {
        for (size_t j = 0; j < listEntries.size(); j++)
        {
          listEntries[j]->first = REL;
          _markerVector[listEntries[j]->second] = REL;
        }
        listEntries.clear();
      }
      listOpen = false;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::markSEPandLIST(Sentence & sentence)
{
  std::vector<Mark> marks;
  vector<Phrase *> const & phrases = sentence.getPhrases();
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
        _markers[i].first = SEP;
        _markerVector[_markers[i].second] = SEP;
      }
      // ; ist immer ein SEP
      else if (phrases[_markers[i].second]->getFirstWord().tokenString == ";")
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
void RobustContextDecomposer::markCommaAndVPs(Sentence & sentence)
{
  vector<Phrase *> const & phrases = sentence.getPhrases();
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
      else if ((phrases[i]->getFirstWord().tokenString == ","
          || phrases[i]->getFirstWord().tokenString == ";") && i != 0)
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
    }
    else if (phrases[i]->getType() == "VP" && _markerVector[i] == NONE)
    {
      _markerVector[i] = VIO;
    }
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::markRelativeClauses(Sentence & sentence)
{
  // only mark restrictive relative clauses: if they are introduced by a
  // comma
  vector<Phrase *> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    // We ignore the first and last token. Accept this.
    if (_markerVector[i] == IGNORE)
      continue;
    if (phrases[i]->getType() == "O" && phrases[i]->getFirstWord().tokenString
        == ",")
    {
      // if the relative clause is preceeded by a comma
      // mark the relative clause
      // and ignore the comma in future
      if (i + 1 < phrases.size() && phrases[i + 1]->startsRelativeClause())
      {
        _markerVector[i] = REL;
      }
      // There can be a coordinating conjunction between the comma and the start
      // as in " , and who " or ", with whom"
      else if (i + 2 < phrases.size() && (phrases[i + 1]->getFirstWord().posTag
          == "CC" || phrases[i + 1]->getFirstWord().posTag == "IN") && phrases[i
          + 2]->startsRelativeClause())
      {
        _markerVector[i] = REL;
      }
      // A comma followed by a Verb in form VBG or VBN starts a relative clause
      // as well
      else if (i + 1 < phrases.size() && (phrases[i + 1]->getFirstWord().posTag
          == "VBG" || phrases[i + 1]->getFirstWord().posTag == "VBN"))
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

// ____________________________________________________________________________
void RobustContextDecomposer::printMarkedSentence(Sentence const & sentence)
{
  std::cout << std::endl << "MARK\tPHRASE" << std::endl;
  vector<Phrase *> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if (_markerVector[i] != NONE)
    {
      printMark(_markerVector[i]);
    }
    std::cout << "\t" << phrases[i]->getPhraseAsString() << std::endl;
  }
  std::cout << std::endl << std::endl;
}

// ____________________________________________________________________________
void RobustContextDecomposer::printMark(Mark mark)
{
  switch (mark)
  {
    case NONE:
      std::cout << "NONE";
      break;
    case LIST:
      std::cout << "LIST";
      break;
    case LISTV:
      std::cout << "LISTV";
      break;
    case SEP:
      std::cout << "SEP";
      break;
    case REL:
      std::cout << "REL";
      break;
    case RED:
      std::cout << "RED";
      break;
    case VIO:
      std::cout << "VIO";
      break;
    case IGNORE:
      std::cout << "IGNORE";
      break;
    default:
      std::cout << "Invalid MARK: " << mark << std::endl;
      exit(1);
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::printChunks(
    const boost::ptr_vector<SemanticChunk> & chunks, bool printGroups) const
{
  if (chunks.size() == 0)
  {
    std::cout << "NO SEMANTIC CHUNKS GENERATED" << std::endl;
  }
  std::cout << "Semantic Chunks(" << chunks.size() << "):" << std::endl;
  for (size_t i = 0; i < chunks.size(); i++)
  {
    std::cout << " Chunk " << i << ":" << std::endl;
    chunks[i].printChunk();
    std::cout << std::endl;
  }
}

// ____________________________________________________________________________
void RobustContextDecomposer::printChunks(
    const std::vector<SyntacticChunk*> & chunks, bool printGroups) const
{
  std::cout << "Syntactic Chunks(" << chunks.size() << "):" << std::endl;
  for (size_t i = 0; i < chunks.size(); i++)
  {
    std::cout << " Chunk " << i << std::endl;
    chunks[i]->printChunk();
  }
  std::cout << std::endl;
}

// ____________________________________________________________________________
std::vector<Context> const RobustContextDecomposer::decompose(
    Sentence & sentence)
{
  std::vector<ContextPtr> contexts = decomposeToPtr(sentence);
  return ContextDecomposerUtil::contextPtrToContext(contexts);
}

