// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/test_common/test_common.h"

#include <vector>

VW::multi_ex vwtest::parse_json(VW::workspace& all, const std::string& line)
{
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(&all));
  VW::parsers::json::read_line_json<true>(
      all, examples, (char*)line.c_str(), line.length(), std::bind(VW::get_unused_example, &all));

  setup_examples(all, examples);

  VW::multi_ex result;
  for (size_t i = 0; i < examples.size(); ++i) { result.push_back(examples[i]); }
  return result;
}

VW::multi_ex vwtest::parse_dsjson(
    VW::workspace& all, std::string line, VW::parsers::json::decision_service_interaction* interaction)
{
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(&all));

  VW::parsers::json::decision_service_interaction local_interaction;
  if (interaction == nullptr) { interaction = &local_interaction; }

  VW::parsers::json::read_line_decision_service_json<true>(
      all, examples, (char*)line.c_str(), line.size(), false, std::bind(VW::get_unused_example, &all), interaction);

  VW::multi_ex result;
  for (const auto& ex : examples) { result.push_back(ex); }
  return result;
}
