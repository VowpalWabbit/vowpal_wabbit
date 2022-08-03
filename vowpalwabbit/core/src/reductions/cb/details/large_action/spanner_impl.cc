// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"

namespace VW
{
namespace cb_explore_adf
{
void spanner_state::compute_spanner(Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors)
{
  assert(static_cast<uint64_t>(U.cols()) == _d);
  _X.setIdentity(_d, _d);
  _S_inv.setIdentity(_d, _d);
  Eigen::VectorXf logdetfact(_d);
  logdetfact.setZero();

  // Compute a basis contained in U.
  for (uint64_t i = 0; i < _d; ++i)
  {
    Eigen::VectorXf Sinvtopei = _S_inv.row(i);

    float max_volume = -1.0f;
    uint64_t U_rid = 0;
    for (auto i = 0; i < U.rows(); ++i)
    {
      float vol = std::abs(U.row(i) * Sinvtopei);
      if (vol > max_volume)
      {
        max_volume = vol;
        U_rid = i;
      }
    }

    // best action is U_rid
    Eigen::VectorXf y = U.row(U_rid);
    y /= shrink_factors[U_rid];

    Eigen::VectorXf pow = logdetfact;
    pow = Eigen::exp(pow.array());
    Eigen::VectorXf powinv = pow;
    powinv = powinv.cwiseInverse();
    y = y.cwiseProduct(powinv);

    Eigen::VectorXf Si = _X.row(i);
    Eigen::VectorXf u = y - Si;
    Eigen::MatrixXf Sinvu = _S_inv * u;
    Eigen::VectorXf vtopSinv = _S_inv.row(i);
    float vtopSinvu = Sinvu(i, 0);

    _S_inv -= (1.f / (1.f + vtopSinvu)) * (Sinvu * vtopSinv.transpose());

    _X.row(i) = y;
    _action_indices[i] = U_rid;

    Eigen::VectorXf tmp = logdetfact;
    tmp = std::log(max_volume) - logdetfact.array();

    Eigen::VectorXf thislogdet = (1.f / _d) * (tmp);
    Eigen::VectorXf pow2 = thislogdet;
    pow2 = Eigen::exp(pow2.array());
    Eigen::VectorXf scale = pow2;
    for (uint64_t col = 0; col < _d; col++) { _S_inv.col(col) = _S_inv.col(col).cwiseProduct(scale); }

    scale = scale.cwiseInverse();
    for (uint64_t col = 0; col < _d; col++) { _X.col(col) = _X.col(col).cwiseProduct(scale); }

    logdetfact += thislogdet;
  }

  // const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));
  // float X_volume = std::abs(_X.determinant());
  // for (int iter = 0; iter < max_iterations; ++iter)
  // {
  //   bool found_larger_volume = false;

  //   // If replacing some row in _X results in larger volume, replace it with the row from U.
  //   for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  //   {
  //     uint64_t U_rid = 0;
  //     float max_volume = 0;
  //     find_max_volume(U, X_rid, _X, max_volume, U_rid);
  //     if (max_volume > _c * X_volume)
  //     {
  //       _X.row(X_rid) = U.row(U_rid);
  //       _action_indices[X_rid] = U_rid;

  //       X_volume = max_volume;
  //       found_larger_volume = true;
  //       break;
  //     }
  //   }

  //   if (!found_larger_volume) { break; }
  // }

  _spanner_bitvec.clear();
  _spanner_bitvec.resize(U.rows(), false);
  for (uint64_t idx : _action_indices) { _spanner_bitvec[idx] = true; }
}

void spanner_state::compute_spanner1(Eigen::MatrixXf& U, size_t _d)
{
  // Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
  // Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf

  // The size of U is K x d, where K is the total number of all actions.
  assert(static_cast<uint64_t>(U.cols()) == _d);
  _X1.setIdentity(_d, _d);

  // Compute a basis contained in U.
  for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  {
    float max_volume = -1.0f;
    uint64_t U_rid = 0;
    find_max_volume(U, X_rid, _X1, max_volume, U_rid);
    _X1.row(X_rid) = U.row(U_rid);
    _action_indices1[X_rid] = U_rid;
  }

  // // Transform the basis into C-approximate spanner.
  // // According to the paper, the total number of iterations needed is O(d*log_c(d)).
  // const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));
  // float X_volume = std::abs(X.determinant());
  // for (int iter = 0; iter < max_iterations; ++iter)
  // {
  //   bool found_larger_volume = false;

  //   // If replacing some row in X results in larger volume, replace it with the row from U.
  //   for (uint64_t X_rid = 0; X_rid < _d; ++X_rid)
  //   {
  //     const auto max_volume_and_row_id = find_max_volume(U, X_rid, X);
  //     const float max_volume = max_volume_and_row_id.first;
  //     if (max_volume > _c * X_volume)
  //     {
  //       uint64_t U_rid = max_volume_and_row_id.second;
  //       X.row(X_rid) = U.row(U_rid);
  //       _action_indices[X_rid] = U_rid;

  //       X_volume = max_volume;
  //       found_larger_volume = true;
  //       break;
  //     }
  //   }

  //   if (!found_larger_volume) { break; }
  // }

  _spanner_bitvec.clear();
  _spanner_bitvec.resize(U.rows(), false);
  for (uint64_t idx : _action_indices) { _spanner_bitvec[idx] = true; }
}

}  // namespace cb_explore_adf
}  // namespace VW