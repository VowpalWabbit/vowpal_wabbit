#pragma once

#include "vw/core/setup_base.h"
#include "vw/core/vw_fwd.h"

#include <string>

namespace VW
{
class cached_learner : public setup_base_i
{
public:
  VW::LEARNER::base_learner* setup_base_learner() override { return _cached; }

  operator bool() const { return !(_cached == nullptr); }

  void delayed_state_attach(VW::workspace& all, VW::config::options_i& options) override
  {
    _options_impl = &options;
    _all_ptr = &all;
  }

  cached_learner(VW::LEARNER::base_learner* learner = nullptr) : _cached(learner) {}

  cached_learner(VW::workspace& all, VW::config::options_i& options, VW::LEARNER::base_learner* learner = nullptr)
      : _cached(learner)
  {
    delayed_state_attach(all, options);
  }

  VW::config::options_i* get_options() override { return _options_impl; }

  VW::workspace* get_all_pointer() override { return _all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn) override { return ""; }

private:
  VW::LEARNER::base_learner* _cached = nullptr;
  VW::config::options_i* _options_impl = nullptr;
  VW::workspace* _all_ptr = nullptr;
};
}  // namespace VW
