#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

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

BOOST_AUTO_TEST_CASE_TEMPLATE(test_feature_is_activated, T, weight_types) // unit test to check if number of bits equal to the threshold in bitset set to 1 for a feature returns true
{
  T w(LENGTH, STRIDE_SHIFT);
  int feature_index=0;
  int threshold=10;
  for(int tag_hash=0;tag_hash<threshold;tag_hash++) // function to set the bits in bitset to 1 equal to the threshold 
  {
    w.set_tag(tag_hash);
    auto& weight = w[feature_index]; 
  }
  BOOST_CHECK_EQUAL(w.is_activated(feature_index),true); 
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_feature_not_activated, T, weight_types) // unit test to check if number of bits less than the threshold in bitset set to 1 for a feature returns false
{
  T w(LENGTH, STRIDE_SHIFT);
  int feature_index=0;
  int threshold=10;
  for(int tag_hash=0;tag_hash<(threshold-1);tag_hash++) // function to set the bits in bitset to 1 equal to the (threshold-1)
  {
    w.set_tag(tag_hash);
    auto& weight = w[feature_index];
  }
  BOOST_CHECK_EQUAL(w.is_activated(feature_index),false); 
}