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
  constexpr auto hash_seed = 0;
  std::string ns_string = "test";
  auto ns_info = VW::make_namespace_info(ns_string, hash_seed, hashall);
  BOOST_CHECK_EQUAL(ns_info.get_namespace_index(), ns_string[0]);

  // We mask off the lower 8 bits to make sure the rest is what we expect.
  auto hashed_string = hashall(ns_string.data(), ns_string.length(), hash_seed);
  hashed_string = hashed_string & (~static_cast<uint64_t>(0xFF));

  auto ns_hash = ns_info.get_hash();
  ns_hash = ns_hash & (~static_cast<uint64_t>(0xFF));

  BOOST_CHECK_EQUAL(hashed_string, ns_hash);
}
