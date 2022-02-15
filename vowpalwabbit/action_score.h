// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "io/io_adapter.h"
#include "io/logger.h"
#include <iterator>
#include "v_array.h"

namespace ACTION_SCORE
{
struct action_score
{
  uint32_t action;
  float score;
};

using action_scores = v_array<action_score>;

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
    VW::io::writer* f, const v_array<action_score>& a_s, const v_array<char>&, VW::io::logger& logger);

std::ostream& operator<<(std::ostream& os, const action_score& a_s);
}  // namespace ACTION_SCORE

namespace VW
{
inline bool action_score_compare(const ACTION_SCORE::action_score& left, const ACTION_SCORE::action_score& right)
{
  if (left.score == right.score) { return left.action < right.action; }
  return left.score < right.score;
}
}  // namespace VW