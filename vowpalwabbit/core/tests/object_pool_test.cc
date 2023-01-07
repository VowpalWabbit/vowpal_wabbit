// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/object_pool.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

class obj
{
public:
  int i;
};

class obj_initializer
{
public:
  obj* operator()(obj* o) { return o; }
};

TEST(ObjectPool, ObjectPoolTest)
{
  {
    VW::object_pool<obj> pool_with_size{50};
    EXPECT_EQ(pool_with_size.size(), 50);
  }

  {
    VW::object_pool<obj, obj_initializer> pool_with_small_chunks{0, obj_initializer{}, 2};
    EXPECT_EQ(pool_with_small_chunks.size(), 0);

    auto o1 = pool_with_small_chunks.get_object();
    EXPECT_EQ(pool_with_small_chunks.size(), 2);

    auto o2 = pool_with_small_chunks.get_object();
    EXPECT_EQ(pool_with_small_chunks.size(), 2);

    auto o3 = pool_with_small_chunks.get_object();
    EXPECT_EQ(pool_with_small_chunks.size(), 4);

    pool_with_small_chunks.return_object(o1);
    pool_with_small_chunks.return_object(o2);
    pool_with_small_chunks.return_object(o3);
  }

  VW::object_pool<obj, obj_initializer> pool{0, obj_initializer{}, 1};
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.empty(), true);

  auto o1 = pool.get_object();
  EXPECT_EQ(pool.size(), 1);
  EXPECT_EQ(pool.empty(), true);

  auto o2 = pool.get_object();
  EXPECT_EQ(pool.size(), 2);
  EXPECT_EQ(pool.empty(), true);

  pool.return_object(o1);
  EXPECT_EQ(pool.size(), 2);
  EXPECT_EQ(pool.empty(), false);

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  obj other_obj;
  EXPECT_EQ(pool.is_from_pool(o2), true);
  EXPECT_EQ(pool.is_from_pool(&other_obj), false);
  VW_WARNING_STATE_POP

  pool.return_object(o2);
}
