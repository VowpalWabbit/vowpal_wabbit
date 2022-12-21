// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/vw.h"
#include "vw/json_parser/parse_example_json.h"

#include <vector>

namespace vwtest
{
template <typename... ArgsT>
std::unique_ptr<VW::config::options_i> make_args(ArgsT const&... args)
{
  return std::unique_ptr<VW::config::options_cli>(new VW::config::options_cli(std::vector<std::string>({args...})));
}

VW::multi_ex parse_json(VW::workspace& all, const std::string& line);

VW::multi_ex parse_dsjson(
    VW::workspace& all, std::string line, VW::details::decision_service_interaction* interaction = nullptr);
}  // namespace vwtest
