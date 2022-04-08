// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "vw/config/options_cli.h"
#include "vw/config/cli_options_serializer.h"

#include <memory>
#include <vector>

using namespace VW::config;

TEST(options_cli_test, typed_options_parsing)
{
  std::vector<std::string> args = {"--str_opt", "test_str", "-i", "5", "--bool_opt", "--float_opt", "4.3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::string str_arg;
  int int_opt;
  bool bool_opt;
  float float_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_arg));
  arg_group.add(make_option("int_opt", int_opt).short_name("i"));
  arg_group.add(make_option("bool_opt", bool_opt));
  arg_group.add(make_option("float_opt", float_opt));

  options->add_and_parse(arg_group);

  EXPECT_EQ(str_arg, "test_str");
  EXPECT_EQ(int_opt, 5);
  EXPECT_EQ(bool_opt, true);
  EXPECT_FLOAT_EQ(float_opt, 4.3f);
}

TEST(options_cli_test, typed_option_collection_parsing)
{
  std::vector<std::string> args = {"--str_opt", "test_str", "another"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  EXPECT_EQ(str_opt, (std::vector<std::string>{"test_str", "another"}));
}

TEST(options_cli_test, typed_option_collection_parsing_equals_long_option)
{
  std::vector<std::string> args = {"--str_opt=value1", "value2", "--str_opt", "value3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  EXPECT_EQ(str_opt, (std::vector<std::string>{"value1", "value2", "value3"}));
}

TEST(options_cli_test, typed_option_collection_parsing_short_option_attached_value)
{
  std::vector<std::string> args = {"-svalue1", "value2", "--str_opt", "value3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt).short_name("s"));

  options->add_and_parse(arg_group);

  EXPECT_EQ(str_opt, (std::vector<std::string>{"value1", "value2", "value3"}));
}

TEST(options_cli_test, list_consume_until_option_like)
{
  std::vector<std::string> args = {
      "--str_opt", "a", "b", "--unknown", "c", "--str_opt", "d", "e", "f", "--str_opt", "--option_like", "g"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  std::vector<std::string> expected{"a", "b", "d", "e", "f", "--option_like", "g"};
  EXPECT_EQ(str_opt, expected);
  const auto positional_tokens = options->get_positional_tokens();
  // There is a slight functional difference here. Boost does not consider --unknown as optional but cli does.
  // The important thing is that "c" was found. --unknown would have resulted in a failure when checking for
  // unregistered options.
  auto found = std::find(positional_tokens.begin(), positional_tokens.end(), "c") != positional_tokens.end();
  EXPECT_TRUE(found);
}

TEST(options_cli_test, list_no_tokens)
{
  std::vector<std::string> args = {"--str_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  bool exception_caught = false;
  try
  {
    options->add_and_parse(arg_group);
  }
  catch (...)
  {
    exception_caught = true;
  }

  EXPECT_TRUE(exception_caught);
}

TEST(options_cli_test, type_conversion_failure)
{
  std::vector<std::string> args = {"--int_opt", "4.3", "--float_opt", "1.2a"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  float float_opt;
  option_group_definition arg_group1("group1");
  arg_group1.add(make_option("float_opt", float_opt));

  int32_t int_opt;
  option_group_definition arg_group2("group2");
  arg_group2.add(make_option("int_opt", int_opt));

  EXPECT_THROW(options->add_and_parse(arg_group1), VW::vw_argument_invalid_value_exception);
  EXPECT_THROW(options->add_and_parse(arg_group2), VW::vw_argument_invalid_value_exception);
}

// Boost PO does not follow these semantics. It is arguably a bug that is fixed with the options_cli implementation
// Essentially boost still parses the --bool_opt when it should have technically been consumed by the --str_opt.
// This is because boost implementation parses each option_description indepdenently whereas the options_cli impl
// essentially continually adds to a global definition as it goes.
TEST(options_cli_test, order_of_parsing)
{
  std::vector<std::string> args = {"--str_opt", "--bool_opt"};

  {
    auto options = std::unique_ptr<options_cli>(new options_cli(args));

    bool bool_opt;
    option_group_definition arg_group1("group1");
    arg_group1.add(make_option("bool_opt", bool_opt));

    std::string str_opt;
    option_group_definition arg_group2("group2");
    arg_group2.add(make_option("str_opt", str_opt));

    options->add_and_parse(arg_group1);
    EXPECT_EQ(bool_opt, true);
    options->add_and_parse(arg_group2);
    EXPECT_EQ(str_opt, "--bool_opt");
  }

  {
    auto options = std::unique_ptr<options_cli>(new options_cli(args));

    bool bool_opt = false;
    option_group_definition arg_group1("group1");
    arg_group1.add(make_option("bool_opt", bool_opt));

    std::string str_opt;
    option_group_definition arg_group2("group2");
    arg_group2.add(make_option("str_opt", str_opt));

    options->add_and_parse(arg_group2);
    EXPECT_EQ(str_opt, "--bool_opt");
    options->add_and_parse(arg_group1);
    EXPECT_EQ(bool_opt, false);
  }
}

TEST(options_cli_test, bool_implicit_and_explicit_options)
{
  std::vector<std::string> args = {"--bool_switch"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  bool bool_switch;
  bool bool_switch_unspecified;
  option_group_definition arg_group("group");
  arg_group.add(make_option("bool_switch", bool_switch));
  arg_group.add(make_option("bool_switch_unspecified", bool_switch_unspecified));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));

  EXPECT_EQ(bool_switch, true);
  EXPECT_EQ(bool_switch_unspecified, false);
}

TEST(options_cli_test, option_missing_required_value)
{
  std::vector<std::string> args = {"--str_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::string str_arg;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_arg));

  bool exception_caught = false;
  try
  {
    options->add_and_parse(arg_group);
  }
  catch (...)
  {
    exception_caught = true;
  }
  EXPECT_EQ(exception_caught, true);
}

TEST(options_cli_test, incorrect_option_type_str_for_int)
{
  std::vector<std::string> args = {"--int_opt", "str"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_THROW(options->add_and_parse(arg_group), VW::vw_argument_invalid_value_exception);
}

TEST(options_cli_test, multiple_locations_one_option)
{
  std::vector<std::string> args = {"--str_opt", "value"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  std::string str_opt_1;
  std::string str_opt_2;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt_1));
  arg_group.add(make_option("str_opt", str_opt_2));

  options->add_and_parse(arg_group);
  EXPECT_EQ(str_opt_1, "value");
  EXPECT_EQ(str_opt_2, "value");
}

TEST(options_cli_test, duplicate_option_clash)
{
  std::vector<std::string> args = {"--the_opt", "s"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("the_opt", int_opt));
  arg_group.add(make_option("the_opt", str_opt));

  EXPECT_THROW(options->add_and_parse(arg_group), VW::vw_exception);
}

TEST(options_cli_test, mismatched_values_duplicate_command_line)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "5"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_THROW(options->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

TEST(options_cli_test, get_positional_tokens)
{
  std::vector<std::string> args = {"d1", "--int_opt", "1", "d2", "--int_opt", "1", "d3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));

  const auto positional_tokens = options->get_positional_tokens();
  EXPECT_EQ(positional_tokens, (std::vector<std::string>{"d1", "d2", "d3"}));
}

TEST(options_cli_test, get_positional_tokens_with_terminator)
{
  std::vector<std::string> args = {"d1", "--int_opt", "1", "d2", "--int_opt", "1", "d3", "--", "ab", "--help"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));

  const auto positional_tokens = options->get_positional_tokens();
  EXPECT_EQ(positional_tokens, (std::vector<std::string>{"d1", "d2", "d3", "ab", "--help"}));
}

TEST(options_cli_test, matching_values_duplicate_command_line)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_EQ(int_opt, 3);
}

TEST(options_cli_test, nonmatching_values_command_line)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "4"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_THROW(options->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

TEST(options_cli_test, nonmatching_values_command_line_with_override)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "4"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt).allow_override());

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_EQ(int_opt, 3);
}

TEST(options_cli_test, add_two_groups)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group1("group1");
  arg_group1.add(make_option("int_opt", int_opt));

  std::string str_opt;
  option_group_definition arg_group2("group2");
  arg_group2.add(make_option("str_opt", str_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group1));
  EXPECT_NO_THROW(options->add_and_parse(arg_group2));
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(str_opt, "test");
}

TEST(options_cli_test, was_supplied_test)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt));
  arg_group.add(make_option("bool_opt", bool_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(str_opt, "test");
  EXPECT_EQ(bool_opt, false);

  EXPECT_EQ(options->was_supplied("int_opt"), true);
  EXPECT_EQ(options->was_supplied("str_opt"), true);
  EXPECT_EQ(options->was_supplied("bool_opt"), false);
  EXPECT_EQ(options->was_supplied("other_opt"), false);
}

TEST(options_cli_test, kept_command_line)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt).keep());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(str_opt, "test");
  EXPECT_EQ(bool_opt, false);
  EXPECT_EQ(other_bool_opt, true);

  cli_options_serializer serializer;
  for (auto& opt : options->get_all_options())
  {
    if (opt->m_keep) { serializer.add(*opt); }
  }

  auto serialized_string = serializer.str();

  EXPECT_NE(serialized_string.find("--str_opt test"), std::string::npos);
  EXPECT_NE(serialized_string.find("--other_bool_opt"), std::string::npos);
  EXPECT_EQ(serialized_string.find("--bool_opt"), std::string::npos);
  EXPECT_EQ(serialized_string.find("--int_opt"), std::string::npos);
}

