#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "namespace_info.h"
#include "hashstring.h"

BOOST_AUTO_TEST_CASE(namespace_info_tests)
{
  std::string ns_string = "test";
  auto ns = VW::make_namespace_info(ns_string, 0, hashall);
  BOOST_CHECK_EQUAL(ns.get_namespace_index(), 't');

  auto hashed_string = hashall(ns_string.data(), ns_string.length(), 0);
  hashed_string = hashed_string & (~static_cast<uint64_t>(0xFF));

  auto ns_hash = ns.get_hash();
  ns_hash = ns_hash & (~static_cast<uint64_t>(0xFF));

  BOOST_CHECK_EQUAL(hashed_string, ns_hash);
}