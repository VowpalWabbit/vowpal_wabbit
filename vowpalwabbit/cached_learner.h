#pragma once

#include "global_data.h"  // to get vw struct
#include "config/options.h"  // to get options_i

namespace VW
{
struct cached_learner : public setup_base_i
{
  VW::LEARNER::base_learner* setup_base_learner() override { return _cached; }

  operator bool() const { return !(_cached == nullptr); }

  void delayed_state_attach(VW::workspace& all, VW::config::options_i& options) override
  {
    options_impl = &options;
    all_ptr = &all;
  }

  cached_learner(VW::LEARNER::base_learner* learner = nullptr) : _cached(learner) {}

  cached_learner(VW::workspace& all, VW::config::options_i& options, VW::LEARNER::base_learner* learner = nullptr)
      : _cached(learner)
  {
    delayed_state_attach(all, options);
  }

  VW::config::options_i* get_options() override { return options_impl; }

  VW::workspace* get_all_pointer() override { return all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn) override { return ""; }

private:
  VW::LEARNER::base_learner* _cached = nullptr;
  VW::config::options_i* options_impl = nullptr;
  VW::workspace* all_ptr = nullptr;
};
}  // namespace VW
