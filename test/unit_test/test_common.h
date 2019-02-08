#pragma once

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <vector>

void inline check_float_vectors(const std::vector<float>& lhs, const std::vector<float>& rhs, const float tolerance_percent) {
  BOOST_CHECK_EQUAL(lhs.size(), rhs.size());
  for (auto l = begin(lhs), r = begin(rhs); l < end(lhs); ++l, ++r) {
    BOOST_CHECK_CLOSE(*l, *r, tolerance_percent);
  }
}

template <typename T>
void check_vectors(const std::vector<T>& lhs, const std::vector<T>& rhs) {
  BOOST_CHECK_EQUAL_COLLECTIONS(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
