// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "options.h"

#include <vector>
#include <string>
#include <memory>

using namespace VW::config;

template <typename T>
std::shared_ptr<T> to_opt_ptr(option_builder<T>&& builder)
{
  return std::dynamic_pointer_cast<T>(option_builder<T>::finalize(std::move(builder)));
}

template <typename T>
std::shared_ptr<T> to_opt_ptr(option_builder<T>& builder)
{
  return to_opt_ptr(std::move(builder));
}

BOOST_AUTO_TEST_CASE(make_option_and_customize) {
  int loc = 0;
  auto opt = to_opt_ptr(make_option("opt", loc).default_value(4).help("Help text").keep().short_name("t"));

  BOOST_CHECK_EQUAL(opt->m_name, "opt");
  BOOST_CHECK_EQUAL(opt->default_value_supplied(), true);
  BOOST_CHECK_EQUAL(opt->default_value(), 4);
  BOOST_CHECK_EQUAL(opt->m_help, "Help text");
  BOOST_CHECK_EQUAL(opt->m_keep, true);
  BOOST_CHECK_EQUAL(opt->m_short_name, "t");
  BOOST_CHECK_EQUAL(opt->m_type_hash, typeid(decltype(loc)).hash_code());
  opt->value(5, true);
  BOOST_CHECK_EQUAL(loc, 5);
}

BOOST_AUTO_TEST_CASE(make_option_no_loc_and_customize)
{
  auto opt = to_opt_ptr(make_option<int>("opt").default_value(4).help("Help text").keep().short_name("t"));

  BOOST_CHECK_EQUAL(opt->m_name, "opt");
  BOOST_CHECK_EQUAL(opt->default_value_supplied(), true);
  BOOST_CHECK_EQUAL(opt->default_value(), 4);
  BOOST_CHECK_EQUAL(opt->m_help, "Help text");
  BOOST_CHECK_EQUAL(opt->m_keep, true);
  BOOST_CHECK_EQUAL(opt->m_short_name, "t");
  BOOST_CHECK_EQUAL(opt->m_type_hash, typeid(int).hash_code());

  opt->value(5);
  BOOST_CHECK_EQUAL(opt->value(), 5);
}

BOOST_AUTO_TEST_CASE(typed_argument_equality) {
  int int_loc;
  int int_loc_other;
  float float_loc;
  auto arg1 = to_opt_ptr(make_option("int_opt", int_loc).default_value(4).help("Help text").keep().short_name("t"));

  auto arg2 =
      to_opt_ptr(make_option("int_opt", int_loc_other).default_value(4).help("Help text").keep().short_name("t"));

  auto param_3 = to_opt_ptr(make_option("float_opt", float_loc).default_value(3.2f).short_name("f"));

  base_option* b1 = static_cast<base_option*>(arg1.get());
  base_option* b2 = static_cast<base_option*>(arg2.get());
  base_option* b3 = static_cast<base_option*>(param_3.get());

  BOOST_CHECK(*arg1 == *arg2);
  BOOST_CHECK(*b1 == *b2);
  BOOST_CHECK(*b1 != *b3);
}

BOOST_AUTO_TEST_CASE(create_argument_group) {
  char loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("opt1", loc).keep());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  BOOST_CHECK_EQUAL(ag.m_name, "g1 Options");
  BOOST_CHECK_EQUAL(ag.m_options[0]->m_name, "opt1");
  BOOST_CHECK_EQUAL(ag.m_options[0]->m_keep, true);
  BOOST_CHECK_EQUAL(ag.m_options[0]->m_type_hash, typeid(decltype(loc)).hash_code());

  BOOST_CHECK_EQUAL(ag.m_options[1]->m_name, "opt2");
  BOOST_CHECK_EQUAL(ag.m_options[1]->m_type_hash, typeid(decltype(loc)).hash_code());

  BOOST_CHECK_EQUAL(ag.m_options[2]->m_name, "opt3");
  BOOST_CHECK_EQUAL(ag.m_options[2]->m_type_hash, typeid(decltype(loc2)).hash_code());

  BOOST_CHECK_EQUAL(ag.m_options[3]->m_name, "opt4");
  BOOST_CHECK_EQUAL(ag.m_options[3]->m_keep, true);
  BOOST_CHECK_EQUAL(ag.m_options[3]->m_type_hash, typeid(decltype(loc2)).hash_code());
}

BOOST_AUTO_TEST_CASE(name_extraction_from_option_group)
{
  char loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  BOOST_CHECK_EQUAL(name_extractor.generated_name, "im_necessary");
  BOOST_CHECK_EQUAL(result, false);
  // was_supplied will always return false
  BOOST_CHECK_EQUAL(name_extractor.was_supplied("opt2"), false);
  BOOST_CHECK_EQUAL(name_extractor.was_supplied("random"), false);

  // should throw since we validate that reductions should use add_parse_and_check_necessary
  BOOST_REQUIRE_THROW(name_extractor.add_and_parse(ag), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(name_extraction_multi_necessary)
{
  char loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc).necessary());
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  BOOST_CHECK_EQUAL(name_extractor.generated_name, "im_necessary_opt2");
  BOOST_CHECK_EQUAL(result, false);
  // was_supplied will always return false
  BOOST_CHECK_EQUAL(name_extractor.was_supplied("opt2"), false);
  BOOST_CHECK_EQUAL(name_extractor.was_supplied("random"), false);

  // should throw since we validate that reductions should use add_parse_and_check_necessary
  BOOST_REQUIRE_THROW(name_extractor.add_and_parse(ag), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(name_extraction_should_throw)
{
  char loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep());
  ag(make_option("opt2", loc));
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();

  // should throw since no .necessary() is defined
  BOOST_REQUIRE_THROW(name_extractor.add_parse_and_check_necessary(ag), VW::vw_exception);

  // should throw since these methods will never be implemented by options_name_extractor
  BOOST_REQUIRE_THROW(name_extractor.help({}), VW::vw_exception);
  auto null_logger = VW::io::create_null_logger();
  BOOST_REQUIRE_THROW(name_extractor.check_unregistered(null_logger), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.get_all_options(), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.get_option("opt2"), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.insert("opt2", "blah"), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.replace("opt2", "blah"), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.get_positional_tokens(), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.tint("nonsense"), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.reset_tint(), VW::vw_exception);
  BOOST_REQUIRE_THROW(name_extractor.get_collection_of_options(), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(name_extraction_recycle)
{
  char loc;
  std::vector<std::string> loc2;
  option_group_definition ag("g1");
  ag(make_option("im_necessary", loc).keep().necessary());
  ag(make_option("opt2", loc).necessary());
  ag.add(make_option("opt3", loc2));
  ag.add(make_option("opt4", loc2).keep());

  auto name_extractor = options_name_extractor();
  // result should always be false
  bool result = name_extractor.add_parse_and_check_necessary(ag);

  BOOST_CHECK_EQUAL(name_extractor.generated_name, "im_necessary_opt2");
  BOOST_CHECK_EQUAL(result, false);

  option_group_definition ag2("g2");
  ag2(make_option("im_necessary_v2", loc).keep().necessary());
  ag2(make_option("opt2", loc).necessary());
  ag2.add(make_option("opt3", loc2));
  ag2.add(make_option("opt4", loc2).keep());

  // result should always be false
  result = name_extractor.add_parse_and_check_necessary(ag2);

  BOOST_CHECK_EQUAL(name_extractor.generated_name, "im_necessary_v2_opt2");
  BOOST_CHECK_EQUAL(result, false);
}
