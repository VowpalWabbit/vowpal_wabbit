// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

#include <memory>
#include <string>

namespace VW
{
// Information on how to implement a custom loss function in Loss functions · VowpalWabbit/vowpal_wabbit Wiki
// https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Loss-functions#how-to-write-my-own-loss-function
class loss_function
{
public:
  // Identifies the type of the implementing loss function, matches the name used in get_loss_function.
  virtual std::string get_type() const = 0;
  virtual float get_parameter() const { return 0.f; }

  // Returns the example loss value.
  virtual float get_loss(const shared_data*, float prediction, float label) const = 0;

  // Returns the update scalar.
  // Based on the invariant update calculation from the Online Importance Weight Aware Updates paper
  // https://arxiv.org/abs/1011.1576
  virtual float get_update(float prediction, float label, float update_scale, float pred_per_update) const = 0;
  virtual float get_unsafe_update(float prediction, float label, float update_scale) const = 0;

  virtual float get_square_grad(float prediction, float label) const = 0;
  virtual float first_derivative(const shared_data*, float prediction, float label) const = 0;
  virtual float second_derivative(const shared_data*, float prediction, float label) const = 0;

  virtual ~loss_function() = default;
};

std::unique_ptr<loss_function> get_loss_function(
    VW::workspace&, const std::string& funcName, float function_parameter_0 = -1.0f, float function_parameter_1 = 1.0f);
}  // namespace VW
