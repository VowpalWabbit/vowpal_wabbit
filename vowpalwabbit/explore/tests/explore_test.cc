// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/explore/explore.h"

#include "vw/core/reductions/cb/cb_explore_pdf.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using namespace VW::continuous_actions;
using std::vector;
using namespace ::testing;

struct pdf_seg
{
  float left;
  float right;
  float pdf_value;
};

bool continuous_action_range_check(std::vector<pdf_seg> scores, float range_min, float range_max)
{
  float chosen_value;
  float pdf_value;
  uint64_t seed = 7791;
  auto scode = exploration::sample_pdf(&seed, begin(scores), end(scores), chosen_value, pdf_value);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  return ((range_min <= chosen_value) && (chosen_value <= range_max));
}

TEST(explore_tests, sample_continuous_action)
{
  EXPECT_TRUE(continuous_action_range_check(
      {{0.f, 20.f, 1.0f}, {20.f, 40.f, 2.0f}, {40.f, 50.f, 3.0f}, {50.f, 70.f, 4.0f}, {70.f, 100.f, 5.0f}}, .0f,
      100.f));

  EXPECT_TRUE(continuous_action_range_check({{1000.f, 1020.f, 1.0f}, {1020.f, 1040.f, 2.0f}, {1040.f, 1050.f, 3.0f},
                                                {1050.f, 1070.f, 4.0f}, {1070.f, 1100.f, 5.0f}},
      1000.0f, 1100.f));
}

TEST(explore_tests, new_sample_pdf)
{
  probability_density_function the_pdf;
  the_pdf.push_back({2.f, 3.5f, 0.1f / 1.5f});
  the_pdf.push_back({3.5f, 4.5f, 0.8f / 1.0f});
  the_pdf.push_back({4.5f, 6.2f, 0.1f / 1.7f});
  const auto str_pdf = VW::to_string(the_pdf);  // 2-3.5:0.0666667,3.5-4.5:0.8,4.5-6.2:0.0588235
  // avoid float precision compare
  EXPECT_EQ(str_pdf.find("2-3.5:"), 0);
  EXPECT_TRUE(str_pdf.find(",3.5-4.5:0.8,4.5-6.2:") > 0);

  uint64_t seed = 7791;
  float chosen_action = 0.f;
  float pdf_value = 0.f;

  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  EXPECT_TRUE(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.back().right && pdf_value > 0.f);
  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  EXPECT_TRUE(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.back().right && pdf_value > 0.f);
}

struct bins_calc
{
  bins_calc(vector<float>&& bins) : _bins(bins), _counts(_bins.size() - 1), _total_samples(0) {}

  void add_to_bin(float val)
  {
    ++_total_samples;
    float left = _bins[0];
    for (size_t i = 1; i < _bins.size(); i++)
    {
      float right = _bins[i];
      if (left <= val && val < right)
      {
        ++_counts[i - 1];
        return;
      }
    }
    std::stringstream err_strm;
    err_strm << "Value " << val << " does not fall in a bin";
    throw std::logic_error(err_strm.str());
  }

  vector<float> _bins{};
  vector<uint32_t> _counts{};
  uint32_t _total_samples;
};

TEST(explore_tests, sample_continuous_action_statistical)
{
  std::vector<pdf_seg> scores = {
      {0.f, 10.f, .005f}, {10.f, 20.f, .01f}, {20.f, 30.f, .0125f}, {30.f, 40.f, .0075f}, {40.f, 100.f, .015f}};

  float chosen_value;
  float pdf_value;
  constexpr float range_min = .0f;
  constexpr float range_max = 100.0f;
  uint64_t random_seed = 7791;
  bins_calc bins({0.f, 10.f, 20.f, 30.f, 40.f, 100.f});

  constexpr size_t iterate_count = 100000;
  for (size_t idx = 0; idx < iterate_count; idx++)
  {
    const auto scode = exploration::sample_pdf(&random_seed, begin(scores), end(scores), chosen_value, pdf_value);
    EXPECT_EQ(scode, S_EXPLORATION_OK);
    EXPECT_TRUE((range_min <= chosen_value) && (chosen_value <= range_max));

    // keep track of where the chosen actions fall
    bins.add_to_bin(chosen_value);
  }

  const float total_mass = std::accumulate(std::begin(scores), std::end(scores), 0.f,
      [](const float& acc, const pdf_seg& rhs) { return acc + (rhs.pdf_value * (rhs.right - rhs.left)); });

  for (uint32_t idx = 0; idx < bins._counts.size(); ++idx)
  {
    EXPECT_NEAR(bins._counts[idx] / (float)bins._total_samples,
        (scores[idx].pdf_value * (scores[idx].right - scores[idx].left)) / total_mass, 1.5f);
  }
}

TEST(explore_tests, sample_after_nomalizing_basic)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));
}

TEST(explore_tests, sample_after_nomalizing_degenerate)
{
  std::vector<float> pdf = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  const std::vector<float> expected = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  uint32_t chosen_index;

  const auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected));
}

TEST(explore_tests, swap_test)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected_pdf = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  EXPECT_EQ(scode, S_EXPLORATION_OK);
  EXPECT_EQ(chosen_index, 3);
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected_pdf));

  scode = exploration::swap_chosen(begin(pdf), end(pdf), chosen_index);
  const std::vector<float> expected_pdf_2 = {0.266666667f, 0.133333333f, 0.2f, 0.066666667f, 0.333333333f};
  EXPECT_THAT(pdf, Pointwise(FloatNear(.0001f), expected_pdf_2));
}

TEST(explore_tests, epsilon_greedy)
{
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_epsilon_greedy(0.4f, 2, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-6f), std::vector<float>{0.1f, 0.1f, 0.7f, 0.1f}));
}

