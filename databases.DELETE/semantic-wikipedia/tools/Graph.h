// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#ifndef SEMANTIC_WIKIPEDIA_TOOLS_GRAPH_H_
#define SEMANTIC_WIKIPEDIA_TOOLS_GRAPH_H_

#include <string>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/HashSet.h"

// A simple graph class for the purpose of demonstrating our coding standards.
class Graph
{
 public:
  // Create the empty graph.
  Graph();

  // Add a vertex to the graph.
  size_t addVertex();

  // Add an edge to the graph.
  void addEdge(size_t u, size_t v);

  void enumeratePathsBetween(size_t u,
      size_t v, ad_utility::HashSet<size_t>* visitedVertices,
      std::vector<std::vector<size_t> >* result) const;

  // Getters.
  size_t getNumVertices() const { return _numVertices; }
  size_t getNumEdges() const { return _numEdges; }

  // The graph as a string, in human-readable form.
  std::string asString();

 private:
  // The adjacency list for each node. Each edge {u,v} is stored twice, in the
  // adjacency list for u and in the adjacency list for v.
  std::vector<std::vector<size_t> > _adjacencyLists;

  // The number of vertices and edges in the graph.
  size_t _numVertices;
  size_t _numEdges;
};

#endif  // SEMANTIC_WIKIPEDIA_TOOLS_GRAPH_H_
