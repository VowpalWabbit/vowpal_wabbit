// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/guard.h"

#include "vw/core/memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class non_copyable_class
{
public:
  int value;
  explicit non_copyable_class(int value) : value(value) {}

  non_copyable_class(const non_copyable_class&) = delete;
  non_copyable_class& operator=(const non_copyable_class&) = delete;
  non_copyable_class(non_copyable_class&& other) = default;
  non_copyable_class& operator=(non_copyable_class&& other) = default;
};

TEST(Guard, SwapGuardExecuteOnScopeEnd)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    EXPECT_EQ(original_location, 1);
    EXPECT_EQ(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    EXPECT_EQ(original_location, 99999);
    EXPECT_EQ(location_to_swap, 1);
  }
  EXPECT_EQ(original_location, 1);
  EXPECT_EQ(location_to_swap, 99999);
}

TEST(Guard, SwapGuardExecuteOnScopeEndNoCopy)
{
  non_copyable_class original_location(1);
  non_copyable_class location_to_swap(99999);

  {
    EXPECT_EQ(original_location.value, 1);
    EXPECT_EQ(location_to_swap.value, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    EXPECT_EQ(original_location.value, 99999);
    EXPECT_EQ(location_to_swap.value, 1);
  }
  EXPECT_EQ(original_location.value, 1);
  EXPECT_EQ(location_to_swap.value, 99999);
}

TEST(Guard, SwapGuardCancel)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    EXPECT_EQ(original_location, 1);
    EXPECT_EQ(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    EXPECT_EQ(original_location, 99999);
    EXPECT_EQ(location_to_swap, 1);
    guard.cancel();
  }
  EXPECT_EQ(original_location, 99999);
  EXPECT_EQ(location_to_swap, 1);
}

TEST(Guard, SwapGuardExplicitForceSwap)
{
  int original_location = 1;
  int location_to_swap = 99999;

  {
    EXPECT_EQ(original_location, 1);
    EXPECT_EQ(location_to_swap, 99999);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    EXPECT_EQ(original_location, 99999);
    EXPECT_EQ(location_to_swap, 1);
    EXPECT_EQ(guard.do_swap(), true);
    EXPECT_EQ(original_location, 1);
    EXPECT_EQ(location_to_swap, 99999);
    EXPECT_EQ(guard.do_swap(), false);
    EXPECT_EQ(original_location, 1);
    EXPECT_EQ(location_to_swap, 99999);
  }
  EXPECT_EQ(original_location, 1);
  EXPECT_EQ(location_to_swap, 99999);
}

TEST(Guard, SwapGuardExecuteOnScopeEndSwapPointers)
{
  int* original_location = new int;
  *original_location = 1;
  int* location_to_swap = new int;
  *location_to_swap = 99999;

  int* original_location_pre_swap = original_location;
  int* location_to_swap_pre_swap = location_to_swap;

  {
    EXPECT_EQ(*original_location, 1);
    EXPECT_EQ(*location_to_swap, 99999);
    EXPECT_EQ(original_location, original_location_pre_swap);
    EXPECT_EQ(location_to_swap, location_to_swap_pre_swap);
    auto guard = VW::swap_guard(original_location, location_to_swap);
    EXPECT_EQ(*original_location, 99999);
    EXPECT_EQ(*location_to_swap, 1);
    EXPECT_EQ(original_location, location_to_swap_pre_swap);
    EXPECT_EQ(location_to_swap, original_location_pre_swap);
  }
  EXPECT_EQ(*original_location, 1);
  EXPECT_EQ(*location_to_swap, 99999);
  EXPECT_EQ(original_location, original_location_pre_swap);
  EXPECT_EQ(location_to_swap, location_to_swap_pre_swap);

  delete original_location;
  delete location_to_swap;
}

TEST(Guard, SwapGuardExecuteTempValue)
{
  int original_location = 1;

  {
    EXPECT_EQ(original_location, 1);
    auto guard = VW::swap_guard(original_location, 9999);
    EXPECT_EQ(original_location, 9999);
  }
  EXPECT_EQ(original_location, 1);
}

TEST(Guard, SwapGuardExecuteTempValueNoCopy)
{
  non_copyable_class original_location(1);

  {
    EXPECT_EQ(original_location.value, 1);
    auto guard = VW::swap_guard(original_location, non_copyable_class(9999));
    EXPECT_EQ(original_location.value, 9999);
  }
  EXPECT_EQ(original_location.value, 1);
}

TEST(Guard, SwapGuardUniquePtr)
{
  std::unique_ptr<int> original_location = VW::make_unique<int>(1);

  {
    std::unique_ptr<int> inner_location = VW::make_unique<int>(9999);
    EXPECT_EQ(*inner_location, 9999);
    EXPECT_EQ(*original_location, 1);
    auto guard = VW::swap_guard(original_location, inner_location);
    EXPECT_EQ(*inner_location, 1);
    EXPECT_EQ(*original_location, 9999);
  }
  EXPECT_EQ(*original_location, 1);
}

TEST(Guard, StashGuardExecuteOnScopeEnd)
{
  int target_location = 999;
  {
    EXPECT_EQ(target_location, 999);
    auto guard = VW::stash_guard(target_location);
    EXPECT_EQ(target_location, 0);
  }
  EXPECT_EQ(target_location, 999);
}

class struct_with_non_trivial_ctor
{
public:
  int value;
  struct_with_non_trivial_ctor() : value(123) {}
};

TEST(Guard, StashGuardUsedDefaultCtor)
{
  struct_with_non_trivial_ctor target_location;
  EXPECT_EQ(target_location.value, 123);

  target_location.value = 456;
  {
    EXPECT_EQ(target_location.value, 456);
    auto guard = VW::stash_guard(target_location);
    EXPECT_EQ(target_location.value, 123);
  }
  EXPECT_EQ(target_location.value, 456);
}
