// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <vector>
#include <map>
#include <string>
#include "util/ContextDecomposerUtil.h"
#include "sentence/Sentence.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"
#include "base/TreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::vector;

namespace ad_decompose
{
map<string, DeepMarkerNodeTypes::TypeEnum> const
  DeepMarkerNodeTypes::stringToTypeMap =
      DeepMarkerNodeTypes::initStringToTypeMap();
map<DeepMarkerNodeTypes::TypeEnum, string> const
  DeepMarkerNodeTypes::typeToStringMap =
      DeepMarkerNodeTypes::initTypeToStringMap();

map<DeepMarkerNodeTypes::TypeEnum, string>
  DeepMarkerNodeTypes::initTypeToStringMap()
{
  map<DeepMarkerNodeTypes::TypeEnum, string> typeToStringMap;
  map<string, DeepMarkerNodeTypes::TypeEnum>::const_iterator it;
  for (it = stringToTypeMap.begin(); it != stringToTypeMap.end(); ++it)
  {
    typeToStringMap.insert(std::make_pair(it->second, it->first));
  }
  return typeToStringMap;
}

map<string, DeepMarkerNodeTypes::TypeEnum>
  DeepMarkerNodeTypes::initStringToTypeMap()
{
  map<string, DeepMarkerNodeTypes::TypeEnum> stringToTypeMap;
  stringToTypeMap.insert(std::make_pair("(ROOT", DeepMarkerNodeTypes::ROOT));
  stringToTypeMap.insert(std::make_pair("(TERMINAL",
      DeepMarkerNodeTypes::TERMINAL));
  stringToTypeMap.insert(std::make_pair("(C", DeepMarkerNodeTypes::C));
  stringToTypeMap.insert(std::make_pair("(C*",
      DeepMarkerNodeTypes::C_STAR));
  stringToTypeMap.insert(std::make_pair("(C^",
      DeepMarkerNodeTypes::C_HAT));
  stringToTypeMap.insert(std::make_pair("(CH",
      DeepMarkerNodeTypes::C_H));
  stringToTypeMap.insert(std::make_pair("(IGNORE",
      DeepMarkerNodeTypes::IGNORE));
  stringToTypeMap.insert(std::make_pair("(ENUM", DeepMarkerNodeTypes::ENUM));
  assert(stringToTypeMap.find("(C*")!= stringToTypeMap.end());
  return stringToTypeMap;
}

// __________________________________________________________________________
bool DeepMarkerSentenceParser::parseSentence(
    Sentence<DeepToken> const & sentence,
    TreeNode<DeepMarkerNodeTypes>::NodeList * roots)
{
  vector<DeepToken *> const & words = sentence.getWords();
  TreeNode<DeepMarkerNodeTypes> * root = new TreeNode<DeepMarkerNodeTypes>();
  assert(root->type() == DeepMarkerNodeTypes::ROOT);
  TreeNode<DeepMarkerNodeTypes> * bottom = root;
  roots->push_back(root);
  DeepMarkerTerminalNode * terminal = NULL;
  // Iterate through all words of the sentence.
  for (size_t i = 0; i < words.size(); ++i)
  {
    size_t markIndex = 0;
    // For each mark of the word. We assume a sequence
    // of opening marks, followed by closing marks.
    for (; markIndex < words[i]->marks.size(); ++markIndex)
    {
      if (words[i]->marks[markIndex] == DeepTokenMarks::CLOSE)
      {
        // After now only closing  brackets are allowed.
        // Quit the inner loop.
        break;
      }
      // Depending on the mark, create a new child, which then
      // becomes the root.
      switch (words[i]->marks[markIndex])
      {
      case DeepTokenMarks::C:
        root = &root->addChild<TreeNode<DeepMarkerNodeTypes> >(
            DeepMarkerNodeTypes::C);
        terminal = NULL;
        break;
      case DeepTokenMarks::C_H:
        root = &root->addChild<TreeNode<DeepMarkerNodeTypes> >(
            DeepMarkerNodeTypes::C_H);
        terminal = NULL;
        break;
      case DeepTokenMarks::C_HAT:
        root = &root->addChild<TreeNode<DeepMarkerNodeTypes> >(
            DeepMarkerNodeTypes::C_HAT);
        terminal = NULL;
        break;
      case DeepTokenMarks::C_STAR:
        root = &root->addChild<TreeNode<DeepMarkerNodeTypes> >(
            DeepMarkerNodeTypes::C_STAR);
        terminal = NULL;
        break;
      case DeepTokenMarks::ENUM:
        root = &root->addChild<TreeNode<DeepMarkerNodeTypes> >(
            DeepMarkerNodeTypes::ENUM);
        terminal = NULL;
        break;
      default:
        // If we get here
        LOG(ERROR)
        << "DeepContextRecombiner: Unknown mark type "
        << words[i]->marks[markIndex] << " ."
            << std::endl;
        return false;
      }
    }
    // Now the word itself.
    if (terminal == NULL) terminal = &root->addChild<DeepMarkerTerminalNode>();
    terminal->addTerminal(words[i]);
    // Now the closing brackets.
    for (; markIndex < words[i]->marks.size(); ++markIndex)
    {
      switch (words[i]->marks[markIndex])
      {
      case DeepTokenMarks::CLOSE:
      {
        root = root->getParent();
        terminal = NULL;
        if (root == NULL)
        {
          LOG(ERROR)
          << "DeepContextRecombiner: Too man closing brackets in sentence "
              << words[i]->completeSearchDocId << " at word "
              << i << ": '" << words[i]->tokenString << "'."<< std::endl;
          return false;
        }
        break;
      }
      default:
        LOG(ERROR)
        << "DeepContextRecombiner: Wrong ordering of brackets or unknown mark "
            << DeepToken::markToString(words[i]->marks[markIndex])
            << " in sentence "
            << words[i]->completeSearchDocId << " at word "
            << words[i]->tokenString << " with marks "
            << DeepToken::marksAsString(words[i]->marks) << std::endl;
        return false;
      }
    }
  }
  // If we get here we must have closed all opening brackets
  // and thus get back to the original root.
  if (root != bottom)
  {
    LOG(ERROR)
    << "DeepContextRecombiner: Not the right number "
        "of closing brackets in sentence "
        << words.front()->completeSearchDocId << std::endl;
    return false;
  }

  return true;
}
// __________________________________________________________________________
template<class Token>
Contexts<Token> DeepRecombiner<Token>::applyOnNode(
    TreeNode<DeepMarkerNodeTypes>* node, Contexts<Token> * immediateResults)
{
  Contexts<Token> result;
  if (node->type() == DeepMarkerNodeTypes::IGNORE)
  {
    // Do nothing.
  }
  // A terminal node has no children -> the single resulting context consists
  // of the sequence of terminals.
  else if (node->type() == DeepMarkerNodeTypes::TERMINAL)
  {
    DeepMarkerTerminalNode const * terminalNode =
      static_cast<DeepMarkerTerminalNode const *>(node);
    // Get the tokens from the terminal node.
    Context<Token> context;
    for (size_t j = 0; j < terminalNode->getTerminals().size(); ++j)
    {
      Token * token = terminalNode->getTerminals()[j];
      context.push_back(token);
    }
    result.addContext(context);
  }
  else if (node->type() == DeepMarkerNodeTypes::C_STAR)
  {
    // First of all construct the child contexts.
    Contexts<Token> relContexts;
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
        relContexts = relContexts
            * applyOnNode(node->getChild(i), immediateResults);
    }

    // Now find the head it must be attached to.
    TreeNode<DeepMarkerNodeTypes>* parent = node->getParent();
    TreeNode<DeepMarkerNodeTypes>* head = NULL;
    // If forwhatever reason we have no parent just return
    // what we have so far.
    if (parent == NULL)
      result = relContexts;
    // We found a valid parent.
    else
    {
      // Find a head below the parent.
      for (size_t i = 0; i < parent->getChildren().size(); ++i)
      {
        // If we get to the current node we are done, because we
        // only attach to heads before the current node.
        if (parent->getChild(i) == node)
          break;
        // If this is a potential head remember it.
        if (parent->getChild(i)->type() == DeepMarkerNodeTypes::C_H)
          head = parent->getChild(i);
      }
      // So we found a potential head.
      if (head != NULL)
      {
        // Attach all so far to the head.
        Contexts<Token> headContexts = applyOnNode(head, immediateResults);
        immediateResults->addContexts(headContexts * relContexts);
      }
      // Instead of removing the node, set its type to IGNORE.
      // Removing a node causes an iteration of children to become
      // invalid. Setting to IGNORE is easier.
      node->setType(DeepMarkerNodeTypes::IGNORE);
    }
  }
  // If this is an enumeration we let the enumeration recombiner
  // handle this, which "OR"s the children.
  else if (node->type() == DeepMarkerNodeTypes::ENUM)
  {
    DeepEnumerationRecombiner<DeepRecombiner, Token> eRecombiner(
        immediateResults);
    result = eRecombiner.apply(node);
  }
  // If this is an enumeration we let the enumeration recombiner
  // handle this, which "OR"s the children.
  else if (node->type() == DeepMarkerNodeTypes::C_HAT)
  {
    // First of all construct the child contexts.
    Contexts<Token> relContexts;
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
        relContexts = relContexts
            * applyOnNode(node->getChild(i), immediateResults);
    }
    immediateResults->addContexts(relContexts);
    // Instead of removing the node, set its type to IGNORE.
    // Removing a node causes an iteration of children to become
    // invalid. Setting to IGNORE is easier.
    node->setType(DeepMarkerNodeTypes::IGNORE);
  }
  // For anything left,  we handle the children with the default mode:
  // "AND" the children.
  else if (node->getChildren().size() > 0)
  {
    Contexts<Token> contexts;
    Context<Token> context;
    // Recursively collect the contexts from the remaining items.
    for (size_t i = 0; i < node->getChildren().size(); ++i)
    {
      contexts = applyOnNode(node->getChild(i), immediateResults);
      // Effecticely this appends contexts, if there is only one
      // context in each set.
      result = result * contexts;
    }
  }
  return result;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> DeepEnumerationRecombiner<DefRecombiner,
