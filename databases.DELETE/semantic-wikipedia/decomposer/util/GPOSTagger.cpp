// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include "./GPOSTagger.h"
#include "./ContextDecomposerUtil.h"


using std::string;

// ____________________________________________________________________________
GPOSTagger::~GPOSTagger()
{
  // TODO(elmar): destroy_tagger causes a segfault - why?
  /*if (_initialized)
    destroy_tagger();*/
}

// ____________________________________________________________________________
GPOSTagger::GPOSTagger(string const & gpostDataDir)
{
  _initialized = false;
  _dataDir = gpostDataDir;
}

// ____________________________________________________________________________
void GPOSTagger::postag(Context & tokens) const
{
  if (!_initialized)
    initialize_tagger();
  // Concatenate to create input for postagger.
  string input = ContextDecomposerUtil::tokenVectorToString(tokens);
  char * output = tag(Tokenizer(const_cast <char*>(input.c_str())), 0);
  // Not nice, but now we parse the output
  string strOutput(output);

  std::vector<std::string> lines;
  // Each token ends up in a seperate line
  boost::split(lines, strOutput, boost::is_any_of("\n"));
  tokens.clear();
  for (size_t l = 0; l < lines.size(); ++l)
  {
    // The last line in the output is empty.
    if (lines[l] == "")
      continue;
    std::vector<std::string> columns;
    // Each line contains the word in the first column and the pos
    // tag in the second column.
    boost::split(columns, lines[l], boost::is_any_of("\t"));
    assert(columns.size() == 2);
    DefaultToken token;
    token.posTag = columns[1];
    token.tokenString = columns[0];
    tokens.push_back(token);
  }
  // std::cout << output << "\n";
  free(output);
}

void GPOSTagger::operator()(Context & tokens)
{
  assert(tokens.size() > 0);
  if (!_initialized)
    initialize_tagger();
  postag(tokens);
}

// ____________________________________________________________________________
void GPOSTagger::initialize_tagger() const
{
  int i = _initialize(const_cast<char*> (_dataDir.c_str()));

  if (i != 0)
  {
    std::cerr << "GPOSTTL initialization failed\n";
    assert(i == 0);
  }
  _initialized = 1;
}


