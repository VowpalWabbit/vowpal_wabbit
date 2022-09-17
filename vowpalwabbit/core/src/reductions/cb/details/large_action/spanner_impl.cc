// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"

namespace VW
{
namespace cb_explore_adf
{
void spanner_state::find_max_volume(
    const Eigen::MatrixXf& U, uint64_t X_rid, Eigen::MatrixXf& X, float& max_volume, uint64_t& U_rid)
{
  // Finds the max volume by replacing row X[X_rid] with some row in U.
  // Returns the max volume, and the row id of U used for replacing X[X_rid].

  max_volume = -1.0f;
  U_rid = 0;

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
}

void spanner_state::compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>&)
{
  // Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
  // Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf

  // The size of U is K x d, where K is the total number of all actions.
  assert(static_cast<uint64_t>(U.cols()) == _d);
  _X.setIdentity(_d, _d);

  // Compute a basis contained in U.
  for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  {
    float max_volume = -1.0f;
    uint64_t U_rid = 0;
    find_max_volume(U, X_rid, _X, max_volume, U_rid);
    _X.row(X_rid) = U.row(U_rid);
    _action_indices[X_rid] = U_rid;
  }

  // Transform the basis into C-approximate spanner.
  // According to the paper, the total number of iterations needed is O(d*log_c(d)).
  const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));
  float X_volume = std::abs(_X.determinant());
  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in X results in larger volume, replace it with the row from U.
    for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
    {
      float max_volume = -1.0f;
      uint64_t U_rid = 0;
      find_max_volume(U, X_rid, _X, max_volume, U_rid);
      if (max_volume > _c * X_volume)
      {
        _X.row(X_rid) = U.row(U_rid);
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

bool spanner_state::is_action_in_spanner(uint32_t action) { return _spanner_bitvec[action]; }

size_t spanner_state::spanner_size() { return _spanner_bitvec.size(); }

void spanner_state::_test_only_set_rank(uint64_t rank) { _action_indices.resize(rank); }

}  // namespace cb_explore_adf
}  // namespace VW