// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <ext/hash_set>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "./SemanticWikipediaDecomposer.h"
#include "./SiddharthanSimplifier.h"
#include "./ContextDecomposerUtil.h"

#define READ 0
#define WRITE 1

// ____________________________________________________________________________
pid_t SiddharthanSimplifier::popen2(const char *command, int *infp, int *outfp)
{
  int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }
    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];
  // The way it was p_stdin[read] in this program is still open
    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];
  // as well as p_stdout[write], they're closed in the fork,
  // but not in the original program
  // fix is ez:
  close(p_stdin[READ]);  // We only write to the forks input anyway
  close(p_stdout[WRITE]);  // and we only read from its output
    return pid;
}

// ____________________________________________________________________________
SiddharthanSimplifier::~SiddharthanSimplifier()
{
}

// ____________________________________________________________________________
void SiddharthanSimplifier::initialize()
{
  // Start the simplifier.
  if (popen2(_simplifyBin.c_str(), &infp, &outfp) <= 0)
  {
      printf("Unable to exec sort\n");
      exit(1);
  }
  sleep(4);
  _initialized = true;
}


// ____________________________________________________________________________
std::vector<Context> const SiddharthanSimplifier::decompose(
    Sentence & sentence)
{
  if (!_initialized)
  {
    initialize();
  }
  std::vector<Context > result;
  ssize_t test;
  char buf[128];

  std::string sentenceStr = sentence.getSentenceAsString() + "\n";
  // std::cout << sentenceStr;
  test = write(infp, sentenceStr.c_str(), sentenceStr.length());
  // sleep(1);
  bool resultSeen = false;
  std::string resultString;
  // Read until we see EOS.
  while (!resultSeen)
    {
    test = read(outfp, buf, 1);
    resultString += buf[0];
    // The last character we read completes EOS.
    if (resultString.length() > 3 && resultString.substr(resultString.length()
        - 3, 3) == "EOS")
    {
      resultSeen = true;
    }
  }
  // Strip of the EOS.
  resultString = resultString.substr(0, resultString.length() - 3);
  std::vector<std::string> contexts;
  // Each context ends up in a seperate line
  boost::split(contexts, resultString, boost::is_any_of("\n"));

  if (_writeToStdOut)
  {
    std::cout << "Input :\t" << sentenceStr;
    std::cout << "Result:\n" << resultString << "\n";
  }

  // TODO(elmar): There must be a better way to do this.
  // The last line is empty so only go to size()-1
  for (size_t i = 0; i < contexts.size() - 1; ++i)
  {
    Context newContext;
    std::vector<string> tokens = ContextDecomposerUtil::tokenizeString(
        contexts[i]);
    for (size_t t = 0; t < tokens.size(); ++t)
    {
      Token token;
      token.tokenString = tokens[t];
      newContext.push_back(token);
    }
    result.push_back(newContext);
  }
  return result;
}

// ____________________________________________________________________________
std::vector<ContextPtr > const SiddharthanSimplifier::decomposeToPtr(
    Sentence & sentence)
{
  std::cerr << "Not implemented for this decomposer.\n";
  assert(false);
  std::vector<ContextPtr> result;
  return result;
}

