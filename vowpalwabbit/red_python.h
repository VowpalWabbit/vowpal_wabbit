// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <string>
#include <vector>
#include "reductions_fwd.h"

namespace RED_PYTHON
{
  //this should live elsewhere?
  class ExternalBinding
  {
  public:
    //not needed added for debugging
    virtual void SetBaseLearner(void* learner) = 0;
    virtual void ActualLearn(example *) = 0;
    virtual void ActualPredict(example *) = 0;

    virtual bool ShouldRegisterUpdate() const { return false; }
    virtual void ActualUpdate(example*) {}

    virtual bool ShouldRegisterSensitivity() const { return false; }
    virtual float ActualSensitivity(example *) { return 0; }

    virtual bool ShouldRegisterFinish() const { return false; }
    virtual void ActualFinish() {}

    virtual bool ShouldRegisterEndPass() const { return false; }
    virtual void ActualEndPass() {}

    virtual bool ShouldRegisterEndExamples() const { return false; }
    virtual void ActualEndExamples() {}

    virtual bool ShouldRegisterFinishExample() const { return false; }
    virtual void ActualFinishExample(example *) {}

    /*
    // How to handle the inputs here?
    virtual bool ShouldRegisterSaveLoad() { return false; }
    virtual void ActualSaveLoad(io_buf&, bool, bool) {}
    */
    
    virtual ~ExternalBinding() = default;
  };
}  // namespace RED_PYTHON


VW::LEARNER::base_learner* red_python_setup(VW::config::options_i& options, vw& all, const std::string& name, RED_PYTHON::ExternalBinding* ext_binding);
VW::LEARNER::base_learner* red_python_base_setup(VW::config::options_i& options, vw& all, const std::string& name, RED_PYTHON::ExternalBinding* ext_binding);
