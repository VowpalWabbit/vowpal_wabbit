// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"
#include "io_buf.h"

namespace wrapper
{
class external_binding
{
public:
  virtual bool should_register_finish_example() = 0;
  virtual bool should_register_saveload() = 0;
  virtual void set_base_learner(void* learner) = 0;
  virtual void actual_learn(example*) = 0;
  virtual void actual_learn(multi_ex*) = 0;
  virtual void actual_predict(example*) = 0;
  virtual void actual_predict(multi_ex*) = 0;
  virtual void actual_finish_example(example*) = 0;
  virtual void actual_finish_example(multi_ex*) = 0;
  virtual void actual_saveload(io_buf* model_file, bool read, bool text) = 0;

  virtual ~external_binding(){};
};
}  // namespace wrapper

VW::LEARNER::base_learner* wrapper_reduction_setup(VW::setup_base_i&, std::unique_ptr<wrapper::external_binding>);
VW::LEARNER::base_learner* wrapper_reduction_multiline_setup(
    VW::setup_base_i&, std::unique_ptr<wrapper::external_binding>);
VW::LEARNER::base_learner* wrapper_reduction_base_setup(VW::setup_base_i&, std::unique_ptr<wrapper::external_binding>);
