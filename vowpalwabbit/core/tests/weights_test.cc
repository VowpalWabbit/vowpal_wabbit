// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/array_parameters.h"
#include "vw/core/array_parameters_dense.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

constexpr auto LENGTH = 16;
constexpr auto STRIDE_SHIFT = 2;

template <typename T>
class WeightTests : public ::testing::Test
{
};

using weight_types = ::testing::Types<VW::sparse_parameters, VW::dense_parameters>;
TYPED_TEST_SUITE(WeightTests, weight_types, );
TYPED_TEST(WeightTests, TestDefaultFunctionWeightInitializationStridedIndex)
{
  TypeParam w(LENGTH, STRIDE_SHIFT);
  auto weight_initializer = [](VW::weight* weights, uint64_t index) { weights[0] = 1.f * index; };
  w.set_default(weight_initializer);
  for (size_t i = 0; i < LENGTH; i++) { EXPECT_FLOAT_EQ(w.strided_index(i), 1.f * (i * w.stride())); }
}

TEST(DenseParametersTest, UninitializedIteratorIsEmptyRange)
{
  // Default constructor leaves _begin as nullptr
  VW::dense_parameters params;

  // Should report as not initialized
  EXPECT_FALSE(params.not_null());

  // Iterators should form an empty range (not crash)
  EXPECT_EQ(params.begin(), params.end());
  EXPECT_EQ(params.cbegin(), params.cend());

  // Iterating should do nothing (not crash)
  int count = 0;
  for (auto it = params.begin(); it != params.end(); ++it) { count++; }
  EXPECT_EQ(count, 0);

  // Const iteration should also work
  count = 0;
  for (auto it = params.cbegin(); it != params.cend(); ++it) { count++; }
  EXPECT_EQ(count, 0);
}