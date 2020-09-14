// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_TOKEN_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_TOKEN_H_

#include <gtest/gtest.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "../codebase/semantic-wikipedia-utils/Globals.h"

using std::string;

// forward declaration
template <class Token> class Entity;
template <class Token> class Sentence;
template <class Token> class Phrase;

// Used for the initial implementations of rule-based
// and ml-based identification.
struct DefaultTokenMarks
{
    enum TypeEnum
    {
      NONE = 0,
      SEP = 1,
      REL_OPEN = 2,
      REL_CLOSE = 3,
      RELA_OPEN = 4,
      RELA_CLOSE = 5,
      LIT_OPEN = 6,
      LIT_CLOSE = 7
    };
};

// Used for the implementation of deep-parser based identification.
struct DeepTokenMarks
{
    enum TypeEnum
    {
      NONE = 0,
      C = 1,
      C_STAR = 2,
      C_HAT = 3,
      C_H = 4,
      ENUM = 5 ,
      CLOSE = 6
    };
};

// Resembles a token in the index file.
// Can e.g be a word, an entity path, or a special word.
template <class Mark>
class Token
{
  public:
    typedef typename Mark::TypeEnum Type;
    explicit Token();
    // The string  of the token as given in the input.
    string tokenString;
    // Document id as provided in input.
    ad_semsearch::Id completeSearchDocId;
    // Hit-score as provided in input.
    ad_semsearch::Score completeSearchScore;
    // Position as provided in input.
    ad_semsearch::Position completeSearchPosition;
    // If this token is a word the position in the sentence, starting at 0.
    ad_semsearch::Position tokenWordPosition;
    // The char position of the token within the sentence, starting at 0.
    ad_semsearch::Position tokenCharPosition;
    // POS-tag as provided in input.
    string posTag;
    // NP-tag as provided in input.
    string npTag;
    // Clause-tag as provided in input.
    string cTag;
    // Broccoli-tag as provided in input.
    string brTag;
    // Is the token part of an entity.
    bool isPartOfEntity;
    // A link back to the complete entity of the token (if it exists).
    Entity<Token<Mark> > const * _entity;
    // A link to the phrase the token belongs to.
    Phrase<Token<Mark> > const * _phrase;
    // A vector of constituent identification marks - initially empty.
    std::vector<Type> marks;
    void appendMark(Type mark);
    void prependMark(Type mark);
    void setBrTags(string const & tags);
    void clearBrTags();

    // Return a string representation of the token,
    // which is just the token string.
    string asString() const;

    // Returns true if the provided enum element
    // opens a clauses.
    static bool isOpenMark(Type mark);
    // Returns true if the provided enum element
    // closes a clauses.
    static bool isCloseMark(Type mark);
    // Convert a single mark to a string representation.
    static string markToString(Type mark);
    // Returns true if any of the marks opens a clause.
    // False otherwise.
    static bool closesClause(std::vector<Type> const & marks);
    // Returns true if any of the marks closes a clause.
    // False otherwise.
    static bool opensClause(std::vector<Type> const & marks);
    // Convert a vector of marks to a string represenation.
    static std::string marksAsString(std::vector<Type> & marks);
    // Convert a string representation of a mark to its enum element.
    static Type stringToMark(string const & markString);
};

typedef Token<DefaultTokenMarks> DefaultToken;
typedef Token<DeepTokenMarks> DeepToken;

// An entity in the index file. Spans certain words and
// is described by a set of paths.
template <class Token>
class Entity
{
  public:
    ~Entity();
    // Add a word this entity references.
    void addWord(Token & word);
    bool hasWord();
    // Add a path that describes this entity. The
    void addPath(Token & path);
    std::vector <Token *> const & getPaths() const;
    // set the flag indicating whether this is a person entity
    void setIsPersonEntity(bool isPersonEntity);
    // is this entity a person
    bool isPersonEntity();
    string const & getEntityId() const;
    bool operator == (Entity const & otherEntity) const;
  private:
  // an ID used to identify an entity
  string _entityId;
  std::vector <Token *> _entityPaths;
  std::vector <Token *> _entityWords;
  bool _isPersonEntity;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_TOKEN_H_
