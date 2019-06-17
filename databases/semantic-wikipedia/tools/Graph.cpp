// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.
// Modified and left as a mess by buchholb
// and haussma ;-)

#include <assert.h>
#include <sstream>
#include <string>
#include <vector>
#include "./Graph.h"

using std::vector;

// _____________________________________________________________________________
Graph::Graph()
{
  _numVertices = 0;
  _numEdges = 0;
}

// _____________________________________________________________________________
size_t Graph::addVertex()
{
  assert(_adjacencyLists.size() == _numVertices);
  _numVertices++;
  _adjacencyLists.resize(_numVertices);
  return _numVertices - 1;
}

// _____________________________________________________________________________
void Graph::addEdge(size_t u, size_t v)
{
  assert(u < _numVertices);
  assert(v < _numVertices);
  _adjacencyLists[u].push_back(v);
  _adjacencyLists[v].push_back(u);
  _numEdges += 2;
}

// _____________________________________________________________________________
std::string Graph::asString()
{
  std::ostringstream os;
  os << "{" << _numVertices << ", " << _numEdges;
  for (size_t i = 0; i < _adjacencyLists.size(); i++)
    for (size_t j = 0; j < _adjacencyLists[i].size(); j++)
      os << ", (" << i << "," << _adjacencyLists[i][j] << ")";
  os << "}";
  return os.str();
}

// _____________________________________________________________________________
void Graph::enumeratePathsBetween(size_t u, size_t v,
    ad_utility::HashSet<size_t>* visitedVertices,
    std::vector<std::vector<size_t> >* result) const
{
  // End recursion?
  if (u == v)
  {
    vector<size_t> final;
    final.push_back(u);
    result->push_back(final);
    return;
  }
  visitedVertices->insert(u);
  const vector<size_t>& edges = _adjacencyLists[u];

  for (size_t i = 0; i < edges.size(); ++i)
  {
    vector<vector<size_t> > paths;

    size_t nextU = edges[i];

    if (visitedVertices->count(nextU) != 0) continue;
    enumeratePathsBetween(nextU, v, visitedVertices, &paths);
    for (size_t j = 0; j < paths.size(); ++j)
    {
        paths[j].push_back(u);
        result->push_back(paths[j]);
    }
  }
}
