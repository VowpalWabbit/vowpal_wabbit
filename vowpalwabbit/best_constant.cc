// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "best_constant.h"

#include "loss_functions.h"

#include <cmath>

bool get_best_constant(loss_function* loss_func, shared_data* sd, float& best_constant, float& best_constant_loss)
{
  if (sd->first_observed_label == FLT_MAX ||  // no non-test labels observed or function was never called
      (loss_func == nullptr) || (sd == nullptr))
  { return false; }

  float label1 = sd->first_observed_label;  // observed labels might be inside [sd->Min_label, sd->Max_label], so
                                            // can't use Min/Max
  float label2 = sd->second_observed_label;
  if (label2 == FLT_MAX)
  {
    label2 = 0;  // if only one label observed, second might be 0
  }

  if (label1 > label2)
  {
    float tmp = label1;
    label1 = label2;
    label2 = tmp;
  }  // as don't use min/max - make sure label1 < label2

  float label1_cnt;
  float label2_cnt;

  if (label1 != label2)
  {
    label1_cnt = static_cast<float>(sd->weighted_labels - label2 * sd->weighted_labeled_examples) / (label1 - label2);
    label2_cnt = static_cast<float>(sd->weighted_labeled_examples) - label1_cnt;
  }
  else
  {
    return false;
  }

  if ((label1_cnt + label2_cnt) <= 0.) { return false; }

  auto funcName = loss_func->getType();
  if (funcName == "squared" || funcName == "Huber" || funcName == "classic")
  { best_constant = static_cast<float>(sd->weighted_labels) / static_cast<float>(sd->weighted_labeled_examples); }
  else if (sd->is_more_than_two_labels_observed)
  {
    // loss functions below don't have generic formuas for constant yet.
    return false;
  }
  else if (funcName == "hinge")
  {
    best_constant = label2_cnt <= label1_cnt ? -1.f : 1.f;
  }
  else if (funcName == "logistic")
  {
    label1 = -1.;  // override {-50, 50} to get proper loss
    label2 = 1.;

    if (label1_cnt <= 0) { best_constant = 1.; }
    else if (label2_cnt <= 0)
    {
      best_constant = -1.;
    }
    else
    {
      best_constant = std::log(label2_cnt / label1_cnt);
    }
  }
  else if (funcName == "quantile" || funcName == "pinball" || funcName == "absolute")
  {
    float tau = loss_func->getParameter();
    float q = tau * (label1_cnt + label2_cnt);
    if (q < label2_cnt) { best_constant = label2; }
    else
    {
      best_constant = label1;
    }
  }
  else
  {
    return false;
  }

  if (!sd->is_more_than_two_labels_observed)
  {
    best_constant_loss = (label1_cnt > 0) ? loss_func->getLoss(sd, best_constant, label1) * label1_cnt : 0.0f;
    best_constant_loss += (label2_cnt > 0) ? loss_func->getLoss(sd, best_constant, label2) * label2_cnt : 0.0f;
    best_constant_loss /= label1_cnt + label2_cnt;
  }
  else
  {
    best_constant_loss = FLT_MIN;
  }

  return true;
}
