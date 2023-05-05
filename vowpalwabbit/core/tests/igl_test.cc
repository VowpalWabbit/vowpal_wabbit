// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
using namespace ::testing;

#include "igl_simulator.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/core/vw_math.h"
#include "vw/test_common/test_common.h"

#include <iostream>
#include <map>

using example_vector = std::vector<std::vector<std::string>>;
using separate_weights_vector = std::vector<std::tuple<size_t, float, float, float, float, float, float>>;

separate_weights_vector get_separate_weights(std::unique_ptr<VW::workspace> vw)
{
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();
  auto end = weights.end();

  separate_weights_vector weights_vector;

  while (iter < end)
  {
    bool non_zero = false;
    for (size_t i = 0; i < 6; i++)
    {
      if (*iter[i] != 0.f) { non_zero = true; }
    }

    if (non_zero)
    {
      weights_vector.emplace_back(
          iter.index_without_stride(), *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
    }
    ++iter;
  }

  return weights_vector;
}

std::vector<separate_weights_vector> split_weights(std::unique_ptr<VW::workspace> vw)
{
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();

  auto end = weights.end();

  separate_weights_vector sl_weights_vector;
  separate_weights_vector multi_weights_vector;
  std::vector<separate_weights_vector> result;

  while (iter < end)
  {
    bool non_zero = false;
    for (int i = 0; i < 6; i++)
    {
      if (*iter[i] != 0.f) { non_zero = true; }
    }
    if (non_zero)
    {
      if ((iter.index_without_stride() & (2 - 1)) == 0)
      {
        sl_weights_vector.emplace_back(
            iter.index_without_stride() >> 1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      }
      else
      {
        multi_weights_vector.emplace_back(
            iter.index_without_stride() >> 1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      }
    }

    ++iter;
  }

  result.emplace_back(sl_weights_vector);
  result.emplace_back(multi_weights_vector);

  return result;
}

TEST(Igl, ModelWeightsEqualToSeparateModelWeights)
{
  example_vector sl_vector = {
      {
          "1 1.0 |User user=Tom time=afternoon |Action action=politics |v v=click ",
          "-1 1.0 |User user=Tom time=afternoon |Action action=sports |v v=click ",
          "-1 1.0 |User user=Tom time=afternoon |Action action=music |v v=click ",
          "-1 1.0 |User user=Tom time=afternoon |Action action=food |v v=click ",
      },
      {
          "-1 1.0 |User user=Anna time=morning |Action action=politics |v v=none ",
          "-1 1.0 |User user=Anna time=morning |Action action=sports |v v=none ",
          "-1 1.0 |User user=Anna time=morning |Action action=music |v v=none ",
          "1 1.0 |User user=Anna time=morning |Action action=food |v v=none ",
      },
      {
          "-1 1.0 |User user=Anna time=morning |Action action=politics |v v=skip ",
          "-1 1.0 |User user=Anna time=morning |Action action=sports |v v=skip ",
          "1 1.0 |User user=Anna time=morning |Action action=music |v v=skip ",
          "-1 1.0 |User user=Anna time=morning |Action action=food |v v=skip ",
      },
      {
          "-1 1.0 |User user=Tom time=afternoon |Action action=politics |v v=none ",
          "-1 1.0 |User user=Tom time=afternoon |Action action=sports |v v=none ",
          "-1 1.0 |User user=Tom time=afternoon |Action action=music |v v=none ",
          "1 1.0 |User user=Tom time=afternoon |Action action=food |v v=none ",
      },
  };

  std::vector<std::string> multi_vector = {
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v=click": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time=afternoon" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time=morning" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=skip": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time=morning" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time=afternoon" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
  };

  std::vector<std::string> igl_vector = {
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v=click": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time=afternoon" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25]})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time=morning" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25]})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=skip": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time=morning" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25]})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time=afternoon" : 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25]})",
  };

  // two vw instance
  auto sl_vw = VW::initialize(vwtest::make_args(
      "--link=logistic", "--loss_function=logistic", "--coin", "--cubic", "cav", "--noconstant", "--quiet"));
  auto multi_vw = VW::initialize(
      vwtest::make_args("--cb_explore_adf", "--coin", "--noconstant", "--dsjson", "-q", "ca", "--quiet"));

  // IGL instance
  auto igl_vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--coin", "--experimental_igl", "--noconstant",
      "--dsjson", "-b", "19", "-q", "ca", "--quiet"));

  // train separately
  for (size_t i = 0; i < sl_vector.size(); i++)
  {
    auto sl_examples = sl_vector[i];
    for (auto& ex_str : sl_examples)
    {
      VW::example* ex = VW::read_example(*sl_vw, ex_str);
      sl_vw->learn(*ex);
      sl_vw->finish_example(*ex);
    }

    auto multi_ex = multi_vector[i];
    auto examples = vwtest::parse_dsjson(*multi_vw, multi_ex);
    VW::setup_examples(*multi_vw, examples);
    multi_vw->learn(examples);
    multi_vw->finish_example(examples);
  }

  // train IGL
  for (auto& ex : igl_vector)
  {
    auto examples = vwtest::parse_dsjson(*igl_vw, ex);
    VW::setup_examples(*igl_vw, examples);

    igl_vw->learn(examples);
    igl_vw->finish_example(examples);
  }

  separate_weights_vector sl_weights = get_separate_weights(std::move(sl_vw));
  separate_weights_vector multi_weights = get_separate_weights(std::move(multi_vw));

  std::vector<separate_weights_vector> igl_weights = split_weights(std::move(igl_vw));

  EXPECT_GT(sl_weights.size(), 0);
  for (size_t i = 0; i < sl_weights.size(); i++)
  {
    EXPECT_EQ(std::get<0>(sl_weights[i]), std::get<0>(igl_weights[0][i]));  // feature hash
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

TEST(Igl, VerifyRewardModelWeightsWithLabelAndWeight)
{
  std::string sl_ex_str = "1 0.25 |User user=Anna time_of_day=afternoon |Action action=politics |v v=none";
  auto sl_vw = VW::initialize(vwtest::make_args(
      "--link=logistic", "--loss_function=logistic", "--coin", "--cubic", "UAv", "--noconstant", "--quiet"));

  VW::example* sl_ex = VW::read_example(*sl_vw, sl_ex_str);
  sl_vw->learn(*sl_ex);
  sl_vw->finish_example(*sl_ex);

  std::string igl_ex_str =
      R"({"_label_cost": 0, "_label_probability": 1.0, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0], "c": {"User": {"user=Anna": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}]}, "p": [1.0]})";
  auto igl_vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--coin", "--experimental_igl", "--noconstant",
      "--dsjson", "-b", "19", "-q", "UA", "--quiet"));

  auto igl_ex = vwtest::parse_dsjson(*igl_vw, igl_ex_str);
  VW::setup_examples(*igl_vw, igl_ex);
  igl_vw->learn(igl_ex);
  igl_vw->finish_example(igl_ex);

  separate_weights_vector sl_weights = get_separate_weights(std::move(sl_vw));
  std::vector<separate_weights_vector> igl_weights = split_weights(std::move(igl_vw));

  EXPECT_GT(sl_weights.size(), 0);
  EXPECT_EQ(sl_weights, igl_weights[0]);
}

