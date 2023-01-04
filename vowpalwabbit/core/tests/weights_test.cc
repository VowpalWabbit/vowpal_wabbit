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