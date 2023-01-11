// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

// Eigen explicit vectorization may not work well with AVX2 when using smaller MAX_ALIGN_BYTES.
// For more info:
// https://eigen.tuxfamily.org/dox/TopicPreprocessorDirectives.html#TopicPreprocessorDirectivesPerformance
#define EIGEN_MAX_ALIGN_BYTES 32

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <set>

namespace VW
{
constexpr float NORM_THRESHOLD = 0.0001f;

template <typename WeightsT>
uint64_t gram_schmidt(WeightsT& weights, uint64_t d, const std::set<uint64_t>& rows)
{
  auto max_col = d;
  for (uint64_t j = 0; j < d; j++)
  {
    for (uint64_t k = 0; k < j; k++)
    {
      float temp = 0;

      for (auto i : rows)
      {
        auto strided_index_j = i + j;
        auto strided_index_k = i + k;
        temp += (static_cast<double>(weights[strided_index_j])) * weights[strided_index_k];
      }
      for (auto i : rows)
      {
        auto strided_index_j = i + j;
        auto strided_index_k = i + k;
        (weights[strided_index_j]) -= static_cast<float>(temp) * weights[strided_index_k];
      }
    }
    double norm = 0;
    for (auto i : rows)
    {
      auto strided_index_j = i + j;
      norm += (static_cast<double>(weights[strided_index_j])) * weights[strided_index_j];
    }

    norm = std::sqrt(norm);
    if (norm < NORM_THRESHOLD)
    {
      for (uint64_t k = j; k < d; ++k)
      {
        for (auto i : rows)
        {
          auto strided_index_k = i + k;
          weights[strided_index_k] = 0.f;
        }
      }
      max_col = j - 1;
      break;
    }
    for (auto i : rows)
    {
      auto strided_index_j = i + j;
      weights[strided_index_j] /= static_cast<float>(norm);
    }
  }
  return max_col;
}

void gram_schmidt(Eigen::MatrixXf& mat);
void gram_schmidt(Eigen::SparseMatrix<float>& mat);
}  // namespace VW