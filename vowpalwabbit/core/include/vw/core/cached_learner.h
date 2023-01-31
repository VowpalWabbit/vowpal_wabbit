#pragma once

#include "vw/core/setup_base.h"
#include "vw/core/vw_fwd.h"

#include <memory>
#include <string>

namespace VW
{
class cached_learner : public setup_base_i
{
public:
  std::shared_ptr<VW::LEARNER::learner> setup_base_learner() override { return _cached; }

  operator bool() const { return !(_cached == nullptr); }

  void delayed_state_attach(VW::workspace& all, VW::config::options_i& options) override
  {
    _options_impl = &options;
    _all_ptr = &all;
  }

  cached_learner() : _cached(nullptr) {}

  cached_learner(std::shared_ptr<VW::LEARNER::learner> learner) : _cached(std::move(learner)) {}

  cached_learner(VW::workspace& all, VW::config::options_i& options, std::shared_ptr<VW::LEARNER::learner> learner)
      : _cached(std::move(learner))
  {
    delayed_state_attach(all, options);
  }

  VW::config::options_i* get_options() override { return _options_impl; }

  VW::workspace* get_all_pointer() override { return _all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn) override { return ""; }

private:
  std::shared_ptr<VW::LEARNER::learner> _cached;
  VW::config::options_i* _options_impl = nullptr;
  VW::workspace* _all_ptr = nullptr;
};
}  // namespace VW
