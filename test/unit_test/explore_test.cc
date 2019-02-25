#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "../../explore/explore.h"

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_basic) {
  std::vector<float> pdf = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
  const std::vector<float> expected = { 0.066666667f,	0.133333333f,	0.2f,	0.266666667f,	0.333333333f };
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

BOOST_AUTO_TEST_CASE(sample_after_nomalizing_degenerate) {
  std::vector<float> pdf = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  const std::vector<float> expected = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  uint32_t chosen_index;

  const auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  check_float_vectors(pdf, expected, .0001f);
}

BOOST_AUTO_TEST_CASE(swap_test) {
  std::vector<float> pdf = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
  const std::vector<float> expected_pdf = { 0.066666667f,	0.133333333f,	0.2f,	0.266666667f,	0.333333333f };
  uint32_t chosen_index;

  auto scode = exploration::sample_after_normalizing(7791, begin(pdf), end(pdf), chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  BOOST_CHECK_EQUAL(chosen_index, 3);
  check_float_vectors(pdf, expected_pdf, .0001f);

  scode = exploration::swap_chosen(begin(pdf), end(pdf), chosen_index);

  const std::vector<float> expected_pdf_2 = { 0.266666667f,	0.133333333f,	0.2f,	0.066666667f,	0.333333333f };
  check_float_vectors(pdf, expected_pdf_2, .0001f);
}
