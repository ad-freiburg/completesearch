// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTRECOMBINER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTRECOMBINER_H_

#include <boost/ptr_container/ptr_vector.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include "base/ContextExtractorBase.h"
#include "base/RecombinerBase.h"
#include "sentence/Sentence.h"
#include "base/ParserBase.h"
#include "base/TreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"


namespace ad_decompose
{
// A type to hold our Tree types.
class MarkerNodeTypes
{
  public:
    // TODO(elmar): Find a better solution for the workaround "COUNT".
    enum TypeEnum
    {
      ROOT = 0, TERMINAL = 1, LIT = 2, ENUM = 3, REL = 4, RELA = 5, COUNT = 6
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


typedef TreeNode<MarkerNodeTypes> MarkerTreeNode;
typedef TerminalNode<MarkerNodeTypes, Phrase<DefaultToken> *>
  MarkerTerminalNode;

// A class to extract contexts based on the tree structure
// provided by the identification phase.
template <class Token>
class ContextRecombiner: public ContextRecombinerBase<Token>
{
  public:
    // Extract contexts from the previously marked sentence,
    // but only return pointers
    // to the tokens for efficiency reasons.
    virtual Contexts<Token> const
    extractContextsPtr(Sentence<Token> & sentence);

    ContextRecombiner() :
        _parseFails(0), _parseSuccess(0)
    {
    }

    virtual ~ContextRecombiner()
    {
      LOG(INFO)
      << "ContextRecombiner failed to parse " << _parseFails << " out of "
          << (_parseFails + _parseSuccess) << " marked sentences.\n";
    }

  private:
    // Counters for failed/successful parses of identified sentences.
    size_t _parseFails;
    size_t _parseSuccess;
};

// Implementation of the Relative Clause recombination.
// Attaches a relative clause and apposition to the noun they
// describe, adds the result to immediateResults and removes the
// attached clause from the tree.
template<class DefRecombiner, class Token>
class RelativeClauseRecombiner: public RecombinerBase<Token,
    MarkerNodeTypes>
{
  public:
    // This is just so the assert below works fine.
    typedef RecombinerBase<Token, MarkerNodeTypes> BaseClass;
    explicit RelativeClauseRecombiner(Contexts<Token> * immediateResults)
    : RecombinerBase<Token, MarkerNodeTypes>(immediateResults, false)
    {
      assert(BaseClass::_immediateResults != NULL);
    }
    virtual ~RelativeClauseRecombiner()
    {
    }
  private:
    virtual Contexts<Token> applyOnNode(TreeNode<MarkerNodeTypes>* node,
        Contexts<Token> * immediateResults);
    // Find the attachment for the relative clause at relIndex in the
    // children of parent. Returns the attachment(s) as Contexts.
    Contexts<Token> findRelativeClauseAttachment(
        TreeNode<MarkerNodeTypes> * parent, size_t relIndex);
    RelativeClauseRecombiner()
    {
    }
};

// A class for recombining enumerations.
template<class DefRecombiner, class Token>
class EnumerationRecombiner: public RecombinerBase<Token, MarkerNodeTypes>
{
  public:
    // This is just so the assert below works fine.
    typedef RecombinerBase<Token, MarkerNodeTypes> BaseClass;

    explicit EnumerationRecombiner(Contexts<Token> * immediateResults) :
        RecombinerBase<Token, MarkerNodeTypes>(immediateResults, false)
    {
      assert(BaseClass::_immediateResults != NULL);
    }
    virtual ~EnumerationRecombiner()
    {
    }
  private:
    // Return the Contexts below an ENUM node. Possibly recursively
    // calls the DefaultRecombiner to extract Contexts if below an ENUM
    // node a LIT node is found.
    Contexts<Token> getEnumerationContexts(
        TreeNode<MarkerNodeTypes> * node,
        Contexts<Token> * immediatResults);
    virtual Contexts<Token> applyOnNode(TreeNode<MarkerNodeTypes>* node,
        Contexts<Token> * immediateResults);
    EnumerationRecombiner()
    {
    }
};

// A class to transform the tree by collating LIT nodes that belong to the
// same enumeration below an ENUM node.
class CollateEnumerationTransformer: public TransformerBase<MarkerNodeTypes>
{
  public:
    virtual ~CollateEnumerationTransformer()
    {
    }
    virtual bool apply(TreeNode<MarkerNodeTypes>* node);
  private:
    // Returns true if the subtree anchored at node continues an enumeration,
    // that is it does not interrupt LIT nodes that belong together.
    bool isEnumerationContinuation(
        TreeNode<MarkerNodeTypes> const & node) const;
};

// The default recombiner which recursively calls itself or the
// RelativeClauseRecombiner/EnumerationRecombiner to recombine
// contexts. RelativeClauseRecombiner/EnumerationRecombiner can also
// recursively apply this class.
template<class Token>
class DefaultRecombiner: public RecombinerBase<Token, MarkerNodeTypes>
{
  public:
    DefaultRecombiner() :
        RecombinerBase<Token, MarkerNodeTypes>()
    {
    }
    explicit DefaultRecombiner(Contexts<Token> * immediateResults) :
        RecombinerBase<Token, MarkerNodeTypes>(immediateResults, false)
    {
    }
    virtual ~DefaultRecombiner()
    {
    }
  private:
    virtual Contexts<Token> applyOnNode(TreeNode<MarkerNodeTypes>* node,
        Contexts<Token> * immediateResults);
};

// Template instantiations.
template class DefaultRecombiner<DefaultToken>;
template class RelativeClauseRecombiner<DefaultRecombiner<DefaultToken>,
    DefaultToken>;
template class EnumerationRecombiner<DefaultRecombiner<DefaultToken>,
    DefaultToken>;

template <class Token>
class DefaultPhraseParser: public ParserBase<MarkerNodeTypes, Token>
{
  public:
    virtual bool parseSentence(Sentence<Token> const & sentence,
        TreeNode<MarkerNodeTypes>::NodeList * roots);
    virtual ~DefaultPhraseParser()
    {
    }
};

template class DefaultPhraseParser<DefaultToken>;

template class TreeNode<MarkerNodeTypes>;
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTRECOMBINER_H_
