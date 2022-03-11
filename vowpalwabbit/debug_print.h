// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <iostream>
#include <utility>
#include <vector>

#include "v_array.h"

namespace VW
{
struct example;
struct example_predict;
using multi_ex = std::vector<example*>;
}

namespace std
{
template <class T, class U>
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair)
{
  os << pair.first << ':' << pair.second;
  return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  os << '[';
  for (const auto& i : vec) os << ' ' << i;
  os << " ]";
  return os;
}
}  // namespace std

namespace VW
{
namespace debug
{
std::string simple_label_to_string(const example& ec);
std::string cb_label_to_string(const example& ec);
std::string scalar_pred_to_string(const example& ec);
std::string a_s_pred_to_string(const example& ec);
std::string prob_dist_pred_to_string(const example& ec);
std::string multiclass_pred_to_string(const example& ec);
std::string features_to_string(const example_predict& ec);
std::string debug_depth_indent_string(const multi_ex& ec);
std::string debug_depth_indent_string(const example& ec);
std::string debug_depth_indent_string(int32_t stack_depth);

}  // namespace debug
}  // namespace VW