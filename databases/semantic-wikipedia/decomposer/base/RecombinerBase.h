// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_RECOMBINERBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_RECOMBINERBASE_H_

#include <algorithm>
#include <string>
#include <vector>

namespace ad_decompose
{
// Forward declarations.
template<class NodeTypes> class TreeNode;

// Base class for recombiners using the visitor patter.
// A recombiner is applied to a node
// in the tree and extracts contexts in its subtree. It possibly
// modifies the tree for example removing relative clauses if
// they were extracted.
template<class Token, class NodeTypes>
class RecombinerBase
{
  public:
    // Default ctor. The ContextType and NodeTypes of the tree are
    // passed.
    RecombinerBase<Token, NodeTypes>() :
        _head(true)
    {
      _immediateResults = new Contexts<Token>;
    }
    // Public invocation to apply the recombiner on a node.
    // Return all recombined contexts. This includes immediate results.
    Contexts<Token> apply(TreeNode<NodeTypes> * node)
    {
      // Forward to the private implementation which may call other
      // recombiners.
      Contexts<Token> result = applyOnNode(node, _immediateResults);
      // If this is the head, that is, the first recombiner in a possible
      // recursive call hierarchy extract from the immediateresults and
      // return them.
      if (_head)
      {
        if (!_immediateResults->empty())
        {
          result = result + *_immediateResults;
        }
      }
      return result;
    }

    // Virtual dtor. Only the head creates the immediateResults and thus must
    // remove them. The parent class dtor is called automatically by each
    // subclass.
    virtual ~RecombinerBase()
    {
      if (_head == true) delete _immediateResults;
    }

  protected:
    // Protected ctor used by subclasses.
    RecombinerBase(Contexts<Token> * immediateResults, bool head) :
        _immediateResults(immediateResults), _head(head)
    {
    }
    // Store "immediate" results, i.e. results that are final and need
    // not be passed up the call hierarchy to be further processed.
    Contexts<Token> * _immediateResults;
    // Is this class the first one in a hierarchical call structure.
    // Influences whether the return value contains immediate results.
    bool _head;
  private:
    // Actual implementation to apply the recombiner on a node.
    // Return all recombined contexts, but put immediate results into the
    // provided container.
    virtual Contexts<Token>
    applyOnNode(TreeNode<NodeTypes>* node,
        Contexts<Token> * immediateResults) = 0;
};

// Base class for a transformer using the visitor pattern.
// It is applied to a node to transform the tree for whatever reasons.
template<class NodeTypes>
class TransformerBase
{
  public:
    virtual bool apply(TreeNode<NodeTypes>* node) = 0;
    virtual ~TransformerBase()
    {
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_RECOMBINERBASE_H_
