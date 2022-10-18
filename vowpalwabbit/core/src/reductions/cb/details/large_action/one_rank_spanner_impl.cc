// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "../large_action_space.h"

namespace VW
{
namespace cb_explore_adf
{
void one_rank_spanner_state::find_max_volume(
    const Eigen::MatrixXf& U, const Eigen::VectorXf& phi, float& max_volume, uint64_t& U_rid)
{
  // find which action (which row of U) will provide the maximum phi * a volume
  // that a will replace the current row in _X and provide the determinant with the maximum volume
  max_volume = -1.0f;
  U_rid = 0;

  for (auto i = 0; i < U.rows(); ++i)
  {
    float vol = std::abs(U.row(i) * phi);
    if (vol > max_volume)
    {
      max_volume = vol;
      U_rid = i;
    }
  }
}

void one_rank_spanner_state::update_inverse(const Eigen::VectorXf& y, const Eigen::VectorXf& Xi, uint64_t i)
{
  /**
   * update the inverse after the replacement of the ith row of X with y
   * Sherman–Morrison formula
   * -----------------------------
   * X' = X + (y - X.row(i)) e_i.transpose = X + u v.transpose
   * X_inv' = X_inv - 1/(1 + v.transpose X_inv u) (X_inv u) (v.transpose X_inv).transpose
   */

  Eigen::VectorXf u = y - Xi;
  Eigen::VectorXf Xinvu = _X_inv * u;
  Eigen::VectorXf vtopXinv = _X_inv.row(i);
  float vtopXinvu = Xinvu(i);

  _X_inv -= (1.f / (1.f + vtopXinvu)) * (Xinvu * vtopXinv.transpose());
}

void one_rank_spanner_state::scale_all(float max_volume, uint64_t num_examples)
{
  /**
   * Scale inverse and X using the log of det(X):
   * for numerical stability det(X) is always 1 and we maintain
   * _log_determinant_factor which is the log of det(X). So we are accumulating det(X) in logspace
   */

  float thislogdet = (1.f / num_examples) * (std::log(max_volume) - _log_determinant_factor);
  float scale = std::exp(thislogdet);
  _X_inv *= scale;
  _X /= scale;

  _log_determinant_factor += thislogdet;
}

void one_rank_spanner_state::rank_one_determinant_update(
    const Eigen::MatrixXf& U, float max_volume, uint64_t U_rid, float shrink_factor, uint64_t i)
{
  // this is the row from U that will replace the current row in X
  // adapt using shrink factor and log determinant
  Eigen::VectorXf y = U.row(U_rid);
  y /= shrink_factor;
  y /= std::exp(_log_determinant_factor);

  update_inverse(y, _X.row(i), i);

  _X.row(i) = y;
  _action_indices[i] = U_rid;

  scale_all(max_volume, U.rows());
}

void one_rank_spanner_state::compute_spanner(
    const Eigen::MatrixXf& U, size_t num_actions_in_spanner, const std::vector<float>& shrink_factors)
{
  /**
   * Implements the C-approximate barycentric spanner algorithm in Figure 2 of the following paper
   * Awerbuch & Kleinberg STOC'04: https://www.cs.cornell.edu/~rdk/papers/OLSP.pdf
   *
   * One rank determinant update approach:
   * -------------------------------------
   * following Sherman–Morrison formula and matrix determinant lemma, updating the inverse and determinants in a
   * one-rank fashion
   * X'_a <- replace row i of X with action a where det(X)=1 **
   * X'_a = X + (a - X.row(i)) e_i.transpose = X + u v.transpose (i.e. e_i== v)
   * det(X'_a) = det(X) (1 + e_i.transpose * X_inverse (a - X.row(i)))
   *               = (1 - (X_inverse.transpose() e_i).transpose X.row(i)) + (X_inverse.transpose() e_i).transpose * a
   *               = 0 + phi.transpose * a (where phi == X_inverse.transpose() e_i -> essentially _X_inv at row i)
   *
   * ** for numerical stability det(X) is always 1 (hence the simplification in the above equation) and we maintain
   * _log_determinant_factor which is the log of det(X). So we are accumulating det(X) in the logspace
   */

  // The size of U is K x d, where K is the total number of all actions
  assert(static_cast<uint64_t>(U.cols()) >= num_actions_in_spanner);
  _X.setIdentity(num_actions_in_spanner, U.cols());
  _X_inv.setIdentity(num_actions_in_spanner, U.cols());
  _log_determinant_factor = 0;

  float max_volume{};
  // Compute a basis contained in U.
  for (uint64_t i = 0; i < num_actions_in_spanner; ++i)
  {
    Eigen::VectorXf phi = _X_inv.row(i);
    uint64_t U_rid;
    find_max_volume(U, phi, max_volume, U_rid);

    // best action is U_rid
    rank_one_determinant_update(U, max_volume, U_rid, shrink_factors[U_rid], i);
  }

  const int max_iterations = static_cast<int>(num_actions_in_spanner * std::log(num_actions_in_spanner) / std::log(_c));
  float X_volume = max_volume;

  for (int iter = 0; iter < max_iterations; ++iter)
  {
    bool found_larger_volume = false;

    // If replacing some row in _X results in larger volume, replace it with the row from U.
    for (uint64_t i = 0; i < num_actions_in_spanner; ++i)
    {
      uint64_t U_rid;
      Eigen::VectorXf phi = _X_inv.row(i);
      find_max_volume(U, phi, max_volume, U_rid);
      if (max_volume > _c * X_volume)
      {
        // best action is U_rid
        rank_one_determinant_update(U, max_volume, U_rid, shrink_factors[U_rid], i);

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

bool one_rank_spanner_state::is_action_in_spanner(uint32_t action) { return _spanner_bitvec[action]; }

size_t one_rank_spanner_state::spanner_size() { return _spanner_bitvec.size(); }

void one_rank_spanner_state::_test_only_set_rank(uint64_t rank) { _action_indices.resize(rank); }

}  // namespace cb_explore_adf
}  // namespace VW