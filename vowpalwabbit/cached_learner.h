#pragma once

#include "global_data.h"  // to get vw struct
#include "options.h"      // to get options_i

namespace VW
{
struct cached_learner : public setup_base_i
{
  VW::LEARNER::base_learner* setup_base_learner() override { return _cached; }

  operator bool() const { return !(_cached == nullptr); }

  cached_learner(vw& all, VW::config::options_i& options, VW::LEARNER::base_learner* learner = nullptr)
      : _cached(learner)
  {
    options_impl = &options;
    all_ptr = &all;
  }

  VW::config::options_i* get_options() override { return options_impl; }

  vw* get_all_pointer() override { return all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn) override { return ""; }

private:
  VW::LEARNER::base_learner* _cached = nullptr;
  VW::config::options_i* options_impl = nullptr;
  vw* all_ptr = nullptr;
};
}  // namespace VW
