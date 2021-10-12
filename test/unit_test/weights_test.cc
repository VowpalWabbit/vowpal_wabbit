// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/mpl/vector.hpp>

#include "array_parameters.h"
#include "array_parameters_dense.h"

#include "test_common.h"

constexpr auto LENGTH = 16;
constexpr auto STRIDE_SHIFT = 2;

using weight_types = boost::mpl::vector<sparse_parameters, dense_parameters>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_default_function_weight_initialization_strided_index, T, weight_types)
{
  T w(LENGTH, STRIDE_SHIFT);
  auto weight_initializer = [](weight* weights, uint64_t index) { weights[0] = 1.f * index; };
  w.set_default(weight_initializer);
  for(size_t i = 0; i < LENGTH; i++)
  {
    BOOST_CHECK_CLOSE(w.strided_index(i), 1.f * (i * w.stride()) , FLOAT_TOL);
  }
}

