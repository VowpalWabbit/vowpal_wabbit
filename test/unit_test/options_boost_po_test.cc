// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/mpl/vector.hpp>

#include "config/options_cli.h"
#include "test_common.h"

#include "memory.h"
#include "config/options_boost_po.h"
#include "config/cli_options_serializer.h"

#include <memory>
#include <vector>

using namespace VW::config;

using option_types = boost::mpl::vector<options_cli, options_boost_po>;

BOOST_AUTO_TEST_CASE_TEMPLATE(typed_options_parsing, T, option_types)
{
  std::vector<std::string> args = {"--str_opt", "test_str", "-i", "5", "--bool_opt", "--float_opt", "4.3"};
  auto options = VW::make_unique<T>(args);

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

  BOOST_CHECK_EQUAL(str_arg, "test_str");
  BOOST_CHECK_EQUAL(int_opt, 5);
  BOOST_CHECK_EQUAL(bool_opt, true);
  BOOST_CHECK_CLOSE(float_opt, 4.3f, 0.001f);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(typed_option_collection_parsing, T, option_types)
{
  std::vector<std::string> args = {"--str_opt", "test_str", "another"};
  auto options = VW::make_unique<T>(args);

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  check_collections_exact(str_opt, std::vector<std::string>{"test_str", "another"});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(typed_option_collection_parsing_equals_long_option, T, option_types)
{
  std::vector<std::string> args = {"--str_opt=value1", "value2", "--str_opt", "value3"};
  auto options = VW::make_unique<T>(args);

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  check_collections_exact(str_opt, std::vector<std::string>{"value1", "value2", "value3"});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(typed_option_collection_parsing_short_option_attached_value, T, option_types)
{
  std::vector<std::string> args = {"-svalue1", "value2", "--str_opt", "value3"};
  auto options = VW::make_unique<T>(args);

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt).short_name("s"));

  options->add_and_parse(arg_group);

  check_collections_exact(str_opt, std::vector<std::string>{"value1", "value2", "value3"});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(list_consume_until_option_like, T, option_types)
{
  std::vector<std::string> args = {
      "--str_opt", "a", "b", "--unknown", "c", "--str_opt", "d", "e", "f", "--str_opt", "--option_like", "g"};
  auto options = VW::make_unique<T>(args);

  std::vector<std::string> str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt));

  options->add_and_parse(arg_group);

  std::vector<std::string> expected{"a", "b", "d", "e", "f", "--option_like", "g"};
  BOOST_TEST(str_opt == expected, boost::test_tools::per_element());
  const auto positional_tokens = options->get_positional_tokens();
  // There is a slight functional difference here. Boost does not consider --unknown as optional but cli does.
  // The important thing is that "c" was found. --unknown would have resulted in a failure when checking for
  // unregistered options.
  auto found = std::find(positional_tokens.begin(), positional_tokens.end(), "c") != positional_tokens.end();
  BOOST_TEST(found);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(list_no_tokens, T, option_types)
{
  std::vector<std::string> args = {"--str_opt"};
  auto options = VW::make_unique<T>(args);

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

  BOOST_TEST(exception_caught);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(type_conversion_failure, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "4.3", "--float_opt", "1.2a"};
  auto options = VW::make_unique<T>(args);

  float float_opt;
  option_group_definition arg_group1("group1");
  arg_group1.add(make_option("float_opt", float_opt));

  int32_t int_opt;
  option_group_definition arg_group2("group2");
  arg_group2.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group1), VW::vw_argument_invalid_value_exception);
  BOOST_CHECK_THROW(options->add_and_parse(arg_group2), VW::vw_argument_invalid_value_exception);
}

