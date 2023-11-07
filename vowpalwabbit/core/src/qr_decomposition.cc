// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "qr_decomposition.h"

namespace VW
{
void gram_schmidt(Eigen::MatrixXf& mat)
{
  for (Eigen::Index i = 0; i < mat.cols(); ++i)
  {
    for (Eigen::Index j = 0; j < i; ++j)
    {
      float r = mat.col(i).dot(mat.col(j));
      mat.col(i) -= r * mat.col(j);
    }
    float norm = mat.col(i).norm();
    if (norm < NORM_THRESHOLD)
    {
      for (Eigen::Index k = i; k < mat.cols(); ++k) { mat.col(k).setZero(); }
      return;
    }
    mat.col(i) *= (1.f / norm);
  }
}

void gram_schmidt(Eigen::SparseMatrix<float>& mat)
{
  for (Eigen::Index i = 0; i < mat.outerSize(); ++i)
  {
    for (Eigen::Index j = 0; j < i; ++j)
    {
      float r = mat.innerVector(i).dot(mat.innerVector(j));
      mat.innerVector(i) -= r * mat.innerVector(j);
    }
    float norm = mat.innerVector(i).norm();
    if (norm < NORM_THRESHOLD)
    {
      for (Eigen::SparseMatrix<float>::InnerIterator it(mat, i); it; ++it) { mat.coeffRef(it.row(), it.col()) = 0.f; }
    }
    mat.innerVector(i) *= (1.f / norm);
  }
}
}  // namespace VW