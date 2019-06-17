// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::ostream;
using std::ostringstream;
using std::setw;
using std::pair;
using std::make_pair;


// A node of a parse tree.
class ParseTreeNode {
 public:
  // String label. For an inner node this is the node type (VP, NP, PP, ADVP,
  // etc.), for a leaf this is the word.
  string _label;
  // Pointers to children. This node is a leaf iff this vector is empty.
  vector<ParseTreeNode*> _children;
  // Pointer to parent (null if root);
  ParseTreeNode* _parent;
  // Depth of node (0 for root node).
  size_t _depth;
};

// A Parse Tree.
class ParseTree {
 public:
  // Construct parse tree from string with typed paranthesis, e.g.
  // (TOP Hannah (VP is writing) (NP a program)). Implemented recursively, call
  // from the outside with startPos = 0 and parentNode = NULL.
  // Returns position in sentence after the parse (for a fully parathesized
  // sentences this should be at the end of the sentence).
  size_t buildFromSentenceRecursive(const string& sentence, size_t startPos,
      ParseTreeNode* parentNode, ostringstream* os);
  void buildFromSentence(const string& sentence);
  // Show (sub)tree rooted at given node and show whole tree.
  void showRecursive(const ParseTreeNode* node);
  void show();

  // Recursive sentence constituent identification as dicussed with Elmar on
  // Wednesday, November 23.
  void sciRecursive(const ParseTreeNode* node, bool isPartOfEnumeration,
      ostream* os);
  string sci();
  // Simply SCI tree: for nodes where are children are LH, merge into a single
  // child of type LH.
  void simplifySciTreeRecursive(ParseTreeNode* node);
  void simplifySciTree();

  // Sentence recombination. Simply returns a vector of subsentences.
  vector<string> scrRecursive(const ParseTreeNode* node, ostringstream* os);
  vector<string> scr();

 private:
  // The root of the tree.  
  ParseTreeNode* _root;
};

// _____________________________________________________________________________
size_t ParseTree::buildFromSentenceRecursive(const string& sentence,
    size_t startPos, ParseTreeNode* parentNode, ostringstream* os) {
  size_t pos = startPos;
  while (pos < sentence.size()) {
    // Case 1: openening paranthesis, e.g. (VP.
    if (sentence[pos] == '(') {
      // Get node type.
      size_t oldPos = ++pos;
      while (pos < sentence.size() && isupper(sentence[pos])) pos++;
      string nodeType = sentence.substr(oldPos, pos - oldPos);
      (*os) << "(" << nodeType << endl;
      // Create new node and recurse.
      ParseTreeNode* node = new ParseTreeNode();
      node->_label = nodeType;
      node->_parent = parentNode;
      node->_depth = parentNode == NULL ? 0 : parentNode->_depth + 1;
      if (parentNode == NULL) _root = node;
      else parentNode->_children.push_back(node);
      size_t newPos = buildFromSentenceRecursive(sentence, pos, node, os);
      pos = newPos;
    }
    // Case 2: closing paranthesis.
    else if (sentence[pos] == ')') {
      assert(parentNode != NULL);
      (*os) << ")" << parentNode->_label << endl;
      pos++;
      return pos;
    }
    // Case 3: whitespace after paranthesis.
    else if (sentence[pos] == ' ') {
      while (pos < sentence.size() && sentence[pos] == ' ') pos++;
    }
    // Case 4: parts between parentheses.
    else {
      assert(parentNode != NULL);
      // Continue scanning string until next ( or ). Omit trailing whitespace.
      size_t oldPos = pos++;
      while (pos < sentence.size() &&
             sentence[pos] != ')' && sentence[pos] != '(') pos++;
      size_t newPos = pos;
      while (pos > 0 && sentence[pos-1] == ' ') pos--;
      string sentencePart = sentence.substr(oldPos, pos - oldPos);
      pos = newPos;
      (*os) << sentencePart << endl;
      // Create new node with that part.
      ParseTreeNode* node = new ParseTreeNode();
      node->_label = sentencePart;
      node->_parent = parentNode;
      node->_depth = parentNode->_depth + 1;
      parentNode->_children.push_back(node);
    }
  }
  return pos;
}

