// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/learner_fwd.h"
#include "vw/core/multi_ex.h"
#include "vw/core/slates_label.h"
#include "vw/core/vw_fwd.h"

#include <memory>
#include <string>
#include <vector>

namespace VW
{
namespace reductions
{
class slates_data
{
public:
  void learn(VW::LEARNER::learner& base, multi_ex& examples);
  void predict(VW::LEARNER::learner& base, multi_ex& examples);

private:
  std::vector<slates::label> _stashed_labels;

  /*
  The primary job of this reduction is to convert slate labels to a form CCB can process.
  Note: Shared must come before action and slot, and all action examples must come before
  slot examples.
  This is an example of this conversion in the form of textual labels:
    slates shared 0.8
    slates action 0
    slates action 0
    slates action 0
    slates action 1
    slates action 1
    slates slot 1:0.8
    slates slot 0:0.6

    ccb shared
    ccb action
    ccb action
    ccb action
    ccb action
    ccb action
    ccb slot 1:0.8:0.8 0,1,2
    ccb slot 3:0.8:0.6 3,4
  */
  template <bool is_learn>
  void learn_or_predict(VW::LEARNER::learner& base, multi_ex& examples);
};

std::shared_ptr<VW::LEARNER::learner> slates_setup(VW::setup_base_i&);
std::string generate_slates_label_printout(const std::vector<example*>& slots);
}  // namespace reductions
}  // namespace VW