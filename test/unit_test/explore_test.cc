// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../../explore/explore.h"

#include "reductions/cb/cb_explore_pdf.h"
#include "test_common.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

using namespace VW::continuous_actions;
using std::vector;

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
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  return ((range_min <= chosen_value) && (chosen_value <= range_max));
}

BOOST_AUTO_TEST_CASE(sample_continuous_action)
{
  BOOST_CHECK(continuous_action_range_check(
      {{0.f, 20.f, 1.0f}, {20.f, 40.f, 2.0f}, {40.f, 50.f, 3.0f}, {50.f, 70.f, 4.0f}, {70.f, 100.f, 5.0f}}, .0f,
      100.f));

  BOOST_CHECK(continuous_action_range_check({{1000.f, 1020.f, 1.0f}, {1020.f, 1040.f, 2.0f}, {1040.f, 1050.f, 3.0f},
                                                {1050.f, 1070.f, 4.0f}, {1070.f, 1100.f, 5.0f}},
      1000.0f, 1100.f));
}

BOOST_AUTO_TEST_CASE(new_sample_pdf)
{
  probability_density_function the_pdf;
  the_pdf.push_back({2.f, 3.5f, 0.1f / 1.5f});
  the_pdf.push_back({3.5f, 4.5f, 0.8f / 1.0f});
  the_pdf.push_back({4.5f, 6.2f, 0.1f / 1.7f});
  const auto str_pdf = VW::to_string(the_pdf);  // 2-3.5:0.0666667,3.5-4.5:0.8,4.5-6.2:0.0588235
  // avoid float precision compare
  BOOST_CHECK_EQUAL(str_pdf.find("2-3.5:"), 0);
  BOOST_CHECK(str_pdf.find(",3.5-4.5:0.8,4.5-6.2:") > 0);

  uint64_t seed = 7791;
  float chosen_action = 0.f;
  float pdf_value = 0.f;

  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  BOOST_CHECK(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.back().right && pdf_value > 0.f);
  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  BOOST_CHECK(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.back().right && pdf_value > 0.f);
}

struct bins_calc
{
  bins_calc(vector<float>&& bins) : _bins(bins), _counts(_bins.size() - 1), _total_samples(0) {}

  void add_to_bin(float val)
  {
    ++_total_samples;
    float left = _bins[0];
    for (int i = 1; i < _bins.size(); i++)
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

BOOST_AUTO_TEST_CASE(sample_continuous_action_statistical)
{
  std::vector<pdf_seg> scores = {
      {0.f, 10.f, .005f}, {10.f, 20.f, .01f}, {20.f, 30.f, .0125f}, {30.f, 40.f, .0075f}, {40.f, 100.f, .015f}};

  float chosen_value;
  float pdf_value;
  constexpr float range_min = .0f;
  constexpr float range_max = 100.0f;
  uint64_t random_seed = 7791;
  bins_calc bins({0.f, 10.f, 20.f, 30.f, 40.f, 100.f});

  constexpr uint32_t iterate_count = 100000;
  for (auto idx = 0; idx < iterate_count; idx++)
  {
    const auto scode = exploration::sample_pdf(&random_seed, begin(scores), end(scores), chosen_value, pdf_value);
    BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
    BOOST_CHECK((range_min <= chosen_value) && (chosen_value <= range_max));

    // keep track of where the chosen actions fall
    bins.add_to_bin(chosen_value);
  }

  const float total_mass = std::accumulate(std::begin(scores), std::end(scores), 0.f,
      [](const float& acc, const pdf_seg& rhs) { return acc + (rhs.pdf_value * (rhs.right - rhs.left)); });

  for (uint32_t idx = 0; idx < bins._counts.size(); ++idx)
  {
    BOOST_CHECK_CLOSE(bins._counts[idx] / (float)bins._total_samples,
        (scores[idx].pdf_value * (scores[idx].right - scores[idx].left)) / total_mass, 1.5f);
  }
}

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_basic)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);
}

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_degenerate)
{
  std::vector<float> pdf = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  const std::vector<float> expected = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  uint32_t chosen_index;

  const auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_collections_with_float_tolerance(pdf, expected, .0001f);
}

BOOST_AUTO_TEST_CASE(swap_test)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected_pdf = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  BOOST_CHECK_EQUAL(chosen_index, 3);
  check_collections_with_float_tolerance(pdf, expected_pdf, .0001f);

  scode = exploration::swap_chosen(begin(pdf), end(pdf), chosen_index);

  const std::vector<float> expected_pdf_2 = {0.266666667f, 0.133333333f, 0.2f, 0.066666667f, 0.333333333f};
  check_collections_with_float_tolerance(pdf, expected_pdf_2, .0001f);
}