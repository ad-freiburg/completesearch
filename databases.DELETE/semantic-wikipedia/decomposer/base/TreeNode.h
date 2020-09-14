// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_TREENODE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_TREENODE_H_

#include <boost/ptr_container/ptr_vector.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include "base/RecombinerBase.h"
#include "util/ContextDecomposerUtil.h"

namespace ad_decompose
{
template<class NodeTypes>
class TreeNode
{
  // TODO(elmar): provide better documentation on the methods.
  public:
    // By storing the roots and children lists of the tree in this
    // container ownership and destruction is guaranteed.
    typedef boost::ptr_vector<TreeNode<NodeTypes> > NodeList;

    TreeNode<NodeTypes>() :
        _type(NodeTypes::ROOT), _parent(NULL)
    {
      initialize();
    }

    // Destructor.
    virtual ~TreeNode()
    {
    }

    // Creates and adds a child to the node.
    // Appends it to the list of children.
    // Also takes ownership.
    template<class Node>
    Node & addChild()
    {
      Node * temp = new Node(this);
      addChild(temp);
      return *temp;
    }

    // Creates and adds a child to the node.
    // Appends it to the list of children.
    // Also takes ownership.
    template<class Node>
    Node & addChild(typename NodeTypes::TypeEnum type)
    {
      Node * temp = new Node(type, this);
      addChild(temp);
      return *temp;
    }

    // Get the type of the node.
    typename NodeTypes::TypeEnum type() const
    {
      return _type;
    }

    // Get a reference to the children vector.
    TreeNode<NodeTypes> * getChild(size_t i)
    {
      return &_children[i];
    }

    // Remove the child at given index. All nodes
    // below are automatically deleted. Indices so far may become
    // invalid.
    virtual void removeChild(size_t childIndex)
    {
      assert(_childTypes[_children[childIndex].type()] > 0);
      _childTypes[_children[childIndex].type()]--;
      _children.erase(_children.begin() + childIndex);
    }

    // Remove the child with given pointer. All nodes
    // below are automatically deleted. Indices so far may become
    // invalid.
    void removeChild(TreeNode<NodeTypes> * node)
    {
      for (size_t i = 0; i < _children.size(); ++i)
      {
        if (node == &_children[i]) removeChild(i);
      }
    }

    // TODO(elmar): Possibly rename this.
    // Combine child nodes at given indices below a new node of
    // provided type. Effectively this moves the child nodes
    // below the newly inserted node. The links between the child nodes
    // and their respective subtrees are left untouched.
    // The new node is inserted at the first index of nodeIndices.
    template<class Node>
    void combineNodes(vector<size_t> & nodeIndices,
        typename NodeTypes::TypeEnum type)
    {
      // Make sure the nodes are processed in ascending order.
      std::sort(nodeIndices.begin(), nodeIndices.end());
      vector<size_t>::const_iterator it;
      Node * node = new Node(type, this);
      vector<TreeNode<NodeTypes> *> toMove;
      addChild(node, nodeIndices.front());
      for (it = nodeIndices.begin(); it < nodeIndices.end(); ++it)
      {
        toMove.push_back(&_children[(*it) + 1]);
      }
      for (size_t i = 0; i < toMove.size(); ++i)
        toMove[i]->moveNode(node);
    }

    // TODO(elmar): Possibly rename this.
    // Merge child nodes at given indices below a new node of
    // provided type. Effectively this moves all grandchildren
    // specified via the child nodes, below a new node of
    // provided type, and deletes the direct child nodes.
    // The new node is inserted at the first index of nodeIndices.
    template<class Node>
    void mergeNodes(vector<size_t> & nodeIndices,
        typename NodeTypes::TypeEnum type)
    {
      // Make sure the nodes are processed in ascending order.
      std::sort(nodeIndices.begin(), nodeIndices.end());
      vector<size_t>::const_iterator it;
      Node * newNode = new Node(type, this);
      vector<TreeNode<NodeTypes> *> toMove;
      // Insert the new node.
      addChild(newNode, nodeIndices.front());
      // Collect the grandchildren to move.
      for (it = nodeIndices.begin(); it < nodeIndices.end(); ++it)
      {
        Node * childNode = &_children[(*it) + 1];
        for (size_t i = 0; i < childNode->getChildren().size(); ++i)
          toMove.push_back(childNode->getChild(i));
      }
      // Now move them.
      for (size_t i = 0; i < toMove.size(); ++i)
        toMove[i]->moveNode(newNode);
      // And delete the old children.
      for (it = nodeIndices.begin(); it < nodeIndices.end(); ++it)
        removeChild(nodeIndices.front() + 1);
    }

