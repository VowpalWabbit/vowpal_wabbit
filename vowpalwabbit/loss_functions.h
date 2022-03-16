// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <memory>
#include <string>

struct shared_data;
namespace VW
{
struct workspace;
}

class loss_function
{
public:
  // Identifies the type of the implementing loss function, matches the name used in getLossFunction.
  virtual std::string getType() const = 0;
  virtual float getParameter() const { return 0.f; }

  // Returns the example loss value.
  virtual float getLoss(const shared_data*, float prediction, float label) const = 0;

  // Returns the update scalar.
  virtual float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const = 0;
  virtual float getUnsafeUpdate(float prediction, float label, float eta_t) const = 0;

  virtual float getSquareGrad(float prediction, float label) const = 0;
  virtual float first_derivative(const shared_data*, float prediction, float label) const = 0;
  virtual float second_derivative(const shared_data*, float prediction, float label) const = 0;

  virtual ~loss_function() = default;
};

std::unique_ptr<loss_function> getLossFunction(
    VW::workspace&, const std::string& funcName, float function_parameter = 0);