// _____________________________________________________________________________
void ParseTree::buildFromSentence(const string& sentence) {
  ostringstream os;
  size_t finalPos = buildFromSentenceRecursive(sentence, 0, NULL, &os);
}

// _____________________________________________________________________________
void ParseTree::showRecursive(const ParseTreeNode* node) {
  assert(node != NULL);
  // Indentation.
  ostringstream indentation;
  for (size_t i = 0; i < node->_depth; i++) indentation << "  ";
  // Case 1: inner node.
  if (node->_children.size() > 0) {
    cout << endl << indentation.str();
    cout << "(" << node->_label << " " << flush;
    for (size_t i = 0; i < node->_children.size(); i++) {
      showRecursive(node->_children[i]);
    }
    cout << ")" << flush;
  }
  // Case 2: leaf node.
  else {
    cout << node->_label;
  }
}

// _____________________________________________________________________________
void ParseTree::show() {
  showRecursive(_root);
  cout << endl;
  cout << endl;
}

// _____________________________________________________________________________
void ParseTree::sciRecursive(const ParseTreeNode* node,
    bool isPartOfEnumeration, ostream* os) {
  assert(node != NULL);
  // Check if the children of this node are an enumeration and of which type.
  // NOTE: Currently supports X enumeration (>= 2 Xs under a X node), where X is
  // one of S or NP or VP.
  size_t listItemCount = 0;
  if (node->_label == "S" || node->_label == "NP" || node->_label == "VP") {
    for (size_t i = 0; i < node->_children.size(); i++) {
      if (node->_children[i]->_label == node->_label) {
        listItemCount++;
      }
    }
  }
  bool childrenAreEnumeration = listItemCount >= 2;
  // Case 1: children are an enumeration.
  // NOTE: if enumeration type is X, take every node of type X as a list item
  // (LI) and everything else as list head (LH). Currently X is one S, VP, NP.
  if (childrenAreEnumeration) {
    (*os) << "(LE " << flush;
    bool listHasBegun = false;
    for (size_t i = 0; i < node->_children.size(); i++) {
      // Is this node an enumeration with >= 2 items?
      bool isListItem = node->_children[i]->_label == node->_label;
      // Case 1.1: list item (LI).
      if (isListItem) {
        (*os) << "(LI " << flush;
        sciRecursive(node->_children[i], true, os);
        (*os) << ")" << flush;
      }
      // Case 1.2: not a list item (LH).
      // NOTE: LH is a misnomer, this is not only the head of the list but
      // anything between list items.
      else {
        (*os) << "(LH " << flush;
        sciRecursive(node->_children[i], true, os);
        (*os) << ")" << flush;
      }
    }
    (*os) << ")" << flush;
  }
  // Case 2: node has children, but they are not an enumeration.
  else if (node->_children.size() > 0) {
    for (size_t i = 0; i < node->_children.size(); i++) {
      sciRecursive(node->_children[i], true, os);
      // sciRecursive(node->_children[i], true, os);
    }
  }
  // Case 3: leaf node.
  else {
    // if (true) {
    if (isPartOfEnumeration) {
      (*os) << "(LH " << flush;
      (*os) << node->_label << flush;
      (*os) << ")" << flush;
    }
    else {
      (*os) << node->_label << " " << flush;
    }
  }
}

// _____________________________________________________________________________
string ParseTree::sci() {
  ostringstream os;
  os << "(LE ";
  sciRecursive(_root, true, &os);
  os << ")";
  return os.str();
}

