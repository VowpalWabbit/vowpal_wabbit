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

TEST(ObjectPool, ObjectPoolTest)
{
  {
    VW::object_pool<obj> pool_with_size{50};
    EXPECT_EQ(pool_with_size.size(), 50);
  }

  VW::object_pool<obj> pool{0};
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.empty(), true);

  auto o1 = pool.get_object();
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.empty(), true);

  auto o2 = pool.get_object();
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.empty(), true);

  pool.return_object(std::move(o1));
  EXPECT_EQ(pool.size(), 1);
  EXPECT_EQ(pool.empty(), false);

  VW_WARNING_STATE_PUSH
  VW_WARNING_DISABLE_DEPRECATED_USAGE
  obj other_obj;
  EXPECT_EQ(pool.is_from_pool(o2.get()), true);
  EXPECT_EQ(pool.is_from_pool(&other_obj), false);
  VW_WARNING_STATE_POP

  pool.return_object(std::move(o2));
}
