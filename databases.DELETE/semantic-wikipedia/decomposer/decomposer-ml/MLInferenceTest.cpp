// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <string>
#include <set>
#include "./MLInference.h"

namespace ad_decompose
{
MLInference inf;

// Test the parseText method.
TEST(MLInferenceTest, solveMyGraph)
{
  MLInference::MyGraph graph;
  MLInference::EdgeDesc edge;
  bool ok;
  MLInference::VertexDesc vIDa = boost::add_vertex(graph);
  graph[vIDa].name = "a";
  graph[vIDa].id = 1;
  graph[vIDa].weight = 1;
  MLInference::VertexDesc vIDb = boost::add_vertex(graph);
  graph[vIDb].name = "b";
  graph[vIDb].id = 2;
  graph[vIDb].weight = 1;
  MLInference::VertexDesc vIDc = boost::add_vertex(graph);
  graph[vIDc].name = "c";
  graph[vIDc].id = 3;
  graph[vIDc].weight = 1;
  MLInference::VertexDesc vIDd = boost::add_vertex(graph);
  graph[vIDd].name = "d";
  graph[vIDd].id = 4;
  graph[vIDd].weight = 1;
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDa, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDd, graph);

  std::set<string> expected1;

  expected1.insert("a");
  expected1.insert("c");
  expected1.insert("d");

  MLInference::MyGraph res = inf.solve(graph);
  boost::graph_traits<MLInference::MyGraph>::vertex_iterator i, end, maxVertex;
  std::cout << "Result: \n";
  std::set<string> actual;

  for (boost::tie(i, end) = vertices(res); i != end; ++i)
  {
    MLInference::Vertex & vertex = res[*i];
    std::cout << vertex.name << "\n";
    actual.insert(vertex.name);
  }
  ASSERT_TRUE(actual == expected1);
}

// Test the parseText method.
TEST(MLInferenceTest, solveKuratowski)
{
  MLInference::MyGraph graph;
  MLInference::EdgeDesc edge;
  bool ok;
  MLInference::VertexDesc vIDa = boost::add_vertex(graph);
  graph[vIDa].name = "a";
  graph[vIDa].id = 1;
  graph[vIDa].weight = 1;
  MLInference::VertexDesc vIDb = boost::add_vertex(graph);
  graph[vIDb].name = "b";
  graph[vIDb].id = 2;
  graph[vIDb].weight = 1;
  MLInference::VertexDesc vIDc = boost::add_vertex(graph);
  graph[vIDc].name = "c";
  graph[vIDc].id = 3;
  graph[vIDc].weight = 1;
  MLInference::VertexDesc vIDd = boost::add_vertex(graph);
  graph[vIDd].name = "d";
  graph[vIDd].id = 4;
  graph[vIDd].weight = 1;
  MLInference::VertexDesc vIDe = boost::add_vertex(graph);
  graph[vIDe].name = "e";
  graph[vIDe].id = 5;
  graph[vIDe].weight = 1;
  MLInference::VertexDesc vIDf = boost::add_vertex(graph);
  graph[vIDf].name = "f";
  graph[vIDf].id = 6;
  graph[vIDf].weight = 2;

  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDf, graph);

  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDf, graph);

  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDf, graph);

  MLInference::MyGraph res = inf.solve(graph);
  boost::graph_traits<MLInference::MyGraph>::vertex_iterator i, end, maxVertex;
  std::set<string> expected1;
  std::set<string> expected2;
  expected1.insert("d");
  expected1.insert("e");
  expected1.insert("f");
  expected2.insert("a");
  expected2.insert("b");
  expected2.insert("c");
  std::set<string> actual;

  std::cout << "Result: \n";
  for (boost::tie(i, end) = vertices(res); i != end; ++i)
  {
    MLInference::Vertex & vertex = res[*i];
    std::cout << vertex.name << "\n";
    actual.insert(vertex.name);
  }
  ASSERT_TRUE(actual == expected1 || actual == expected2);
}