    // TODO(elmar): need to test this
    // Remove the child identified by the index in the tree hierarchy,
    // and insert all grandchildren in the list of children instead.
    void eraseChild(size_t childIndex)
    {
      NodeList const & grandChildren =
          _children[childIndex].getChildren();
      for (size_t i = 0; i < grandChildren.size(); ++i)
      {
        // Add the child at the current index, this takes
        // ownership.
        _children[childIndex].getChild(i)->_parent = this;
        addChild(_children[childIndex].getChild(i), childIndex);
        // The childnode at childIndex was moved due to the insertion.
        childIndex++;
      }
      // The child node needs to give up ownership of the children.
      _children[childIndex].releaseChildren();
      // All children have been moved and ownership was transferred,
      // now we can remove the child.
      removeChild(childIndex);
    }

    // Move the node to a new parent.
    void moveNode(TreeNode<NodeTypes> * newParent)
    {
      _parent->releaseChild(this);
      newParent->addChild(this);
      _parent = newParent;
    }

    // Move the node to a new parent.
    void setType(typename NodeTypes::TypeEnum type)
    {
      // Update _childTypes of parent.
      if (_parent != NULL)
      {
        _parent->_childTypes[_type]--;
        _parent->_childTypes[type]++;
      }
      // Update type;
      _type = type;
    }

    // TODO(elmar): need to test this.
    // Move the node to a new parent at specified position.
    void moveNode(TreeNode<NodeTypes> * newParent, size_t pos)
    {
      _parent->releaseChild(this);
      newParent->addChild(this, pos);
      _parent = newParent;
    }


    // Same as above but a const-variant.
    NodeList const & getChildren() const
    {
      return _children;
    }

    // Get a reference to the node's parent pointer;
    // May return NULL if no parent is present.
    TreeNode<NodeTypes> * getParent() const
    {
      return _parent;
    }

    // Return true if this node has at least one child of the
    // provided type.
    bool hasChildType(typename NodeTypes::TypeEnum type) const
    {
      return (_childTypes[type] > 0);
    }

    // Return the node's type as string. Default Implementation.
    virtual string typeAsString() const
    {
      std::ostringstream s;
      string typeString;
      if (!ContextDecomposerUtil::mapLookup(NodeTypes::typeToStringMap, _type,
          &typeString))
      {
        s << "Type_" << _type;
        typeString = s.str();
      }
      return typeString;
    }

    // Apply a transformer to this tree-node. This possibly modifies the tree.
    virtual void applyTransformation(TransformerBase<NodeTypes> * t)
    {
      t->apply(this);
    }

    // Apply a recombiner to this tree-node. This possibly modifies the tree.
    template<class ContextType>
    Contexts<ContextType> applyRecombiner(
        RecombinerBase<ContextType, NodeTypes> * r)
    {
      return r->apply(this);
    }

    // Return a multi-line string representation of
    // the tree rooted at this node.
    virtual string treeAsString(string indent = "") const
    {
      string result = "\n" + indent + typeAsString();
      for (size_t i = 0; i < _children.size(); ++i)
      {
        result += _children[i].treeAsString(indent + " ");
      }
      result += ")";
      return result;
    }

    // Return a flat string representation of the tree rooted at this node.
    virtual string treeAsFlatString() const
    {
      string result = typeAsString() + " ";
      for (size_t i = 0; i < _children.size(); ++i)
      {
        result += _children[i].treeAsFlatString();
      }
      result += ")";
      return result;
    }

  protected:

    TreeNode<NodeTypes>(typename NodeTypes::TypeEnum type,
        TreeNode<NodeTypes> * parent) :
        _type(type), _parent(parent)
    {
      assert(_parent != NULL);
      initialize();
    }

