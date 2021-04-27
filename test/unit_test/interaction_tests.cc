#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <memory>
#include <array>

#include "generate_interactions.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(interaction_generic_expand_wildcard_only)
{
  std::vector<namespace_index> namespaces = {'a', 'b'};
  auto result = expand_generic(namespaces, 2);

  namespace_interactions interactions;
  std::vector<std::vector<namespace_index>> compare_set = {{'a', 'b'}, {'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  check_vector_of_vectors_exact(result, compare_set);
}
