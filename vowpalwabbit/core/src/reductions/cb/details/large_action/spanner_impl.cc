// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"

namespace VW
{
namespace cb_explore_adf
{
void find_fast_max_vol(const Eigen::MatrixXf& U, const Eigen::VectorXf& Sinvtopei, float& max_volume, uint64_t& U_rid)
{
  for (auto i = 0; i < U.rows(); ++i)
  {
    float vol = std::abs(U.row(i) * Sinvtopei);
    if (vol > max_volume)
    {
      max_volume = vol;
      U_rid = i;
    }
  }
}

Eigen::VectorXf get_row_to_replace(Eigen::VectorXf y, float shrink_factor, Eigen::VectorXf log_determinant_factor)
{
  y /= shrink_factor;

  Eigen::VectorXf pow = Eigen::exp(log_determinant_factor.array());
  pow = pow.cwiseInverse();
  return y.cwiseProduct(pow);
}

void update_inverse(Eigen::MatrixXf& _S_inv, const Eigen::VectorXf& y, const Eigen::VectorXf& Si, uint64_t i)
{
  Eigen::VectorXf u = y - Si;
  Eigen::MatrixXf Sinvu = _S_inv * u;
  Eigen::VectorXf vtopSinv = _S_inv.row(i);
  float vtopSinvu = Sinvu(i, 0);

  _S_inv -= (1.f / (1.f + vtopSinvu)) * (Sinvu * vtopSinv.transpose());
}

void update_all(
    Eigen::MatrixXf& _S_inv, Eigen::MatrixXf& _X, Eigen::VectorXf& log_determinant_factor, uint64_t _d, float max_volume)
{
  Eigen::VectorXf thislogdet = (1.f / _d) * std::log(max_volume) - log_determinant_factor.array();
  Eigen::VectorXf scale = Eigen::exp(thislogdet.array());
  for (uint64_t col = 0; col < _d; col++) { _S_inv.col(col) = _S_inv.col(col).cwiseProduct(scale); }

  scale = scale.cwiseInverse();
  for (uint64_t col = 0; col < _d; col++) { _X.col(col) = _X.col(col).cwiseProduct(scale); }

  log_determinant_factor += thislogdet;
}

void spanner_state::compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors)
{
  assert(static_cast<uint64_t>(U.cols()) == _d);
  _X.setIdentity(_d, _d);
  _S_inv.setIdentity(_d, _d);
  Eigen::VectorXf log_determinant_factor(_d);
  log_determinant_factor.setZero();

  // Compute a basis contained in U.
  for (uint64_t i = 0; i < _d; ++i)
  {
    Eigen::VectorXf Sinvtopei = _S_inv.row(i);
    // max volume returns the various determinants
    float max_volume = -1.0f;
    uint64_t U_rid = 0;
    find_fast_max_vol(U, Sinvtopei, max_volume, U_rid);

    // best action is U_rid
    Eigen::VectorXf y = get_row_to_replace(U.row(U_rid), shrink_factors[U_rid], log_determinant_factor);
    update_inverse(_S_inv, y, _X.row(i), i);

    _X.row(i) = y;
    _action_indices[i] = U_rid;

    update_all(_S_inv, _X, log_determinant_factor, _d, max_volume);
  }

  const int max_iterations = static_cast<int>(_d * std::log(_d) / std::log(_c));

  // TODO replace this with the one-rank determinant (max_vol?)
  float X_volume = std::abs(_X.determinant());

  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in _X results in larger volume, replace it with the row from U.
    for (uint64_t i = 0; i < _d; ++i)
    {
      float max_volume = -1.0f;
      uint64_t U_rid = 0;
      Eigen::VectorXf Sinvtopei = _S_inv.row(i);
      find_fast_max_vol(U, Sinvtopei, max_volume, U_rid);
      if (max_volume > _c * X_volume)
      {
        // best action is U_rid
        Eigen::VectorXf y = get_row_to_replace(U.row(U_rid), shrink_factors[U_rid], log_determinant_factor);
        update_inverse(_S_inv, y, _X.row(i), i);

        _X.row(i) = y;
        _action_indices[i] = U_rid;

        update_all(_S_inv, _X, log_determinant_factor, _d, max_volume);

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

void spanner_state::compute_spanner1(const Eigen::MatrixXf& U, size_t _d)
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

}  // namespace cb_explore_adf
}  // namespace VW