Token>::applyOnNode(
    TreeNode<DeepMarkerNodeTypes>* node, Contexts<Token> * immediateResults)
{
  assert(node->type() == DeepMarkerNodeTypes::ENUM);
  Contexts<Token> enContexts = getEnumerationContexts(node,
          immediateResults);
  return enContexts;
}

// __________________________________________________________________________
template<class DefRecombiner, class Token>
Contexts<Token> DeepEnumerationRecombiner<DefRecombiner,
Token>::getEnumerationContexts(
    TreeNode<DeepMarkerNodeTypes>* enumNode,
    Contexts<Token> * immediateResults)
{
  Contexts<Token> enumContexts;
  Contexts<Token> tContexts;
  DefRecombiner dr(immediateResults);
  for (size_t i = 0; i < enumNode->getChildren().size(); ++i)
  {
    /*// Below an enum node only terminal and list item nodes are allowed.
    assert(
        enumNode->getChild(i)->type() == (DeepMarkerNodeTypes::TERMINAL)
        || enumNode->getChild(i)->type() == (DeepMarkerNodeTypes::LI));*/
    if (enumNode->getChild(i)->type() == DeepMarkerNodeTypes::C
        || enumNode->getChild(i)->type() == DeepMarkerNodeTypes::C_H)
    {
      // Recursively construct the contexts of the list item.
      Contexts<Token> litContexts = dr.apply(enumNode->getChild(i));
      // Multiply with the terminal contexts up to now and append.
      enumContexts.addContexts(litContexts);
      // enumContexts.addContexts(tContexts * litContexts);
      // Clear the empty context.
      // tContexts.clear();
    }
    // For now just continue - this means anything within an enumeration, if
    // it is not part of a LI is ignored!. For now this is a valid assumption,
    // because we do not find enumerations that span non-trivial not-LIs.
    else
    {
      continue;
      // Note: this currently prepends a sequence of LS to the first LI
      // that occurs, and ignores the last one.
      // It is unclear (for me at the moment) what to do with that e.g.
      // LS LI LS LI LS LI LS -> where should the LS components belong to.
      // Usually those should be meaningless anyways, e.g. "and" , "or" etc.
      // Contexts<Token> t = dr.apply(enumNode->getChild(i));
      // tContexts.addContexts(t);
    }
  }
  return enumContexts;
}



