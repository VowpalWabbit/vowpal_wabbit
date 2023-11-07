// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/scope_exit.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ScopeExit, ExecuteOnScopeEnd)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
  }
  EXPECT_EQ(calls, 1);
}

TEST(ScopeExit, Cancel)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
    guard.cancel();
  }
  EXPECT_EQ(calls, 0);
}

TEST(ScopeExit, ExplicitCall)
{
  int calls = 0;
  {
    auto guard = VW::scope_exit([&calls]() { calls++; });
    EXPECT_EQ(calls, 0);

    guard.call();
    EXPECT_EQ(calls, 1);
  }
  EXPECT_EQ(calls, 1);
}