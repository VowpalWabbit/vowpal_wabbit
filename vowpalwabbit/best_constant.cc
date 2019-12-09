// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "best_constant.h"

bool get_best_constant(vw& all, float& best_constant, float& best_constant_loss)
{
  if (all.sd->first_observed_label == FLT_MAX ||  // no non-test labels observed or function was never called
      (all.loss == nullptr) || (all.sd == nullptr))
    return false;

  float label1 = all.sd->first_observed_label;  // observed labels might be inside [sd->Min_label, sd->Max_label], so
                                                // can't use Min/Max
  float label2 = (all.sd->second_observed_label == FLT_MAX)
      ? 0
      : all.sd->second_observed_label;  // if only one label observed, second might be 0
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
    label1_cnt = (float)(all.sd->weighted_labels - label2 * all.sd->weighted_labeled_examples) / (label1 - label2);
    label2_cnt = (float)all.sd->weighted_labeled_examples - label1_cnt;
  }
  else
    return false;

  if ((label1_cnt + label2_cnt) <= 0.)
    return false;

  auto funcName = all.loss->getType();
  if (funcName.compare("squared") == 0 || funcName.compare("Huber") == 0 || funcName.compare("classic") == 0)
    best_constant = (float)all.sd->weighted_labels / (float)(all.sd->weighted_labeled_examples);
  else if (all.sd->is_more_than_two_labels_observed)
  {
    // loss functions below don't have generic formuas for constant yet.
    return false;
  }
  else if (funcName.compare("hinge") == 0)
  {
    best_constant = label2_cnt <= label1_cnt ? -1.f : 1.f;
  }
  else if (funcName.compare("logistic") == 0)
  {
    label1 = -1.;  // override {-50, 50} to get proper loss
    label2 = 1.;

    if (label1_cnt <= 0)
      best_constant = 1.;
    else if (label2_cnt <= 0)
      best_constant = -1.;
    else
      best_constant = log(label2_cnt / label1_cnt);
  }
  else if (funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0)
  {
    float tau = 0.5;

    if (all.options->was_supplied("quantile_tau"))
      tau = all.options->get_typed_option<float>("quantile_tau").value();

    float q = tau * (label1_cnt + label2_cnt);
    if (q < label2_cnt)
      best_constant = label2;
    else
      best_constant = label1;
  }
  else
    return false;

  if (!all.sd->is_more_than_two_labels_observed)
  {
    best_constant_loss = (label1_cnt > 0) ? all.loss->getLoss(all.sd, best_constant, label1) * label1_cnt : 0.0f;
    best_constant_loss += (label2_cnt > 0) ? all.loss->getLoss(all.sd, best_constant, label2) * label2_cnt : 0.0f;
    best_constant_loss /= label1_cnt + label2_cnt;
  }
  else
    best_constant_loss = FLT_MIN;

  return true;
}
