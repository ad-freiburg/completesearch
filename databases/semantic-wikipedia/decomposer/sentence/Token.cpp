// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "util/ContextDecomposerUtil.h"
#include "sentence/Token.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

// For internal representation of strings of POS tags
// and phrase tags.
#define POSTAG_LENGTH 5
#define POSTAG_FORMAT "%-5s"
#define PHRASETAG_LENGTH 6
#define PHRASETAG_FORMAT "%-6s"

using std::string;


// ____________________________________________________________________________
template <class Token>
Entity<Token>::~Entity()
{
}

// ____________________________________________________________________________
template <class Token>
void Entity<Token>::addWord(Token & word)
{
  _entityWords.push_back(&word);
}

// ____________________________________________________________________________
template <class Token>
bool Entity<Token>::hasWord()
{
  return(!_entityWords.empty());
}


// ____________________________________________________________________________
template <class Token>
void Entity<Token>::setIsPersonEntity(bool isPersonEntity)
{
  _isPersonEntity = isPersonEntity;
}

// ____________________________________________________________________________
template <class Token>
bool Entity<Token>::isPersonEntity()
{
  return _isPersonEntity;
}

// ____________________________________________________________________________
template <class Token>
void Entity<Token>::addPath(Token & path)
{
  _entityPaths.push_back(&path);
  _entityId = path.tokenString;
}

// ____________________________________________________________________________
template <class Token>
std::vector <Token *> const & Entity<Token>::getPaths() const
{
  return _entityPaths;
}

// ____________________________________________________________________________
template <class Token>
bool Entity<Token>::operator == (
    Entity<Token> const & otherEntity) const
{
  // TODO(elmar): this is slow, make faster
  return (_entityId == otherEntity.getEntityId());
}

// ____________________________________________________________________________
template <class Token>
const string & Entity<Token>::getEntityId() const
{
  return _entityId;
}


// ____________________________________________________________________________
template <class Mark>
Token<Mark>::Token()
:tokenString(""), completeSearchDocId(0), completeSearchScore(0),
  completeSearchPosition(0) , tokenWordPosition(0),
  tokenCharPosition(0), posTag(""), npTag(""), brTag("*"),
  isPartOfEntity(false), _entity(NULL), _phrase(NULL)
{
  // std::cout << "New token created \"<< std::flush;
}

// ____________________________________________________________________________
template <class Mark>
void Token<Mark>::setBrTags(string const & tags)
{
  brTag = tags;
  if (tags != "*")
  {
    std::vector<string> brTags;
    boost::split(brTags, tags, boost::is_any_of(","));
    for (size_t i = 0; i < brTags.size(); ++i)
    {
      marks.push_back(Token::stringToMark(brTags[i]));
    }
  }
}

// ____________________________________________________________________________
template <class Mark>
void Token<Mark>::clearBrTags()
{
  brTag = "*";
  marks.clear();
}


// ____________________________________________________________________________
template <class Mark>
void Token<Mark>::appendMark(Type mark)
{
  if (marks.size() == 0)
    brTag = Token::markToString(mark);
  else
    brTag = brTag+","+Token::markToString(mark);
  marks.push_back(mark);
}

// ____________________________________________________________________________
template <class Mark>
void Token<Mark>::prependMark(Type mark)
{
  marks.insert(marks.begin(), mark);
  if (brTag == "*" || brTag == "")
    brTag = Token::markToString(mark);
  else
    brTag = Token::markToString(mark)+","+brTag;
}

// ____________________________________________________________________________
template <class Mark>
string Token<Mark>::asString() const
{
  return tokenString;
}

// ____________________________________________________________________________
template <>
bool Token<DeepTokenMarks>::isOpenMark(DeepTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DeepTokenMarks::C):
      return true;
      break;
    case(DeepTokenMarks::C_STAR):
      return true;
      break;
    case(DeepTokenMarks::C_H):
      return true;
      break;
    case(DeepTokenMarks::C_HAT):
      return true;
      break;
    case(DeepTokenMarks::ENUM):
      return true;
      break;
    default:
      return false;
  }
}

// ____________________________________________________________________________
template <>
bool Token<DefaultTokenMarks>::isOpenMark(DefaultTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DefaultTokenMarks::REL_OPEN):
      return true;
      break;
    case(DefaultTokenMarks::LIT_OPEN):
      return true;
      break;
    case(DefaultTokenMarks::RELA_OPEN):
      return true;
      break;
    default:
      return false;
  }
}

// ____________________________________________________________________________
template <>
bool Token<DefaultTokenMarks>::isCloseMark(DefaultTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DefaultTokenMarks::REL_CLOSE):
      return true;
      break;
    case(DefaultTokenMarks::LIT_CLOSE):
      return true;
      break;
    case(DefaultTokenMarks::RELA_CLOSE):
      return true;
      break;
    default:
      return false;
  }
}

