// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
using namespace ::testing;

#include "vw/test_common/test_common.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/vw_math.h"
#include "vw/core/merge.h"

#include "vw/core/vw.h"
#include "vw/core/parser.h"

#include <gtest/gtest.h>

#include <map>
#include <iostream>

using example_vector = std::vector<std::vector<std::string>>;
using ftrl_weights_vector = std::vector<std::tuple<float, float, float, float, float, float>>;
using separate_weights_vector = std::vector<std::tuple<size_t, float, float, float, float, float, float>>;

example_vector sl_vector = {
  {
    "1 1.25 |c id=2 |a id=5 |v v=none",
    "-1 0.9375 |c id=2 |a id=1 |v v=none",
    "-1 0.9375 |c id=2 |a id=2 |v v=none",
    "-1 0.9375 |c id=2 |a id=4 |v v=none",
    "-1 0.9375 |c id=2 |a id=0 |v v=none"
  },
  {
    "-1 0.9375 |c id=0 |a id=0 |v v=none",
    "1 1.25 |c id=0 |a id=1 |v v=none",
    "-1 0.9375 |c id=0 |a id=3 |v v=none",
    "-1 0.9375 |c id=0 |a id=2 |v v=none",
    "-1 0.9375 |c id=0 |a id=4 |v v=none"
  },
  {
    "-1 0.9375 |c id=3 |a id=1 |v v=none",
    "-1 0.9375 |c id=3 |a id=4 |v v=none",
    "1 1.25 |c id=3 |a id=5 |v v=none",
    "-1 0.9375 |c id=3 |a id=2 |v v=none",
    "-1 0.9375 |c id=3 |a id=0 |v v=none"
  },
  {
    "-1 0.9375 |c id=0 |a id=5 |v v=none",
    "-1 0.9375 |c id=0 |a id=4 |v v=none",
    "1 1.25 |c id=0 |a id=1 |v v=none",
    "-1 0.9375 |c id=0 |a id=3 |v v=none",
    "-1 0.9375 |c id=0 |a id=2 |v v=none"
  },
  {
    "-1 0.9375 |c id=4 |a id=0 |v v=none",
    "-1 0.9375 |c id=4 |a id=1 |v v=none",
    "-1 0.9375 |c id=4 |a id=2 |v v=none",
    "1 1.25 |c id=4 |a id=3 |v v=none",
    "-1 0.9375 |c id=4 |a id=4 |v v=none"
  }
};

// , "DefinitelyBad": false
// , "DefinitelyBad": false
// , "DefinitelyBad": false
// , "DefinitelyBad": false
// , "DefinitelyBad": false

std::vector<std::string> multi_vector = {
  R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v=none": 1}}], "a": [5, 1, 2, 4, 0], "c": {"c": {"id=2": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2]})",
  R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v=none": 1}}], "a": [0, 1, 3, 2, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2]})",
  R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=none": 1}}], "a": [1, 4, 5, 2, 0], "c": {"c": {"id=3": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2]})",
  R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=none": 1}}], "a": [5, 4, 1, 3, 2], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2]})",
  R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}}], "a": [0, 1, 2, 3, 4], "c": {"c": {"id=4": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2]})"
};

separate_weights_vector get_separate_weights(std::unique_ptr<VW::workspace> vw) {
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();
  auto end = weights.end();

  separate_weights_vector weights_vector;

  while(iter < end) {
    bool non_zero = false;
    for (int i=0; i < 6; i++) {
      if (*iter[i] != 0.f) {
        non_zero = true;
      }
    }

    if (non_zero) {
      weights_vector.emplace_back(iter.index_without_stride(), *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
    }
    ++iter;
  }

  // print_separate_weights(weights_vector);
  return weights_vector;
}

std::vector<separate_weights_vector> split_weights(std::unique_ptr<VW::workspace> vw) {
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();

  auto end = weights.end();

  separate_weights_vector sl_weights_vector;
  separate_weights_vector multi_weights_vector;
  std::vector<separate_weights_vector> result;

  while (iter < end) {
    bool non_zero = false;
    for (int i=0; i < 6; i++) {
      if (*iter[i] != 0.f) {
        non_zero = true;
      }
    }
    if (non_zero) {
      if ((iter.index_without_stride() & (2 - 1)) == 0) {
        // first model
        sl_weights_vector.emplace_back(iter.index_without_stride()>>1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      } else {
        // second model
        multi_weights_vector.emplace_back(iter.index_without_stride()>>1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      }
    }

    ++iter;
  }

  // print_separate_weights(sl_weights_vector);

  // print_separate_weights(multi_weights_vector);

  result.emplace_back(sl_weights_vector);
  result.emplace_back(multi_weights_vector);

  return result;
}

TEST(igl_test, igl_model_weights_are_equal_to_train_models_separately) {
  // two vw instance
  auto sl_vw = VW::initialize(vwtest::make_args("--link=logistic", "--loss_function=logistic", "--coin", "--cubic", "cav", "--noconstant", "--quiet"));
  auto multi_vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--coin", "--noconstant", "--dsjson", "-q", "ca", "--quiet"));

  // IGL instance
  auto igl_vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--coin", "--experimental_igl", "--noconstant", "--dsjson", "-b", "19", "-q", "ca", "--quiet"));

  // train separately
  for (size_t i = 0; i < multi_vector.size(); i++) {
    auto sl_examples = sl_vector[i];
    for (auto& ex_str : sl_examples) {
      VW::example* ex = VW::read_example(*sl_vw, ex_str);
      sl_vw->learn(*ex);
      VW::finish_example(*sl_vw, *ex);
    }

    auto multi_ex = multi_vector[i];
    auto examples = vwtest::parse_dsjson(*multi_vw, multi_ex);
    VW::setup_examples(*multi_vw, examples);
    multi_vw->learn(examples);
    VW::finish_example(*multi_vw, examples);
  }

  // train IGL
  for (size_t i = 0; i < multi_vector.size(); i++) {
    std::string json_text = multi_vector[i];
    auto examples = vwtest::parse_dsjson(*igl_vw, json_text);
    VW::setup_examples(*igl_vw, examples);

    igl_vw->learn(examples);
    VW::finish_example(*igl_vw, examples);
  }

  // separate weights
  separate_weights_vector sl_weights = get_separate_weights(std::move(sl_vw));
  separate_weights_vector multi_weights = get_separate_weights(std::move(multi_vw));

  // split IGL weights
  std::vector<separate_weights_vector> igl_weights = split_weights(std::move(igl_vw));

  EXPECT_GT(sl_weights.size(), 0);
  for (size_t i = 0; i < sl_weights.size(); i++) {
    EXPECT_EQ(std::get<0>(sl_weights[i]), std::get<0>(igl_weights[0][i])); // feature hash
    EXPECT_NEAR(std::get<1>(sl_weights[i]), std::get<1>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
    EXPECT_NEAR(std::get<2>(sl_weights[i]), std::get<2>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
    EXPECT_NEAR(std::get<3>(sl_weights[i]), std::get<3>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
    EXPECT_NEAR(std::get<4>(sl_weights[i]), std::get<4>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
    EXPECT_NEAR(std::get<5>(sl_weights[i]), std::get<5>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
    EXPECT_NEAR(std::get<6>(sl_weights[i]), std::get<6>(igl_weights[0][i]), vwtest::EXPLICIT_FLOAT_TOL);
  }

  EXPECT_GT(multi_weights.size(), 0);
  EXPECT_EQ(multi_weights, igl_weights[1]);
}