// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <ext/hash_set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "util/ContextDecomposerUtil.h"
#include "./MLInference.h"

using std::pair;
using std::string;
using std::vector;
namespace ad_decompose
{
// ____________________________________________________________________________
MLInference::MLInference()
{
}

// ____________________________________________________________________________
MLInference::MyGraph MLInference::solve(MLInference::MyGraph graph) const
{
  if (num_vertices(graph) < FALLBACK_GREEDY_SIZE)
    return solveByEnumeration(graph);
  else
    return solveGreedy(graph);
}

// ____________________________________________________________________________
MLInference::MyGraph MLInference::solveGreedy(MLInference::MyGraph graph) const
{
  boost::graph_traits<MyGraph>::vertex_iterator i, end, maxNode;
  bool newNodeFound = true;
  MyGraph resultGraph;

  // Set all to unvisited.
  for (boost::tie(i, end) = vertices(graph); i != end; ++i)
  {
    graph[*i].mark = Vertex::NOTVISITED;
  }

  // TODO(elmar): can increase performance if list
  // of nodes is sorted by weight.
  while (newNodeFound)
  {
    newNodeFound = false;
    float maxWeight = 0;
    VertexDesc maxVertex = 0;
    for (boost::tie(i, end) = vertices(graph); i != end; ++i)
    {
      Vertex & v = graph[*i];
      if (v.mark != Vertex::VISITED && v.weight > maxWeight)
      {
        maxWeight = v.weight;
        newNodeFound = true;
        maxVertex = *i;
      }
    }
    if (newNodeFound)
    {
      // This node belongs to the result.
      MLInference::VertexDesc vIDMax = boost::add_vertex(resultGraph);
      // std::cout << "ADDED NEW VERTEX\n";
      resultGraph[vIDMax] = graph[maxVertex];
      // Mark this node visited.
      graph[maxVertex].mark = Vertex::VISITED;
      boost::graph_traits<MyGraph>::adjacency_iterator ai, a_end, next;
      // Mark adjacent nodes visited.
      for (boost::tie(ai, a_end) = adjacent_vertices(maxVertex, graph); ai
          != a_end; ++ai)
      {
        graph[*ai].mark = Vertex::VISITED;
      }
    }
  }
  return resultGraph;
}

// ____________________________________________________________________________
MLInference::MyGraph MLInference::solveByEnumeration(
    MLInference::MyGraph graph) const
{
  boost::graph_traits<MyGraph>::vertex_iterator i, end;
  boost::graph_traits<MyGraph>::adjacency_iterator ai, a_end, next;
  boost::graph_traits<MyGraph>::degree_size_type maxDegree = 0;
  int maxId;
  VertexDesc maxVertex = 0;
  // Vertex maxVertex;
  // std::cout << "Call\n";
  // std::cout << "Graph has " << num_vertices(graph) << " vertices\n";

  // Get vertex with maximum degree.
  for (boost::tie(i, end) = vertices(graph); i != end; ++i)
  {
    Vertex & vertex = graph[*i];
    // std::cout << vertex.name << "\n";
    if (maxDegree < out_degree(*i, graph))
    {
      maxVertex = *i;
      maxDegree = out_degree(*i, graph);
      maxId = vertex.id;
      // std::cout << (maxDegree) << " edges\n";
    }
  }
  // We only have disconnected nodes. This is an independent set.
  if (maxDegree == 0)
  {
    // std::cout << "Returning here\n";
    return graph;
  }
  else
  {
    // This creates a copy
    Vertex maxV = graph[maxVertex];
    MyGraph n1 = MyGraph(graph);
    // std::cout << "HERE - " << maxVertex << "\n";

    // Delete the node with highest degree.
    deleteNodeId(&graph, maxVertex);

    // Delete the node with highest degree and its
    // adjacent nodes.
    deleteNodeAndAdjacentNodes(&n1, maxV.id);

    MyGraph res1 = solve(graph);
    MyGraph res2 = solve(n1);

    MLInference::VertexDesc newMax = boost::add_vertex(res2);
    res2[newMax] = maxV;

    if (weight(res1) > weight(res2))
    {
      return res1;
    }
    else
    {
      // Return the graph that contains the node with highest degree,
      // but not its adjacent nodes.
      return res2;
    }
  }
  assert(false);
  return graph;
}

// ____________________________________________________________________________
double MLInference::weight(MLInference::MyGraph const & graph) const
{
  double weight = 0;
  boost::graph_traits<MyGraph>::vertex_iterator i, end;
  for (boost::tie(i, end) = vertices(graph); i != end; ++i)
  {
    weight += graph[*i].weight;
  }
  return weight;
}

// ____________________________________________________________________________
void MLInference::deleteNodeId(MLInference::MyGraph * graph, VertexDesc v) const
{
  /*boost::graph_traits<MyGraph>::vertex_iterator i, end;
   for (boost::tie(i, end) = vertices(*graph); i != end; ++i)
   {
   Vertex & vertex = ((*graph)[*i]);
   if (vertex.id == id)
   {
   // std::cout << "Deleted\n";
   clear_vertex(*i, *graph);
   remove_vertex(*i, *graph);
   return;
   }
   }
   */
  clear_vertex(v, *graph);
  remove_vertex(v, *graph);
}

// ____________________________________________________________________________
vector<MLInference::Vertex> MLInference::constructAndSolveGraph(
    vector<Vertex> const & verticesV) const
{
  MyGraph graph;
  int id = 0;
  // Add the vertices.
  for (size_t v = 0; v < verticesV.size(); ++v)
  {
    // std::cout << "Adding vertex \n";
    MLInference::VertexDesc vID = boost::add_vertex(graph);
    // vID is the index of a new Vertex
    graph[vID].start = verticesV[v].start;
    // std::cout << "start " << graph[vID].start << "\n" ;
    graph[vID].end = verticesV[v].end;
    // std::cout << "end " << graph[vID].end << "\n" ;
    graph[vID].type = verticesV[v].type;
    // std::cout << "type " << graph[vID].type << "\n" ;
    graph[vID].weight = verticesV[v].weight;
    graph[vID].mark = Vertex::NOTVISITED;
    graph[vID].id = ++id;
  }
  // std::cout << num_vertices(graph) << " vertices" <<  "added.\n";

  // Add the edges.
  boost::graph_traits<MyGraph>::vertex_iterator j, j_end;
  boost::graph_traits<MyGraph>::vertex_iterator i, i_end;
  MLInference::EdgeDesc edge;
  bool ok;
  // int k = 0;
  for (boost::tie(i, i_end) = vertices(graph); i != i_end; ++i)
  {
    Vertex & vertexi = graph[*i];
    boost::tie(j, j_end) = vertices(graph);
    vertexi.mark = Vertex::VISITED;

    for (; j != j_end; ++j)
    {
      Vertex & vertexj = graph[*j];
      // If it is marked visited it has been checked for edges to all
      // vertices. No need to check again.
      if (vertexj.mark == Vertex::VISITED || vertexj.id == vertexi.id)
        continue;
      if (intersects(vertexi, vertexj) || vertexj.start == vertexi.start)
      {
        //  std::cout << "Adding edge between " << vertexi.start << ","
        //      << vertexi.end << " and " << vertexj.start <<","
        //      << vertexj.end << "\n";
        boost::tie(edge, ok) = boost::add_edge(*i, *j, graph);
        // k++;
      }
    }
    // std::cout << k << " edges to be " <<  "added.\n";
  }

  MyGraph resultGraph = solve(graph);
  vector<Vertex> result;
  for (boost::tie(i, i_end) = vertices(resultGraph); i != i_end; ++i)
  {
    Vertex & vertex = resultGraph[*i];
    result.push_back(vertex);
  }
  return result;
}

// ____________________________________________________________________________
bool MLInference::intersects(Vertex const & vertexA, Vertex const & vertexB)
  const
{
  return intersects(vertexA.start, vertexA.end, vertexB.start, vertexB.end);
}

// ____________________________________________________________________________
bool MLInference::intersects(size_t startA, size_t endA, size_t startB,
    size_t endB) const
{
  if (startA < startB && startB <= endA && endA < endB)
    return true;
  else if (startB < startA && startA <= endB && endB < endA)
    return true;
  return false;
}

// ____________________________________________________________________________
void MLInference::deleteNodeAndAdjacentNodes(MLInference::MyGraph * graph,
    int id) const
{
  // TODO(elmar): This remains a mystery. Collecting the vertex_descriptors in
  // a vector and deleting them then is fine.
  // The reverse order (w/o collecting in a vector) does not work however
  // and segfaults. This is not according to documentation where deletion
  // with a list as backing datastructure must not invalidate other descriptors.

  boost::graph_traits<MyGraph>::vertex_iterator i, end, center;
  boost::graph_traits<MyGraph>::adjacency_iterator ai, a_end, next;
  std::vector<VertexDesc> toDelete;
  bool found = false;
  for (boost::tie(i, end) = vertices(*graph); i != end; ++i)
  {
    Vertex & vertex = (*graph)[*i];
    if (vertex.id == id)
    {
      found = true;
      center = i;
      // toDelete.push_back(*i);
      break;
    }
  }
  if (found)
  {
    boost::tie(ai, a_end) = adjacent_vertices(*center, *graph);
    for (next = ai; ai != a_end; ai = next)
    {
      ++next;
      toDelete.push_back(*ai);
      // deleteNodeId(graph,*ai);
      // std::cout << "deleting\n";
      // clear_vertex(*ai, *graph);
      // remove_vertex(*ai, *graph);
      // std::cout << "Deleted\n";
    }
    deleteNodeId(graph, *i);
  }
  for (size_t i = 0; i < toDelete.size(); ++i)
    deleteNodeId(graph, toDelete[i]);
}
}