// Test the parseText method.
TEST(MLInferenceTest, solveWheel)
{
  MLInference::MyGraph graph;
  MLInference::EdgeDesc edge;
  bool ok;
  MLInference::VertexDesc vIDa = boost::add_vertex(graph);
  graph[vIDa].name = "a";
  graph[vIDa].id = 1;
  graph[vIDa].weight = 1;
  MLInference::VertexDesc vIDb = boost::add_vertex(graph);
  graph[vIDb].name = "b";
  graph[vIDb].id = 2;
  graph[vIDb].weight = 2;
  MLInference::VertexDesc vIDc = boost::add_vertex(graph);
  graph[vIDc].name = "c";
  graph[vIDc].id = 3;
  graph[vIDc].weight = 1;
  MLInference::VertexDesc vIDd = boost::add_vertex(graph);
  graph[vIDd].name = "d";
  graph[vIDd].id = 4;
  graph[vIDd].weight = 1;
  MLInference::VertexDesc vIDe = boost::add_vertex(graph);
  graph[vIDe].name = "e";
  graph[vIDe].id = 5;
  graph[vIDe].weight = 1;
  MLInference::VertexDesc vIDf = boost::add_vertex(graph);
  graph[vIDf].name = "f";
  graph[vIDf].id = 6;
  graph[vIDf].weight = 1;
  MLInference::VertexDesc vIDg = boost::add_vertex(graph);
  graph[vIDg].name = "g";
  graph[vIDg].id = 7;
  graph[vIDg].weight = 1;
  MLInference::VertexDesc vIDh = boost::add_vertex(graph);
  graph[vIDh].name = "h";
  graph[vIDh].id = 8;
  graph[vIDh].weight = 1;

  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDb, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDc, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDf, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDg, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDh, graph);

  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDc, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDd, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDe, vIDf, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDf, vIDg, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDg, vIDh, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDh, vIDb, graph);

  MLInference::MyGraph res = inf.solve(graph);
  boost::graph_traits<MLInference::MyGraph>::vertex_iterator i, end, maxVertex;
  std::set<string> expected1;
  std::set<string> expected2;
  expected1.insert("b");
  expected1.insert("d");
  expected1.insert("f");
  expected2.insert("b");
  expected2.insert("e");
  expected2.insert("g");
  std::set<string> actual;

  std::cout << "Result: \n";
  for (boost::tie(i, end) = vertices(res); i != end; ++i)
  {
    MLInference::Vertex & vertex = res[*i];
    std::cout << vertex.name << "\n";
    actual.insert(vertex.name);
  }
  ASSERT_TRUE(actual == expected1 || actual == expected2);
}

// Test the parseText method.
TEST(MLInferenceTest, solveCube)
{
  MLInference::MyGraph graph;
  MLInference::EdgeDesc edge;
  bool ok;
  MLInference::VertexDesc vIDa = boost::add_vertex(graph);
  graph[vIDa].name = "a";
  graph[vIDa].id = 1;
  graph[vIDa].weight = 1;
  MLInference::VertexDesc vIDb = boost::add_vertex(graph);
  graph[vIDb].name = "b";
  graph[vIDb].id = 2;
  graph[vIDb].weight = 1;
  MLInference::VertexDesc vIDc = boost::add_vertex(graph);
  graph[vIDc].name = "c";
  graph[vIDc].id = 3;
  graph[vIDc].weight = 1;
  MLInference::VertexDesc vIDd = boost::add_vertex(graph);
  graph[vIDd].name = "d";
  graph[vIDd].id = 4;
  graph[vIDd].weight = 1;
  MLInference::VertexDesc vIDe = boost::add_vertex(graph);
  graph[vIDe].name = "e";
  graph[vIDe].id = 5;
  graph[vIDe].weight = 1;
  MLInference::VertexDesc vIDf = boost::add_vertex(graph);
  graph[vIDf].name = "f";
  graph[vIDf].id = 6;
  graph[vIDf].weight = 1;
  MLInference::VertexDesc vIDg = boost::add_vertex(graph);
  graph[vIDg].name = "g";
  graph[vIDg].id = 7;
  graph[vIDg].weight = 1;
  MLInference::VertexDesc vIDh = boost::add_vertex(graph);
  graph[vIDh].name = "h";
  graph[vIDh].id = 8;
  graph[vIDh].weight = 1;

  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDb, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDc, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDd, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDd, vIDa, graph);

  boost::tie(edge, ok) = boost::add_edge(vIDe, vIDf, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDf, vIDg, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDg, vIDh, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDh, vIDe, graph);

  boost::tie(edge, ok) = boost::add_edge(vIDa, vIDe, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDb, vIDf, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDc, vIDg, graph);
  boost::tie(edge, ok) = boost::add_edge(vIDd, vIDh, graph);

  MLInference::MyGraph res = inf.solve(graph);
  boost::graph_traits<MLInference::MyGraph>::vertex_iterator i, end, maxVertex;
  std::cout << "Result: \n";
  std::set<string> expected1;
  std::set<string> expected2;
  expected1.insert("a");
  expected1.insert("h");
  expected1.insert("f");
  expected1.insert("c");
  expected2.insert("b");
  expected2.insert("d");
  expected2.insert("e");
  expected2.insert("g");
  std::set<string> actual;
  for (boost::tie(i, end) = vertices(res); i != end; ++i)
  {
    MLInference::Vertex & vertex = res[*i];
    std::cout << vertex.name << "\n";
    actual.insert(vertex.name);
  }
  ASSERT_TRUE(actual == expected1 || actual == expected2);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
