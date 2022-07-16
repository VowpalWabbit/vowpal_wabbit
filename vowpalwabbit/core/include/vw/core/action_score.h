// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/constant.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <iterator>
#include <string>

namespace ACTION_SCORE
{
struct action_score
{
  uint32_t action;
  float score;
};

using action_scores = VW::v_array<action_score>;

class score_iterator
{
  action_score* _p;

public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = float;
  using difference_type = long;
  using pointer = float*;
  using reference = float;

  score_iterator(action_score* p) : _p(p) {}

  score_iterator& operator++()
  {
    ++_p;
    return *this;
  }

  score_iterator operator+(size_t n) { return {_p + n}; }

  bool operator==(const score_iterator& other) const { return _p == other._p; }

  bool operator!=(const score_iterator& other) const { return _p != other._p; }

  bool operator<(const score_iterator& other) const { return _p < other._p; }

  size_t operator-(const score_iterator& other) const { return _p - other._p; }

  float& operator*() { return _p->score; }
};

inline score_iterator begin_scores(action_scores& a_s) { return {a_s.begin()}; }

inline score_iterator end_scores(action_scores& a_s) { return {a_s.end()}; }

void print_action_score(
    VW::io::writer* f, const VW::v_array<action_score>& a_s, const VW::v_array<char>&, VW::io::logger& logger);

std::ostream& operator<<(std::ostream& os, const action_score& a_s);
}  // namespace ACTION_SCORE

namespace VW
{
constexpr inline bool action_score_compare_lt(
    const ACTION_SCORE::action_score& left, const ACTION_SCORE::action_score& right)
{
  return (left.score == right.score) ? left.action < right.action : left.score < right.score;
}
constexpr inline bool action_score_compare_gt(
    const ACTION_SCORE::action_score& left, const ACTION_SCORE::action_score& right)
{
  return (left.score == right.score) ? left.action > right.action : left.score > right.score;
}

std::string to_string(
    const ACTION_SCORE::action_scores& action_scores_or_probs, int decimal_precision = DEFAULT_FLOAT_PRECISION);
}  // namespace VW