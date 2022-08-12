// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"

namespace VW
{
namespace cb_explore_adf
{
std::pair<float, uint64_t> spanner_state::find_max_volume(Eigen::MatrixXf& U, uint64_t X_rid, Eigen::MatrixXf& X)
{
  // Finds the max volume by replacing row X[X_rid] with some row in U.
  // Returns the max volume, and the row id of U used for replacing X[X_rid].

  float max_volume = -1.0f;
  uint64_t U_rid{};
  const Eigen::RowVectorXf original_row = X.row(X_rid);

  for (auto i = 0; i < U.rows(); ++i)
  {
    X.row(X_rid) = U.row(i);
    const float volume = std::abs(X.determinant());
    if (volume > max_volume)
    {
      max_volume = volume;
      U_rid = i;
    }
  }
  X.row(X_rid) = original_row;

  assert(max_volume >= 0.0f);
  return {max_volume, U_rid};
}

void spanner_state::compute_spanner(Eigen::MatrixXf& U, size_t _d)
{
  // Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
  // Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf

  // The size of U is K x d, where K is the total number of all actions.
  assert(static_cast<uint64_t>(U.cols()) == _d);
  Eigen::MatrixXf X = Eigen::MatrixXf::Identity(_d, _d);

  // Compute a basis contained in U.
  for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  {
    uint64_t U_rid = find_max_volume(U, X_rid, X).second;
    X.row(X_rid) = U.row(U_rid);
    _action_indices[X_rid] = U_rid;
  }

  // Transform the basis into C-approximate spanner.
  // According to the paper, the total number of iterations needed is O(d*log_c(d)).
  const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));
  float X_volume = std::abs(X.determinant());
  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in X results in larger volume, replace it with the row from U.
    for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
    {
      const auto max_volume_and_row_id = find_max_volume(U, X_rid, X);
      const float max_volume = max_volume_and_row_id.first;
      if (max_volume > _c * X_volume)
      {
        uint64_t U_rid = max_volume_and_row_id.second;
        X.row(X_rid) = U.row(U_rid);
        _action_indices[X_rid] = U_rid;

        X_volume = max_volume;
        found_larger_volume = true;
        break;
      }
    }

    if (!found_larger_volume) { break; }
  }

  _spanner_bitvec.clear();
  _spanner_bitvec.resize(U.rows(), false);
  for (uint64_t idx : _action_indices) { _spanner_bitvec[idx] = true; }
}

}  // namespace cb_explore_adf
}  // namespace VW