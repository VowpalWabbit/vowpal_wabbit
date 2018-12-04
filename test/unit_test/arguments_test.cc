#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "arguments.h"

#include <vector>
#include <string>

BOOST_AUTO_TEST_CASE(make_argument_and_customize) {
  int loc;
  auto arg = make_typed_arg("arg", &loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  BOOST_CHECK_EQUAL(arg.m_name, "arg");
  BOOST_CHECK_EQUAL(arg.m_default_supplied, true);
  BOOST_CHECK_EQUAL(arg.m_default_value, 4);
  BOOST_CHECK_EQUAL(arg.m_help, "Help text");
  BOOST_CHECK_EQUAL(arg.m_keep, true);
  BOOST_CHECK_EQUAL(arg.m_short_name, "t");
  BOOST_CHECK_EQUAL(arg.m_locations.size(), 1);
  BOOST_CHECK_EQUAL(arg.m_locations[0], &loc);
  BOOST_CHECK_EQUAL(arg.m_type_hash, typeid(decltype(loc)).hash_code());
}

BOOST_AUTO_TEST_CASE(typed_argument_equality) {
  int int_loc;
  int int_loc_other;
  float float_loc;
  auto arg1 = make_typed_arg("int_arg", &int_loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto arg2 = make_typed_arg("int_arg", &int_loc_other)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto param_3 = make_typed_arg("float_arg", &float_loc)
    .default_value(3.2f)
    .short_name("f");

  base_argument* b1 = &arg1;
  base_argument* b2 = &arg2;
  base_argument* b3 = &param_3;

  BOOST_CHECK(arg1 == arg2);
  BOOST_CHECK(*b1 == *b2);
  BOOST_CHECK(*b1 != *b3);
}

BOOST_AUTO_TEST_CASE(create_argument_group) {
  char loc;
  std::vector<std::string> loc2;
  argument_group_definition ag("g1");
  ag(make_typed_arg("arg1", &loc).keep());
  ag.add(make_typed_arg("arg2", &loc2));

  BOOST_CHECK_EQUAL(ag.m_name, "g1");
  BOOST_CHECK_EQUAL(ag.m_arguments[0]->m_name, "arg1");
  BOOST_CHECK_EQUAL(ag.m_arguments[0]->m_keep, true);
  BOOST_CHECK_EQUAL(ag.m_arguments[0]->m_type_hash, typeid(decltype(loc)).hash_code());

  BOOST_CHECK_EQUAL(ag.m_arguments[1]->m_name, "arg2");
  BOOST_CHECK_EQUAL(ag.m_arguments[1]->m_type_hash, typeid(decltype(loc2)).hash_code());
}
