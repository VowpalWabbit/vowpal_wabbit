// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"

#include <memory>
#include <string>

namespace VW
{
class setup_base_i;

// use raw function pointer here instead of std::function because
// this type should be hashable to be used in std::unordered_map
using reduction_setup_fn = std::shared_ptr<VW::LEARNER::learner> (*)(VW::setup_base_i&);

class setup_base_i
{
public:
  virtual void delayed_state_attach(VW::workspace&, VW::config::options_i&) = 0;

  virtual std::shared_ptr<VW::LEARNER::learner> setup_base_learner() = 0;

  // this one we can share freely
  virtual VW::config::options_i* get_options() = 0;

  // in reality we would want to be more specific than this
  // to start hiding global state away
  virtual VW::workspace* get_all_pointer() = 0;

  virtual std::string get_setupfn_name(reduction_setup_fn setup) = 0;

  virtual ~setup_base_i() = default;
};

}  // namespace VW