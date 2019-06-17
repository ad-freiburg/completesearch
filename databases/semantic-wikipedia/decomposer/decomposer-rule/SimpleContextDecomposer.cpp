// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <boost/regex.hpp>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "base/SemanticWikipediaDecomposer.h"
#include "decomposer-rule/SimpleContextDecomposer.h"
#include "util/ContextDecomposerUtil.h"

#define ADJ  "(JJ(R |S |  ) )"
#define NOUN "(NN(S |P |PS|  ) )"

#define DET_ADJ_NOUN DET "*" ADJ "*" NOUN "+"
#define COMMA "(,    )"
#define DET "(DT   )"
#define PREPOSITION "(IN   )"
#define VERB "(V..  )"
// gerund: taking
#define VERB_G "(V.G  )"
#define RELPRON "(W..  )"


// what to do about " , in essence , " ", for example ," ", in turn ,"

// thomas o. paine , the deputy administrator of nasa , had
#define APP1 NOUN COMMA DET_ADJ_NOUN PREPOSITION DET_ADJ_NOUN COMMA VERB
// project tiger , a tiger conservation project , is launched
#define APP2 NOUN COMMA DET_ADJ_NOUN COMMA VERB
// an amoeba , from the order amoebida ,
#define APP3 NOUN COMMA PREPOSITION DET_ADJ_NOUN COMMA VERB
// injections , which lasted for a year ,
#define REL1 NOUN COMMA RELPRON VERB DET_ADJ_NOUN COMMA

// ____________________________________________________________________________
SimpleContextDecomposer::SimpleContextDecomposer()
{
  std::string factRegexes[] =
  {
    APP1,
    APP2,
    APP3,
    REL1
  };
  for (uint16_t i = 0; i < sizeof(factRegexes)/sizeof(factRegexes[0]) ; i++)
  {
    // std::cout << "\"" << factRegexes[i] << "\"" << std::endl;
    boost::regex simpleRegex(factRegexes[i]);
    _factRegexes.push_back(simpleRegex);
  }
}

// ____________________________________________________________________________
SimpleContextDecomposer::~SimpleContextDecomposer()
{
}

// ____________________________________________________________________________
int SimpleContextDecomposer::extractFacts(
    Sentence const & sentence) const
{
  uint16_t hits = 0;
  for (uint16_t i = 0; i < _factRegexes.size(); i++)
  {
    boost::match_results <std::string::const_iterator> what;
    std::string::const_iterator start, end;
    std::string posSentence = sentence.getPOSSentenceAsString();
    start = posSentence.begin();
    end = posSentence.end();
    while (boost::regex_search(start, end, what, _factRegexes[i]))
    {
      hits++;
      std::cout << "HIT Sentence : " << sentence.getSentenceAsString()
        << std::endl;
      std::cout << "HIT EXTRACT  : " << (what[0].first-start) << ": ";

      uint16_t startIndex = (what[0].first-start)/5;
      uint16_t endIndex = (what[0].second-start)/5;
      boost::ptr_vector <Token> sentenceWords =
        sentence.getWords();
      for (uint16_t j = startIndex; j< endIndex; j++)
      {
        std::cout << sentenceWords[j].tokenString
          << (sentenceWords[j].isPartOfEntity? "(E)" : "") << " ";
      }
      std::cout << std::endl;
      std::cout << "HIT Pattern  : " << _factRegexes[i] << ": "
        << std::endl;
      std::cout << "HIT POS  : " << sentence.getPOSSentenceAsString()
        << std::endl << std::endl;
      start = what[0].second;
    }
  }
  return hits;
}

// ____________________________________________________________________________
std::vector<ContextPtr >  const SimpleContextDecomposer::decomposeToPtr(
    Sentence & sentence)
{
  // first perform some trivial chunking, this should chunk
  // finer than necessary!
  // so hopefully we avoid having to recurse into the chunks
  simpleChunkSentence(sentence);
  std::cout << "### AFTER SIMPLE CHUNKING" << std::endl;
  printChunks(sentence.getSyntacticChunks());

  contractOtherCollocations(sentence);
  contractConjunctions(sentence);
  identifyAndCombinePhraseGroups(sentence);
  markEnumerations(sentence);
  std::cout << "### AFTER COMBINUNG CHUNKS" << std::endl;
  printChunks(sentence.getSyntacticChunks(), false);

  extractRelativeClauses(sentence);
  extractAppositions(sentence);
  std::cout << "### EXTRACTED FACTS" << std::endl;
  printChunks(sentence.getSyntacticChunks(), false);
  printChunks(sentence.getSemanticChunks(), false);

  std::cout << std::endl << std::endl << std::endl;

  return sentence.produceContextsPtr();
}

