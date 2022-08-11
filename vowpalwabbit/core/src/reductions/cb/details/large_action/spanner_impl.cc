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

Eigen::VectorXf get_row_to_replace(Eigen::VectorXf y, float shrink_factor, float log_determinant_factor)
{
  y /= shrink_factor;
  y /= std::exp(log_determinant_factor);
  return y;
}

void update_inverse(Eigen::MatrixXf& Inv, const Eigen::VectorXf& y, const Eigen::VectorXf& Xi, uint64_t i)
{
  Eigen::VectorXf u = y - Xi;
  Eigen::MatrixXf Sinvu = Inv * u;
  Eigen::VectorXf vtopSinv = Inv.row(i);
  float vtopSinvu = Sinvu(i, 0);

  Inv -= (1.f / (1.f + vtopSinvu)) * (Sinvu * vtopSinv.transpose());
}

void update_all(
    Eigen::MatrixXf& Inv, Eigen::MatrixXf& X, float& log_determinant_factor, float max_volume, uint64_t num_examples)
{
  float thislogdet = (1.f / num_examples) * (std::log(max_volume) - log_determinant_factor);
  float scale = std::exp(thislogdet);
  Inv *= scale;
  X /= scale;

  log_determinant_factor += thislogdet;
}

void spanner_state::compute_spanner(const Eigen::MatrixXf& U, size_t _d, const std::vector<float>& shrink_factors)
{
  assert(static_cast<uint64_t>(U.cols()) == _d);
  _X.setIdentity(_d, _d);
  _Inv.setIdentity(_d, _d);
  float log_determinant_factor = 0;

  float max_volume;
  // Compute a basis contained in U.
  for (uint64_t i = 0; i < _d; ++i)
  {
    Eigen::VectorXf Sinvtopei = _Inv.row(i);
    // max volume returns the various determinants
    max_volume = -1.0f;
    uint64_t U_rid = 0;
    find_fast_max_vol(U, Sinvtopei, max_volume, U_rid);

    // best action is U_rid
    Eigen::VectorXf y = get_row_to_replace(U.row(U_rid), shrink_factors[U_rid], log_determinant_factor);
    update_inverse(_Inv, y, _X.row(i), i);

    _X.row(i) = y;
    _action_indices[i] = U_rid;

    update_all(_Inv, _X, log_determinant_factor, max_volume, U.rows());
  }

  const int max_iterations = static_cast<int>(_d * std::log(_d));
  float X_volume = max_volume;

  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in _X results in larger volume, replace it with the row from U.
    for (uint64_t i = 0; i < _d; ++i)
    {
      float max_volume = -1.0f;
      uint64_t U_rid = 0;
      Eigen::VectorXf Sinvtopei = _Inv.row(i);
      find_fast_max_vol(U, Sinvtopei, max_volume, U_rid);
      if (max_volume > _c * X_volume)
      {
        // best action is U_rid
        Eigen::VectorXf y = get_row_to_replace(U.row(U_rid), shrink_factors[U_rid], log_determinant_factor);
        update_inverse(_Inv, y, _X.row(i), i);

        _X.row(i) = y;
        _action_indices[i] = U_rid;

        update_all(_Inv, _X, log_determinant_factor, max_volume, U.rows());

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