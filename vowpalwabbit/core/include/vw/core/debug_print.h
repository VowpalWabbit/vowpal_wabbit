// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <ostream>
#include <utility>
#include <vector>

namespace VW
{
class example;
class example_predict;
using multi_ex = std::vector<example*>;
}  // namespace VW

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