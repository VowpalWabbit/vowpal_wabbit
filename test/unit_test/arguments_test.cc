#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "arguments.h"

BOOST_AUTO_TEST_CASE(make_parameter_and_customize) {
  int loc;
  auto param = make_typed_parameter("param", &loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  BOOST_CHECK_EQUAL(param.m_name, "param");
  BOOST_CHECK_EQUAL(param.m_default_supplied, true);
  BOOST_CHECK_EQUAL(param.m_default_value, 4);
  BOOST_CHECK_EQUAL(param.m_help, "Help text");
  BOOST_CHECK_EQUAL(param.m_keep, true);
  BOOST_CHECK_EQUAL(param.m_short_name, "t");
  BOOST_CHECK_EQUAL(param.m_locations.size(), 1);
  BOOST_CHECK_EQUAL(param.m_locations[0], &loc);
  BOOST_CHECK_EQUAL(param.m_type_hash, typeid(decltype(loc)).hash_code());
}

BOOST_AUTO_TEST_CASE(typed_parameter_equality) {
  int int_loc;
  int int_loc_other;
  float float_loc;
  auto param_1 = make_typed_parameter("int_param", &int_loc)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto param_2 = make_typed_parameter("int_param", &int_loc_other)
    .default_value(4)
    .help("Help text")
    .keep()
    .short_name("t");

  auto param_3 = make_typed_parameter("float_param", &float_loc)
    .default_value(3.2f)
    .short_name("f");

  base_parameter* b1 = &param_1;
  base_parameter* b2 = &param_2;
  base_parameter* b3 = &param_3;

  BOOST_CHECK(param_1 == param_2);
  BOOST_CHECK(*b1 == *b2);
  BOOST_CHECK(*b1 != *b3);
}
BOOST_AUTO_TEST_CASE(create_option_group) {
}
