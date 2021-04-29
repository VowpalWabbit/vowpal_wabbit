#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <memory>
#include <array>

#include "generate_interactions.h"
#include "interactions.h"
#include "test_common.h"

BOOST_AUTO_TEST_CASE(interaction_generic_expand_wildcard_only)
{
  std::set<namespace_index> namespaces = {'a', 'b'};
  auto result = generate_combinations_with_repetition(namespaces, 2);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(interaction_generic_with_duplicates_expand_wildcard_only)
{
  std::set<namespace_index> namespaces = {'a', 'b'};
  auto result = generate_permutations_with_repetition(namespaces, 2);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'b'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  check_vector_of_vectors_exact(result, compare_set);
}


BOOST_AUTO_TEST_CASE(sort_and_filter_interactions)
{
  std::vector<std::vector<namespace_index>> input = {{'b', 'a'}, {'a', 'b','a'}, {'a', 'a'}, {'b', 'b'}};

  size_t removed_count = 0;
  size_t sorted_count = 0;
  INTERACTIONS::sort_and_filter_duplicate_interactions(input, false, removed_count, sorted_count);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'b','a'}, {'a', 'a'}, {'b', 'b'}};
  check_vector_of_vectors_exact(input, compare_set);
}
