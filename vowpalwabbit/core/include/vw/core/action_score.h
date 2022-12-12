// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/constant.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <iterator>
#include <string>

namespace VW
{
class action_score
{
public:
  uint32_t action;
  float score;
};
using action_scores = VW::v_array<action_score>;

constexpr inline bool operator<(const action_score& left, const action_score& right)
{
  return (left.score == right.score) ? left.action < right.action : left.score < right.score;
}
constexpr inline bool operator>(const action_score& left, const action_score& right)
{
  return (left.score == right.score) ? left.action > right.action : left.score > right.score;
}
std::ostream& operator<<(std::ostream& os, const action_score& a_s);

namespace details
{
void print_action_score(
    VW::io::writer* f, const VW::v_array<action_score>& a_s, const VW::v_array<char>&, VW::io::logger& logger);
}

std::string to_string(
    const action_scores& action_scores_or_probs, int decimal_precision = details::DEFAULT_FLOAT_PRECISION);

class action_scores_score_iterator
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = float;
  using difference_type = long;
  using pointer = float*;
  using reference = float;

  action_scores_score_iterator(action_score* p) : _p(p) {}

  action_scores_score_iterator& operator++()
  {
    ++_p;
    return *this;
  }

  action_scores_score_iterator operator+(size_t n) { return {_p + n}; }

  bool operator==(const action_scores_score_iterator& other) const { return _p == other._p; }

  bool operator!=(const action_scores_score_iterator& other) const { return _p != other._p; }

  bool operator<(const action_scores_score_iterator& other) const { return _p < other._p; }

  size_t operator-(const action_scores_score_iterator& other) const { return _p - other._p; }

  float& operator*() { return _p->score; }
  float operator*() const { return _p->score; }

private:
  action_score* _p;
};

inline action_scores_score_iterator begin_scores(action_scores& a_s) { return {a_s.begin()}; }
inline action_scores_score_iterator end_scores(action_scores& a_s) { return {a_s.end()}; }
namespace model_utils
{
size_t read_model_field(io_buf& io, action_score& a_s);

size_t write_model_field(io_buf& io, const action_score a_s, const std::string& upstream_name, bool text);
}  // namespace model_utils
}  // namespace VW

namespace ACTION_SCORE  // NOLINT
{
using action_score VW_DEPRECATED(
    "ACTION_SCORE::action_score renamed to VW::action_score. ACTION_SCORE::action_score will be removed in VW 10.") =
    VW::action_score;

using action_scores VW_DEPRECATED(
    "ACTION_SCORE::action_scores renamed to VW::action_scores. ACTION_SCORE::action_scores will be removed in VW 10.") =
    VW::v_array<VW::action_score>;
using score_iterator VW_DEPRECATED(
    "ACTION_SCORE::score_iterator renamed to VW::action_scores_score_iterator. ACTION_SCORE::score_iterator will be "
    "removed in VW 10.") = VW::action_scores_score_iterator;

VW_DEPRECATED(
    "ACTION_SCORE::begin_scores renamed to VW::begin_scores. ACTION_SCORE::begin_scores will be removed in VW 10.")
inline VW::action_scores_score_iterator begin_scores(VW::action_scores& a_s) { return VW::begin_scores(a_s); }

VW_DEPRECATED("ACTION_SCORE::end_scores renamed to VW::end_scores. ACTION_SCORE::end_scores will be removed in VW 10.")
inline VW::action_scores_score_iterator end_scores(VW::action_scores& a_s) { return VW::end_scores(a_s); }
}  // namespace ACTION_SCORE