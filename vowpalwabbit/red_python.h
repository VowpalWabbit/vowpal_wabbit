// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <vector>
#include "reductions_fwd.h"
#include "io_buf.h"

namespace RED_PYTHON
{
class ExternalBinding
{
public:
  virtual void SetBaseLearner(void* learner) = 0;
  virtual void ActualLearn(example*) = 0;
  virtual void ActualPredict(example*) = 0;
  virtual void ActualSaveLoad(io_buf* model_file, bool read, bool text) = 0;
  virtual bool ShouldRegisterFinishExample() = 0;
  virtual bool ShouldRegisterSaveLoad() = 0;
  virtual void ActualFinishExample(example*) = 0;

  virtual void ActualLearn(multi_ex*) = 0;
  virtual void ActualPredict(multi_ex*) = 0;
  virtual void ActualFinishExample(multi_ex*) = 0;

  virtual ~ExternalBinding(){};
};
}  // namespace RED_PYTHON

VW::LEARNER::base_learner* red_python_setup(VW::setup_base_i&, std::unique_ptr<RED_PYTHON::ExternalBinding>);
VW::LEARNER::base_learner* red_python_setup_normie(VW::setup_base_i&);
VW::LEARNER::base_learner* red_python_multiline_setup(VW::setup_base_i&);
VW::LEARNER::base_learner* red_python_base_setup(VW::setup_base_i&);
