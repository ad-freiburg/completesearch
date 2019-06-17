// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Hannah Bast <bast>, Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "./SemanticWikipediaDecomposer.h"
#include "./SiddharthanContextDecomposer.h"
#include "./ContextDecomposerUtil.h"

// ____________________________________________________________________________
SiddharthanContextDecomposer::SiddharthanContextDecomposer(bool writeToStdOut)
{
  _writeToStdOut = writeToStdOut;
}


// ____________________________________________________________________________
SiddharthanContextDecomposer::~SiddharthanContextDecomposer()
{
}

// ____________________________________________________________________________
std::vector<ContextPtr > const SiddharthanContextDecomposer::decomposeToPtr(
    Sentence & sentence)
{
  // markers just contains all marks in order of appearance
  _markers.clear();
  // _chunkMarkerVector has a mark for each syntactic chunk
  _chunkMarkerVector.clear();
  // markerVector has a mark for each phrase
  _phraseMarkerVector.clear();
  // mark all phrases with none
  _phraseMarkerVector.insert(_phraseMarkerVector.begin(),
      sentence.getPhrases().size(), NONE);
  // mark all commas
  markCommas(sentence);
  // mark all relative clauses
  markNonRestrictiveRelativeClausesStart(sentence);

  if (_writeToStdOut)
  {
    std::cout << "After Marking: " << std::endl;
    printMarkedSentence(sentence);
  }

  // create syntactic chunks according to our markings
  // createSyntacticChunks(sentence);
  // printChunks(sentence.getSyntacticChunks());
  // create semantic chunks from the marked RELs
  // createSemanticRELChunks(sentence);
  // create semantic chunks from the rest
  // createSemanticChunks(sentence);
  // printChunks(sentence.getSemanticChunks());
  // printChunks(sentence.getSyntacticChunks());

  return sentence.produceContextsPtr();
}


// ____________________________________________________________________________
void SiddharthanContextDecomposer::markCommas(
    Sentence & sentence)
{
  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if (phrases[i].getType() == "O" && phrases[i].getWords().size() == 1
        && phrases[i].getWords()[0]->tokenString == ",")
    {
      _phraseMarkerVector[i] = COMMA;
    }
  }
}


// ____________________________________________________________________________
size_t SiddharthanContextDecomposer::decideRestrictiveRelativeClauseEnd(
    Sentence& sentence, size_t rRelClauseStart)
{
  // nrRelClauseStart is the start - we decide where it ends and mark on the way

  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  std::string pronounX = phrases[rRelClauseStart].getFirstWord().tokenString;
  _markers.clear();
  size_t currentToken = rRelClauseStart;
  // select all following COMMAs in _markers
  for (size_t j = rRelClauseStart; j < _phraseMarkerVector.size(); ++j)
  {
    if (_phraseMarkerVector[j] == COMMA)
      _markers.push_back(std::make_pair(_phraseMarkerVector[j], j));
    else if (_phraseMarkerVector[j] == SEP)
      break;
  }

  // IF the relative pronoun is immediately followed by a comma THEN
  // jump to the token after the next comma
  if (phrases.size() > rRelClauseStart + 1
      && _phraseMarkerVector[rRelClauseStart + 1] == COMMA)
  {
    // there is a comma after that one
    if (_markers.size() > 1)
    {
      currentToken = _markers[1].second;
    }
    // there is only one comma - we are done
    else
    {
      _phraseMarkerVector.back() = R_REL_END;
      // we are done
      return _phraseMarkerVector.size();
    }
  }
  // jump forwards past one verb group and one noun group
  for (size_t i = currentToken; i < phrases.size(); ++i)
  {
    if (phrases[i].getType() == "VP")
    {
      // IF a complementiser was encountered after the verb group,
      // or the verb group contained a saying verb THEN jump ahead
      // past the next verb group as well
      if ((i + 1 < phrases.size() && phrases[i + 1].getWords()[0]->posTag
          == "IN") || phrases[i].containsSayingWord())
      {
        continue;
      }
      else
      {
        currentToken = i + 1;
        break;
      }
    }
  }
  for (size_t i = currentToken; i < phrases.size(); ++i)
  {
    if (phrases[i].getType() == "NP")
    {
     currentToken = i + 1;
     break;
    }
  }
  // maybe we didn't find a VP and NP then we are done
  if (currentToken >= phrases.size())
  {
    _phraseMarkerVector.back() = R_REL_END;
    // we are done
    return _phraseMarkerVector.size();
  }
  // FOR each comma, colon, semicolon, verb group or
  // relative pronoun (processing in a left to right order) DO
  for (size_t i = currentToken; i < phrases.size(); ++i)
  {
    if (phrases[i].getType() == "O")
    {
      // IF colon or semicolon or end of enclosing clause THEN END CLAUSE
      if (phrases[i].getFirstWord().tokenString == ":"
          || phrases[i].getFirstWord().tokenString == ";"
          || _phraseMarkerVector[i] == SEP)
      {
        _phraseMarkerVector[i] = R_REL_END;
        break;
      }
      else if (i + 1 < phrases.size() && _phraseMarkerVector[i] == COMMA)
      {
        // IF a comma followed by an appositive THEN INTERNAL comma
        if (_phraseMarkerVector[i] == APPOS)
        {
          continue;
        }
        // IF a comma followed by a verb group THEN
        //   IF the verb has POS "VB{N|G} THEN INTERNAL comma
        else if (phrases[i + 1].getType() == "VP" &&
        (phrases[i + 1].getWords()[0]->posTag == "VBG"
            || phrases[i + 1].getWords()[0]->posTag == "VBN"))
        {
          _phraseMarkerVector[i] = INT_COMMA;
       }
        // IF an implicit conjunction of adjectives or adverbs like "JJ, JJ"
        // or "RB, RB" then INTERNAL clause
        else if ((phrases[i + 1].getWords()[0]->posTag == "JJ"
            && phrases[i - 1].getWords()[0]->posTag == "JJ)")
            || (phrases[i + 1].getWords()[0]->posTag == "RB"
                && phrases[i - 1].getWords()[0]->posTag == "RB"))
        {
          _phraseMarkerVector[i] = INT_CLAUSE;
        }
      }
      // von hier an ist der pseudocode bullshit
    }
  }
  return 0;
}

