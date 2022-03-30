#pragma once

#include "setup_base.h"
#include "vw_fwd.h"

#include <tuple>
#include <vector>

using reduction_setup_fn = VW::LEARNER::base_learner* (*)(VW::setup_base_i&);

namespace VW
{
struct default_reduction_stack_setup : public setup_base_i
{
  default_reduction_stack_setup(VW::workspace& all, VW::config::options_i& options);
  // using this constructor implies a later call into delayed_state_attach
  // see parse_args.cc:instantiate_learner(..)
  default_reduction_stack_setup();

  void delayed_state_attach(VW::workspace& all, VW::config::options_i& options) override;

  // this function consumes all the reduction_stack until it's able to construct a base_learner
  // same signature as the old setup_base(...) from parse_args.cc
  VW::LEARNER::base_learner* setup_base_learner() override;

  VW::config::options_i* get_options() override { return options_impl; }

  VW::workspace* get_all_pointer() override { return all_ptr; }

  std::string get_setupfn_name(reduction_setup_fn setup) override;

private:
  VW::config::options_i* options_impl = nullptr;
  VW::workspace* all_ptr = nullptr;

protected:
  std::vector<std::tuple<std::string, reduction_setup_fn>> reduction_stack;
};
}  // namespace VW
