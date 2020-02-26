#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "../../explore/explore.h"
bool continuous_action_range_check(std::vector<float> scores, float range_min, float range_max)
{
  float chosen_value;
  uint64_t seed = 7791;
  auto scode = exploration::sample_pdf(&seed, begin(scores), end(scores), range_min, range_max, chosen_value);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  return ((range_min <= chosen_value) && (chosen_value <= range_max));
}

BOOST_AUTO_TEST_CASE(sample_continuous_action)
{
  BOOST_CHECK(continuous_action_range_check({1.0f, 2.0f, 3.0f, 4.0f, 5.0f}, .0f, 100.f));
  BOOST_CHECK(continuous_action_range_check({1.0f, 2.0f, 3.0f, 4.0f, 5.0f}, 1000.0f, 1100.f));
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
  std::vector<float> scores = {.005f, .01f, .0125f, .0075f, .015f, .0f};

  float chosen_value;
  const float range_min = .0f;
  const float range_max = 100.0f;
  uint64_t random_seed = 7791;
  bins_calc bins(0.f, 100.0f, (uint32_t)scores.size()-1);

  const uint32_t iterate_count = 100000;
  for (auto idx = 0; idx < iterate_count; idx++)
  {
    const auto scode =
        exploration::sample_pdf(&random_seed, begin(scores), end(scores), range_min, range_max, chosen_value);
    BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
    BOOST_CHECK((range_min <= chosen_value) && (chosen_value <= range_max));

    // advance random seed
    random_seed = uniform_hash(&random_seed, sizeof(random_seed), random_seed);
    random_seed = uniform_hash(&random_seed, sizeof(random_seed), random_seed);

    // keep track of where the chosen actions fall
    bins.add_to_bin(chosen_value);
  }

  const float total_scores = std::accumulate(std::begin(scores), std::end(scores), 0.f);
  for (uint32_t idx = 0; idx < bins._num_bins; ++idx)
  {
    BOOST_CHECK_CLOSE(bins._counts[idx]/(float)bins._total_samples, scores[idx] / total_scores, 1.5f);
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