// ____________________________________________________________________________
size_t SiddharthanContextDecomposer::decideNonRestrictiveRelativeClauseEnd(
    Sentence& sentence, size_t nrRelClauseStart)
{
  // nrRelClauseStart is the start - we decide where it ends and mark on the way
  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  _markers.clear();
  // LET n be the number of commas between ", {who|which}"
  // and the end of the sentence or enclosing clause (SEP)
  for (size_t j = nrRelClauseStart; j < _phraseMarkerVector.size(); ++j)
  {
    if (_phraseMarkerVector[j] == COMMA)
      _markers.push_back(std::make_pair(_phraseMarkerVector[j], j));
    else if (_phraseMarkerVector[j] == SEP)
      break;
  }
  // the next comma we are looking at
  size_t k = 0;
  // if n = 0 THEN clause extends till end of sentence
  if (_markers.size() == 0)
  {
    _phraseMarkerVector.back() = NR_REL_END;
    // we are done
    return _phraseMarkerVector.size();
  }
  // IF n > 0 THEN a decision needs to be made at each comma as follows
  else
  {
    // IF the relative pronoun is immediately followed by a comma THEN
    // jump to the token after the next comma
    if (phrases.size() > nrRelClauseStart + 1
        && _phraseMarkerVector[nrRelClauseStart + 1] == COMMA)
    {
      // there is a comma after that one
      if (_markers.size() > 1)
        k = 1;
      // there is only one comma - we are done
      else
      {
        _phraseMarkerVector.back() = NR_REL_END;
        // we are done
        return _phraseMarkerVector.size();
      }
    }
  }
  // For each comma (scanning from left to right) DO
  for (; k < _markers.size(); k++)
  {
    size_t phraseIndex = _markers[k].second;
    // range check - if nothing follows the comma this is an error
    if (phrases.size() <= phraseIndex + 1)
    {
      _phraseMarkerVector.back() = NR_REL_END;
      // we are done
      return _phraseMarkerVector.size();
    }
    // IF followed by an appositive THEN INTERNAL comma

    // IF followed by a verb group THEN
    //   IF the verb has POS "VB{N|G} THEN INTERNAL comma
    if (phrases[phraseIndex + 1].getType() == "VP" &&
    (phrases[phraseIndex + 1].getWords()[0]->posTag == "VBG"
        || phrases[phraseIndex + 1].getWords()[0]->posTag == "VBN"))
    {
      _phraseMarkerVector[phraseIndex] = INT_COMMA;
      std::cout << "attack_attack";
    }
    // IF an implicit conjunction of adjectives or adverbs like "JJ, JJ"
    // or "RB, RB" then INTERNAL clause
    else if ((phrases[phraseIndex + 1].getWords()[0]->posTag == "JJ"
        && phrases[phraseIndex - 1].getWords()[0]->posTag == "JJ)")
        || (phrases[phraseIndex + 1].getWords()[0]->posTag == "RB"
            && phrases[phraseIndex - 1].getWords()[0]->posTag == "RB"))
    {
      _phraseMarkerVector[phraseIndex] = INT_CLAUSE;
    }
    // IF it is a Pronoun_X clause where Pronoun_X={who|which} then
    //   IF ", CC Pronoun_X" THEN INTERNAL clause and DELETE "Pronoun_X"
    else if (phrases.size() > phraseIndex + 2
        && phrases[phraseIndex + 1].getWords()[0]->posTag == "CC"
        && (phrases[phraseIndex + 2].getWords()[0]->posTag == "WDT"
            || phrases[phraseIndex + 2].getWords()[0]->posTag == "WP"))
    {
      _phraseMarkerVector[phraseIndex] = INT_CLAUSE;
    }
    //   IF ",{who|which|that}" THEN INTERNAL comma
    else if (phrases[phraseIndex + 1].getWords()[0]->posTag == "WDT"
        || phrases[phraseIndex + 1].getWords()[0]->posTag == "WP")
    {
      _phraseMarkerVector[phraseIndex] = INT_COMMA;
    }
    else
    {
      _phraseMarkerVector[phraseIndex] = NR_REL_END;
      return phraseIndex + 1;
    }
  }
  _phraseMarkerVector.back() = NR_REL_END;
  // we are done
  return _phraseMarkerVector.size();
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::markNonRestrictiveRelativeClausesStart(
    Sentence& sentence)
{
  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if ((phrases[i].startsRelativeClause() && i != 0))
    {
      // if the relative clause is preceeded by a comma
      // mark the relative clause
      // and ignore the comma in future
      if (phrases[i - 1].getWords().size() == 1
          && phrases[i - 1].getFirstWord().tokenString == ",")
      {
        _phraseMarkerVector[i] = NR_REL_START;
        _phraseMarkerVector[i - 1] = IGNORE;
        i = decideNonRestrictiveRelativeClauseEnd(sentence, i);
      }
    }
  }
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::markRestrictiveRelativeClausesStart(
    Sentence& sentence)
{
  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if ((phrases[i].startsRelativeClause() && i != 0))
    {
      // only if the relative clause is not preceeded by a comma
      // mark the relative clause as restrictive
      if (phrases[i - 1].getWords().size() != 1
          || phrases[i - 1].getFirstWord().tokenString != ",")
      {
        _phraseMarkerVector[i] = R_REL_START;
        i = decideRestrictiveRelativeClauseEnd(sentence, i);
      }
    }
  }
}




