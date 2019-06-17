// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANSIMPLIFIER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANSIMPLIFIER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include <hash_set>
#include "./sentence/Sentence.h"
#include "./ContextDecomposerBase.h"

// Interface to the simplifier of Siddharthan. Calls a subprocess
// that does the simplification and parses its output.
class SiddharthanSimplifier: public ContextDecomposerBase
{
  public:
    explicit SiddharthanSimplifier(std::string const & simplifyPath,
        bool writeToStdOut) :
      _simplifyBin(simplifyPath), _writeToStdOut(writeToStdOut), _initialized(
          false) { }
    virtual ~SiddharthanSimplifier();
    virtual std::vector<Context> const decompose(
        Sentence & sentence);
    virtual std::vector<ContextPtr> const
        decomposeToPtr(Sentence & sentence);

  private:
    // Open a subprocess that can be written to STDIN and read from STDOUT.
    pid_t popen2(const char *command, int *infp, int *outfp);
    // Path to the binary that calls the simplify system.
    std::string _simplifyBin;
    // The filehandles for STDIN/STDOUT.
    int infp, outfp;
    void initialize();
    bool _writeToStdOut;
    bool _initialized;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANSIMPLIFIER_H_
