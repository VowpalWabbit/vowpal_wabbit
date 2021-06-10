#pragma once

#include "reductions_fwd.h"

struct vw;

typedef VW::LEARNER::base_learner* (*reduction_setup_fn)(VW::setup_base_fn&, VW::config::options_i&, vw&);

// TODO: to be deleted; just to test refactor
void instantiate_learner(VW::config::options_i& options, vw& all);

namespace VW
{
struct status_quo : public setup_base_fn
{
  std::vector<std::string> enabled_reductions;

  void print_enabled_reductions(vw& all);

  status_quo(vw& all);

  // this function consumes all the reduction_stack until it's able to construct a base_learner
  // same signature as the old setup_base(...) from parse_args.cc
  VW::LEARNER::base_learner* operator()(VW::config::options_i& options, vw& all);

private:
  std::vector<std::tuple<std::string, reduction_setup_fn>> reduction_stack;
};
}  // namespace VW
