// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>

#include <gtest/gtest.h>
#include <vector>
#include "./Graph.h"

// _____________________________________________________________________________
TEST(GraphTest, constructor)
{
  Graph graph;
  ASSERT_EQ("{0, 0}", graph.asString());
}

// _____________________________________________________________________________
TEST(GraphTest, addVertex)
{
  Graph graph;
  graph.addVertex();
  ASSERT_EQ("{1, 0}", graph.asString());
  graph.addVertex();
  ASSERT_EQ("{2, 0}", graph.asString());
  graph.addVertex();
  ASSERT_EQ("{3, 0}", graph.asString());
}

// _____________________________________________________________________________
TEST(GraphTest, addEdge)
{
  Graph graph;
  graph.addVertex();
  graph.addVertex();
  graph.addVertex();
  graph.addEdge(0, 2);
  ASSERT_EQ("{3, 2, (0,2), (2,0)}", graph.asString());
  graph.addEdge(1, 2);
  ASSERT_EQ("{3, 4, (0,2), (1,2), (2,0), (2,1)}", graph.asString());
}

// _____________________________________________________________________________
TEST(GraphTest, enumeratePaths)
{
  Graph graph;
  graph.addVertex();
  graph.addVertex();
  graph.addVertex();
  graph.addEdge(0, 2);
  ASSERT_EQ("{3, 2, (0,2), (2,0)}", graph.asString());
  graph.addEdge(1, 2);
  ASSERT_EQ("{3, 4, (0,2), (1,2), (2,0), (2,1)}", graph.asString());
  graph.addVertex();
  graph.addEdge(1, 3);
  graph.addEdge(0, 3);
  graph.addVertex();
  std::vector<std::vector<size_t> > result;
  ad_utility::HashSet<size_t> vis;
  graph.enumeratePathsBetween(0, 3, &vis, &result);
  for (size_t i = 0; i < result.size(); ++i)
  {
    std::cout << "Path " << i << ": ";
    for (size_t j = 0; j < result[i].size(); ++j)
    {
      std::cout << result[i][j] << " ";
    }
    std::cout << std::endl;
  }
  ASSERT_EQ(size_t(2), result.size());
}