// _____________________________________________________________________________
void ParseTree::simplifySciTreeRecursive(ParseTreeNode* node) {
  assert(node != NULL);
  if (node->_children.size() == 0) return;
  // Check if all children are leaves of type LH and merge along the way.
  size_t count = 0;
  ostringstream leavesMerged;
  for (size_t i = 0; i < node->_children.size(); i++) {
    if (node->_children[i]->_label == "LH" &&
        node->_children[i]->_children.size() == 1) {
      count++;
      if (i > 0) leavesMerged << " ";
      leavesMerged << node->_children[i]->_children[0]->_label;
    }
  }
  // If yes, replace by single child with merged contents.
  if (count > 1 && count == node->_children.size()) {
    node->_children.resize(1);
    node->_children[0]->_children[0]->_label = leavesMerged.str();
  }
  // Otherwise recurse.
  else {
    for (size_t i = 0; i < node->_children.size(); i++) {
      simplifySciTreeRecursive(node->_children[i]);
    }
  }
}

// _____________________________________________________________________________
void ParseTree::simplifySciTree() {
  simplifySciTreeRecursive(_root);
}


// _____________________________________________________________________________
vector<string> ParseTree::scrRecursive(const ParseTreeNode* node,
    ostringstream *os) {
  assert(node != NULL);
  int verbosity = 0;
  if (verbosity > 0) {
    cout << "BEG node#" << node << ": " << node->_label << "(#children = "
         << node->_children.size() << ")" << endl;
  }
  // CASE 1: this is an (LH node above a leaf node) => just return
  // a single subsentence with the contents.
  if (node->_children.size() == 0) {
    // NOTE: Should never come here!
    cout << "!!!" << node->_label << endl;
    assert(false);
    vector<string> result(1);
    result[0] = node->_label;
    return result;
  }
  if (node->_children[0]->_children.size() == 0) {
    // cout << "Is a leaf" << endl;
    vector<string> result(1);
    result[0] = node->_children[0]->_label;
    if (verbosity > 0) {
      cout << "END node#" << node << ": " << node->_label << "(#children = "
           << node->_children.size() << ")" << endl;
    }
    return result;
  }
  // CASE 2: not a leaf node.
  // First identify all "blocks", where each block is a maximal sequence of
  // adjacent LH siblings, or all LI nodes of an LE node. If the first child is
  // an LE node, an empty LH block will be added before. if the last child is an
  // LE node, an empty LH block will be added after. This ensures that we always
  // obtain a sequence of the kind LH+ LI+ LH+ ... LH+ LI+ LH+, where + means
  // one or more.
  // The second component of the pair is the sub-result for that node, which
  // will be computed (recursively) below.
  vector<vector<pair<ParseTreeNode*, vector<string> > > > blocks;
  vector<string> emptyResult;
  vector<pair<ParseTreeNode*, vector<string> > > emptyBlock;
  for (size_t i = 0; i < node->_children.size(); i++) {
    // If everything is correct this procedure is only called for nodes which have
    // only LH and LE children. (LI nodes are always children of LE nodes.)
    string type = node->_children[i]->_label;
    assert(type != "LI");
    // Case: LE node. Add all LI children to block. Non-LI children from LE
    // nodes are simply omitted.
    if (type == "LE") {
      vector<pair<ParseTreeNode*, vector<string> > > block;
      for (size_t j = 0; j < node->_children[i]->_children.size(); j++) {
        ParseTreeNode* child = node->_children[i]->_children[j];
        // cout << j << ": " << liNode->_label << endl;
        if (child->_label == "LI") {
          block.push_back(make_pair(child, emptyResult));
        }
      }
      assert(block.size() >= 2);  // Enumeration has >= 2 LIs.
      // If this the first child, add an empty LH block before. This ensures that we
      // always start with an LH block.
      if (i == 0) blocks.push_back(emptyBlock);
      // Now add the block of LI items.
      blocks.push_back(block);
      // If this is the last child, add an empty LH block in the end. This
      // ensures that we always end with an LH block.
      if (i + 1 == node->_children.size()) blocks.push_back(emptyBlock);
    }
    // Case: Non-LE node which means it's an LH node. If this is the first
    // non-LE node in a run (= current block size is even), start a new block,
    // otherwise append to existing block.
    else {
      if (blocks.size() % 2 == 0) blocks.push_back(emptyBlock);
      assert(blocks.size() > 0);
      blocks.back().push_back(make_pair(node->_children[i], emptyResult));
    }
  }
  // DEBUG: Just output the blocks for now..
  assert(blocks.size() > 0);
  if (verbosity > 0) {
    for (size_t i = 0; i < blocks.size(); i++) {
      cout << "Block " << i << ": " << flush;
      for (size_t j = 0; j < blocks[i].size(); j++) {
        ParseTreeNode* node = blocks[i][j].first;
        cout << "node#" << node << " (" << node->_label << ") " << flush;
      }
      cout << endl;
    }
    cout << endl;
  }
  size_t numEnums = blocks.size() / 2;
  assert(blocks.size() == 2 * numEnums + 1);
  // Recursively get the result for each item from each block.
  for (size_t i = 0; i < blocks.size(); i++) {
    for (size_t j = 0; j < blocks[i].size(); j++) {
      ParseTreeNode* node = blocks[i][j].first;
      blocks[i][j].second = scrRecursive(node, os);
      if (verbosity > 0) {
        cout << "Subresult for block#" << i << ", part#" << j << "): ";
        for (size_t k = 0; k < blocks[i][j].second.size(); k++) {
          cout << "[" << blocks[i][j].second[k] << "]";
        }
        cout << endl;
      }
    }
  }
  // Now "multiply it out".
  // To that end first construct an "outer" counter that iterates over all
  // possible combinations of indexes in each of the enumerations. Inititalized
  // to position 0 in each enumeration. The second component is the max value
  // for the respective digit.
  vector<pair<size_t, size_t> > outerCounter;
  for (size_t i = 0; i < numEnums; i++)
    outerCounter.push_back(make_pair(0, blocks[2*i+1].size()));
  outerCounter.push_back(make_pair(0, 2));
  vector<string> result;
  while (true) {
    // Now construct an inner counter that iterates over the result sets
    // (recursively computed above) of the items determined by the outer counter.
    // Also maintain a map of block indices, so that we know which counter
    // (index in a 1-dimensional array) belongs to which block (indices in a
    // 2-dimensional array).
    vector<pair<size_t, size_t> > innerCounter;
    vector<pair<size_t, size_t> > blockIndicesMap;
    for (size_t i = 0; i < blocks.size(); i++) {
      // From the (even-numbered) LH blocks consider everything.
      if (i % 2 == 0) {
        for (size_t j = 0; j < blocks[i].size(); j++) {
          innerCounter.push_back(make_pair(0, blocks[i][j].second.size()));
          blockIndicesMap.push_back(make_pair(i, j));
          // cout << "C:  i = " << i << ", j = " << j << ", max = " <<  blocks[i][j].second.size() << endl;
        }
      }
      // From enum blocks consider just the item corresponding to the current
      // outer counter digit.
      else {
        size_t j = outerCounter[i/2].first;
        innerCounter.push_back(make_pair(0, blocks[i][j].second.size()));
        blockIndicesMap.push_back(make_pair(i, j));
      }
    }
    innerCounter.push_back(make_pair(0, 2));
    // Add the combination corresponding to the current inner counter value.
    while (true) {
      ostringstream os;
      for (size_t c = 0; c < blockIndicesMap.size(); c++) {
        size_t i = blockIndicesMap[c].first;
        size_t j = blockIndicesMap[c].second;
        size_t k = innerCounter[c].first;
        // cout << "i = " << i << ", j = " << j << ", k = " <<  k << endl;
        if (c > 0) os << " ";
        // if (os.str().size() > 0) os << " ";
        os << blocks[i][j].second[k];
      }
      result.push_back(os.str());
      // Advance inner counter. If (artificial) final digit increases to 1 we are done.
      for (size_t i = 0; i < innerCounter.size(); i++) {
        innerCounter[i].first++;
        if (innerCounter[i].first < innerCounter[i].second) break;
        innerCounter[i].first = 0;
      }
      if (innerCounter.back().first > 0) break;
    }
    // Advance outer counter. If (artificial) final digit increases to 1 we are done.
    for (size_t i = 0; i < outerCounter.size(); i++) {
      outerCounter[i].first++;
      if (outerCounter[i].first < outerCounter[i].second) break;
      outerCounter[i].first = 0;
    }
    if (outerCounter.back().first > 0) break;
  }
  if (verbosity > 0) {
    cout << "END node#" << node << ": " << node->_label << "(#children = "
         << node->_children.size() << ")" << endl;
  }
  return result;
}


