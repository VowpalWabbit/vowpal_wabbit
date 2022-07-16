// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <Eigen/Dense>
#include <Eigen/SparseCore>

namespace VW
{
void gram_schmidt(Eigen::MatrixXf& mat);
void gram_schmidt(Eigen::SparseMatrix<float>& mat);
}  // namespace VW