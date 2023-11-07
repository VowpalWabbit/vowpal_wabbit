// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <vector>

namespace VW
{
namespace cb_graph_feedback
{

struct triplet
{
  uint64_t row;
  uint64_t col;
  float val;

  triplet(uint64_t row_, uint64_t col_, float val_) : row(row_), col(col_), val(val_) {}
};

class reduction_features
{
public:
  std::vector<triplet> triplets;

  void push_triplet(uint64_t row, uint64_t col, float val) { triplets.push_back(triplet{row, col, val}); }

  void clear() { triplets.clear(); }
};

}  // namespace cb_graph_feedback
}  // namespace VW