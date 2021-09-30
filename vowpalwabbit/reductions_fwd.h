// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
// Forward declaration header to declare the basic components used by VW reductions

#include <vector>
#include "v_array.h"

// forward declarations
struct example;
using multi_ex = std::vector<example*>;
struct random_state;


namespace VW
{
struct workspace;
namespace LEARNER
{
template <class T, class E>
struct learner;
using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;
}  // namespace LEARNER
namespace config
{
struct options_i;
}  // namespace config

struct setup_base_i;
typedef VW::LEARNER::base_learner* (*reduction_setup_fn)(VW::setup_base_i&);

struct setup_base_i
{
  virtual void delayed_state_attach(VW::workspace&, VW::config::options_i&) = 0;

  virtual VW::LEARNER::base_learner* setup_base_learner() = 0;

  // this one we can share freely
  virtual VW::config::options_i* get_options() = 0;

  // in reality we would want to be more specific than this
  // to start hiding global state away
  virtual VW::workspace* get_all_pointer() = 0;

  virtual std::string get_setupfn_name(reduction_setup_fn setup) = 0;

  virtual ~setup_base_i() = default;
};
}  // namespace VW
