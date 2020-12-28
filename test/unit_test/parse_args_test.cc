#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_args.h"
#include "options_boost_po.h"

#include <vector>
#include <string>

using namespace VW::config;

BOOST_AUTO_TEST_CASE(merge_from_header_strings_empty_options_dup)
{
  const std::vector<std::string> strings;
  auto opts = VW::make_unique<options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, true, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(false, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(false, opts->was_supplied("bool_opt"));
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_empty_options_nodup)
{
  const std::vector<std::string> strings;
  auto opts = VW::make_unique<options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, false, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(false, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(false, opts->was_supplied("bool_opt"));
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_single_key_only_dup)
{
  const std::vector<std::string> strings{"--bool_opt"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, true, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(false, opts->was_supplied("some_opt"));
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_single_key_only_nodup)
{
  const std::vector<std::string> strings{"--bool_opt"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, false, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(false, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_single_key_value_dup)
{
  const std::vector<std::string> strings{"--int_opt", "3"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, true, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(false, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(false, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_single_key_value_nodup)
{
  const std::vector<std::string> strings{"--int_opt", "3"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, false, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(false, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(false, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_bool_and_int_opt_dup)
{
  const std::vector<std::string> strings{"--bool_opt", "--int_opt", "3"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, true, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_bool_and_int_opt_nodup)
{
  const std::vector<std::string> strings{"--bool_opt", "--int_opt", "3"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, false, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_int_and_bool_opt_dup)
{
  const std::vector<std::string> strings{"--int_opt", "3", "--bool_opt"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, true, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}

BOOST_AUTO_TEST_CASE(merge_from_header_strings_int_and_bool_opt_nodup)
{
  const std::vector<std::string> strings{"--int_opt", "3", "--bool_opt"};
  auto opts = VW::make_unique<VW::config::options_boost_po>(std::vector<std::string>());

  merge_options_from_header_strings(strings, false, *opts);

  int int_opt;
  bool bool_opt;

  option_group_definition arg_group("");
  arg_group.add(make_option("int_opt", int_opt));

  arg_group.add(make_option("bool_opt", bool_opt));

  BOOST_CHECK_NO_THROW(opts->add_and_parse(arg_group));

  BOOST_CHECK_EQUAL(2, opts->get_all_options().size());

  BOOST_CHECK_EQUAL(true, opts->was_supplied("int_opt"));
  BOOST_CHECK_EQUAL(3, int_opt);
  BOOST_CHECK_EQUAL(true, opts->was_supplied("bool_opt"));
  BOOST_CHECK_EQUAL(true, bool_opt);
}
