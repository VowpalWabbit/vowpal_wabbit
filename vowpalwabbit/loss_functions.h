// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <string>
#include <memory>

#include "io/logger.h"

struct shared_data;
namespace VW
{
struct workspace;
}

class loss_function
{
public:
  // Identifies the type of the implementing loss function, matches the name used in getLossFunction.
  virtual std::string getType() = 0;
  virtual float getParameter() { return 0.f; }

  // Returns the example loss value.
  virtual float getLoss(shared_data*, float prediction, float label) = 0;

  // Returns the update scalar.
  virtual float getUpdate(float prediction, float label, float update_scale, float pred_per_update) = 0;
  virtual float getUnsafeUpdate(float prediction, float label, float eta_t) = 0;

  virtual float getSquareGrad(float prediction, float label) = 0;
  virtual float first_derivative(shared_data*, float prediction, float label) = 0;
  virtual float second_derivative(shared_data*, float prediction, float label) = 0;

  virtual ~loss_function() = default;
};

std::unique_ptr<loss_function> getLossFunction(
    VW::workspace&, const std::string& funcName, float function_parameter = 0);
