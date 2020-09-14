// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <hash_set>
#include "base/ContextDecomposerBase.h"

using std::string;

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_GPOSTAGGER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_GPOSTAGGER_H_

class POSTaggerBase
{
  public:
    // POSTagger base
    virtual ~POSTaggerBase()
    {
    }
    // POSTag a set of tokens that form a string.
    virtual void postag(Context & tokens) const = 0;
    // The operator to postag on the tokens;
    virtual void operator()(Context & tokens) = 0;
};

extern "C"
{
  extern int _initialize(char *basepath);
  extern char *tag(char *buf, int enhanced_penntag);
  extern char * Tokenizer(char *buff);
  extern void destroy_tagger(void);
}

class GPOSTagger: public POSTaggerBase
{
  public:
    virtual ~GPOSTagger();
    explicit GPOSTagger(string const & gpostDataDir);
    // POSTag a set of tokens taht form a string
    virtual void postag(Context  & tokens) const;
    virtual void operator()(Context & tokens);
  private:

    string _dataDir;
    mutable bool _initialized;
    void initialize_tagger() const;
};


#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_UTIL_GPOSTAGGER_H_
