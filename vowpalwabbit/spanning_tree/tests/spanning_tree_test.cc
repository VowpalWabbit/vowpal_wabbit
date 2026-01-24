// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/spanning_tree/spanning_tree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Test construction with default port
TEST(SpanningTreeTest, ConstructWithDefaultPort)
{
  // Use quiet mode to suppress output
  VW::spanning_tree st(26543, true);

  EXPECT_EQ(st.bound_port(), 26543);
}

// Test construction with port 0 (OS assigns an available port)
TEST(SpanningTreeTest, ConstructWithPortZero)
{
  VW::spanning_tree st(0, true);

  // OS should have assigned a port > 0
  EXPECT_GT(st.bound_port(), 0);
}

// Test that bound_port returns the correct port after construction
TEST(SpanningTreeTest, BoundPortReturnsCorrectValue)
{
  VW::spanning_tree st(0, true);
  uint16_t port = st.bound_port();

  // Should be a valid ephemeral port
  EXPECT_GT(port, 0);
}

// Test quiet mode construction doesn't crash
TEST(SpanningTreeTest, QuietModeConstruction)
{
  VW::spanning_tree st(0, true);  // quiet = true
  EXPECT_GT(st.bound_port(), 0);
}

// Test non-quiet mode construction doesn't crash
TEST(SpanningTreeTest, NonQuietModeConstruction)
{
  VW::spanning_tree st(0, false);  // quiet = false
  EXPECT_GT(st.bound_port(), 0);
}

// Test multiple instances can be created with different ports
TEST(SpanningTreeTest, MultipleInstances)
{
  VW::spanning_tree st1(0, true);
  VW::spanning_tree st2(0, true);

  uint16_t port1 = st1.bound_port();
  uint16_t port2 = st2.bound_port();

  EXPECT_GT(port1, 0);
  EXPECT_GT(port2, 0);
  // Ports should be different since OS assigns them
  EXPECT_NE(port1, port2);
}

// Test that destructor works cleanly without start()
TEST(SpanningTreeTest, DestructorWithoutStart)
{
  {
    VW::spanning_tree st(0, true);
    EXPECT_GT(st.bound_port(), 0);
    // Destructor called here without start()
  }
  // If we get here without crashing, the test passed
  SUCCEED();
}