TEST(explore_tests, epsilon_greedy_top_action_out_of_bounds)
{
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_epsilon_greedy(0.4f, 8, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-6f), std::vector<float>{0.1f, 0.1f, 0.1f, 0.7f}));
}

TEST(explore_tests, epsilon_greedy_bad_range)
{
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_epsilon_greedy(0.4f, 0, begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_epsilon_greedy(0.4f, 0, &x, &x - 3));
  EXPECT_THAT(pdf.size(), 0);
}

TEST(explore_tests, softmax)
{
  std::vector<float> scores = {1, 2, 3, 8};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{0.128f, 0.157f, 0.192f, 0.522f}));
}

TEST(explore_tests, softmax_imbalanced)
{
  std::vector<float> scores = {1, 2, 3};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{0.269f, 0.328f, 0.401f, 0}));
}

TEST(explore_tests, softmax_imbalanced2)
{
  std::vector<float> scores = {1, 2, 3, 8, 4};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{0.128f, 0.157f, 0.192f, 0.522f}));
}

TEST(explore_tests, softmax_bad_range)
{
  std::vector<float> scores;
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(
      E_EXPLORATION_BAD_RANGE, exploration::generate_softmax(0.2f, begin(scores), end(scores), begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_softmax(0.2f, begin(scores), end(scores), &x, &x - 3));
}

TEST(explore_tests, bag)
{
  std::vector<uint16_t> top_actions = {0, 0, 1, 1};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{0, 0, 0.5, 0.5f}));
}

TEST(explore_tests, bag10)
{
  std::vector<uint16_t> top_actions = {10};
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{1.f, 0, 0, 0}));
}

TEST(explore_tests, bag_empty)
{
  std::vector<uint16_t> top_actions;
  std::vector<float> pdf(4);
  EXPECT_THAT(S_EXPLORATION_OK, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{1.f, 0, 0, 0}));
}

TEST(explore_tests, bag_bad_range)
{
  std::vector<uint16_t> top_actions;
  std::vector<float> pdf;
  float x;

  EXPECT_THAT(
      E_EXPLORATION_BAD_RANGE, exploration::generate_bag(begin(top_actions), end(top_actions), begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::generate_bag(begin(top_actions), end(top_actions), &x, &x - 3));
}

TEST(explore_tests, enforce_minimum_probability)
{
  std::vector<float> pdf = {1.f, 0, 0};
  exploration::enforce_minimum_probability(0.3f, true, begin(pdf), end(pdf));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{.8f, .1f, .1f}));
}

TEST(explore_tests, enforce_minimum_probability_no_zeros)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(0.6f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{.8f, .2f, .0f}));
}

TEST(explore_tests, enforce_minimum_probability_uniform)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(1.f, true, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{.25f, .25f, .25f, .25f}));
}

TEST(explore_tests, enforce_minimum_probability_uniform_no_zeros)
{
  std::vector<float> pdf = {0.9f, 0.1f, 0};
  EXPECT_THAT(S_EXPLORATION_OK, exploration::enforce_minimum_probability(1.f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-3f), std::vector<float>{.5f, .5f, .0f}));
}

TEST(explore_tests, enforce_minimum_probability_bad_range)
{
  std::vector<float> pdf;
  float x;
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::enforce_minimum_probability(1.f, false, begin(pdf), end(pdf)));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::enforce_minimum_probability(1.f, false, &x, &x - 3));
}

TEST(explore_tests, sampling)
{
  std::vector<float> pdf = {0.8f, 0.1f, 0.1f};
  std::vector<float> histogram(3);

  size_t rep = 10000;
  uint32_t chosen_index;
  for (size_t i = 0; i < rep; i++)
  {
    std::stringstream s;
    s << "abcde" << i;
    ASSERT_EQ(0, exploration::sample_after_normalizing(s.str().c_str(), std::begin(pdf), std::end(pdf), chosen_index));

    histogram[chosen_index]++;
  }
  for (auto& d : histogram) d /= rep;

  EXPECT_THAT(pdf, Pointwise(FloatNear(1e-2f), histogram));
}

TEST(explore_tests, sampling_rank_bad_range)
{
  std::vector<float> pdf;
  std::vector<float> scores;
  std::vector<int> ranking(3);
  float x;
  uint32_t chosen_index;

  EXPECT_THAT(E_EXPLORATION_BAD_RANGE,
      exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));
  EXPECT_THAT(E_EXPLORATION_BAD_RANGE, exploration::sample_after_normalizing("abc", &x, &x - 3, chosen_index));
}

TEST(explore_tests, sampling_rank_zero_pdf)
{
  std::vector<float> pdf = {0.f, 0.f, 0.f};
  std::vector<float> expected_pdf = {1.f, 0.f, 0.f};

  uint32_t chosen_index;

  EXPECT_THAT(
      S_EXPLORATION_OK, exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));

  EXPECT_THAT(expected_pdf, Pointwise(FloatNear(1e-2f), pdf));
}

TEST(explore_tests, sampling_rank_negative_pdf)
{
  std::vector<float> pdf = {1.0f, -2.f, -3.f};
  std::vector<float> expected_pdf = {1.f, 0.f, 0.f};
  std::vector<int> ranking(3);
  uint32_t chosen_index;

  EXPECT_THAT(
      S_EXPLORATION_OK, exploration::sample_after_normalizing("abc", std::begin(pdf), std::end(pdf), chosen_index));
  EXPECT_THAT(expected_pdf, Pointwise(FloatNear(1e-2f), pdf));
  EXPECT_THAT(0, chosen_index);
}
