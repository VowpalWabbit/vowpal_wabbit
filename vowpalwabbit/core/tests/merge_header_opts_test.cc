// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/memory.h"
#include "vw/core/parse_args.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace VW::config;

TEST(MergeOptsFromHeader, StringsNoOptsSkip)
{
  const std::vector<std::string> strings;
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(2, opts->get_all_options().size());

  EXPECT_EQ(false, opts->was_supplied("int_opt"));
  EXPECT_EQ(-1, int_opt);
  EXPECT_EQ(false, opts->was_supplied("bool_opt"));
  EXPECT_EQ(false, bool_opt);
}

TEST(MergeOptsFromHeader, StringsNoOptsNoskip)
{
  const std::vector<std::string> strings;
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(2, opts->get_all_options().size());

  EXPECT_EQ(false, opts->was_supplied("int_opt"));
  EXPECT_EQ(-1, int_opt);
  EXPECT_EQ(false, opts->was_supplied("bool_opt"));
  EXPECT_EQ(false, bool_opt);
}

TEST(MergeOptsFromHeader, StringsBoolOptSkip)
{
  const std::vector<std::string> strings{"--bool_opt"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(2, opts->get_all_options().size());

  EXPECT_EQ(false, opts->was_supplied("some_opt"));
  EXPECT_EQ(-1, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsBoolOptNoskip)
{
  const std::vector<std::string> strings{"--bool_opt"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(false, opts->was_supplied("int_opt"));
  EXPECT_EQ(-1, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsIntOptSkip)
{
  const std::vector<std::string> strings{"--int_opt", "3"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(false, opts->was_supplied("bool_opt"));
  EXPECT_EQ(false, bool_opt);
}

TEST(MergeOptsFromHeader, StringsIntOptNoskip)
{
  const std::vector<std::string> strings{"--int_opt", "3"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(false, opts->was_supplied("bool_opt"));
  EXPECT_EQ(false, bool_opt);
}

TEST(MergeOptsFromHeader, StringsBoolIntOptSkip)
{
  const std::vector<std::string> strings{"--bool_opt", "--int_opt", "3"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsBoolIntOptNoskip)
{
  const std::vector<std::string> strings{"--bool_opt", "--int_opt", "3"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsIntBoolOptSkip)
{
  const std::vector<std::string> strings{"--int_opt", "3", "--bool_opt"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsIntBoolOptNoskip)
{
  const std::vector<std::string> strings{"--int_opt", "3", "--bool_opt"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt = -1;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt"));
  EXPECT_EQ(3, int_opt);
  EXPECT_EQ(true, opts->was_supplied("bool_opt"));
  EXPECT_EQ(true, bool_opt);
}

TEST(MergeOptsFromHeader, StringsBoolIntInteractionBoolIntOptSkip)
{
  const std::vector<std::string> strings{
      "--bool_opt1", "--int_opt1", "3", "--interactions", "::", "--bool_opt2", "--int_opt2", "4"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(false, opts->was_supplied("interactions"));
  EXPECT_EQ("NONE", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsBoolIntInteractionBoolIntOptNoskip)
{
  const std::vector<std::string> strings{
      "--bool_opt1", "--int_opt1", "3", "--interactions", "::", "--bool_opt2", "--int_opt2", "4"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(true, opts->was_supplied("interactions"));
  EXPECT_EQ("::", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsBoolIntInteractionIntBoolOptSkip)
{
  const std::vector<std::string> strings{
      "--bool_opt1", "--int_opt1", "3", "--interactions", "::", "--int_opt2", "4", "--bool_opt2"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(false, opts->was_supplied("interactions"));
  EXPECT_EQ("NONE", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsBoolIntInteractionIntBoolOptNoskip)
{
  const std::vector<std::string> strings{
      "--bool_opt1", "--int_opt1", "3", "--interactions", "::", "--int_opt2", "4", "--bool_opt2"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(true, opts->was_supplied("interactions"));
  EXPECT_EQ("::", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsIntBoolInteractionBoolIntOptSkip)
{
  const std::vector<std::string> strings{
      "--int_opt1", "3", "--bool_opt1", "--interactions", "::", "--bool_opt2", "--int_opt2", "4"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(false, opts->was_supplied("interactions"));
  EXPECT_EQ("NONE", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsIntBoolInteractionBoolIntOptNoskip)
{
  const std::vector<std::string> strings{
      "--int_opt1", "3", "--bool_opt1", "--interactions", "::", "--bool_opt2", "--int_opt2", "4"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(true, opts->was_supplied("interactions"));
  EXPECT_EQ("::", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsIntBoolInteractionIntBoolOptSkip)
{
  const std::vector<std::string> strings{
      "--int_opt1", "3", "--bool_opt1", "--interactions", "::", "--int_opt2", "4", "--bool_opt2"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, true, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(false, opts->was_supplied("interactions"));
  EXPECT_EQ("NONE", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, StringsIntBoolInteractionIntBoolOptNoskip)
{
  const std::vector<std::string> strings{
      "--int_opt1", "3", "--bool_opt1", "--interactions", "::", "--int_opt2", "4", "--bool_opt2"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  int int_opt1 = -1;
  bool bool_opt1;
  std::string interactions = "NONE";
  int int_opt2 = -1;
  bool bool_opt2;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt1", int_opt1));
  arg_group.add(make_option("bool_opt1", bool_opt1));
  arg_group.add(make_option("interactions", interactions));
  arg_group.add(make_option("int_opt2", int_opt2));
  arg_group.add(make_option("bool_opt2", bool_opt2));

  EXPECT_NO_THROW(opts->add_and_parse(arg_group));

  EXPECT_EQ(true, opts->was_supplied("int_opt1"));
  EXPECT_EQ(3, int_opt1);
  EXPECT_EQ(true, opts->was_supplied("bool_opt1"));
  EXPECT_EQ(true, bool_opt1);
  EXPECT_EQ(true, opts->was_supplied("interactions"));
  EXPECT_EQ("::", interactions);
  EXPECT_EQ(true, opts->was_supplied("int_opt2"));
  EXPECT_EQ(4, int_opt2);
  EXPECT_EQ(true, opts->was_supplied("bool_opt2"));
  EXPECT_EQ(true, bool_opt2);
}

TEST(MergeOptsFromHeader, MergeOptionsFromCcbHeader)
{
  const std::vector<std::string> strings{"--dsjson", "--epsilon", "0.2", "--ccb_explore_adf"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  EXPECT_EQ(true, is_ccb_model);
}

TEST(MergeOptsFromHeader, MergeOptionFromCbHeader)
{
  const std::vector<std::string> strings{"--dsjson", "--epsilon", "0.2", "--cb_explore_adf"};
  auto opts = VW::make_unique<options_cli>(std::vector<std::string>());

  bool is_ccb_model = false;
  VW::details::merge_options_from_header_strings(strings, false, *opts, is_ccb_model);

  EXPECT_EQ(false, is_ccb_model);
}