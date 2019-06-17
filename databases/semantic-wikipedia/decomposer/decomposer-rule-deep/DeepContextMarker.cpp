// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <string>
#include <map>
#include <vector>
#include "decomposer-rule-deep/DeepContextMarker.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "sentence/Sentence.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

namespace ad_decompose
{
map<string, DeepParserNodeTypes::TypeEnum> const
  DeepParserNodeTypes::stringToTypeMap =
      DeepParserNodeTypes::initStringToTypeMap();
map<DeepParserNodeTypes::TypeEnum, string> const
  DeepParserNodeTypes::typeToStringMap =
      DeepParserNodeTypes::initTypeToStringMap();

map<DeepParserNodeTypes::TypeEnum, string>
  DeepParserNodeTypes::initTypeToStringMap()
{
  map<DeepParserNodeTypes::TypeEnum, string> typeToStringMap;
  map<string, DeepParserNodeTypes::TypeEnum>::const_iterator it;
  for (it = stringToTypeMap.begin(); it != stringToTypeMap.end(); ++it)
  {
    typeToStringMap.insert(std::make_pair(it->second, it->first));
  }
  return typeToStringMap;
}

map<string, DeepParserNodeTypes::TypeEnum>
  DeepParserNodeTypes::initStringToTypeMap()
{
  map<string, DeepParserNodeTypes::TypeEnum> stringToTypeMap;
  //  ROOT = 0,
  stringToTypeMap.insert(std::make_pair("(ROOT", DeepParserNodeTypes::ROOT));
  //  ADJP = 1,
  stringToTypeMap.insert(std::make_pair("(ADJP", DeepParserNodeTypes::ADJP));
  //  ADVP = 2,
  stringToTypeMap.insert(std::make_pair("(ADVP", DeepParserNodeTypes::ADVP));
  //  CONJP = 3,
  stringToTypeMap.insert(std::make_pair("(CONJP", DeepParserNodeTypes::CONJP));
  //  FRAG = 4,
  stringToTypeMap.insert(std::make_pair("(FRAG", DeepParserNodeTypes::FRAG));
  //  INTJ = 5,
  stringToTypeMap.insert(std::make_pair("(INTJ", DeepParserNodeTypes::INTJ));
  //  LST = 6,
  stringToTypeMap.insert(std::make_pair("(LST", DeepParserNodeTypes::LST));
  //  NAC = 7,
  stringToTypeMap.insert(std::make_pair("(NAC", DeepParserNodeTypes::NAC));
  //  NP = 8,
  stringToTypeMap.insert(std::make_pair("(NP", DeepParserNodeTypes::NP));
  //  NX = 9,
  stringToTypeMap.insert(std::make_pair("(NX", DeepParserNodeTypes::NX));
  //  PP = 10,
  stringToTypeMap.insert(std::make_pair("(PP", DeepParserNodeTypes::PP));
  //  PRN = 11,
  stringToTypeMap.insert(std::make_pair("(PRN", DeepParserNodeTypes::PRN));
  //  PRT = 12,
  stringToTypeMap.insert(std::make_pair("(PRT", DeepParserNodeTypes::PRT));
  //  PRT_ADVP = 13,
  stringToTypeMap.insert(
      std::make_pair("(PRT|ADVP", DeepParserNodeTypes::PRT_ADVP));
  //  QP = 14,
  stringToTypeMap.insert(std::make_pair("(QP", DeepParserNodeTypes::QP));
  //  RRC = 15,
  stringToTypeMap.insert(std::make_pair("(RRC", DeepParserNodeTypes::RRC));
  //  S = 16,
  stringToTypeMap.insert(std::make_pair("(S", DeepParserNodeTypes::S));
  //  SBAR = 17,
  stringToTypeMap.insert(std::make_pair("(SBAR", DeepParserNodeTypes::SBAR));
  //  SBARQ = 18,
  stringToTypeMap.insert(std::make_pair("(SBARQ", DeepParserNodeTypes::SBARQ));
  //  SINV = 19,
  stringToTypeMap.insert(std::make_pair("(SINV", DeepParserNodeTypes::SINV));
  //  SQ = 20,
  stringToTypeMap.insert(std::make_pair("(SQ", DeepParserNodeTypes::SQ));
  //  TOP = 21,
  stringToTypeMap.insert(std::make_pair("(TOP", DeepParserNodeTypes::TOP));
  //  UCP = 22,
  stringToTypeMap.insert(std::make_pair("(UCP", DeepParserNodeTypes::UCP));
  //  VP = 23,
  stringToTypeMap.insert(std::make_pair("(VP", DeepParserNodeTypes::VP));
  //  WHADJP = 24,
  stringToTypeMap.insert(
      std::make_pair("(WHADJP", DeepParserNodeTypes::WHADJP));
  //  WHADVP = 25,
  stringToTypeMap.insert(
      std::make_pair("(WHADVP", DeepParserNodeTypes::WHADVP));
  //  WHNP = 26,
  stringToTypeMap.insert(std::make_pair("(WHNP", DeepParserNodeTypes::WHNP));
  //  WHPP = 27,
  stringToTypeMap.insert(std::make_pair("(WHPP", DeepParserNodeTypes::WHPP));
  //  X = 28,
  stringToTypeMap.insert(std::make_pair("(S1", DeepParserNodeTypes::S1));
  //  X = 28,
  stringToTypeMap.insert(std::make_pair("(X", DeepParserNodeTypes::X));
  assert(stringToTypeMap.find("(WHPP")!= stringToTypeMap.end());
  return stringToTypeMap;
}

// ____________________________________________________________________________
void DeepContextMarker::markSentence(Sentence<DeepToken> * sentence)
{
  DeepParserNodeList rootList;
  // Currently the parser will always return only one root.
  if (_parser.parseSentence(*sentence, &rootList))
  {
    markTree(rootList);
    LOG(DEBUG)
    << "Deep parser output as tree:\n" << rootList[0].treeAsString()
        << std::endl;
    LOG(DEBUG)
    << "Resulting marking:\n" << sentence->asStringWithWordMarks()
        << std::endl;
  }
  else
  {
    // This currently does nothing.
    _parser.repairParseTree(*sentence, &rootList);
    _parserErrors++;
  }
}

// ____________________________________________________________________________
void DeepContextMarker::markTree(
    TreeNode<DeepParserNodeTypes>::NodeList & rootList)
{
  // TreeNode<DeepMarkerNodeTypes>::NodeList rootListDeep;
  // constructSCITree(rootList[0],rootListDeep);

  // The whole sentence is one enum.
  // Mark the first word.
  markLeftMostLeaf(rootList[0], DeepTokenMarks::ENUM);
  for (size_t i = 0; i < rootList.size(); ++i)
  {
    // Each root is one list item.
    // Mark the first word.
    markLeftMostLeaf(rootList[0], DeepTokenMarks::C);
    // Now process each independent subtree.
    markSubtree(rootList[i]);
    // Mark the last word.
    markRightMostLeaf(rootList[0], DeepTokenMarks::CLOSE);
  }
  // Mark the last word.
  markRightMostLeaf(rootList[0], DeepTokenMarks::CLOSE);
}

// ____________________________________________________________________________
void DeepContextMarker::initEnumContinuationMap()
{
  _enumContinuationMap.insert("and");
  _enumContinuationMap.insert("or");
  _enumContinuationMap.insert(",");
  _enumContinuationMap.insert("but");
  _enumContinuationMap.insert("as");
  _enumContinuationMap.insert("well");
}


// ____________________________________________________________________________
bool DeepContextMarker::isRelativeClause(DeepParserNode const & root)
{
  // Relative clauses are subordinate clauses -> SBAR
  if (root.type() == DeepParserNodeTypes::SBAR)
  {
    // A relative clause begins with "which" -> WHNP or "in which","to which"
    // -> WHPP
    DeepParserNodeList const & children = root.getChildren();
    if (children.size() > 0 &&
        (children[0].type() == DeepParserNodeTypes::WHNP
        || children[0].type() == DeepParserNodeTypes::WHPP
        || children[0].type() == DeepParserNodeTypes::WHADVP))
      return true;
  }
  return false;
}

// ____________________________________________________________________________
bool
  DeepContextMarker::attachRelativeClause(DeepParserNode const & root,
      size_t relativeClauesNodeIndex, size_t * head)
{
  size_t headResult = root.getChildren().size() + 1;
  DeepParserNodeList const & children = root.getChildren();
  for (size_t i = 0; i < relativeClauesNodeIndex; ++i)
  {
    if (children[i].type() == DeepParserNodeTypes::NP)
      headResult = i;
  }
  if (headResult != root.getChildren().size() + 1)
  {
    *head = headResult;
    return true;
  }
  return false;
}

// ____________________________________________________________________________
void DeepContextMarker::nestRelativeClauses(DeepParserNode & root)
{
  DeepParserNodeList const & children = root.getChildren();
  for (size_t i = 0; i < children.size(); ++i)
  {
    nestRelativeClauses(*root.getChild(i));
    if (isRelativeClause(children[i]))
    {
      size_t head;
      if (attachRelativeClause(root, i, &head))
      {
        // Indices contains the indices from head to i (including).
        vector<size_t> indices;
        for (size_t j = head; j <= i; ++j)
          {
          indices.push_back(j);
          // We can now perform the recursive call.
          }
        root.combineNodes<DeepParserNode>(indices, children[head].type());
        // i-head+1 children were deleted, and 1 child was added.
        // current i is then at i - (i-head)
        i = i - (i - head);
      }
    }
  }
}

// ____________________________________________________________________________
bool DeepContextMarker::isEnumerationContinuation(DeepParserNode const & node)
{
  // A conjunctive phrase is a possible continuation.
  if (node.type() == DeepParserNodeTypes::CONJP) return true;
  // For a terminal node we need to look at the terminals.
  else if (node.type() == DeepParserNodeTypes::TERMINAL)
  {
    DeepParserTerminalNode const * terminal =
        static_cast<DeepParserTerminalNode const *>(&node);
    vector<DeepToken *> const & tokens = terminal->getTerminals();
    assert(tokens.size() > 0);
    // If each token is in the set of allowed tokens this is
    // a valid continuation.
    for (size_t i = 0; i < tokens.size(); ++i)
      if (_enumContinuationMap.count(tokens[i]->tokenString) == 0)
        return false;
    return true;
  }
  return false;
}

// ____________________________________________________________________________
bool DeepContextMarker::isC_HAT(DeepParserNode const & node)
{
  // For now a C_HAT is a PP that starts with According.
  if (node.type() == DeepParserNodeTypes::PP)
  {
    if (node.getChildren().size() > 0
        && node.getChildren()[0].type() == DeepParserNodeTypes::TERMINAL)
    {
      DeepParserTerminalNode const * terminal =
          static_cast<DeepParserTerminalNode const *>(&node.getChildren()[0]);
      vector<DeepToken *> const & tokens = terminal->getTerminals();
      if (tokens.size() > 0 &&
          ad_utility::getLowercase(tokens[0]->tokenString) == "according")
        return true;
    }
  }
  return false;
}

// ____________________________________________________________________________
void DeepContextMarker::findEnumerations(DeepParserNode const & root,
    vector<vector<size_t> > * enumerations)
{
  DeepParserNodeList const & children = root.getChildren();
  // At a TERMINAL node we are done.
  if (root.type() == DeepParserNodeTypes::TERMINAL) return;
  // Put all found enumerations here. There can be several enumerations.
  // The one enumeration we are working on.
  vector<size_t> enumeration;
  // We make one iteration to find enumeration elements.
  if (children.size() > 1)
  {
    // Push the first child.
    enumeration.push_back(0);
    // Start iterating at the second child.
    for (size_t i = 1; i < children.size(); ++i)
    {
      // If an enumeration is running, and this child is of the same type
      // add it to the enumeration and continue with next child.
      if (children[i].type() == children[enumeration.back()].type())
      {
        enumeration.push_back(i);
      }
      // No enumeration is running and this is no continuation.
      else if (!isEnumerationContinuation(children[i]))
      {
        // The last enumeration is good. Remember it.
        if (enumeration.size() > 1)
        {
          enumerations->push_back(enumeration);
        }
        // In any case, clear the current collection.
        enumeration.clear();
        enumeration.push_back(i);
      }
    }
    // Check again after the loop.
    if (enumeration.size() > 1)
    {
      enumerations->push_back(enumeration);
    }
  }
}

// ____________________________________________________________________________
void DeepContextMarker::markLeftMostLeaf(DeepParserNode const & node,
    DeepTokenMarks::TypeEnum mark)
{
  if (node.type() == DeepParserNodeTypes::TERMINAL)
  {
    DeepParserTerminalNode const * terminal =
        static_cast<DeepParserTerminalNode const *>(&node);
    vector<DeepToken *> const & tokens = terminal->getTerminals();
    assert(tokens.size() > 0);
    tokens.front()->appendMark(mark);
  }
  else
  {
    assert(node.getChildren().size() > 0);
    markLeftMostLeaf(node.getChildren().front(), mark);
  }
}

// ____________________________________________________________________________
void DeepContextMarker::markRightMostLeaf(DeepParserNode const & node,
    DeepTokenMarks::TypeEnum mark)
{
  if (node.type() == DeepParserNodeTypes::TERMINAL)
  {
    DeepParserTerminalNode const * terminal =
        static_cast<DeepParserTerminalNode const *>(&node);
    vector<DeepToken *> const & tokens = terminal->getTerminals();
    assert(tokens.size() > 0);
    tokens.back()->appendMark(mark);
  }
  else
  {
    assert(node.getChildren().size() > 0);
    markRightMostLeaf(node.getChildren().back(), mark);
  }
}

// ____________________________________________________________________________
void DeepContextMarker::markSubtree(DeepParserNode const & root)
{
  DeepParserNodeList const & children = root.getChildren();
  vector<vector<size_t> > enumerations;
  // Find the enumerations.
  findEnumerations(root, &enumerations);
  // An index to iterate through the children.
  size_t child = 0;
  // We found all enumerations. Mark them and recursively find enumerations.
  bool lsOpen = false;
  bool headOpen = false;
  bool insideEnum = false;
  if (enumerations.size() > 0)
  {
    // The enumeration - outer counter.
    size_t e = 0;
    // The enumeration node index - inner counter.
    size_t n = 0;
    for (; child < children.size(); ++child)
    {
      // The child matches an enumeration element.
      if (enumerations[e][n] == child)
      {
        // If a list surrounding was opened before, close it.
        if (lsOpen)
        {
          markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
          lsOpen = false;
        }

        // If this is the first element in the enumeration we need
        // to open the enum.
        if (n == 0)
        {
          // Mark the enum start
          markLeftMostLeaf(children[child], DeepTokenMarks::ENUM);
          insideEnum = true;
        }
        // Mark the list item start
        markLeftMostLeaf(children[child], DeepTokenMarks::C);

        // If it is a C_HAT and occurs within an enumeration, just mark
        // it. It can also be a list element.
        if (isC_HAT(children[child]))
        {
          markLeftMostLeaf(children[child], DeepTokenMarks::C_HAT);
          headOpen = true;
        }
        // If this list item is an NP it is a potential head.
        else if (children[child].type() == DeepParserNodeTypes::NP)
        {
          markLeftMostLeaf(children[child], DeepTokenMarks::C_H);
          headOpen = true;
        }
        // recursively analyse the list item
        markSubtree(children[child]);
        // mark the list item end
        markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
        // At the end we just close the potential head if we saw one.
        if (headOpen)
        {
          // Mark the head end.
          markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
          headOpen = false;
        }
        // This was the last element of an enumeration.
        // Continue with next enumeration?
        if (n == enumerations[e].size() - 1)
        {
          // mark the enum end
          markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
          insideEnum = false;
          // A next enumeration exists.
          if (e < enumerations.size() - 1)
          {
            ++e;
            n = 0;
          }
          // We are done, this was the last element of the last enumeration.
          else
          {
            ++child;
            break;
          }
        }
        // There is a next element in this enumeration.
        else
          ++n;
      }
      // The current child is not a list item, but we are either inside
      // an enumeration and not looking at a list item, or we are between
      // enumerations.
      else if (isRelativeClause(children[child]))
      {
         if (lsOpen)
           markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
          lsOpen = false;
          // Mark relative clause start.
          markLeftMostLeaf(children[child], DeepTokenMarks::C_STAR);
          // Recursively mark the clause.
          markSubtree(children[child]);
          // Mark the relative clause end.
          markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
      }
      // This is outside of an enumeration.
      else if (!insideEnum)
      {
        // If we are outside an enumeration and this is an NP,
        // it is a potential head.
        if (children[child].type() == DeepParserNodeTypes::NP)
        {
          if (lsOpen)
            markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
           lsOpen = false;
          markLeftMostLeaf(children[child], DeepTokenMarks::C_H);
          // Recursively mark the clause.
          markSubtree(children[child]);
          // Mark the relative clause end.
          markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
        }
        else if (isC_HAT(children[child]))
        {
          if (lsOpen)
            markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
           lsOpen = false;
          markLeftMostLeaf(children[child], DeepTokenMarks::C_HAT);
          // Recursively analyse the item.
          markSubtree(children[child]);
          // Mark the end.
          markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
        }
        // Else if no list surrounding was openend, open one.
        // Inside an enumeration (insideEnum == true) we only mark
        // list items, but if we are outside each terminal must be
        // part of a constituent.
        else if (!lsOpen)
        {
          markLeftMostLeaf(children[child], DeepTokenMarks::C);
          lsOpen = true;
          // Recursively mark the clause.
          markSubtree(children[child]);
        }
      }
      // The implicit final case here is that insideEnum == true
      // and then we just do nothing.
    }
  }
  // We finished processing all enumerations, now just process each
  // remaining child. (This is the default if no enumeration is present
  // within the children).
  if (child < children.size())
  {
    // We can be sure the last surrounding was closed, because the code
    // for the last element of the last enumerations took care of that.
    lsOpen = false;
    for (; child < children.size(); ++child)
    {
      if (isRelativeClause(children[child]))
      {
        // Close the opened clause.
        if (lsOpen)
          markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
        lsOpen = false;
        // Mark relative clause start.
        markLeftMostLeaf(children[child], DeepTokenMarks::C_STAR);
        // Recursively mark the clause.
        markSubtree(children[child]);
        // Mark the relative clause end.
        markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
      }
      else if (children[child].type() == DeepParserNodeTypes::NP)
      {
        // Close the opened clause.
        if (lsOpen)
          markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
        lsOpen = false;
        // Mark the potential head start.
        markLeftMostLeaf(children[child], DeepTokenMarks::C_H);
        // Recursively mark the clause.
        markSubtree(children[child]);
        // Mark the potential head end.
        markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
      }
      else if (isC_HAT(children[child]))
      {
        if (lsOpen)
          markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
        lsOpen = false;
        markLeftMostLeaf(children[child], DeepTokenMarks::C_HAT);
        // Recursively analyse the item.
        markSubtree(children[child]);
        // Mark the end.
        markRightMostLeaf(children[child], DeepTokenMarks::CLOSE);
      }
      else
      {
        // Start a list surrounding here.
        if (!lsOpen)
          markLeftMostLeaf(children[child], DeepTokenMarks::C);
        lsOpen = true;
        markSubtree(children[child]);
      }
    }
    // Close the last bracket if it is open.
    if (lsOpen)
      markRightMostLeaf(children[child - 1], DeepTokenMarks::CLOSE);
  }
}

// ____________________________________________________________________________
void DeepParserSentenceParser::repairParseTree(
    Sentence<DeepToken> const & sentence,
    TreeNode<DeepParserNodeTypes>::NodeList * rootList) const
{
}

// ____________________________________________________________________________
bool DeepParserSentenceParser::parseSentence(
    Sentence<DeepToken> const & sentence,
    TreeNode<DeepParserNodeTypes>::NodeList * roots)
{
  vector<DeepToken *> const & words = sentence.getWords();
  DeepContextMarker::DeepParserNode * root =
      new DeepContextMarker::DeepParserNode();
  DeepContextMarker::DeepParserNode * bottom = root;
  DeepContextMarker::DeepParserTerminalNode * terminal = NULL;
  roots->push_back(root);
  for (size_t i = 0; i < words.size(); ++i)
  {
    vector<string> tokenClauseTags;
    StringToNodeTypeMap::const_iterator it;
    boost::split(tokenClauseTags, words[i]->cTag, boost::is_any_of(","));
    for (size_t j = 0; j < tokenClauseTags.size(); ++j)
    {
      // std::cout << tokenClauseTags[j] << std::endl;
      // Only consider non-trivial tags.
      if (tokenClauseTags[j] != "*" && tokenClauseTags[j][0] != TAG_CLOSE)
      {
        // This opens a new, known clause.
        DeepParserNodeTypes::TypeEnum enu = DeepParserNodeTypes::S;
        if (ContextDecomposerUtil::mapLookup(
            DeepParserNodeTypes::stringToTypeMap,
            tokenClauseTags[j],
            &enu))
        {
          root = &root->addChild<DeepContextMarker::DeepParserNode>(enu);
          terminal = NULL;
        }
        // This is a new unknown tag. Assume it opens an unknown clause.
        else
        {
          LOG(WARN)
          << "DeepContextMarker: Unknown tag " << tokenClauseTags[j] << ". "
              << "Will treat as unknown clause but continue." << std::endl;
          root = &root->addChild<DeepContextMarker::DeepParserNode>(
              DeepParserNodeTypes::OTHER);
          terminal = NULL;
        }
      }
    }
    if (terminal == NULL) terminal = &root->addChild<
        DeepContextMarker::DeepParserTerminalNode>();
    // Add the current word to exactly one terminal.
    terminal->addTerminal(words[i]);

    for (size_t j = 0; j < tokenClauseTags.size(); ++j)
    {
      // This closes a clause.
      if (tokenClauseTags[j][0] == TAG_CLOSE)
      {
        // Just a sanity check. Could be removed for performance reasons.
        if (tokenClauseTags[j].size() > 1)
        {
          LOG(WARN)
          << "DeepContextMarker: Closing tag wrong: '" << tokenClauseTags[j]
              << "'. " << "Will treat as single closing tag and continue."
              << std::endl;
        }
        root = root->getParent();
        terminal = NULL;
        // This happens if the bracketing of the parser is not correct.
        if (root == NULL)
        {
          LOG(WARN)
          << "DeepContextMarker: Too many closing brackets" << " in sentence "
              << words[i]->completeSearchDocId << " at token '"
              << words[i]->tokenString << "' which has the following tags: '"
              << words[i]->cTag << "'" << std::endl;
          return false;
        }
      }
    }
  }
  // If we get here we must have closed all opening brackets
  // and thus get back to the original root.
  if (root != bottom)
  {
    LOG(WARN)
    << "DeepContextMarker: Deep-parser output inconsistent. "
        << "Missing closing brackets at the end of sentence "
        << sentence.getWords().front()->completeSearchDocId << "."
        << std::endl;
    return false;
  }
  return true;
}
}
