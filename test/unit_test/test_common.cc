// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"

multi_ex parse_json(VW::workspace& all, const std::string& line)
{
  VW::v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(&all));
  VW::read_line_json_s<true>(
      all, examples, (char*)line.c_str(), line.length(), (VW::example_factory_t)&VW::get_unused_example, (void*)&all);

  multi_ex result;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    result.push_back(examples[i]);
  }
  return result;
}

multi_ex parse_dsjson(VW::workspace& all, std::string line, DecisionServiceInteraction* interaction)
{
  VW::v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(&all));

  DecisionServiceInteraction local_interaction;
  if (interaction == nullptr) { interaction = &local_interaction; }

  VW::read_line_decision_service_json<true>(all, examples, (char*)line.c_str(), line.size(), false,
      (VW::example_factory_t)&VW::get_unused_example, (void*)&all, interaction);

  multi_ex result;
  for (const auto& ex : examples) { result.push_back(ex); }
  return result;
}

bool is_invoked_with(const std::string& arg)
{
  for (size_t i = 0; i < boost::unit_test::framework::master_test_suite().argc; i++)
  {
    if (VW::string_view(boost::unit_test::framework::master_test_suite().argv[i]).find(arg) != std::string::npos)
    { return true; } }
  return false;
}