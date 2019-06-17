// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_CONTEXT_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_CONTEXT_H_

#include <sentence/Sentence.h>
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <string>

using std::string;

namespace ad_decompose
{
// This is a wrapper class around an actual implementations to allow
// different types of contexts to be used.
template<class Token>
class Context
{
  public:
    // This defines the actual implementation of a token.
    // For now it is just a vector of token pointers.
    typedef std::vector<Token const *> ContextType;
    typedef typename ContextType::iterator iterator;
    typedef typename ContextType::const_iterator const_iterator;

    // Forward to the implementation.
    typename ContextType::const_iterator begin() const
    {
      return _value.begin();
    }

    // Forward to the implementation.
    typename ContextType::const_iterator end() const
    {
      return _value.end();
    }

    // Forward to the implementation.
    typename ContextType::iterator begin()
    {
      return _value.begin();
    }

    // Forward to the implementation.
    typename ContextType::iterator end()
    {
      return _value.end();
    }

    Token const * operator[](size_t i) const
    {
      return _value[i];
    }

    // Forward to the implementation.
    void insert(typename ContextType::iterator insertEnd,
        typename ContextType::const_iterator begin,
        typename ContextType::const_iterator end)
    {
      _value.insert(insertEnd, begin, end);
    }

    // Addition operator. Just appends otherContext to this context.
    Context<Token> const operator+(
        Context<Token> const & otherContext) const
    {
      Context<Token> res;
      res.insert(res.end(), begin(), end());
      res.insert(res.end(), otherContext.begin(), otherContext.end());
      return res;
    }

    string asString() const
    {
      std::ostringstream result;
      for (size_t i = 0; i < _value.size(); ++i)
      {
        result << _value[i]->tokenString;
        if (i != _value.size() - 1) result << " ";
      }
      return result.str();
    }

    void addPhrase(Phrase<Token> const & phrase)
    {
      for (size_t i = 0; i < phrase.getWords().size(); ++i)
      {
        _value.push_back(phrase.getWords()[i]);
      }
    }

    // Erase token at iterator it. Invalidates all iterators.
    void erase(Context<Token>::iterator it)
    {
      _value.erase(it);
    }

    // Clear the contents. Forward to implementation.
    void clear()
    {
      _value.clear();
    }

    size_t size() const
    {
      return _value.size();
    }

    // Append another token. Forward to implementation.
    void push_back(Token * token)
    {
      _value.push_back(token);
    }

    // TODO(elmar): this should of course be private.
    // Not sure if this ContextWrapper object is a good idea after all.
    // Probably the templates are too much and we should use ONE
    // object throughout the whole code. This one is (at time of writing this)
    // only used in the new recombinator.
    ContextType _value;

  private:
};

template class Context<DefaultToken>;
template class Context<DeepToken>;

// This is a wrapper class around a set of contexts to allow
// different types of contexts to be used. Internally it wraps
// each context in the Context wrapper object.
template<class Token>
class Contexts
{
  public:
    // This defines the actual implementation of a token.
    // For now it is just a vector of token pointers.
    typedef std::vector<Token const *> ContextType;

    // Forward to the implementation.
    typename vector<Context<Token> >::const_iterator begin() const
    {
      return _value.begin();
    }

    // Forward to the implementation.
    typename vector<Context<Token> >::const_iterator end() const
    {
      return _value.end();
    }

    // Forward to the implementation.
    typename vector<Context<Token> >::iterator begin()
    {
      return _value.begin();
    }

    // Forward to the implementation.
    typename vector<Context<Token> >::iterator end()
    {
      return _value.end();
    }

    // Addition operator. Constructs the union of both sets of contexts..
    Contexts<Token> const operator+(
        Contexts<Token> const & otherContexts) const
    {
      Contexts<Token> res;
      res._value.insert(res._value.end(), _value.begin(), _value.end());
      res._value.insert(res._value.end(), otherContexts._value.begin(),
          otherContexts._value.end());
      return res;
    }

    // Multiplication operator. Constructs the cartesian product of
    // two sets of contexts. Addition of contexts means appending,
    // whereas the usual cartesian product uses the union.
    Contexts<Token> const operator*(
        Contexts<Token> const & otherContexts) const
    {
      Contexts<Token> result;
      if (_value.size() == 0) result = otherContexts;
      else if (otherContexts._value.size() == 0) result = *this;
      else
      {
        for (size_t i = 0; i < _value.size(); ++i)
        {
          for (size_t j = 0; j < otherContexts.size(); ++j)
          {
            result.addContext((*this)[i] + otherContexts[j]);
          }
        }
      }
      return result;
    }

    // Just add the single context to the set of contexts.
    void addContext(Context<Token> const & toAdd)
    {
      _value.push_back(toAdd);
    }

    Context<Token> const & operator[](size_t i) const
    {
      return _value[i];
    }

    size_t size() const
    {
      return _value.size();
    }

    // The same as the addition operator, but adds to this
    // instance instead of returning a new one.
    void addContexts(Contexts<Token> const & contexts)
    {
      _value.insert(_value.end(), contexts._value.begin(),
          contexts._value.end());
    }

    string asString() const
    {
      std::ostringstream result;
      for (size_t i = 0; i < _value.size(); ++i)
      {
        result << "Context" << i << ": ";
        result << _value[i].asString();
        result << "\n";
      }
      return result.str();
    }

    bool empty() const
    {
      return _value.empty();
    }

    void clear()
    {
      _value.clear();
    }

    // Return const ref of implementation value.
    Context<Token> const & operator[](size_t i)
    {
      return _value[i];
    }

  private:
    vector<Context<Token> > _value;
};
template class Contexts<DeepToken>;
template class Contexts<DefaultToken>;
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_CONTEXT_H_
