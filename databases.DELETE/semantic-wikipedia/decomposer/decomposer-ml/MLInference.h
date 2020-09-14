// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLINFERENCE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLINFERENCE_H_
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>

#define MAX_VERTICES 10000
#define FALLBACK_GREEDY_SIZE 30

using std::string;
using std::set;
using std::vector;
namespace ad_decompose
{
class MLInference
{
  public:
    struct Vertex
    {
        enum Mark
        {
          NOTVISITED, VISITED
        } mark;
        int id;
        size_t start;
        size_t end;
        double weight;
        string name;
        string type;
    };

    struct Edge
    {
    };

    typedef boost::adjacency_list<boost::listS, boost::listS,
        boost::undirectedS, Vertex, Edge> MyGraph;

    typedef MyGraph::vertex_descriptor VertexDesc;
    typedef MyGraph::edge_descriptor EdgeDesc;

    MyGraph graph;
    MLInference();
    // Solve by enumerating all possibilities.
    MyGraph solveByEnumeration(MyGraph graph) const;
    // Solve using a greedy approach.
    MyGraph solveGreedy(MyGraph graph) const;
    // Decide on whether to use the greedy or the optimal approach.
    MyGraph solve(MyGraph graph) const;
    vector<Vertex> constructAndSolveGraph(vector<Vertex> const &) const;
  private:
    // Return true if the two ranges defined by startA/B and endA/B intersect.
    bool
        intersects(size_t startA, size_t endA, size_t startB,
            size_t endB) const;
    // Uses method above.
    bool intersects(Vertex const & vertexA, Vertex const & vertexB) const;
    // Return the sum of the weights of all nodes in the graph.
    double weight(MLInference::MyGraph const & graph) const;
    // TODO(elmar): this is super-inefficient. Need to look for alternative.
    void deleteNodeAndAdjacentNodes(MLInference::MyGraph * graph, int id) const;
    // TODO(elmar): this is super-inefficient. Need to look for alternative.
    void deleteNodeId(MLInference::MyGraph * graph, VertexDesc v) const;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLINFERENCE_H_
