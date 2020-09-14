// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_PARSERBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_PARSERBASE_H_

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <string>
#include <vector>
#include "sentence/Sentence.h"
#include "base/TreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::map;
using std::string;

namespace ad_decompose
{
// The base class for parsers. The parser gets a sentence structure
// and return a (list of) tree(s).
template<class NodeTypes, class Token>
class ParserBase
{
  public:
    // Parse the provided sentence and put the root(s) into
    // the result.
    virtual bool
    parseSentence(Sentence<Token> const & sentence,
        typename TreeNode<NodeTypes>::NodeList * roots) = 0;
    // Same as above, but parse the sentence from a string and build
    // the sentence structure instead of using it.
    template<class TerminalNodeType>
    bool parseSentenceFromString(
        std::string const & sentenceStr,
        Sentence<Token> * sentence,
        typename TreeNode<NodeTypes>::NodeList * roots,
        map<string, typename NodeTypes::TypeEnum> const & stringToNodeTypeMap)
    {
      typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
      boost::char_separator<char> sep(" ", "()", boost::drop_empty_tokens);
      Tokenizer tok(sentenceStr, sep);
      vector<string> tokens;
      for (Tokenizer::iterator iter = tok.begin(); iter != tok.end(); ++iter)
      {
        string token = *iter;
        // Opening bracket, read next token to get complete token.
        if (token== "(")
        {
         ++iter;
         assert(iter != tok.end());
         token += *(iter);
          tokens.push_back(token);
        }
        else
          tokens.push_back(token);
      }

      // Create the mandatory root.
      TreeNode<NodeTypes> * root = new TreeNode<NodeTypes>();
      TerminalNodeType * terminal = NULL;
      typename map<string, typename NodeTypes::TypeEnum>::const_iterator mapIt;
      roots->push_back(root);
      for (size_t i = 0; i < tokens.size(); ++i)
      {
        // Opening bracket.
        if ((mapIt = stringToNodeTypeMap.find(tokens[i]))
            != stringToNodeTypeMap.end())
        {
          root = &root->template addChild<TreeNode<NodeTypes> >(mapIt->second);
          terminal = NULL;
        }
        // Closing bracket.
        else if (tokens[i] == ")")
        {
          // This means too many closing brackets.
          if (root == NULL)
          {
            LOG(ERROR) << "Could not parse string correctly. Surplus closing"
                << " bracket at token " << i << "." << std::endl;
            return false;
          }
          root = root->getParent();
          terminal = NULL;
        }
        // This is some terminal...
        else
        {
          Token * t = new Token();
          t->tokenString = tokens[i];
          sentence->storeToken(t);
          sentence->appendWord(t);
          if (terminal == NULL) terminal = &(root->template addChild<
              TerminalNodeType>());
          terminal->addTerminal(t);
        }
      }
      if (&roots->back() != root)
      {
        LOG(ERROR)
        << "Could not parse string correctly. Missing closing"
            << " bracket(s) at end of sentence." << std::endl;
        return false;
      }
      else
        return true;
    }

    virtual ~ParserBase()
    {
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_PARSERBASE_H_