// ____________________________________________________________________________
void SiddharthanContextDecomposer::createSemanticChunks(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  SemanticChunk * head = chunks[0]->getSemanticChunk();
  chunks.erase(chunks.begin());
  _chunkMarkerVector.erase(_chunkMarkerVector.begin());
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    if (_chunkMarkerVector[i] == LIST)
    {
      head->appendSingleEnumerationElement(*chunks[i]);
      chunks.erase(chunks.begin()+i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin()+i);
      --i;
    }
    else if (_chunkMarkerVector[i] == SEP)
    {
      head = chunks[i]->getSemanticChunk();
      chunks.erase(chunks.begin()+i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin()+i);
      --i;
    }
  }
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::createSemanticRELChunks(
    Sentence& sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
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
        head = chunks[i-1]->getSemanticChunk();
      head->addRelativeChunk(*chunks[i]);
      // remember this chunk in case a list starts with the next chunk (a SEP
      // would end the REL)
      previousChunk = chunks[i];
      // remove from chunk-list and marker-list
      chunks.erase(chunks.begin()+i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin()+i);
      i--;
      relOpen = true;
    }
    else if (relOpen && _chunkMarkerVector[i] == LIST)
    {
      head = previousChunk->getSemanticChunk();
      head->appendSingleEnumerationElement(*chunks[i]);
      // std::cout << "RELLIST";
      // remove from chunk-list and marker-list
      chunks.erase(chunks.begin()+i);
      _chunkMarkerVector.erase(_chunkMarkerVector.begin()+i);
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
void SiddharthanContextDecomposer::createSyntacticChunks(
    Sentence& sentence)
{
  boost::ptr_vector <Phrase> const & phrases = sentence.getPhrases();
  SyntacticChunk * synChunk = new SyntacticChunk(sentence);
  _chunkMarkerVector.push_back(_phraseMarkerVector[0]);
  sentence.appendAndStoreSyntacticChunk(*synChunk);
  for (size_t i = 0; i < _phraseMarkerVector.size(); ++i)
  {
    if (_phraseMarkerVector[i] != NONE && _phraseMarkerVector[i] != IGNORE
        && _phraseMarkerVector[i] != VIO)
    {
      synChunk = new SyntacticChunk(sentence);
      _chunkMarkerVector.push_back(_phraseMarkerVector[i]);
      sentence.appendAndStoreSyntacticChunk(*synChunk);
      synChunk->appendPhrase(phrases[i]);
    }
    else
      synChunk->appendPhrase(phrases[i]);
  }
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::markSEPandLIST(
    Sentence& sentence)
{
  std::vector <Mark> marks;
  boost::ptr_vector <Phrase> const & phrases = sentence.getPhrases();


  for (size_t i = 0; i < _markers.size(); ++i)
  {
    if (_markers[i].first == RED)
    {
    // SEP wenn nächste Farbe VIO, und nicht als nächstes Verb
    if (i < (_markers.size() - 1)
          && _markers[i+1].first == VIO
          && !phrases[_markers[i+1].second].getWords()[0]->posTag[0] == 'V')
      {
        _markers[i].first = SEP;
        _phraseMarkerVector[_markers[i].second] = SEP;
      }
    // sonst LIST
      else
      {
        _markers[i].first = LIST;
        _phraseMarkerVector[_markers[i].second] = LIST;
      }
    }
  }
}



// ____________________________________________________________________________
void SiddharthanContextDecomposer::printMarkedSentence(
    Sentence const & sentence)
{
  std::cout << std::endl << "MARK\tPHRASE" << std::endl;
  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
  for (size_t i = 0; i < phrases.size(); i++)
  {
    if (_phraseMarkerVector[i] != NONE)
    {
      printMark(_phraseMarkerVector[i]);
    }
    std::cout << "\t" << phrases[i].getPhraseAsString() << std::endl;
  }
  std::cout << std::endl << std::endl;
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::printMark(Mark mark)
{
  switch (mark)
  {
    case NONE:
      std::cout << "NONE";
      break;
    case LIST:
      std::cout << "LIST";
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
    case INT_CLAUSE:
      std::cout << "INT_CLAUSE";
      break;
    case INT_COMMA:
      std::cout << "INT_COMMA";
      break;
    case COMMA:
      std::cout << "COMMA";
      break;
    case NR_REL_START:
      std::cout << "NR_REL_START";
      break;
    case NR_REL_END:
      std::cout << "NR_REL_END";
      break;
    default:
      std::cout << "Invalid MARK: " << mark << std::endl;
      exit(1);
  }
}


// ____________________________________________________________________________
void SiddharthanContextDecomposer::printChunks(
    const boost::ptr_vector <SemanticChunk> & chunks, bool printGroups) const
{
  if (chunks.size() == 0)
  {
    std::cout << "NO SEMANTIC CHUNKS GENERATED" << std::endl;
  }
  std::cout << "Semantic Chunks(" << chunks.size() << "):" <<
    std::endl;
  for (size_t i = 0; i < chunks.size(); i++)
  {
    std::cout << " Chunk " << i << ":" << std::endl;
    chunks[i].printChunk();
    std::cout << std::endl;
  }
}

// ____________________________________________________________________________
void SiddharthanContextDecomposer::printChunks(
    const std::vector <SyntacticChunk*> & chunks, bool printGroups) const
{
  std::cout  <<
    "Syntactic Chunks(" << chunks.size() << "):" <<
      std::endl;
  for (size_t i = 0; i < chunks.size(); i++)
  {
    std::cout << " Chunk " << i << std::endl;
    chunks[i]->printChunk();
  }
  std::cout << std::endl;
}

// ____________________________________________________________________________
std::vector<Context> const SiddharthanContextDecomposer::decompose(
    Sentence & sentence)
{
  std::vector<ContextPtr> contexts = decomposeToPtr(sentence);
  return ContextDecomposerUtil::contextPtrToContext(contexts);
}

