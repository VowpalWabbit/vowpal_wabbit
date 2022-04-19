#pragma once

#include "vw/core/example_predict.h"

#include <string>
#include <vector>

// command line option parsing
namespace vw_slim
{
template <class T>
void find_opt(std::string const& command_line_args, std::string arg_name, std::vector<T>& out_values);

std::vector<std::string> find_opt(std::string const& command_line_args, std::string arg_name);

bool find_opt_float(std::string const& command_line_args, std::string arg_name, float& value);

bool find_opt_int(std::string const& command_line_args, std::string arg_name, int& value);
}  // namespace vw_slim