// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

#include "./Graph.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;
using std::vector;


#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"


namespace ad_utility
{
void doStuff(char* inputFileName)
{
  File input(inputFileName, "r");
  string line;
  char buf[2048];

  Graph graph;
  HashMap<string, int> categoryIndices("__adutils_empty_key");

  typedef HashMap<string, int>::const_iterator mapit;
  vector<string> indexToCategory;

  size_t lineNum = 0;
  // Build the graph
  while (input.readLine(&line, buf, 2048))
  {
    ++lineNum;
    while (!startsWith(line, ":ct:"))
    {
      std::cerr << "Line " << lineNum << " messed up: " << line << std::endl;
      input.readLine(&line, buf, 2048);
      ++lineNum;
    }
    string parent = ad_utility::getLastPartOfString(line, ':');
    input.readLine(&line, buf, 2048);
    ++lineNum;
    string child = ad_utility::getLastPartOfString(line, ':');

    size_t pIndex;
    size_t cIndex;
    mapit it = categoryIndices.find(parent);
    if (it != categoryIndices.end())
    {
      pIndex = it->second;
    }
    else
    {
      pIndex = graph.addVertex();
      categoryIndices[parent] = pIndex;
    }
     it = categoryIndices.find(child);
    if (it != categoryIndices.end())
    {
      cIndex = it->second;
    }
    else
    {
      cIndex = graph.addVertex();
      categoryIndices[child] = cIndex;
    }
    graph.addEdge(pIndex, cIndex);
  }
  LOG(INFO) << "Size of graph: " << graph.getNumVertices()
      << std::endl << std::endl;
  string source("Anarchism");
  string target("Medicine");
  LOG(INFO) << "Finding paths between " << source << " and " << target
      << std::endl;
  size_t start = categoryIndices[source];
  size_t end = categoryIndices[target];
  LOG(INFO) << "Indices " << start << " and " << end << std::endl;
  vector<vector<size_t> > result;
  HashSet<size_t> visited;
  graph.enumeratePathsBetween(start, end, &visited, &result);
  for (size_t i = 0; i < result.size(); ++i)
  {
    std::cout << "Path " << i << ": ";
    for (size_t j = 0; j < result[i].size(); ++j)
    {
      std::cout << result[i][result[i].size() - (j + 1)] << ": "
          << indexToCategory[result[i][result[i].size() - (j + 1)]] << ", ";
    }
    std::cout << std::endl;
  }
}
}
// Available options.
struct option options[] =
{
  {"example-option", required_argument, NULL, 'o'},
  {NULL, 0, NULL, 0}
};

// Main function.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON << "<PROGRAM NAME>, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string exampleOption = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "e:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'e':
      exampleOption = optarg;
      break;
    default:
      cout << endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  ad_utility::doStuff(argv[1]);
  return 0;
}