TEST(options_cli_test, unregistered_options)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_EQ(int_opt, 3);

  std::vector<std::string> warnings;
  EXPECT_THROW(warnings = options->check_unregistered(), VW::vw_exception);
}

TEST(options_cli_test, check_necessary)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt).necessary());
  arg_group.add(make_option("str_opt", str_opt).keep());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  bool result;
  EXPECT_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // result should be true since int_opt is present
  EXPECT_EQ(result, true);
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(str_opt, "test");
  EXPECT_EQ(bool_opt, false);
  EXPECT_EQ(other_bool_opt, true);
}

TEST(options_cli_test, check_missing_necessary)
{
  // "int_opt" is necessary but missing from cmd line
  std::vector<std::string> args = {"--str_opt", "test", "--other_bool_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt).necessary());
  arg_group.add(make_option("str_opt", str_opt).keep());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  bool result;
  EXPECT_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // result should be false since int_opt is not present
  EXPECT_EQ(result, false);
  EXPECT_EQ(str_opt, "test");
  EXPECT_EQ(bool_opt, false);
  EXPECT_EQ(other_bool_opt, true);
}

TEST(options_cli_test, check_multiple_necessary_and_short_name)
{
  std::vector<std::string> args = {"-i", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt).necessary().short_name("i"));
  arg_group.add(make_option("str_opt", str_opt).keep().necessary());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  bool result;
  EXPECT_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // should be true since both necessary options are present
  EXPECT_EQ(result, true);
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(str_opt, "test");
  EXPECT_EQ(options->was_supplied("int_opt"), true);
  EXPECT_EQ(options->was_supplied("str_opt"), true);
  EXPECT_EQ(bool_opt, false);
  EXPECT_EQ(other_bool_opt, true);
}

