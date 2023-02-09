#pragma once

#include "vw/core/setup_base.h"
#include "vw/core/vw_fwd.h"

#include <tuple>
#include <unordered_map>
#include <vector>

namespace VW
{
class default_reduction_stack_setup : public setup_base_i
{
public:
  default_reduction_stack_setup(VW::workspace& all, VW::config::options_i& options);
  // using this constructor implies a later call into delayed_state_attach
  // see parse_args.cc:instantiate_learner(..)
  default_reduction_stack_setup();

  void delayed_state_attach(VW::workspace& all, VW::config::options_i& options) override;

  // This function consumes all the reduction_stack until it's able to construct a learner
  // Same signature as the old setup_base(...) from parse_args.cc
  std::shared_ptr<VW::LEARNER::learner> setup_base_learner() override;

  VW::config::options_i* get_options() override { return _options_impl; }

  VW::workspace* get_all_pointer() override { return _all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn setup) override;

private:
  VW::config::options_i* _options_impl = nullptr;
  VW::workspace* _all_ptr = nullptr;
  std::shared_ptr<VW::LEARNER::learner> _base;

protected:
  std::vector<std::tuple<std::string, reduction_setup_fn>> _reduction_stack;
  std::unordered_map<reduction_setup_fn, std::string> _setup_name_map;
};
}  // namespace VW
