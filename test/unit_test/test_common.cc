// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"

#include "parse_example_json.h"
#include "json_utils.h"

multi_ex parse_json(
    VW::workspace& all, const std::string& line, const std::unordered_map<uint64_t, example*>* dedup_examples)
{
  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(&all));
  auto json_parser = VW::make_json_parser(all);
  json_parser->parse_object(const_cast<char*>(line.c_str()), line.size(), dedup_examples, examples);

  multi_ex result;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    result.push_back(examples[i]);
  }
  return result;
}

multi_ex parse_dsjson(VW::workspace& all, std::string line, DecisionServiceInteraction* interaction)
{
  v_array<example*> examples;
  examples.push_back(&VW::get_unused_example(&all));
  auto dsjson_parser = VW::make_dsjson_parser(
      all, all.options->was_supplied("extra_metrics"), false /* destructive_parse*/, all.example_parser->strict_parse);

  DecisionServiceInteraction local_interaction;
  if (interaction == nullptr) { interaction = &local_interaction; }

  dsjson_parser->parse_object(const_cast<char*>(line.c_str()), line.size(), examples, *interaction);

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