TEST(options_cli_test, check_multiple_necessary_one_missing)
{
  std::vector<std::string> args = {"--int_opt", "3", "--other_bool_opt"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt).necessary().short_name("i"));
  arg_group.add(make_option("str_opt", str_opt).keep().necessary());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  bool result;
  EXPECT_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // should be false since str_opt is missing (even if int_opt is present and necessary!)
  EXPECT_EQ(result, false);
  EXPECT_EQ(int_opt, 3);
  EXPECT_EQ(options->was_supplied("int_opt"), true);
  EXPECT_EQ(options->was_supplied("str_opt"), false);
  EXPECT_EQ(bool_opt, false);
  EXPECT_EQ(other_bool_opt, true);
}

TEST(options_cli_test, check_was_supplied_common_prefix_before)
{
  std::vector<std::string> args = {"--int_opt_two", "3"};
  auto options = std::unique_ptr<options_cli>(new options_cli(args));

  EXPECT_TRUE(!options->was_supplied("int_opt"));
  EXPECT_TRUE(options->was_supplied("int_opt_two"));

  int int_opt;
  int int_opt_two;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("int_opt_two", int_opt_two));

  EXPECT_NO_THROW(options->add_and_parse(arg_group));
  EXPECT_TRUE(!options->was_supplied("int_opt"));
  EXPECT_TRUE(options->was_supplied("int_opt_two"));
}
