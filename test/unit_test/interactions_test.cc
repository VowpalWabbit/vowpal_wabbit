#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <cstddef>
#include <cstdint>

#include "test_common.h"
#include "vw.h"
#include "gd_predict.h"
#include "interactions.h"

struct eval_gen_data
{
  size_t& new_features_cnt;
  float& new_features_value;
  eval_gen_data(size_t& features_cnt, float& features_value)
      : new_features_cnt(features_cnt), new_features_value(features_value)
  {
  }
};

void ft_cnt(eval_gen_data& dat, const float fx, const uint64_t)
{
  ++dat.new_features_cnt;
  dat.new_features_value += fx * fx;
}

// eval_count_of_generated_ft_naive() is an alternative way of implementation of
// eval_count_of_generated_ft() it just calls generate_interactions() with small
// function which counts generated features and sums their squared values. We
// use it to validate the with more fast (?) analytic solution
void eval_count_of_generated_ft_naive(vw& all, example_predict& ec, size_t& new_features_cnt, float& new_features_value)
{
  // Only makes sense to do this when not in permutations mode.
  assert(!all.permutations);

  new_features_cnt = 0;
  new_features_value = 0.;

  v_array<float> results;

  eval_gen_data dat(new_features_cnt, new_features_value);
  size_t ignored = 0;
  INTERACTIONS::generate_interactions<eval_gen_data, uint64_t, ft_cnt,false, nullptr>(all, ec, dat, ignored);
}

inline void noop_func(float& unused_dat, const float ft_weight, const uint64_t ft_idx) {}

BOOST_AUTO_TEST_CASE(eval_count_of_generated_ft_test)
{
  auto& vw = *VW::initialize("--quiet -q ::", nullptr, false, nullptr, nullptr);
  auto* ex = VW::read_example(vw, std::string("3 |f a b c |e x y z"));

  size_t naive_features_count;
  float naive_features_value;
  eval_count_of_generated_ft_naive(vw, *ex, naive_features_count, naive_features_value);

  size_t fast_features_count;
  float fast_features_value;
  INTERACTIONS::eval_count_of_generated_ft(vw.permutations, ex->interactions->interactions, ex->feature_space, fast_features_count, fast_features_value);

  BOOST_CHECK_EQUAL(naive_features_count, fast_features_count);
  BOOST_CHECK_CLOSE(naive_features_value, fast_features_value, FLOAT_TOL);

  // Prediction will count the interacted features, so we can compare that too.
  vw.predict(*ex);
  BOOST_CHECK_EQUAL(naive_features_count, ex->num_features_from_interactions);
  VW::finish_example(vw, *ex);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(eval_count_of_generated_ft_permuations_test)
{
  auto& vw = *VW::initialize("--quiet -q :: --permutations", nullptr, false, nullptr, nullptr);
  auto* ex = VW::read_example(vw, std::string("3 |f a b c |e x y z"));

  size_t fast_features_count;
  float fast_features_value;
  INTERACTIONS::eval_count_of_generated_ft(vw.permutations, ex->interactions->interactions, ex->feature_space, fast_features_count, fast_features_value);

  vw.predict(*ex);
  BOOST_CHECK_EQUAL(fast_features_count, ex->num_features_from_interactions);

  VW::finish_example(vw, *ex);
  VW::finish(vw);
}
