// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw_fwd.h"

#include <memory>
#include <string>

// Information on how to implement a custom loss function in Loss functions · VowpalWabbit/vowpal_wabbit Wiki https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Loss-functions#how-to-write-my-own-loss-function
class loss_function
{
public:
  // Identifies the type of the implementing loss function, matches the name used in getLossFunction.
  virtual std::string getType() const = 0;
  virtual float getParameter() const { return 0.f; }

  // Returns the example loss value.
  virtual float getLoss(const shared_data*, float prediction, float label) const = 0;

  // Returns the update scalar.
  // Based on the invariant update calculation from the Online Importance Weight Aware Updates paper https://arxiv.org/abs/1011.1576
  virtual float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const = 0;
  virtual float getUnsafeUpdate(float prediction, float label, float update_scale) const = 0;

  virtual float getSquareGrad(float prediction, float label) const = 0;
  virtual float first_derivative(const shared_data*, float prediction, float label) const = 0;
  virtual float second_derivative(const shared_data*, float prediction, float label) const = 0;

  virtual ~loss_function() = default;
};

std::unique_ptr<loss_function> getLossFunction(
    VW::workspace&, const std::string& funcName, float function_parameter = 0);
