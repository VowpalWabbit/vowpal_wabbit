// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <string>
#include "parse_primitives.h"

struct shared_data;
struct vw;

class loss_function
{
 public:
  // Identifies the type of the implementing loss function, matches the name used in getLossFunction.
  virtual std::string getType() = 0;

  /*
   * getLoss evaluates the example loss.
   * The function returns the loss value
   */
  // virtual float getLoss(example *&ec, gd_vars &vars) = 0;
  virtual float getLoss(shared_data*, float prediction, float label) = 0;

  /*
   * getUpdate evaluates the update scalar
   * The function return the update scalar
   */
  virtual float getUpdate(float prediction, float label, float update_scale, float pred_per_update) = 0;
  virtual float getUnsafeUpdate(float prediction, float label, float eta_t) = 0;

  // the number of examples of the opposite label such that updating with
  // that number results in the opposite label.
  // 0 = prediction + pred_per_update
  //      * getUpdate(prediction, opposite, pred_per_update*getRevertingWeight(), pred_per_update)
  virtual float getRevertingWeight(shared_data*, float prediction, float eta_t) = 0;
  virtual float getSquareGrad(float prediction, float label) = 0;
  virtual float first_derivative(shared_data*, float prediction, float label) = 0;
  virtual float second_derivative(shared_data*, float prediction, float label) = 0;
  virtual ~loss_function(){};
};

loss_function* getLossFunction(vw&, std::string funcName, float function_parameter = 0);
