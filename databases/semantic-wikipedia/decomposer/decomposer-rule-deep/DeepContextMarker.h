// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_DEEP_DEEPCONTEXTMARKER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_DEEP_DEEPCONTEXTMARKER_H_

#include <map>
#include <string>
#include <vector>
#include "base/ContextMarkerBase.h"
#include "base/TreeNode.h"
#include "base/ParserBase.h"
#include "util/ContextDecomposerUtil.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

namespace ad_decompose
{
// A class to hold our TreeNode types.
class DeepParserNodeTypes
{
  public:
    // TODO(elmar): Find a better solution for the workaround "COUNT".
    enum TypeEnum
    {
      ROOT = 0,
      ADJP = 1,
      ADVP = 2,
      CONJP = 3,
      FRAG = 4,
      INTJ = 5,
      LST = 6,
      NAC = 7,
      NP = 8,
      NX = 9,
      PP = 10,
      PRN = 11,
      PRT = 12,
      PRT_ADVP = 13,
      QP = 14,
      RRC = 15,
      S = 16,
      SBAR = 17,
      SBARQ = 18,
      SINV = 19,
      SQ = 20,
      TOP = 21,
      UCP = 22,
      VP = 23,
      WHADJP = 24,
      WHADVP = 25,
      WHNP = 26,
      WHPP = 27,
      X = 28,
      OTHER = 29,
      S1 = 30,
      TERMINAL = 31,
      COUNT = 32
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


// A parser that reads the results of a deep parser to build sentence
// structure.
// TODO(elmar): Evaluate whether we need a different parser each time.
// It seems the only difference is where the information for the next
// token comes from (i.e. a combined string as part of a token or
// a vector of marks as part of the token).
class DeepParserSentenceParser: public ParserBase<DeepParserNodeTypes,
  DeepToken>
{
  public:
    typedef map<string, DeepParserNodeTypes::TypeEnum> StringToNodeTypeMap;

    static const char TAG_CLOSE = ')';

    DeepParserSentenceParser()
    {
    }

    // If parsing a sentence fails, this tries to repair the parsed
    // structure.
    void repairParseTree(Sentence<DeepToken> const & sentence,
        TreeNode<DeepParserNodeTypes>::NodeList * rootList) const;

    // Parse the provided sentence into a tree. Each root
    // found is put into the nodelist.
    virtual bool parseSentence(Sentence<DeepToken> const & sentence,
        TreeNode<DeepParserNodeTypes>::NodeList * roots);

    virtual ~DeepParserSentenceParser()
    {
    }

  private:
};


// A marker(SCI) that uses the results of a deep parser for constituent
// identification.
class DeepContextMarker: public ContextMarkerBase<DeepToken>
{
  public:
    typedef TerminalNode<DeepParserNodeTypes, DeepToken *>
        DeepParserTerminalNode;
    typedef TreeNode<DeepParserNodeTypes> DeepParserNode;
    typedef DeepParserNode::NodeList DeepParserNodeList;

    DeepContextMarker()
    :_parser(), _parserErrors(0)
    {
      initEnumContinuationMap();
    }

    // Mark the sentence.
    virtual void markSentence(Sentence<DeepToken> * sentence);

    // Mark the sentence, but it is provided parsed, as a tree.
    void markTree(DeepParserNodeList & rootList);

    virtual ~DeepContextMarker()
    {
    }

    string statsAsString()
    {
      std::stringstream s;
      s << "DeepContextMarker failed to parse " << _parserErrors
          << " sentences using the deep parser output." << std::endl;
      return s.str();
    }

  private:
    DeepParserSentenceParser _parser;

    // A map with strings that continue an enumeration,
    // such as the words "and", "or" etc.
    ad_utility::HashSet<string> _enumContinuationMap;

    // This nests relative clauses below a new node with the head
    // (and anything inbetween) they are attached to.
    void nestRelativeClauses(DeepParserNode & root);
    FRIEND_TEST(DeepContextMarker, testNestRelativeClauses);

    // Return true if the subtree rooted at the provided node is
    // a relative clause. For this means root is an SBAR clause
    // and the first child is of type WHNP.
    bool isRelativeClause(DeepParserNode const & root);
    FRIEND_TEST(DeepContextMarker, testIsRelativeClause);

    // Attach the relative clause at relativeClauesNodeIndex below root.
    // Return true if the attachment was possible. The value of the index
    // to the node it should be attached to is provided in "head".
    // Return false if it could not be attached.
    // Trivial implementation: attach to the closest preceeding NP.
    bool attachRelativeClause(DeepParserNode const & root,
        size_t relativeClauesNodeIndex, size_t * head);
    FRIEND_TEST(DeepContextMarker, testAttachRelativeClause);

    // Return true if the node resembles a C_HAT - a node that represents
    // a context on its own.
    bool isC_HAT(DeepParserNode const & node);
    FRIEND_TEST(DeepContextMarker, testIsC_HAT);


    // Mark the whole subtree below root.
    // This marks enumerations, potential heads and relative clauses.
    void markSubtree(DeepParserNode const & root);
    FRIEND_TEST(DeepContextMarker, testMarkSubtree);

    // This finds several enumerations within the children of root,
    // Currently consecutive clauses of same type
    // are considered an enumeration, each clause representing
    // a list item. The enumeration may be interrupted by "filling"
    // clauses, such as clauses consisting only of the word(s) "but",
    // "and", "as well as" etc.
    // E.g. the type sequence of the children: X Y Y C Y X Z Z C Z X
    // should find two enumerations (Y, Y, Y) and (X, X, X) given
    // that C is an enumeration continuation. (also X!=Y and X!=Z).
    void findEnumerations(DeepParserNode const & root,
        vector< vector <size_t> > * enumerations);
    FRIEND_TEST(DeepContextMarker, testFindEnumerations);


    // Return true if the provided subtree rooted at node can act as a
    // continuation of an enumeration. E.g. if it only consists of the
    // words "but", "or", "as well as" etc..
    bool isEnumerationContinuation(DeepParserNode const & node);
    FRIEND_TEST(DeepContextMarker, testIsEnumerationContinuation);

    // Mark the left-most leaf (which is a terminal) of the subtree rooted
    // at node, with the provided mark. Marks are always appended to already
    // existing marks.
    void markLeftMostLeaf(DeepParserNode const & node
        , DeepTokenMarks::TypeEnum mark);
    FRIEND_TEST(DeepContextMarker, markLeftMostLeaf);

    // Mark the right-most leaf (which is a terminal) of the subtree rooted
    // at node, with the provided mark. Marks are always appended to already
    // existing marks.
    void markRightMostLeaf(DeepParserNode const & node
        , DeepTokenMarks::TypeEnum mark);
    FRIEND_TEST(DeepContextMarker, markRightMostLeaf);

    // Initialise the _enumContinuationMap.
    void initEnumContinuationMap();

    // Count parser errors.
    int _parserErrors;
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_DEEP_DEEPCONTEXTMARKER_H_ //NOLINT
