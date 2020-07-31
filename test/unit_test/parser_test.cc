#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "parse_args.h"

BOOST_AUTO_TEST_CASE(spoof_hex_encoded_namespace_test)
{
  BOOST_CHECK_EQUAL(spoof_hex_encoded_namespaces("test"), "test");
  BOOST_CHECK_EQUAL(spoof_hex_encoded_namespaces("10"), "10");
  BOOST_CHECK_EQUAL(spoof_hex_encoded_namespaces("\\x01"), "\x01");
  BOOST_CHECK_EQUAL(spoof_hex_encoded_namespaces("\\xab"), "\xab");
  BOOST_CHECK_EQUAL(spoof_hex_encoded_namespaces("\\x01 unrelated \\x56"), "\x01 unrelated \x56");
}