TEST(Igl, TrainingConverges)
{
  std::vector<std::string> igl_args = {"--cb_explore_adf", "--epsilon", "0.2", "--dsjson", "--coin",
      "--experimental_igl", "--noconstant", "-b", "19", "-q", "UA", "--quiet"};

  const size_t num_iterations = 2500;
  const size_t seed = 378123;

  auto igl_vw = VW::initialize(vwtest::make_args(igl_args));

  igl_simulator::igl_sim sim(seed);
  auto ctr_vector = sim.run_simulation(igl_vw.get(), 1, num_iterations);

  EXPECT_GT(ctr_vector.back(), 0.6f);
}

TEST(Igl, SaveResume)
{
  std::vector<std::string> igl_args = {"--cb_explore_adf", "--epsilon", "0.2", "--dsjson", "--coin",
      "--experimental_igl", "--noconstant", "-b", "19", "-q", "UA", "--quiet"};

  const size_t num_iterations = 500;
  const size_t seed = 777;

  auto igl_no_save = VW::initialize(vwtest::make_args(igl_args));

  igl_simulator::igl_sim sim(seed);
  auto ctr_vector = sim.run_simulation(igl_no_save.get(), 1, num_iterations);

  // save_resume
  const size_t split = num_iterations / 2;
  auto igl_first = VW::initialize(VW::make_unique<VW::config::options_cli>(igl_args));
  igl_simulator::igl_sim sim_split(seed);
  sim_split.run_simulation(igl_first.get(), 1, split);

  auto backing_vector = std::make_shared<std::vector<char>>();
  {
    VW::io_buf io_writer;
    io_writer.add_file(VW::io::create_vector_writer(backing_vector));
    VW::save_predictor(*igl_first, io_writer);
    io_writer.flush();
  }

  igl_first->finish();
  igl_first.reset();

  auto igl_second = VW::initialize(VW::make_unique<VW::config::options_cli>(igl_args),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  // continue
  auto ctr_save_resume = sim_split.run_simulation(igl_second.get(), split + 1, split);
  igl_second->finish();

  EXPECT_EQ(ctr_vector, ctr_save_resume);
}

TEST(Igl, VerifyPredictOnlyModelEqualsToCbModel)
{
  // clang-format off
  std::vector<std::string> igl_vector = {
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time_of_day=morning": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=click": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=like": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": -0.5})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v=skip": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time_of_day=morning": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
      R"({"_label_cost": -1, "_label_probability": 0.8500000016763806, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=click": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Tom": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.04999999944120647, 0.04999999944120647, 0.8500000016763806, 0.04999999944120647], "_original_label_cost": -0.5})",
      R"({"_label_cost": 0, "_label_probability": 0.8500000016763806, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time_of_day=morning": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.04999999944120647, 0.04999999944120647, 0.8500000016763806, 0.04999999944120647], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.3166666677221655, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time_of_day=morning": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.3166666677221655, 0.3166666677221655, 0.04999999683350349, 0.3166666677221655], "_original_label_cost": 0})",
      R"({"_label_cost": 0, "_label_probability": 0.8500000016763806, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v=none": 1}, "_definitely_bad": false}], "a": [0, 1, 2, 3], "c": {"User": {"user=Anna": 1, "time_of_day=afternoon": 1}, "_multi": [{"Action": {"action=politics": 1}}, {"Action": {"action=sports": 1}}, {"Action": {"action=music": 1}}, {"Action": {"action=food": 1}}]}, "p": [0.04999999944120647, 0.04999999944120647, 0.8500000016763806, 0.04999999944120647], "_original_label_cost": 0})"
  };
  // clang-format on

  // IGL instance
  auto igl_vw = VW::initialize(vwtest::make_args("--experimental_igl", "--coin", "--cb_explore_adf", "--dsjson",
      "--noconstant", "--quiet", "-q", "UA", "--predict_only_model", "-b", "18"));

  auto multi_vw = VW::initialize(vwtest::make_args("--cb_explore_adf", "--coin", "--dsjson", "-q", "UA", "--noconstant",
      "--quiet", "-b", "17", "--predict_only_model"));

  // train cb model
  for (size_t i = 0; i < igl_vector.size(); i++)
  {
    auto multi_ex = igl_vector[i];
    auto examples = vwtest::parse_dsjson(*multi_vw, multi_ex);
    VW::setup_examples(*multi_vw, examples);
    multi_vw->learn(examples);
    multi_vw->finish_example(examples);
  }

  separate_weights_vector multi_weights = get_separate_weights(std::move(multi_vw));

  // train IGL
  for (auto& ex : igl_vector)
  {
    auto examples = vwtest::parse_dsjson(*igl_vw, ex);
    VW::setup_examples(*igl_vw, examples);

    igl_vw->learn(examples);
    igl_vw->finish_example(examples);
  }

  std::vector<separate_weights_vector> igl_weights = split_weights(std::move(igl_vw));

  EXPECT_GT(multi_weights.size(), 0);
  EXPECT_EQ(multi_weights, igl_weights[1]);
}