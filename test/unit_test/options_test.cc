#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "options.h"

#include <vector>
#include <string>

using namespace VW::config;

BOOST_AUTO_TEST_CASE(make_option_and_customize) {
  int loc;
  auto opt = make_option("opt", loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  BOOST_CHECK_EQUAL(opt.m_name, "opt");
  BOOST_CHECK_EQUAL(opt.default_value_supplied(), true);
  BOOST_CHECK_EQUAL(opt.default_value(), 4);
  BOOST_CHECK_EQUAL(opt.m_help, "Help text");
  BOOST_CHECK_EQUAL(opt.m_keep, true);
  BOOST_CHECK_EQUAL(opt.m_short_name, "t");
  BOOST_CHECK_EQUAL(opt.m_location, loc);
  BOOST_CHECK_EQUAL(opt.m_type_hash, typeid(decltype(loc)).hash_code());
}

BOOST_AUTO_TEST_CASE(typed_argument_equality) {
  int int_loc;
  int int_loc_other;
  float float_loc;
  auto arg1 = make_option("int_opt", int_loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto arg2 = make_option("int_opt", int_loc_other)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto param_3 = make_option("float_opt", float_loc)
    .default_value(3.2f)
    .short_name("f");

  base_option* b1 = &arg1;
  base_option* b2 = &arg2;
  base_option* b3 = &param_3;

  BOOST_CHECK(arg1 == arg2);
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

  BOOST_CHECK_EQUAL(ag.m_name, "g1");
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
