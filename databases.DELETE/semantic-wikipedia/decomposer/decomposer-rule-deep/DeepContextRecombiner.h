// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOER_RULE_DEEP_DEEPCONTEXTRECOMBINER_H_ //NOLINT
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOER_RULE_DEEP_DEEPCONTEXTRECOMBINER_H_ //NOLINT

#include <boost/ptr_container/ptr_vector.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "base/ContextExtractorBase.h"
#include "base/RecombinerBase.h"
#include "sentence/Sentence.h"
#include "base/ParserBase.h"
#include "base/TreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "util/ContextDecomposerUtil.h"

namespace ad_decompose
{
// A type to hold our Tree types for the deep parser identification.
class DeepMarkerNodeTypes
{
  public:
    // TODO(elmar): Find a better solution for the workaround "COUNT".
    enum TypeEnum
    {
      ROOT = 0,
      TERMINAL = 1,
      C = 2,
      C_STAR = 3,
      C_HAT = 4,
      C_H = 5,
      ENUM = 6,
      IGNORE = 7,  // A special type to indicate this node should be ignored.
      COUNT = 8
    };

    static map<TypeEnum, string> const typeToStringMap;
    static map<string, TypeEnum> const stringToTypeMap;

  private:

    // Initialize the type to string map. The implementation
    // just reverses the string to type map, thus it has to
    // be initialized beforehand.
    static map<TypeEnum, string> initTypeToStringMap();

    // Initialize the string to type map.
    static map<string, TypeEnum> initStringToTypeMap();
};

typedef TreeNode<DeepMarkerNodeTypes> DeepMarkerTreeNode;
typedef TerminalNode<DeepMarkerNodeTypes, DeepToken *> DeepMarkerTerminalNode;

class DeepMarkerSentenceParser: public ParserBase<DeepMarkerNodeTypes,
  DeepToken>
{
  public:
    virtual bool parseSentence(Sentence<DeepToken> const & sentence,
        TreeNode<DeepMarkerNodeTypes>::NodeList * roots);
    virtual ~DeepMarkerSentenceParser()
    {
    }
};

// The default recombiner which recursively calls itself or the
// RelativeClauseRecombiner/EnumerationRecombiner to recombine
// contexts. RelativeClauseRecombiner/EnumerationRecombiner can also
// recursively apply this class.
template<class Token>
class DeepRecombiner: public RecombinerBase<Token, DeepMarkerNodeTypes>
{
  public:
  DeepRecombiner() :
        RecombinerBase<Token, DeepMarkerNodeTypes>()
    {
    }
    explicit DeepRecombiner(Contexts<Token> * immediateResults) :
        RecombinerBase<Token, DeepMarkerNodeTypes>(immediateResults, false)
    {
    }
    virtual ~DeepRecombiner()
    {
    }
  private:
    virtual Contexts<Token> applyOnNode(TreeNode<DeepMarkerNodeTypes>* node,
        Contexts<Token> * immediateResults);
};

// A class for recombining enumerations.
template<class DefRecombiner, class Token>
class DeepEnumerationRecombiner : public RecombinerBase<Token,
    DeepMarkerNodeTypes>
{
  public:
    // This is just so the assert below works fine.
    typedef RecombinerBase<Token, DeepMarkerNodeTypes> BaseClass;

    explicit DeepEnumerationRecombiner(Contexts<Token> * immediateResults) :
        RecombinerBase<Token, DeepMarkerNodeTypes>(immediateResults, false)
    {
      assert(BaseClass::_immediateResults != NULL);
    }
    virtual ~DeepEnumerationRecombiner()
    {
    }
  private:
    // Return the Contexts below an ENUM node. Possibly recursively
    // calls the DefaultRecombiner to extract Contexts if below an ENUM
    // node a LIT node is found.
    Contexts<Token> getEnumerationContexts(
        TreeNode<DeepMarkerNodeTypes> * node,
        Contexts<Token> * immediatResults);
    virtual Contexts<Token> applyOnNode(TreeNode<DeepMarkerNodeTypes>* node,
        Contexts<Token> * immediateResults);
    DeepEnumerationRecombiner()
    {
    }
};
// A class to transform the tree by simplifying the tree structure
// without influencing the resulting contexts.
class SimplifyDeepMarkerTree: public TransformerBase<DeepMarkerNodeTypes>
{
  public:
    virtual ~SimplifyDeepMarkerTree()
    {
    }
    virtual bool apply(TreeNode<DeepMarkerNodeTypes>* node);
  private:
    // Merge adjacent nodes of type LS and LS_STAR.
    bool simplifyAdjacentNodes(TreeNode<DeepMarkerNodeTypes>* node);
    // If a node only consists of children of the same type we can
    // simply remove the node in the hierarchy.
    bool simplifySameChildNodes(TreeNode<DeepMarkerNodeTypes>* node);
    // Remove children nodes of type LS if the passed node is also of
    // type LS.
    bool mergeNestedLS(TreeNode<DeepMarkerNodeTypes>* node);
};


// TODO(elmar): this should be reworked.
// A class to update the token marks of a sentence, to correspond
// to the tree below the node.
class ReplaceTokenMarksTransformer: public TransformerBase<DeepMarkerNodeTypes>
{
  public:
    explicit ReplaceTokenMarksTransformer(Sentence<DeepToken> * sentence) :
        _sentence(sentence)
    {
    }
    virtual ~ReplaceTokenMarksTransformer()
    {
    }
    virtual bool apply(TreeNode<DeepMarkerNodeTypes>* node);
  private:
    void writeTokenMarks(TreeNode<DeepMarkerNodeTypes>* node);
    void markLeftMostLeaf(TreeNode<DeepMarkerNodeTypes> const & node,
        DeepTokenMarks::TypeEnum mark);
    void markRightMostLeaf(TreeNode<DeepMarkerNodeTypes> const & node,
        DeepTokenMarks::TypeEnum mark);

    Sentence<DeepToken> * _sentence;
};


// A class to extract contexts based on the tree structure
// provided by the identification phase.
template<class Token>
class DeepContextRecombiner: public ContextRecombinerBase<Token>
{
  public:
    // Extract contexts from the previously marked sentence,
    // but only return pointers
    // to the tokens for efficiency reasons.
    virtual Contexts<Token> const
    extractContextsPtr(Sentence<Token> & sentence)
    {
      TreeNode<DeepMarkerNodeTypes>::NodeList rootList;
      _parser.parseSentence(sentence, &rootList);
      SimplifyDeepMarkerTree simplyfier;
      simplyfier.apply(&rootList[0]);
      ReplaceTokenMarksTransformer replacer(&sentence);
      replacer.apply(rootList[0].getChild(0));
      DeepRecombiner<Token> recombiner;
      Contexts<Token> res = recombiner.apply(&rootList[0]);
      LOG(DEBUG)
        << "Resulting contexts: " << std::endl
        << res.asString() << std::endl;
      return res;
    }

    DeepContextRecombiner() :
        _parser(), _parseFails(0), _parseSuccess(0)
    {
    }

    virtual ~DeepContextRecombiner()
    {
    }

    string statsAsString()
    {
      std::stringstream s;
      s << "ContextRecombiner failed to parse " << _parseFails << " out of "
          << (_parseFails + _parseSuccess) << " marked sentences.\n";
      return s.str();
    }

  private:
    // The parser to parse the markings provided by SCI.
    DeepMarkerSentenceParser _parser;
    // Counters for failed/successful parses of identified sentences.
    size_t _parseFails;
    size_t _parseSuccess;
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOER_RULE_DEEP_DEEPCONTEXTRECOMBINER_H_//NOLINT
