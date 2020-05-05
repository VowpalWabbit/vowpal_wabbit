#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "../../explore/explore.h"

#include "cb_explore_pdf.h"

using namespace VW::actions_pdf;

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
  BOOST_CHECK(continuous_action_range_check({
    {0.f,20.f,1.0f},
    {20.f,40.f,2.0f},
    {40.f,50.f,3.0f},
    {50.f,70.f,4.0f},
    {70.f,100.f,5.0f}}, .0f, 100.f));

  BOOST_CHECK(continuous_action_range_check({
    {1000.f, 1020.f, 1.0f},
    {1020.f, 1040.f, 2.0f},
    {1040.f, 1050.f, 3.0f},
    {1050.f, 1070.f, 4.0f},
    {1070.f, 1100.f, 5.0f}}, 1000.0f, 1100.f));
}

BOOST_AUTO_TEST_CASE(new_sample_pdf)
{
  pdf_new the_pdf = v_init<::pdf_segment_new>();
  the_pdf.push_back({2.f, 3.5f, 0.1f / 1.5f});
  the_pdf.push_back({3.5f, 4.5f, 0.8f / 1.0f});
  the_pdf.push_back({4.5f, 6.2f, 0.1f / 1.7f});
  const auto str_pdf = to_string(the_pdf);
  std::cout << str_pdf << std::endl;
  uint64_t seed = 7791;
  float chosen_action = 0.f;
  float pdf_value = 0.f;

  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  BOOST_CHECK(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.last().right && pdf_value > 0.f);
  exploration::sample_pdf(&seed, std::begin(the_pdf), std::end(the_pdf), chosen_action, pdf_value);
  BOOST_CHECK(chosen_action >= the_pdf[0].left && chosen_action <= the_pdf.last().right && pdf_value > 0.f);
}

struct bins_calc
{
  bins_calc(float start, float end, uint32_t num_bins)
      : _start(start)
      , _end(end)
      , _num_bins(num_bins)
      , _bin_size((_end - _start) / _num_bins)
      , _total_samples(0)
      , _counts(num_bins)
  {
  }

  void add_to_bin(float val)
  {
    ++_total_samples;
    for (uint32_t i = 0; i < _num_bins; i++)
    {
      if (val <= (_bin_size * (i + 1.0f)))
      {
        ++_counts[i];
        return;
      }
    }
  }

  const float _start;
  const float _end;
  const uint32_t _num_bins;
  const float _bin_size;
  uint32_t _total_samples;
  std::vector<uint32_t> _counts;
};

BOOST_AUTO_TEST_CASE(sample_continuous_action_statistical)
{
  std::vector<pdf_seg> scores = {
    {0.f, 10.f,.005f},
    {10.f,20.f,.01f},
    {20.f,30.f,.0125f},
    {30.f,40.f,.0075f},
    {40.f,100.f,.015f}};

  float chosen_value;
  float pdf_value;
  const float range_min = .0f;
  const float range_max = 100.0f;
  uint64_t random_seed = 7791;
  bins_calc bins(0.f, 100.0f, (uint32_t)scores.size() - 1);

  const uint32_t iterate_count = 100000;
  for (auto idx = 0; idx < iterate_count; idx++)
  {
    const auto scode =
        exploration::sample_pdf(&random_seed, begin(scores), end(scores), chosen_value, pdf_value);
    BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
    BOOST_CHECK((range_min <= chosen_value) && (chosen_value <= range_max));

    // advance random seed
    random_seed = uniform_hash(&random_seed, sizeof(random_seed), random_seed);
    random_seed = uniform_hash(&random_seed, sizeof(random_seed), random_seed);

    // keep track of where the chosen actions fall
    bins.add_to_bin(chosen_value);
  }

  const float total_scores =
      std::accumulate(std::begin(scores), std::end(scores), 0.f, [](const float& acc, const pdf_seg& rhs) { return acc + rhs.pdf_value;
      });
  for (uint32_t idx = 0; idx < bins._num_bins; ++idx)
  {
    BOOST_CHECK_CLOSE(bins._counts[idx] / (float)bins._total_samples, scores[idx].pdf_value / total_scores, 1.5f);
  }
}

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_basic)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);

  scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);
}

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_degenerate)
{
  std::vector<float> pdf = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  const std::vector<float> expected = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  uint32_t chosen_index;

  const auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);
}

BOOST_AUTO_TEST_CASE(swap_test)
{
  std::vector<float> pdf = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  const std::vector<float> expected_pdf = {0.066666667f, 0.133333333f, 0.2f, 0.266666667f, 0.333333333f};
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  BOOST_CHECK_EQUAL(chosen_index, 3);
  check_float_vectors(pdf, expected_pdf, .0001f);

  scode = exploration::swap_chosen(begin(pdf), end(pdf), chosen_index);

  const std::vector<float> expected_pdf_2 = {0.266666667f, 0.133333333f, 0.2f, 0.066666667f, 0.333333333f};
  check_float_vectors(pdf, expected_pdf_2, .0001f);
}