// Boost PO does not follow these semantics. It is arguably a bug that is fixed with the options_cli implementation
// Essentially boost still parses the --bool_opt when it should have technically been consumed by the --str_opt.
// This is because boost implementation parses each option_description indepdenently whereas the options_cli impl
// essentially continually adds to a global definition as it goes.
BOOST_AUTO_TEST_CASE(order_of_parsing)
{
  std::vector<std::string> args = {"--str_opt", "--bool_opt"};
  {
    auto options = VW::make_unique<options_cli>(args);

    bool bool_opt;
    option_group_definition arg_group1("group1");
    arg_group1.add(make_option("bool_opt", bool_opt));

    std::string str_opt;
    option_group_definition arg_group2("group2");
    arg_group2.add(make_option("str_opt", str_opt));

    options->add_and_parse(arg_group1);
    BOOST_CHECK_EQUAL(bool_opt, true);
    options->add_and_parse(arg_group2);
    BOOST_CHECK_EQUAL(str_opt, "--bool_opt");
  }

  {
    auto options = VW::make_unique<options_cli>(args);

    bool bool_opt = false;
    option_group_definition arg_group1("group1");
    arg_group1.add(make_option("bool_opt", bool_opt));

    std::string str_opt;
    option_group_definition arg_group2("group2");
    arg_group2.add(make_option("str_opt", str_opt));

    options->add_and_parse(arg_group2);
    BOOST_CHECK_EQUAL(str_opt, "--bool_opt");
    options->add_and_parse(arg_group1);
    BOOST_CHECK_EQUAL(bool_opt, false);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(bool_implicit_and_explicit_options, T, option_types)
{
  std::vector<std::string> args = {"--bool_switch"};
  auto options = VW::make_unique<T>(args);

  bool bool_switch;
  bool bool_switch_unspecified;
  option_group_definition arg_group("group");
  arg_group.add(make_option("bool_switch", bool_switch));
  arg_group.add(make_option("bool_switch_unspecified", bool_switch_unspecified));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(bool_switch, true);
  BOOST_CHECK_EQUAL(bool_switch_unspecified, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(option_missing_required_value, T, option_types)
{
  std::vector<std::string> args = {"--str_opt"};
  auto options = VW::make_unique<T>(args);

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
  BOOST_CHECK_EQUAL(exception_caught, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(incorrect_option_type_str_for_int, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "str"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_argument_invalid_value_exception);
}

BOOST_AUTO_TEST_CASE(multiple_locations_one_option)
{
  std::vector<std::string> args = {"--str_opt", "value"};
  auto options = VW::make_unique<options_boost_po>(args);

  std::string str_opt_1;
  std::string str_opt_2;
  option_group_definition arg_group("group");
  arg_group.add(make_option("str_opt", str_opt_1));
  arg_group.add(make_option("str_opt", str_opt_2));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(duplicate_option_clash, T, option_types)
{
  std::vector<std::string> args = {"--the_opt", "s"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  std::string str_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("the_opt", int_opt));
  arg_group.add(make_option("the_opt", str_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(mismatched_values_duplicate_command_line, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "5"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_positional_tokens, T, option_types)
{
  std::vector<std::string> args = {"d1", "--int_opt", "1", "d2", "--int_opt", "1", "d3"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));

  const auto positional_tokens = options->get_positional_tokens();
  check_collections_exact(positional_tokens, std::vector<std::string>{"d1", "d2", "d3"});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_positional_tokens_with_terminator, T, option_types)
{
  std::vector<std::string> args = {"d1", "--int_opt", "1", "d2", "--int_opt", "1", "d3", "--", "ab", "--help"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));

  const auto positional_tokens = options->get_positional_tokens();
  check_collections_exact(positional_tokens, std::vector<std::string>{"d1", "d2", "d3", "ab", "--help"});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(matching_values_duplicate_command_line, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "3"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(nonmatching_values_command_line, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "4"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_THROW(options->add_and_parse(arg_group), VW::vw_argument_disagreement_exception);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(nonmatching_values_command_line_with_override, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--int_opt", "4"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group");
  arg_group.add(make_option("int_opt", int_opt).allow_override());

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(add_two_groups, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group1("group1");
  arg_group1.add(make_option("int_opt", int_opt));

  std::string str_opt;
  option_group_definition arg_group2("group2");
  arg_group2.add(make_option("str_opt", str_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group1));
  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group2));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(was_supplied_test, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt));
  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);

  BOOST_CHECK_EQUAL(options->was_supplied("int_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("str_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("bool_opt"), false);
  BOOST_CHECK_EQUAL(options->was_supplied("other_opt"), false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(kept_command_line, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  std::string str_opt;
  bool bool_opt;
  bool other_bool_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("str_opt", str_opt).keep());
  arg_group.add(make_option("bool_opt", bool_opt).keep());
  arg_group.add(make_option("other_bool_opt", other_bool_opt).keep());

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);

  cli_options_serializer serializer;
  for (auto& opt : options->get_all_options())
  {
    if (opt->m_keep) { serializer.add(*opt); }
  }

  auto serialized_string = serializer.str();

  BOOST_CHECK_NE(serialized_string.find("--str_opt test"), std::string::npos);
  BOOST_CHECK_NE(serialized_string.find("--other_bool_opt"), std::string::npos);
  BOOST_CHECK_EQUAL(serialized_string.find("--bool_opt"), std::string::npos);
  BOOST_CHECK_EQUAL(serialized_string.find("--int_opt"), std::string::npos);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(unregistered_options, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test"};
  auto options = VW::make_unique<T>(args);

  int int_opt;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_CHECK_EQUAL(int_opt, 3);

  auto null_logger = VW::io::create_null_logger();
  BOOST_CHECK_THROW(options->check_unregistered(null_logger), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(check_necessary, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = VW::make_unique<T>(args);

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
  BOOST_CHECK_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // result should be true since int_opt is present
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(check_missing_necessary, T, option_types)
{
  // "int_opt" is necessary but missing from cmd line
  std::vector<std::string> args = {"--str_opt", "test", "--other_bool_opt"};
  auto options = VW::make_unique<T>(args);

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
  BOOST_CHECK_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // result should be false since int_opt is not present
  BOOST_CHECK_EQUAL(result, false);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(check_multiple_necessary_and_short_name, T, option_types)
{
  std::vector<std::string> args = {"-i", "3", "--str_opt", "test", "--other_bool_opt"};
  auto options = VW::make_unique<T>(args);

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
  BOOST_CHECK_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // should be true since both necessary options are present
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(str_opt, "test");
  BOOST_CHECK_EQUAL(options->was_supplied("int_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("str_opt"), true);
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(check_multiple_necessary_one_missing, T, option_types)
{
  std::vector<std::string> args = {"--int_opt", "3", "--other_bool_opt"};
  auto options = VW::make_unique<T>(args);

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
  BOOST_CHECK_NO_THROW(result = options->add_parse_and_check_necessary(arg_group));
  // should be false since str_opt is missing (even if int_opt is present and necessary!)
  BOOST_CHECK_EQUAL(result, false);
  BOOST_CHECK_EQUAL(int_opt, 3);
  BOOST_CHECK_EQUAL(options->was_supplied("int_opt"), true);
  BOOST_CHECK_EQUAL(options->was_supplied("str_opt"), false);
  BOOST_CHECK_EQUAL(bool_opt, false);
  BOOST_CHECK_EQUAL(other_bool_opt, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(check_was_supplied_common_prefix_before, T, option_types)
{
  std::vector<std::string> args = {"--int_opt_two","3"};
  auto options = VW::make_unique<T>(args);

  BOOST_TEST(!options->was_supplied("int_opt"));
  BOOST_TEST(options->was_supplied("int_opt_two"));

  int int_opt;
  int int_opt_two;
  option_group_definition arg_group("group1");
  arg_group.add(make_option("int_opt", int_opt));
  arg_group.add(make_option("int_opt_two", int_opt_two));

  BOOST_CHECK_NO_THROW(options->add_and_parse(arg_group));
  BOOST_TEST(!options->was_supplied("int_opt"));
  BOOST_TEST(options->was_supplied("int_opt_two"));
}