// _____________________________________________________________________________
vector<string> ParseTree::scr() {
  ostringstream os;
  vector<string> result = scrRecursive(_root, &os);
  cout << os.str() << endl;
  return result;
}


// Main function.
int main(int argc, char** argv) {
  // Some example sentences.
  const char* examples[] = {
    "(TOP (NP Hannah) (VP does like) (NP (NP apples) (CC and) (NP oranges)))",
    "(TOP (NP Charlie Chaplin) (VP added (NP (NP Peter) (NP Paul) (CC and) (NP Mary))"
      " (PP to (NP his stock companies (NP (NP one) (CC and) (NP two))))))",
    "(TOP (S (PP For (NP (NP one year) (CC or) (NP two years))) (NP Polnareff)"
      " (VP was (VP (VP locked (PRT up) (PP at (NP (NP Royal Monceau) (PP in (NP Paris)))))"
      " (CC and) (CC ,) (VP (S (VP surrounded (PP by (NP (NP friends) (CC and) (NP alcohol)))))) (CC ,)"
      " (VP recorded (NP Kama Sutra) (PP with (S (NP Mike Oldfield) (VP adding (NP some guitar parts)))))))))",
    "(TOP (S (NP Cherwell) (VP argued (SBAR that"
      " (S (S (NP unemployment) (VP would (VP rise (PP to (NP (QP (one million))))))) (CC and)"
      " (S (NP inflation) (VP could (VP increase (SBAR if (S (NP the plan (VP was (VP implemented)))))))))))))",
  };
  int numExamples = sizeof(examples) / sizeof(const char*);
 
  // Parse command line arguments.
  if (argc != 2) {
    cout << endl;
    cout << "Usage: ./sci <sentence (in quotes) or index of example sentence below>" << endl;
    cout << endl;
    for (int i = 0; i < numExamples; i++) {
      cout << (i + 1) << ". " << examples[i] << endl;
    }
    cout << endl;
    exit(1);
  }
  int i = atoi(argv[1]);
  string sentence = i > 0 && i <= numExamples ? examples[i-1] : argv[1];

  // Construct parse tree and show.
  cout << endl;
  cout << "Original sentence: " << sentence << endl;
  ParseTree parseTree;
  parseTree.buildFromSentence(sentence);
  // parseTree.show();

  // Do SCI.
  string result = parseTree.sci();
  cout << endl;
  cout << "After SCI: " << result;
  ParseTree parseTree2;
  parseTree2.buildFromSentence(result);
  cout << endl;
  parseTree2.simplifySciTree();
  parseTree2.show();

  // Do SCR and show the result (one line per subsentence).
  cout << "After SCR:" << endl;
  vector<string> contexts = parseTree2.scr();
  for (size_t i = 0; i < contexts.size(); i++) {
    cout << setw(2) << (i+1) << ". " << contexts[i] << "." << endl;
  }
  cout << endl;
}

// Debugging stuff.

// Show outer counter.
// cout << "Outer counter [" << blocks.size() << "] = " << flush;
// for (size_t i = 0; i < outerCounter.size(); i++)
//   cout << outerCounter[i].first << "[" << outerCounter[i].second << "]" << flush;
// cout << endl;

// Show inner counter.
// cout << "Inner counter [" << blocks.size() << "] = " << flush;
// for (size_t i = 0; i < innerCounter.size(); i++)
//   cout << innerCounter[i].first << "[" << innerCounter[i].second << "]" << flush;
// cout << endl;