// ____________________________________________________________________________
bool ReplaceTokenMarksTransformer::apply(TreeNode<DeepMarkerNodeTypes>* node)
{
  // Clear all tokens tags.
  vector<DeepToken *> tokens = _sentence->getWords();
  for (size_t i = 0; i < tokens.size(); ++i)
    tokens[i]->clearBrTags();
  // Write the new tags, starting with this node.
  writeTokenMarks(node);
  return true;
}

// ____________________________________________________________________________
void ReplaceTokenMarksTransformer::writeTokenMarks(
    TreeNode<DeepMarkerNodeTypes>* node)
{
  switch (node->type())
  {
  case DeepMarkerNodeTypes::ENUM:
  {
    markLeftMostLeaf(*node, DeepTokenMarks::ENUM);
    markRightMostLeaf(*node, DeepTokenMarks::CLOSE);
    break;
  }
  case DeepMarkerNodeTypes::C:
  {
    markLeftMostLeaf(*node, DeepTokenMarks::C);
    markRightMostLeaf(*node, DeepTokenMarks::CLOSE);
    break;
  }
  case DeepMarkerNodeTypes::C_H:
  {
    markLeftMostLeaf(*node, DeepTokenMarks::C_H);
    markRightMostLeaf(*node, DeepTokenMarks::CLOSE);
    break;
  }
  case DeepMarkerNodeTypes::C_HAT:
  {
    markLeftMostLeaf(*node, DeepTokenMarks::C_HAT);
    markRightMostLeaf(*node, DeepTokenMarks::CLOSE);
    break;
  }
  case DeepMarkerNodeTypes::C_STAR:
  {
    markLeftMostLeaf(*node, DeepTokenMarks::C_STAR);
    markRightMostLeaf(*node, DeepTokenMarks::CLOSE);
    break;
  }
  default:
    break;
  }
  for (size_t i = 0; i < node->getChildren().size(); ++i)
    writeTokenMarks(node->getChild(i));
}

