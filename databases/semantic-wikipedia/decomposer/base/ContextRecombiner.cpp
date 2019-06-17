// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <vector>
#include <map>
#include <string>
#include "util/ContextDecomposerUtil.h"
#include "sentence/Sentence.h"
#include "base/ContextRecombiner.h"
#include "base/TreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::vector;
using std::map;
using std::string;

namespace ad_decompose
{
map<string, MarkerNodeTypes::TypeEnum> const MarkerNodeTypes::stringToTypeMap =
    MarkerNodeTypes::initStringToTypeMap();
map<MarkerNodeTypes::TypeEnum, string> const MarkerNodeTypes::typeToStringMap =
    MarkerNodeTypes::initTypeToStringMap();

map<MarkerNodeTypes::TypeEnum, string> MarkerNodeTypes::initTypeToStringMap()
{
  map<MarkerNodeTypes::TypeEnum, string> typeToStringMap;
  map<string, MarkerNodeTypes::TypeEnum>::const_iterator it;
  for (it = stringToTypeMap.begin(); it != stringToTypeMap.end(); ++it)
  {
    typeToStringMap.insert(std::make_pair(it->second, it->first));
  }
  return typeToStringMap;
}

map<string, MarkerNodeTypes::TypeEnum> MarkerNodeTypes::initStringToTypeMap()
{
  map<string, MarkerNodeTypes::TypeEnum> stringToTypeMap;
  stringToTypeMap.insert(std::make_pair("(ROOT", MarkerNodeTypes::ROOT));
  stringToTypeMap.insert(std::make_pair("(TERMINAL",
      MarkerNodeTypes::TERMINAL));
  stringToTypeMap.insert(std::make_pair("(LIT", MarkerNodeTypes::LIT));
  stringToTypeMap.insert(std::make_pair("(ENUM", MarkerNodeTypes::ENUM));
  stringToTypeMap.insert(std::make_pair("(REL", MarkerNodeTypes::REL));
  stringToTypeMap.insert(std::make_pair("(RELA", MarkerNodeTypes::RELA));
  assert(stringToTypeMap.find("(REL")!= stringToTypeMap.end());
  return stringToTypeMap;
}

// __________________________________________________________________________
template<class Token>
Contexts<Token> DefaultRecombiner<Token>::applyOnNode(
    TreeNode<MarkerNodeTypes>* node, Contexts<Token> * immediateResults)
{
  Contexts<Token> result;
  if (node->hasChildType(MarkerNodeTypes::REL)
      || node->hasChildType(MarkerNodeTypes::RELA))
  {
    RelativeClauseRecombiner<DefaultRecombiner, Token> rcRecombiner(
        immediateResults);
    rcRecombiner.apply(node);
    // Afterwards these children must have been removed.
    assert(
        !node->hasChildType(MarkerNodeTypes::REL)
        && !node->hasChildType(MarkerNodeTypes::RELA));
  }
  if (node->hasChildType(MarkerNodeTypes::ENUM))
  {
    EnumerationRecombiner<DefaultRecombiner, Token> eRecombiner(
        immediateResults);
    result = eRecombiner.apply(node);
  }
  // If something is left...
  else if (node->getChildren().size() > 0)
  {
    Context<Token> context;
    // Just return whatever non-terminals are left.
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
      // Only terminals may be left.
      assert(node->getChildren()[i].type() == MarkerNodeTypes::TERMINAL);
      MarkerTerminalNode const & terminalNode =
          static_cast<MarkerTerminalNode const &>(node->getChildren()[i]);
      // From each terminal node get the phrases.
      for (size_t j = 0; j < terminalNode.getTerminals().size(); ++j)
      {
        Phrase<Token> * phrase = terminalNode.getTerminals()[j];
        context.addPhrase(*phrase);
      }
    }
    result.addContext(context);
  }
  return result;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> RelativeClauseRecombiner<DefRecombiner,
Token>::findRelativeClauseAttachment(
    TreeNode<MarkerNodeTypes> * parent, size_t relIndex)
{
  // Attach to correct phrase.
  Contexts<Token> head;
  bool attached = false;
  for (size_t j = relIndex; j > 0; --j)
  {
    if (parent->getChild(j - 1)->type() == MarkerNodeTypes::TERMINAL)
    {
      MarkerTerminalNode * node =
          static_cast<MarkerTerminalNode *>(parent->getChild(j - 1));
      for (size_t k = node->getTerminals().size(); k > 0; --k)
      {
        if (node->getTerminals()[k - 1]->getType() == "NP")
        {
          Context<Token> c;
          c.addPhrase(*(node->getTerminals()[k - 1]));
          head.addContext(c);
          attached = true;
          break;
        }
      }
      if (attached) break;
    }
    // TODO(elmar): this is actually a workaround to emulate the old
    // behaviour ignoring the nesting when attaching relative clauses...
    else
    {
      Contexts<Token> recRes = findRelativeClauseAttachment(
          parent->getChild(j - 1),
          parent->getChild(j - 1)->getChildren().size());
      if (recRes.size() > 0)
      {
        head = recRes;
        break;
      }
    }
  }
  return head;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> RelativeClauseRecombiner<DefRecombiner,
Token>::applyOnNode(
    TreeNode<MarkerNodeTypes>* node, Contexts<Token> * immediateResults)
{
  Contexts<Token> res;
  for (size_t i = 0; i < node->getChildren().size();)
  {
    // Recursively generate contexts of MarkerNodeTypes clause.
    if (node->getChildren()[i].type() == MarkerNodeTypes::REL
        || node->getChildren()[i].type() == MarkerNodeTypes::RELA)
    {
      Contexts<Token> head = findRelativeClauseAttachment(node, i);
      DefRecombiner dr(immediateResults);
      Contexts<Token> attachContexts = dr.apply(node->getChild(i));
      Contexts<Token> result = (head * attachContexts);
      node->removeChild(i);
      immediateResults->addContexts(result);
    }
    else
    {
      ++i;
    }
  }
  return res;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> EnumerationRecombiner<DefRecombiner,
Token>::applyOnNode(
    TreeNode<MarkerNodeTypes>* node, Contexts<Token> * immediateResults)
{
  Contexts<Token> result;
  DefRecombiner r(immediateResults);
  vector<Contexts<Token> > enumContexts;
  for (size_t i = 0; i < node->getChildren().size(); ++i)
  {
    if (node->getChild(i)->type() == MarkerNodeTypes::TERMINAL)
    {
      Contexts<Token> s;
      Context<Token> c;
      MarkerTerminalNode * terminalNode =
          static_cast<MarkerTerminalNode *>(node->getChild(i));
      for (size_t j = 0; j < terminalNode->getTerminals().size(); ++j)
      {
        c.addPhrase(*terminalNode->getTerminals()[j]);
      }
      s.addContext(c);
      enumContexts.push_back(s);
    }
    // Recursively generate contexts for each enumeration.
    if (node->getChild(i)->type() == MarkerNodeTypes::ENUM)
    {
      TreeNode<MarkerNodeTypes> * enumNode = node->getChild(i);
      Contexts<Token> enContexts = getEnumerationContexts(enumNode,
          immediateResults);
      enumContexts.push_back(enContexts);
    }
    // Create permutations of selected enumeration items.
  }
  for (size_t i = 0; i < enumContexts.size(); ++i)
    result = result * enumContexts[i];
  return result;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> EnumerationRecombiner<DefRecombiner,
Token>::getEnumerationContexts(
    TreeNode<MarkerNodeTypes>* enumNode,
    Contexts<Token> * immediateResults)
{
  Contexts<Token> enumContexts;
  Contexts<Token> tContexts;
  for (size_t i = 0; i < enumNode->getChildren().size(); ++i)
  {
    // Below an enum node only terminal and list item nodes are allowed.
    assert(
        enumNode->getChild(i)->type() == (MarkerNodeTypes::TERMINAL)
        || enumNode->getChild(i)->type() == (MarkerNodeTypes::LIT));

    if (enumNode->getChild(i)->type() == (MarkerNodeTypes::TERMINAL))
    {
      Context<Token> t;
      MarkerTerminalNode * terminalNode =
          static_cast<MarkerTerminalNode *>(enumNode->getChild(i));
      for (size_t j = 0; j < terminalNode->getTerminals().size(); ++j)
      {
        t.addPhrase(*terminalNode->getTerminals()[j]);
      }
      tContexts.addContext(t);
    }
    else if (enumNode->getChild(i)->type() == (MarkerNodeTypes::LIT))
    {
      // Recursively construct the contexts of the list item.
      DefRecombiner dr(immediateResults);
      Contexts<Token> litContexts = dr.apply(enumNode->getChild(i));
      // Multiply with the contexts up to now and append.
      enumContexts.addContexts(tContexts * litContexts);
      // Clear the empty context.
      tContexts.clear();
    }
  }
  return enumContexts;
}

// __________________________________________________________________________
bool CollateEnumerationTransformer::apply(TreeNode<MarkerNodeTypes>* node)
{
  if (node->type() == MarkerNodeTypes::TERMINAL) return true;
  else
  {
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
      apply(node->getChild(i));
    }
  }
  if (node->hasChildType(MarkerNodeTypes::LIT))
  {
    // All children by index that belong to the same enumeration.
    vector<size_t> enumerationsIndex;
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
      if (node->getChildren()[i].type() == MarkerNodeTypes::LIT)
      {
        enumerationsIndex.push_back(i);
      }
      else if (!enumerationsIndex.empty())
      {
        if (node->getChildren()[i].type() == MarkerNodeTypes::TERMINAL
            && isEnumerationContinuation(node->getChildren()[i]))
          enumerationsIndex.push_back(i);
        else
        {
          node->combineNodes<TreeNode<MarkerNodeTypes> >(enumerationsIndex,
              MarkerNodeTypes::ENUM);
          // Now we removed some nodes from the children list, therefore
          // we need to continue at the right index.
          i -= enumerationsIndex.size()+1;
          enumerationsIndex.clear();
        }
      }
    }
    if (!enumerationsIndex.empty())
      {
      node->combineNodes<TreeNode<MarkerNodeTypes> >(enumerationsIndex,
          MarkerNodeTypes::ENUM);
      }
  }
  return true;
}

// __________________________________________________________________________
bool CollateEnumerationTransformer::isEnumerationContinuation(
    TreeNode<MarkerNodeTypes> const & node) const
{
  assert(node.type() == MarkerNodeTypes::TERMINAL);
  MarkerTerminalNode const & terminalNode =
      static_cast<MarkerTerminalNode const &>(node);
  for (size_t i = 0; i < terminalNode.getTerminals().size(); ++i)
  {
    if (!(terminalNode.getTerminals()[i]->getType() == "O")
        && !(terminalNode.getTerminals()[i]->getType() == "CONJP"))
      return false;
  }
  return true;
}

// __________________________________________________________________________
template <class Token>
Contexts<Token> const ContextRecombiner<Token>::extractContextsPtr(
    Sentence<Token> & sentence)
{
  LOG(TRACE)
  << "Starting to extract contexts." << std::endl;
  Contexts<Token> result;
  DefaultPhraseParser<Token> parser;
  TreeNode<MarkerNodeTypes>::NodeList rootList;
  bool parse_result = parser.parseSentence(sentence, &rootList);
  if (!parse_result)
  {
    Context<Token> singleContext;
    for (size_t i = 0; i < sentence.getWords().size(); ++i)
      singleContext.push_back(sentence.getWords()[i]);
    result.addContext(singleContext);
    ++_parseFails;
    LOG(INFO)
    << "ContextRecombiner Failed to parse sentence - "
        "context identification was inconsistent.\n";
  }
  else
  {
    for (size_t i = 0; i < rootList.size(); ++i)
    {
      TreeNode<MarkerNodeTypes> & root = rootList[i];
      LOG(TRACE)
      << "Original Tree:\n" << root.treeAsString();
      CollateEnumerationTransformer er;
      root.applyTransformation(&er);
      LOG(TRACE)
      << "Tree after collecting enumerations: \n" << root.treeAsString();
      DefaultRecombiner<Token> r;
      Contexts<Token> res = root.applyRecombiner(&r);
      LOG(TRACE)
      << "Extracted contexts:\n";
      for (size_t i = 0; i < res.size(); ++i)
      {
        LOG(TRACE)
        << "Context" << i << ": " << res[i].asString() << "\n";
        result.addContext(res[i]);
        // result.insert(result.end(), res.begin(), res.end());
      }
    }
    ++_parseSuccess;
  }
  LOG(TRACE)
  << "Done extracting contexts." << std::endl;
  return result;
}

// __________________________________________________________________________
template <class Token>
bool DefaultPhraseParser<Token>::parseSentence(Sentence<Token >const & sentence,
    TreeNode<MarkerNodeTypes>::NodeList * roots)
{
  vector<Phrase<Token> *> const & phrases = sentence.getPhrases();
  TreeNode<MarkerNodeTypes> * root = new TreeNode<MarkerNodeTypes>();
  TreeNode<MarkerNodeTypes> * bottom = root;
  roots->push_back(root);
  bool separatorTerminal = false;
  MarkerTerminalNode * terminal = NULL;
  for (size_t i = 0; i < phrases.size(); ++i)
  {
    Token const & firstWord = phrases[i]->getFirstWord();
    Token const & lastWord = phrases[i]->getLastWord();
    for (size_t j = 0; j < firstWord.marks.size(); ++j)
    {
      switch (firstWord.marks[j])
      {
      case DefaultTokenMarks::REL_OPEN:
        root = &root->addChild<TreeNode<MarkerNodeTypes> >(
            MarkerNodeTypes::REL);
        terminal = NULL;
        break;
      case DefaultTokenMarks::RELA_OPEN:
        root = &root->addChild<TreeNode<MarkerNodeTypes> >(
            MarkerNodeTypes::RELA);
        terminal = NULL;
        break;
      case DefaultTokenMarks::LIT_OPEN:
        root = &root->addChild<TreeNode<MarkerNodeTypes> >(
            MarkerNodeTypes::LIT);
        terminal = NULL;
        break;
      case DefaultTokenMarks::SEP:
        // Only create a new root if the root node is not empty.
        // Note that terminals are attached to  a seperate MarkerTerminalNode
        // and thus as long as terminals were present the root
        // node is not empty.
        if (roots->back().getChildren().size() > 0)
        {
          // If we get here we must have closed all opening brackets
          // and thus get back to the original root.
          if (root != bottom) return false;
          root = new TreeNode<MarkerNodeTypes>();
          bottom = root;
          roots->push_back(root);
        }
        separatorTerminal = true;
        terminal = NULL;
        continue;
      default:
        break;
      }
    }
    // A separator terminal is simply ignored and we continue with
    // the next phrase.
    if (separatorTerminal)
    {
      separatorTerminal = false;
      continue;
    }
    if (terminal == NULL) terminal = &root->addChild<MarkerTerminalNode>();
    terminal->addTerminal(phrases[i]);
    for (size_t j = 0; j < lastWord.marks.size(); ++j)
    {
      // This completely ignores the order in which typed brackets are closed.
      if (Token::isCloseMark(lastWord.marks[j]))
      {
        root = root->getParent();
        terminal = NULL;
      }
    }
  }
  // If we get here we must have closed all opening brackets
  // and thus get back to the original root.
  if (root != bottom) return false;
  // If the last created root did not receive children erase it.
  if (roots->back().getChildren().size() == 0) roots->erase(roots->end() - 1);
  return true;
}

template class ContextRecombiner<DefaultToken>;
}

