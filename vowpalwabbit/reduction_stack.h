#pragma once

#include "reductions_fwd.h"

struct vw;

typedef VW::LEARNER::base_learner* (*reduction_setup_fn)(VW::setup_base_i&, VW::config::options_i&, vw&);

namespace VW
{
struct default_reduction_stack_setup : public setup_base_i
{
  default_reduction_stack_setup(vw& all);

  // this function consumes all the reduction_stack until it's able to construct a base_learner
  // same signature as the old setup_base(...) from parse_args.cc
  VW::LEARNER::base_learner* operator()(VW::config::options_i& options, vw& all) override;

private:
  std::vector<std::tuple<std::string, reduction_setup_fn>> reduction_stack;
};
}  // namespace VW