    FRIEND_TEST(TreeNode, testAddChildPriv);
    void addChild(TreeNode<NodeTypes> * node)
    {
      _children.push_back(node);
      _childTypes[node->type()]++;
    }

    FRIEND_TEST(TreeNode, testAddChildIdxPriv);
    // Add a child at position pos. Invalidates iterators etc.
    void addChild(TreeNode<NodeTypes> * node, size_t pos)
    {
      assert(pos < _children.size());
      _children.insert(_children.begin() + pos, node);
      _childTypes[node->type()]++;
    }

    FRIEND_TEST(TreeNode, testReleaseChildPriv);
    // Release the child ownership. This also removes
    // it from the list of children!
    void releaseChild(TreeNode<NodeTypes> * childNode)
    {
      for (size_t i = 0; i < _children.size(); ++i)
      {
        if (&_children[i] == childNode)
        {
          _children.release(_children.begin() + i).release();
          assert(_childTypes[childNode->type()]>0);
          _childTypes[childNode->type()]--;
        }
      }
    }

    // Release all child ownerships. This also removes
    // them from the list of children, i.e.
    // the node will not have children afterwards.
    void releaseChildren()
    {
      for (size_t i = 0; i < _children.size();)
      {
        releaseChild(size_t(0));
      }
      assert(_children.size() == 0);
    }

    // TODO(elmar): need to test this
    // Release the child ownership. This also removes
    // it from the list of children!
    void releaseChild(size_t index)
    {
      typename NodeTypes::TypeEnum type = _children[index].type();
      _children.release(_children.begin() + index).release();
      assert(_childTypes[type]>0);
      _childTypes[type]--;
    }

    // Type of this node.
    typename NodeTypes::TypeEnum _type;
    // The children of this node.
    NodeList _children;
    // The parent of this node.
    TreeNode<NodeTypes> * _parent;
    // Children types present.
    size_t _childTypes[NodeTypes::COUNT];

  private:
    // These are private and not to be used.
    explicit TreeNode<NodeTypes>(TreeNode<NodeTypes> const & node);
    TreeNode<NodeTypes> const & operator=(TreeNode<NodeTypes> const & node);

    void initialize()
    {
      for (size_t i = 0; i < NodeTypes::COUNT; ++i)
        _childTypes[i] = 0;
    }
};

template<class NodeTypes, class TerminalType>
class TerminalNode: public TreeNode<NodeTypes>
{
  public:
    typedef TreeNode<NodeTypes> BaesClass;
    // Single argument constructors are explicit to avoid implicit use
    // in conversions.
    explicit TerminalNode(TreeNode<NodeTypes> * parent) :
        TreeNode<NodeTypes>(NodeTypes::TERMINAL, parent)
    {
      assert(parent != NULL);
    }
    virtual ~TerminalNode()
    {
    }

    // Append a phrase/terminal.
    void addTerminal(TerminalType terminal)
    {
      _terminals.push_back(terminal);
    }

    vector<TerminalType> const & getTerminals() const
    {
      return _terminals;
    }

    vector<TerminalType> & getTerminals()
    {
      return _terminals;
    }

    // Return the node's type as string.
    virtual string typeAsString() const
    {
      return "TerminalNode";
    }

    // In addition to the type, print the terminals.
    virtual string treeAsString(string indent) const
    {
      // string result = " Terminals:";
      string result;
      for (size_t i = 0; i < _terminals.size(); ++i)
      {
        result += " " + _terminals[i]->asString();
      }
      // result += "\n";
      for (size_t i = 0; i < BaesClass::_children.size(); ++i)
      {
        result += BaesClass::_children[i].treeAsString(indent + " ");
      }
      return result;
    }

    // In addition to the type, print the terminals.
    virtual string treeAsFlatString() const
    {
      string result;
      for (size_t i = 0; i < _terminals.size(); ++i)
      {
        result += _terminals[i]->asString() + " ";
      }
      for (size_t i = 0; i < BaesClass::_children.size(); ++i)
      {
        result += BaesClass::_children[i].treeAsFlatString();
      }
      return result;
    }

  private:
    vector<TerminalType> _terminals;
    explicit TerminalNode(TerminalNode const & node);
    TerminalNode const & operator=(TerminalNode const & node);
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_TREENODE_H_
