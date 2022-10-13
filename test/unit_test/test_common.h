// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/action_score.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/shared_data.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

constexpr float FLOAT_TOL = 0.0001f;

inline void compare(float l, float r, float tol) { BOOST_CHECK_CLOSE(l, r, tol); }
inline void compare(const VW::action_score& l, const VW::action_score& r, float float_tolerance)
{
  BOOST_CHECK_EQUAL(l.action, r.action);
  BOOST_CHECK_CLOSE(l.score, r.score, float_tolerance);
}

template <typename ContainerOneT, typename ContainerTwoT>
void check_collections_with_float_tolerance(
    const ContainerOneT& lhs, const ContainerTwoT& rhs, float float_tolerance = FLOAT_TOL)
{
  BOOST_CHECK_EQUAL(lhs.size(), rhs.size());
  auto l = std::begin(lhs);
  auto r = std::begin(rhs);
  for (; l < std::end(lhs); ++l, ++r) { compare(*l, *r, float_tolerance); }
}

template <template <typename...> class ContainerOneT, template <typename...> class ContainerTwoT, typename T>
void check_collections_exact(const ContainerOneT<T>& lhs, const ContainerTwoT<T>& rhs)
{
  BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
}

template <typename T>
void check_vector_of_vectors_exact(const std::vector<std::vector<T>>& lhs, const std::vector<std::vector<T>>& rhs)
{
  BOOST_CHECK_EQUAL(lhs.size(), rhs.size());
  for (size_t i = 0; i < lhs.size(); i++)
  { BOOST_CHECK_EQUAL_COLLECTIONS(lhs[i].begin(), lhs[i].end(), rhs[i].begin(), rhs[i].end()); }
}

VW::multi_ex parse_json(VW::workspace& all, const std::string& line);

VW::multi_ex parse_dsjson(VW::workspace& all, std::string line, DecisionServiceInteraction* interaction = nullptr);

bool is_invoked_with(const std::string& arg);

namespace VW
{
inline std::ostream& operator<<(std::ostream& os, const namespace_extent& extent)
{
  os << "{" << extent.begin_index << "," << extent.end_index << "," << extent.hash << "}";
  return os;
}
}  // namespace VW

namespace std
{
inline std::ostream& operator<<(std::ostream& os, VW::slates::example_type ex_type)
{
  os << VW::to_string(ex_type);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, VW::ccb_example_type ex_type)
{
  os << VW::to_string(ex_type);
  return os;
}
}  // namespace std