// ____________________________________________________________________________
template <>
bool Token<DeepTokenMarks>::isCloseMark(DeepTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DeepTokenMarks::CLOSE):
      return true;
      break;

    default:
      return false;
  }
}

// ____________________________________________________________________________
template <>
DefaultTokenMarks::TypeEnum Token<DefaultTokenMarks>::stringToMark(
    string const & markString)
{
  if (markString == "LIT(")
  {
    return DefaultTokenMarks::LIT_OPEN;
  }
  else if (markString == "LIT)")
  {
    return DefaultTokenMarks::LIT_CLOSE;
  }
  else if (markString == "REL(")
  {
    return DefaultTokenMarks::REL_OPEN;
  }
  else if (markString == "REL)")
  {
    return DefaultTokenMarks::REL_CLOSE;
  }
  else if (markString == "RELA(")
  {
    return DefaultTokenMarks::RELA_OPEN;
  }
  else if (markString == "RELA)")
  {
    return DefaultTokenMarks::RELA_CLOSE;
  }
  else if (markString == "SEP")
  {
    return DefaultTokenMarks::SEP;
  }
  else
  {
    LOG(ERROR)  << "Invalid mark in input: '" << markString <<  "'"
        << std::endl;
    exit(1);
  }
}

// ____________________________________________________________________________
template <>
DeepTokenMarks::TypeEnum Token<DeepTokenMarks>::stringToMark(
    string const & markString)
{
  if (markString == ")")
  {
    return DeepTokenMarks::CLOSE;
  }
  else if (markString == "(C*")
  {
    return DeepTokenMarks::C_STAR;
  }
  else if (markString == "(C")
  {
    return DeepTokenMarks::C;
  }
  else if (markString == "(CH")
  {
    return DeepTokenMarks::C_H;
  }
  else if (markString == "(C'")
  {
    return DeepTokenMarks::C_HAT;
  }
  else if (markString == "(ENUM")
  {
    return DeepTokenMarks::ENUM;
  }
  else
  {
    LOG(ERROR)  << "Invalid mark in input: '" << markString <<  "'"
        << std::endl;
    exit(1);
  }
}

// ____________________________________________________________________________
template <>
string Token<DefaultTokenMarks>::markToString(DefaultTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DefaultTokenMarks::LIT_OPEN):
      return "LIT(";
    case(DefaultTokenMarks::LIT_CLOSE):
      return "LIT)";
    case(DefaultTokenMarks::REL_OPEN):
      return "REL(";
    case(DefaultTokenMarks::REL_CLOSE):
      return "REL)";
    case(DefaultTokenMarks::RELA_OPEN):
      return "RELA(";
    case(DefaultTokenMarks::RELA_CLOSE):
      return "RELA)";
    case(DefaultTokenMarks::SEP):
      return "SEP";
    default:
      LOG(ERROR)  << "Invalid mark in input: '" << mark << "'" << std::endl;
      exit(1);
  }
  return "Nonse";
}

// ____________________________________________________________________________
template <>
string Token<DeepTokenMarks>::markToString(DeepTokenMarks::TypeEnum mark)
{
  switch (mark)
  {
    case(DeepTokenMarks::C):
      return "(C";
    case(DeepTokenMarks::C_STAR):
      return "(C*";
    case(DeepTokenMarks::C_H):
      return "(CH";
    case(DeepTokenMarks::C_HAT):
      return "(C'";
    case(DeepTokenMarks::ENUM):
      return "(ENUM";
    case(DeepTokenMarks::CLOSE):
      return ")";
    default:
      LOG(ERROR)  << "Invalid mark in input: '" << mark << "'" << std::endl;
      exit(1);
  }
  return "Nonse";
}



// ____________________________________________________________________________
template <>
bool Token<DefaultTokenMarks>::opensClause(
    std::vector<DefaultTokenMarks::TypeEnum> const & marks)
{
  for (size_t i = 0; i < marks.size(); ++i)
  {
    if (Token::isOpenMark(marks[i]))
      return true;
  }
  return false;
}

// ____________________________________________________________________________
template <>
bool Token<DefaultTokenMarks>::closesClause(
    std::vector<DefaultTokenMarks::TypeEnum> const & marks)
{
  for (size_t i = 0; i < marks.size(); ++i)
  {
    if (Token::isCloseMark(marks[i]))
      return true;
  }
  return false;
}

// ____________________________________________________________________________
template<class Mark>
std::string Token<Mark>::marksAsString(std::vector<Type> & marks)
{
  std::string s = "";
  for (size_t i = 0; i < marks.size(); ++i)
  {
    s += Token<Mark>::markToString(marks[i]);
  }
  return s;
}

template class Token<DeepTokenMarks>;
template class Token<DefaultTokenMarks>;
template class Entity<DefaultToken>;
template class Entity<DeepToken>;


