// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
// Forward declaration header to declare the basic components used by VW reductions

#include <vector>

// forward declarations
struct example;
using multi_ex = std::vector<example*>;
template <class T>
struct v_array;
struct random_state;
struct vw;

namespace LEARNER
{
template <class T, class E>
struct learner;
using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;
}  // namespace LEARNER

namespace VW
{
namespace config
{
struct options_i;
}  // namespace config
}  // namespace VW
