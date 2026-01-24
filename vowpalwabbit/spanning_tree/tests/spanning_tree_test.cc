// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/spanning_tree/spanning_tree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <vector>

using namespace VW::spanning_tree_internal;

// Helper function to verify tree structure
// Returns true if all nodes have valid parent pointers and the tree is connected
bool verify_tree_structure(int* parent, uint16_t* kid_count, size_t total, int root)
{
  // Root should have no parent
  if (parent[root] != -1) { return false; }

  // Count actual children for each node
  std::vector<int> actual_kid_count(total, 0);
  for (size_t i = 0; i < total; i++)
  {
    if (parent[i] != -1)
    {
      if (parent[i] < 0 || static_cast<size_t>(parent[i]) >= total) { return false; }
      actual_kid_count[parent[i]]++;
    }
  }

  // Verify kid_count matches actual
  for (size_t i = 0; i < total; i++)
  {
    if (kid_count[i] != actual_kid_count[i]) { return false; }
  }

  return true;
}

TEST(SpanningTreeTests, BuildTreeSingleNode)
{
  int parent[1];
  uint16_t kid_count[1];

  int root = build_tree(parent, kid_count, 1, 0);

  EXPECT_EQ(root, 0);
  EXPECT_EQ(kid_count[0], 0);  // Single node has no children
}

TEST(SpanningTreeTests, BuildTreeTwoNodes)
{
  int parent[2];
  uint16_t kid_count[2];

  int root = build_tree(parent, kid_count, 2, 0);
  parent[root] = -1;  // Set root's parent to -1

  // For 2 nodes: height = floor(log2(2)) = 1, root = 2^1 - 1 = 1
  // left_count = 1, so node 0 is the left child of node 1
  EXPECT_EQ(root, 1);
  EXPECT_EQ(kid_count[1], 1);  // Root has 1 child
  EXPECT_EQ(parent[0], 1);     // Node 0's parent is root (node 1)
  EXPECT_TRUE(verify_tree_structure(parent, kid_count, 2, root));
}

TEST(SpanningTreeTests, BuildTreeThreeNodes)
{
  int parent[3];
  uint16_t kid_count[3];

  int root = build_tree(parent, kid_count, 3, 0);
  parent[root] = -1;

  // For 3 nodes: height = floor(log2(3)) = 1, so root index = 2^1 - 1 = 0
  // Left subtree has 1 node at offset 0
  // Root is at position 0 + 0 = 0... wait that's wrong
  // Actually: height = 1, root_rel = 2^1 - 1 = 0, left_count = 0
  // Let me recalculate: log(3)/log(2) = 1.58, floor = 1
  // root = 2^1 - 1 = 1, left_count = 1
  // So root is at position 1

  EXPECT_EQ(root, 1);
  EXPECT_TRUE(verify_tree_structure(parent, kid_count, 3, root));
}

TEST(SpanningTreeTests, BuildTreeFourNodes)
{
  int parent[4];
  uint16_t kid_count[4];

  int root = build_tree(parent, kid_count, 4, 0);
  parent[root] = -1;

  // For 4 nodes: height = floor(log2(4)) = 2, root = 2^2 - 1 = 3
  // But wait, 4 nodes means left subtree of 3 nodes...
  // Let's just verify the structure is valid
  EXPECT_TRUE(verify_tree_structure(parent, kid_count, 4, root));
  EXPECT_GE(root, 0);
  EXPECT_LT(root, 4);
}

TEST(SpanningTreeTests, BuildTreeSevenNodes)
{
  int parent[7];
  uint16_t kid_count[7];

  int root = build_tree(parent, kid_count, 7, 0);
  parent[root] = -1;

  // 7 nodes is a perfect binary tree (2^3 - 1 = 7)
  // height = floor(log2(7)) = 2, root = 2^2 - 1 = 3
  EXPECT_EQ(root, 3);
  EXPECT_TRUE(verify_tree_structure(parent, kid_count, 7, root));

  // Root should have 2 children
  EXPECT_EQ(kid_count[3], 2);
}

TEST(SpanningTreeTests, BuildTreeEightNodes)
{
  int parent[8];
  uint16_t kid_count[8];

  int root = build_tree(parent, kid_count, 8, 0);
  parent[root] = -1;

  EXPECT_TRUE(verify_tree_structure(parent, kid_count, 8, root));
}

TEST(SpanningTreeTests, BuildTreeWithOffset)
{
  // Test that offset works correctly
  const size_t total = 5;
  const int offset = 10;
  std::vector<int> parent(total + offset, -999);
  std::vector<uint16_t> kid_count(total + offset, 999);

  int root = build_tree(parent.data(), kid_count.data(), total, offset);
  parent[root] = -1;

  // Root should be >= offset
  EXPECT_GE(root, offset);
  EXPECT_LT(root, static_cast<int>(total + offset));

  // Verify structure considering offset
  for (size_t i = offset; i < total + offset; i++)
  {
    if (static_cast<int>(i) == root)
    {
      EXPECT_EQ(parent[i], -1);
    }
    else
    {
      EXPECT_GE(parent[i], offset);
      EXPECT_LT(parent[i], static_cast<int>(total + offset));
    }
  }
}

TEST(SpanningTreeTests, BuildTreeLargeNodeCount)
{
  // Test with 100 nodes to ensure algorithm works at scale
  const size_t total = 100;
  std::vector<int> parent(total);
  std::vector<uint16_t> kid_count(total);

  int root = build_tree(parent.data(), kid_count.data(), total, 0);
  parent[root] = -1;

  EXPECT_TRUE(verify_tree_structure(parent.data(), kid_count.data(), total, root));

  // Count total children to verify tree connectivity
  size_t total_children = 0;
  for (size_t i = 0; i < total; i++) { total_children += kid_count[i]; }
  // In a tree with n nodes, there are n-1 edges (parent-child relationships)
  EXPECT_EQ(total_children, total - 1);
}

TEST(SpanningTreeTests, BuildTreeAllNodesReachable)
{
  // Verify all nodes can reach the root
  const size_t total = 15;
  std::vector<int> parent(total);
  std::vector<uint16_t> kid_count(total);

  int root = build_tree(parent.data(), kid_count.data(), total, 0);
  parent[root] = -1;

  for (size_t i = 0; i < total; i++)
  {
    int current = static_cast<int>(i);
    int steps = 0;
    while (current != root && steps < static_cast<int>(total))
    {
      current = parent[current];
      steps++;
    }
    EXPECT_EQ(current, root) << "Node " << i << " cannot reach root";
    EXPECT_LT(steps, static_cast<int>(total)) << "Cycle detected from node " << i;
  }
}