void SimpleContextDecomposer::contractOtherCollocations(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();

  // TODO(elmar): externalize this
  std::string contractStrings[6][2]  = { {"in", "which"},
    {"just", "as"},
    {"whenever", "and"},
    {"something", "which"},
    {"wherever", "and"},
    {"proposing", "that"}};
  size_t NUM_CONTRACTSTRINGS = 6;

  // go through all chunks
  for (size_t i = 0; i < chunks.size(); i++)
  {
    // go through all expressions to concat
    for (size_t j = 0; j < NUM_CONTRACTSTRINGS; j++)
    {
      if ( i > 0
          && chunks[i]->getFirstWord().tokenString == contractStrings[j][1]
          && chunks[i-1]->getLastWord().tokenString == contractStrings[j][0])
      {
        std::cout << "CONTRACTING BY SAYING" << std::endl;
        chunks[i-1]->appendChunk(*chunks[i]);
        chunks.erase(chunks.begin()+i);
        i--;
      }
    }
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::contractConjunctions(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();

  for (size_t i = 0; i < chunks.size(); i++)
  {
    // if the chunk starts with a coordinating conjunction and contains
    // no verbphrase we append it to the previous chunk
    if ( i > 0 && chunks[i]->getWords()[0]->posTag == "CC"
        && !chunks[i]->containsVerbPhrase())
      //  && !chunks[i-1]->startsWithComma())
    {
        std::cout << "CONTRACTING BY RULE" << std::endl;
      chunks[i-1]->appendChunk(*chunks[i]);
      chunks.erase(chunks.begin()+i);
      i--;
    }
    // if the chunk starts with a coordinating conjunction and is followed
    // by a verbphrase it cannot stand alone and we attach it to the previous
    // chunk
    else if ( i > 0 && chunks[i]->getWords()[0]->posTag == "CC"
        && chunks[i]->startsWithVerbPhrase() && !chunks[i-1]->startsWithComma())
    {
      std::cout << "CONTRACTING SUBCHUNK BY RULE" << std::endl;
      // get the previous semantic chunk or create if it doesnt exist yet
      // TODO(elmar): change this, eiter move to seperate function or change
      // design
      SemanticChunk * semanticChunk = chunks[i-1]->getSemanticChunk();
      std::vector <SyntacticChunk *> enumeration;
      enumeration.push_back(chunks[i]);
      semanticChunk->addEnumeration(enumeration);
      // now append and erase the previous chunk
      chunks[i-1]->appendChunk(*chunks[i]);
      chunks.erase(chunks.begin()+i);
      i--;
    }
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::markEnumerations(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  // points to the index of the last enumeration chunk
  // or -1 to indicate no enumeration is currently open
  bool enumerationOpen = false;
  size_t startEnumIndex = 0;
  std::vector <SyntacticChunk *> enumChunks;
  std::vector <size_t> deleteIndexes;
  // TODO(elmar): what about recursive enumerations?
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    // check for a possible enumeration start
    if (!enumerationOpen && chunks[i]->endsWithNounPhrase())
    {
      enumChunks.push_back(chunks[i]);
      enumerationOpen = true;
      startEnumIndex = i;
      deleteIndexes.push_back(i);
      continue;
    }
    // end of the enumeration
    else if (enumerationOpen && (startEnumIndex + 1) < i
        && chunks[i]->startsWithCC())
    {
      std::cout << "TRIVIAL ENUMERATION FOUND" << std::endl;
      enumChunks.push_back(chunks[i]);
      deleteIndexes.push_back(i);

      // add a new fact
      SemanticChunk * semanticChunk = enumChunks.front()->getSemanticChunk();
      for (size_t j = 1; j < enumChunks.size(); ++j)
      {
        // because indexes must be in ascending order
        enumChunks[j]->setPartOfEnumeration(true);
        // chunks.erase(chunks.begin()+(deleteIndexes[j]-j+1));
        // chunks.erase(chunks.begin()+j);
      }
      enumChunks.erase(enumChunks.begin());
      semanticChunk->addEnumeration(enumChunks);
      // because we remove some stuff continue with the same element
      // i -= deleteIndexes.size();
      enumChunks.clear();
      deleteIndexes.clear();
      enumerationOpen = false;
      continue;
    }
    // trivial enumeration - element does not contain a verbphrase
    // and starts with a comma
    else if (enumerationOpen
        && !chunks[i]->containsVerbPhrase()
        && chunks[i]->startsWithComma()
        && chunks[i]->getWords().size() > 1)
    {
      enumChunks.push_back(chunks[i]);
      deleteIndexes.push_back(i);
      continue;
    }
    else
    {
      deleteIndexes.clear();
      enumChunks.clear();
      enumerationOpen = false;
    }
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::extractAppositions(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  // points to the index of the last enumeration chunk
  for (size_t i = 0; i < chunks.size(); ++i)
  {
    // if the chunk is not part of an enumeration
    // and it contains no verb phrase, or stats with one
    if (i > 0
        && !chunks[i]->isPartOfEnumeration()
        && chunks[i]->startsWithComma()
        && !chunks[i]->containsVerbPhrase())
    {
      std::cout << "EXTRACT APPOSITION BY RULE" << std::endl;
      SemanticChunk * semanticChunk = chunks[i-1]->getLastSemanticChunk();
      // sentence.appendAndStoreSemanticChunk(*fact);
      // TODO(elmar): select the last entity that matches (person nonperson)
      semanticChunk->addAppositionChunk(*chunks[i]);
      chunks.erase(chunks.begin()+i);
      --i;
    }
    // TODO(elmar): this is a relative clause by definition
    else if (i > 0
        && !chunks[i]->isPartOfEnumeration()
        && chunks[i]->startsWithComma()
        && chunks[i]->startsWithVBGN())
    {
      std::cout << "EXTRACT VB-APPOSITION STARTING BY RULE" << std::endl;
      SemanticChunk * semanticChunk = chunks[i-1]->getLastSemanticChunk();
      semanticChunk->addAppositionChunk(*chunks[i]);
      chunks.erase(chunks.begin()+i);
    }
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::extractRelativeClauses(
    Sentence& sentence)
{
  std::vector <SyntacticChunk* > & chunks = sentence.getSyntacticChunks();
  for (size_t i = chunks.size(); i > 0 ;)
  {
    i--;
    // if the chunk starts with a relative (pronoun)
    if (chunks[i]->startsRelativeClause() && chunks[i]->getPhrases().size() > 1)
    {
      // and the previous chunk ends with a noun
      if ( i > 0 && chunks[i-1]->endsWithNounPhrase())
      {
        std::cout << "EXTRACT RELATIVE CLAUSE BY RULE" << std::endl;
        // add a new fact
        SemanticChunk * semanticChunk = chunks[i-1]->getLastSemanticChunk();
        // TODO(elmar): select the last entity that matches (person nonperson)
        semanticChunk->addRelativeChunk(*chunks[i]);
        // chunks.erase(chunks.begin()+i);
      }
    }
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::simpleChunkSentence(
    Sentence& sentence)
{
  SyntacticChunk * chunk = new SyntacticChunk(sentence);
  sentence.appendAndStoreSyntacticChunk(*chunk);

  boost::ptr_vector <Phrase> const & phrases = sentence.getPhrases();
  bool previousPhraseStop = false;
  for (size_t i = 0; i < phrases.size(); i++)
  {
    // trivial: begin a new chunk with O-phrase and SBAR-phrase, but only if
    // it is not the first phrase(else we already have a chunk)
    // O-phrase: 70% = "," - 20% = "and" , 2% = "but", 2% = "or"
    // TODO(elmar): externalize and extend the "stopwords"
    if ((phrases[i].isStopPhrase())  && !previousPhraseStop
        && i != 0)
    {
      chunk = new SyntacticChunk(sentence);
      previousPhraseStop = true;
      sentence.appendAndStoreSyntacticChunk(*chunk);
    }
    else if (!phrases[i].isStopPhrase())
    {
      previousPhraseStop = false;
    }
    chunk->appendPhrase(phrases[i]);
  }
}

// ____________________________________________________________________________
void SimpleContextDecomposer::printChunks(
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
void SimpleContextDecomposer::printChunks(
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
void SimpleContextDecomposer::identifyAndCombinePhraseGroups(
    Sentence & sentence)
{
  std::vector <SyntacticChunk *> & chunks = sentence.getSyntacticChunks();
  /* for (size_t i = 0; i < chunks.size(); i++)
  {
    PhraseGroup * group = NULL;
    bool isPrevNP = false;
    std::vector <Phrase const *> const & phrases = chunks[i]->getPhrases();
    for (size_t j = 0; j < phrases.size(); j++)
    {
      if (phrases[j]->getType() == "VP")
      {
        group = new PhraseGroup();
        chunks[i]->appendAndStorePhraseGroup(*group);
        isPrevNP = false;
      }
      else if (!isPrevNP)
      {
        group = new PhraseGroup();
        chunks[i]->appendAndStorePhraseGroup(*group);
        isPrevNP = true;
      }
      group->appendPhrase(*phrases[j]);
    }
  } */
}

// ____________________________________________________________________________
std::vector<Context> const SimpleContextDecomposer::decompose(
    Sentence & sentence)
{
  std::vector<ContextPtr> contexts = decomposeToPtr(sentence);
  return ContextDecomposerUtil::contextPtrToContext(contexts);
}