// ____________________________________________________________________________
void ReplaceTokenMarksTransformer::markLeftMostLeaf(
    TreeNode<DeepMarkerNodeTypes> const & node,
    DeepTokenMarks::TypeEnum mark)
{
  if (node.type() == DeepMarkerNodeTypes::TERMINAL)
  {
    TerminalNode<DeepMarkerNodeTypes, DeepToken *> const * terminal =
        static_cast<TerminalNode<DeepMarkerNodeTypes, DeepToken *> const *>
    (&node);
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
void ReplaceTokenMarksTransformer::markRightMostLeaf(
    TreeNode<DeepMarkerNodeTypes> const & node,
    DeepTokenMarks::TypeEnum mark)
{
  if (node.type() == DeepMarkerNodeTypes::TERMINAL)
  {
    TerminalNode<DeepMarkerNodeTypes, DeepToken *> const * terminal =
        static_cast<TerminalNode<DeepMarkerNodeTypes, DeepToken *> const *>
    (&node);
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
bool SimplifyDeepMarkerTree::apply(TreeNode<DeepMarkerNodeTypes>* node)
{
  TreeNode<DeepMarkerNodeTypes>::NodeList const & children =
      node->getChildren();

  // Trivial return case.
  if (children.size() == 0)
    return true;
  bool changed = false;
  // First simplify adjacent children.
  // changed = simplifyAdjacentNodes(node) || changed;

  // Simplify if we only have children of the same type.
  changed = simplifySameChildNodes(node) || changed;

  // Remove C nodes below other C nodes.
  changed = mergeNestedLS(node) || changed;

  // Afterwards we apply to the same node, because the new
  // children might be simplified again.
  if (changed) apply(node);
  // If nothing was changed, recurse to the children.
  else
  {
    for (size_t i = 0; i < children.size(); ++i)
    {
      apply(node->getChild(i));
    }
  }
  return true;
}

// ____________________________________________________________________________
bool SimplifyDeepMarkerTree::simplifySameChildNodes(
    TreeNode<DeepMarkerNodeTypes>* node)
{
  TreeNode<DeepMarkerNodeTypes>::NodeList const & children =
      node->getChildren();
  bool onlyNodeType = true;
  bool removed = false;
  // Check if there are only children of same type.
  for (size_t i = 0; i < children.size(); ++i)
  {
    if (children[i].type() != node->type())
    {
      onlyNodeType = false;
      break;
    }
  }
  // If yes, remove all the children, and attach their children
  // to the current node instead.
  if (onlyNodeType)
  {
    // Store the current maximum index, because children will be added
    // after there.
    size_t maxChildIdx = children.size();
    for (size_t i = 0; i < maxChildIdx; ++i)
    {
      for (size_t j = 0; j < children[i].getChildren().size(); ++j)
      {
        node->getChild(i)->getChild(j)->moveNode(node);
        // The number of children of child j, decreased. The same index will
        // belong to another child now.
        --j;
      }
    }
    // All the grandchildren have been moved, but the children are still there.
    // Remove those.
    for (size_t i = 0; i < maxChildIdx; ++i)
    {
      removed = true;
      node->removeChild(size_t(0));
    }
  }
  return removed;
}
// ____________________________________________________________________________
bool SimplifyDeepMarkerTree::simplifyAdjacentNodes(
    TreeNode<DeepMarkerNodeTypes>* node)
{
  TreeNode<DeepMarkerNodeTypes>::NodeList const & children =
      node->getChildren();
  // Below an ENUM node we do not merge.
  if (node->type() == DeepMarkerNodeTypes::ENUM)
    return false;
  bool changed = false;
  // A vector to hold the indices that can be merged.
  vector<size_t> mergeIndices;
  for (size_t i = 0; i < children.size(); ++i)
  {
    // The next child is a continuation of same type.
    // Append the index and continue.
    if (mergeIndices.size() > 0 &&
        children[mergeIndices.back()].type() == children[i].type())
      mergeIndices.push_back(i);
    // The next child is not of same type or no start of a
    // sequence was observed.
    else
    {
      // If there is something to merge - do it.
      if (mergeIndices.size() > 1)
      {
        node->mergeNodes<TreeNode<DeepMarkerNodeTypes> >(mergeIndices,
            children[mergeIndices.back()].type());
        // After merging mergeIndices.size() - 1 children were removed.
        // The current node is therefore at: i - (size - 1)
        i = i - (mergeIndices.size() - 1);
        changed = true;
      }
      // Clear the indices.
      mergeIndices.clear();
      // Add new child.
      if (children[i].type() == DeepMarkerNodeTypes::C ||
          children[i].type() == DeepMarkerNodeTypes::C_STAR)
        mergeIndices.push_back(i);
    }
  }
  // If there is something to merge - do it.
  if (mergeIndices.size() > 1)
  {
    node->mergeNodes<TreeNode<DeepMarkerNodeTypes> >(mergeIndices,
        children[mergeIndices.back()].type());
    changed = true;
  }
  return changed;
}

// ____________________________________________________________________________
bool SimplifyDeepMarkerTree::mergeNestedLS(TreeNode<DeepMarkerNodeTypes>* node)
{
  TreeNode<DeepMarkerNodeTypes>::NodeList const & children =
      node->getChildren();
  bool changed = false;
  if (node->type() == DeepMarkerNodeTypes::C
      || node->type() == DeepMarkerNodeTypes::C_H)
  {
    for (size_t i = 0; i < children.size(); ++i)
    {
      if (children[i].type() == DeepMarkerNodeTypes::C)
      {
        TreeNode<DeepMarkerNodeTypes>::NodeList const & grandChildren =
            children[i].getChildren();
        size_t numGrandChildren = grandChildren.size();
        bool canBeErased = true;
        // for (size_t j = 0; j < grandChildren.size(); ++j)
        // {
        //   if (grandChildren[j].type() != DeepMarkerNodeTypes::LS
        //       && grandChildren[j].type() != DeepMarkerNodeTypes::TERMINAL)
        //     {
        //       canBeErased = false;
        //       break;
        //     }
        // }
        // Erase the child in the hierarchy.
        if (canBeErased)
        {
          changed = true;
          node->eraseChild(i);
          i += (numGrandChildren - 1);
        }
      }
    }
  }
  // Remove single C below C_HAT.
  else if (node->type() == DeepMarkerNodeTypes::C_HAT)
  {
    if (children.size() == 1
        && children[0].type() == DeepMarkerNodeTypes::C)
    {
      node->eraseChild(0);
    }
  }

  return changed;
}

template class DeepRecombiner<DeepToken>;
template class DeepEnumerationRecombiner<DeepRecombiner<DeepToken>, DeepToken>;
}

