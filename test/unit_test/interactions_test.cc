#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <array>
#include <vector>

#include "test_common.h"
#include "vw.h"
#include "gd_predict.h"
#include "gd.h"
#include "interactions.h"
#include "parse_args.h"
#include "constant.h"
#include "interactions_predict.h"

namespace std
{
std::ostream& operator<<(std::ostream& os, const std::pair<namespace_index, uint64_t>& obj)
{
  return os << "_ns_char: " << obj.first << " _ns_hash: " << obj.second;
}
}  // namespace std

struct eval_gen_data
{
  size_t& new_features_cnt;
  float& new_features_value;
  eval_gen_data(size_t& features_cnt, float& features_value)
      : new_features_cnt(features_cnt), new_features_value(features_value)
  {
  }
};

void ft_cnt(eval_gen_data& dat, const float fx, const uint64_t)
{
  ++dat.new_features_cnt;
  dat.new_features_value += fx * fx;
}

// eval_count_of_generated_ft_naive() is an alternative way of implementation of
// eval_count_of_generated_ft() it just calls generate_interactions() with small
// function which counts generated features and sums their squared values. We
// use it to validate the with more fast (?) analytic solution
template <INTERACTIONS::generate_func_t generate_func, bool leave_duplicate_interactions>
void eval_count_of_generated_ft_naive(vw& all, example_predict& ec, size_t& new_features_cnt, float& new_features_value)
{
  // Only makes sense to do this when not in permutations mode.
  assert(!all.permutations);

  new_features_cnt = 0;
  new_features_value = 0.;

  auto interactions = INTERACTIONS::compile_interactions<generate_func, leave_duplicate_interactions>(
      all.interactions, std::set<namespace_index>(ec.indices.begin(), ec.indices.end()));

  v_array<float> results;

  eval_gen_data dat(new_features_cnt, new_features_value);
  size_t ignored = 0;
  ec.interactions = &interactions;
  INTERACTIONS::generate_interactions<eval_gen_data, uint64_t, ft_cnt, false, nullptr>(all, ec, dat, ignored);
  ec.interactions = &all.interactions;
}

inline void noop_func(float& unused_dat, const float ft_weight, const uint64_t ft_idx) {}

BOOST_AUTO_TEST_CASE(eval_count_of_generated_ft_test)
{
  auto& vw = *VW::initialize("--quiet -q :: --noconstant", nullptr, false, nullptr, nullptr);
  auto* ex = VW::read_example(vw, "3 |f a b c |e x y z");

  size_t naive_features_count;
  float naive_features_value;
  eval_count_of_generated_ft_naive<INTERACTIONS::generate_namespace_combinations_with_repetition, false>(
      vw, *ex, naive_features_count, naive_features_value);

  auto interactions =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_combinations_with_repetition, false>(
          vw.interactions, std::set<namespace_index>(ex->indices.begin(), ex->indices.end()));
  ex->interactions = &interactions;
  ex->extent_interactions = &vw.extent_interactions;
  size_t fast_features_count;
  float fast_features_value;
  INTERACTIONS::eval_count_of_generated_ft(vw.permutations, *ex->interactions, *ex->extent_interactions,
      ex->feature_space, fast_features_count, fast_features_value);
  ex->interactions = &vw.interactions;

  BOOST_CHECK_EQUAL(naive_features_count, fast_features_count);
  BOOST_CHECK_CLOSE(naive_features_value, fast_features_value, FLOAT_TOL);

  // Prediction will count the interacted features, so we can compare that too.
  vw.predict(*ex);
  BOOST_CHECK_EQUAL(naive_features_count, ex->num_features_from_interactions);
  VW::finish_example(vw, *ex);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(eval_count_of_generated_ft_permuations_test)
{
  auto& vw = *VW::initialize(
      "--quiet -q :: --leave_duplicate_interactions --permutations --noconstant", nullptr, false, nullptr, nullptr);
  auto* ex = VW::read_example(vw, "3 |f a b c |e x y z");

  auto interactions =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_permutations_with_repetition, true>(
          vw.interactions, std::set<namespace_index>(ex->indices.begin(), ex->indices.end()));
  ex->interactions = &interactions;
  ex->extent_interactions = &vw.extent_interactions;
  size_t fast_features_count;
  float fast_features_value;
  INTERACTIONS::eval_count_of_generated_ft(vw.permutations, *ex->interactions, *ex->extent_interactions,
      ex->feature_space, fast_features_count, fast_features_value);
  ex->interactions = &vw.interactions;

  vw.predict(*ex);
  BOOST_CHECK_EQUAL(fast_features_count, ex->num_features_from_interactions);

  VW::finish_example(vw, *ex);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(interaction_generic_expand_wildcard_only)
{
  std::set<namespace_index> namespaces = {'a', 'b'};
  auto result = INTERACTIONS::generate_namespace_combinations_with_repetition(namespaces, 2);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(interaction_generic_with_duplicates_expand_wildcard_only)
{
  std::set<namespace_index> namespaces = {'a', 'b'};
  auto result = INTERACTIONS::generate_namespace_permutations_with_repetition(namespaces, 2);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'b'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(sort_and_filter_interactions)
{
  std::vector<std::vector<namespace_index>> input = {{'b', 'a'}, {'a', 'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

  size_t removed_count = 0;
  size_t sorted_count = 0;
  INTERACTIONS::sort_and_filter_duplicate_interactions(input, false, removed_count, sorted_count);

  std::vector<std::vector<namespace_index>> compare_set = {{'b', 'a'}, {'a', 'a', 'b'}, {'a', 'a'}, {'b', 'b'}};
  check_vector_of_vectors_exact(input, compare_set);
}

template <typename T>
void sort_all(std::vector<std::vector<T>>& interactions)
{
  for (auto& interaction : interactions) { std::sort(interaction.begin(), interaction.end()); }
  std::sort(interactions.begin(), interactions.end());
}

BOOST_AUTO_TEST_CASE(compile_interactions_quadratic_permutations_and_combinations_same)
{
  std::set<namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<namespace_index>> interactions = {{':', 'a'}};

  // Permutations implies leave duplicate interactions (second template arg)
  auto result_perms =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_permutations_with_repetition, true>(
          interactions, indices);
  auto result_combs =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_combinations_with_repetition, false>(
          interactions, indices);

  std::vector<std::vector<namespace_index>> compare_set = {{'a', 'a'}, {'b', 'a'}, {'c', 'a'}, {'d', 'a'}};

  sort_all(compare_set);
  sort_all(result_perms);
  sort_all(result_combs);
  check_vector_of_vectors_exact(result_perms, compare_set);
  check_vector_of_vectors_exact(result_combs, compare_set);
}

BOOST_AUTO_TEST_CASE(compile_interactions_quadratic_combinations)
{
  std::set<namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<namespace_index>> interactions = {{':', ':'}};

  auto result =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_combinations_with_repetition, false>(
          interactions, indices);

  std::vector<std::vector<namespace_index>> compare_set = {{'a', 'a'}, {'a', 'b'}, {'a', 'c'}, {'a', 'd'}, {'b', 'b'},
      {'b', 'c'}, {'b', 'd'}, {'c', 'c'}, {'c', 'd'}, {'d', 'd'}};

  sort_all(compare_set);
  sort_all(result);
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(compile_interactions_quadratic_permutations)
{
  std::set<namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<namespace_index>> interactions = {{':', ':'}};

  auto result = INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_permutations_with_repetition, true>(
      interactions, indices);

  std::vector<std::vector<namespace_index>> compare_set = {{'a', 'a'}, {'a', 'b'}, {'a', 'c'}, {'a', 'd'}, {'b', 'a'},
      {'b', 'b'}, {'b', 'c'}, {'b', 'd'}, {'c', 'a'}, {'c', 'b'}, {'c', 'c'}, {'c', 'd'}, {'d', 'a'}, {'d', 'b'},
      {'d', 'c'}, {'d', 'd'}};

  sort_all(compare_set);
  sort_all(result);
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(compile_interactions_cubic_combinations)
{
  std::set<namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<namespace_index>> interactions = {{':', ':', ':'}};

  auto result =
      INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_combinations_with_repetition, false>(
          interactions, indices);

  std::vector<std::vector<namespace_index>> compare_set = {
      {'a', 'a', 'a'},
      {'a', 'a', 'b'},
      {'a', 'a', 'c'},
      {'a', 'a', 'd'},
      {'a', 'b', 'b'},
      {'a', 'b', 'c'},
      {'a', 'b', 'd'},
      {'a', 'c', 'c'},
      {'a', 'c', 'd'},
      {'a', 'd', 'd'},
      {'b', 'b', 'b'},
      {'b', 'b', 'c'},
      {'b', 'b', 'd'},
      {'b', 'c', 'c'},
      {'b', 'c', 'd'},
      {'b', 'd', 'd'},
      {'c', 'c', 'c'},
      {'c', 'c', 'd'},
      {'c', 'd', 'd'},
      {'d', 'd', 'd'},
  };

  sort_all(compare_set);
  sort_all(result);
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(compile_interactions_cubic_permutations)
{
  std::set<namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<namespace_index>> interactions = {{':', ':', ':'}};

  auto result = INTERACTIONS::compile_interactions<INTERACTIONS::generate_namespace_permutations_with_repetition, true>(
      interactions, indices);

  std::vector<std::vector<namespace_index>> compare_set = {
      {'a', 'a', 'a'}, {'a', 'a', 'b'}, {'a', 'a', 'c'}, {'a', 'a', 'd'}, {'a', 'b', 'a'}, {'a', 'b', 'b'},
      {'a', 'b', 'c'}, {'a', 'b', 'd'}, {'a', 'c', 'a'}, {'a', 'c', 'b'}, {'a', 'c', 'c'}, {'a', 'c', 'd'},
      {'a', 'd', 'a'}, {'a', 'd', 'b'}, {'a', 'd', 'c'}, {'a', 'd', 'd'}, {'b', 'a', 'a'}, {'b', 'a', 'b'},
      {'b', 'a', 'c'}, {'b', 'a', 'd'}, {'b', 'b', 'a'}, {'b', 'b', 'b'}, {'b', 'b', 'c'}, {'b', 'b', 'd'},
      {'b', 'c', 'a'}, {'b', 'c', 'b'}, {'b', 'c', 'c'}, {'b', 'c', 'd'}, {'b', 'd', 'a'}, {'b', 'd', 'b'},
      {'b', 'd', 'c'}, {'b', 'd', 'd'}, {'c', 'a', 'a'}, {'c', 'a', 'b'}, {'c', 'a', 'c'}, {'c', 'a', 'd'},
      {'c', 'b', 'a'}, {'c', 'b', 'b'}, {'c', 'b', 'c'}, {'c', 'b', 'd'}, {'c', 'c', 'a'}, {'c', 'c', 'b'},
      {'c', 'c', 'c'}, {'c', 'c', 'd'}, {'c', 'd', 'a'}, {'c', 'd', 'b'}, {'c', 'd', 'c'}, {'c', 'd', 'd'},
      {'d', 'a', 'a'}, {'d', 'a', 'b'}, {'d', 'a', 'c'}, {'d', 'a', 'd'}, {'d', 'b', 'a'}, {'d', 'b', 'b'},
      {'d', 'b', 'c'}, {'d', 'b', 'd'}, {'d', 'c', 'a'}, {'d', 'c', 'b'}, {'d', 'c', 'c'}, {'d', 'c', 'd'},
      {'d', 'd', 'a'}, {'d', 'd', 'b'}, {'d', 'd', 'c'}, {'d', 'd', 'd'}

  };

  sort_all(compare_set);
  sort_all(result);
  check_vector_of_vectors_exact(result, compare_set);
}

BOOST_AUTO_TEST_CASE(parse_full_name_interactions_test)
{
  auto* vw = VW::initialize("");

  {
    auto a = parse_full_name_interactions(*vw, "a|b");
    std::vector<extent_term> expected = {
        extent_term{'a', VW::hash_space(*vw, "a")}, extent_term{'b', VW::hash_space(*vw, "b")}};
    check_collections_exact(a, expected);
  }

  {
    auto a = parse_full_name_interactions(*vw, "art|bat|and");
    std::vector<extent_term> expected = {extent_term{'a', VW::hash_space(*vw, "art")},
        extent_term{'b', VW::hash_space(*vw, "bat")}, extent_term{'a', VW::hash_space(*vw, "and")}};
    check_collections_exact(a, expected);
  }

  {
    auto a = parse_full_name_interactions(*vw, "art|:|and");
    std::vector<extent_term> expected = {extent_term{'a', VW::hash_space(*vw, "art")},
        extent_term{wildcard_namespace, wildcard_namespace}, extent_term{'a', VW::hash_space(*vw, "and")}};
    check_collections_exact(a, expected);
  }

  BOOST_REQUIRE_THROW(parse_full_name_interactions(*vw, "||"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_full_name_interactions(*vw, "||||"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_full_name_interactions(*vw, "|a|||b"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_full_name_interactions(*vw, "abc|::"), VW::vw_exception);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(extent_vs_char_interactions)
{
  auto* vw_char_inter = VW::initialize("--quiet -q AB");
  auto* vw_extent_inter = VW::initialize("--quiet --new_full_interactions group1|group2");
  auto cleanup = VW::scope_exit(
      [&]()
      {
        VW::finish(*vw_char_inter);
        VW::finish(*vw_extent_inter);
      });

  auto parse_and_return_num_fts = [&](const char* char_inter_example,
                                      const char* extent_inter_example) -> std::pair<size_t, size_t>
  {
    auto* ex_char = VW::read_example(*vw_char_inter, char_inter_example);
    auto* ex_extent = VW::read_example(*vw_extent_inter, extent_inter_example);
    vw_char_inter->predict(*ex_char);
    vw_extent_inter->predict(*ex_extent);
    vw_char_inter->finish_example(*ex_char);
    vw_extent_inter->finish_example(*ex_extent);
    return std::make_pair(vw_char_inter->sd->total_features, vw_extent_inter->sd->total_features);
  };

  size_t num_char_fts = 0;
  size_t num_extent_fts = 0;

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group1 a b c |group2 a b c d");
  BOOST_REQUIRE_EQUAL(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group1 c |group1 a b |group2 a b c d");
  BOOST_REQUIRE_EQUAL(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group2 a d |group1 c |group1 a b |group2 b c");
  BOOST_REQUIRE_EQUAL(num_char_fts, num_extent_fts);
}

BOOST_AUTO_TEST_CASE(extent_interaction_expansion_test)
{
  auto* vw = VW::initialize("--quiet");
  auto* ex = VW::read_example(*vw, "|user_info a b c |user_geo a b c d |user_info a b |another a b c |extra a b |extra_filler a |extra a b |extra_filler a |extra a b");
  auto cleanup = VW::scope_exit(
      [&]()
      {
        VW::finish_example(*vw, *ex);
        VW::finish(*vw);
      });

  {
    const auto extent_terms = parse_full_name_interactions(*vw, "user_info|user_info");
    auto pairs = INTERACTIONS::generate_generic_extent_combination(ex->feature_space, extent_terms);
    BOOST_REQUIRE_EQUAL(pairs.size(), 3);
    BOOST_REQUIRE_EQUAL(pairs[0].size(), 2);
  }

  {
    const auto extent_terms = parse_full_name_interactions(*vw, "user_info|user_info|user_info");
    auto triples = INTERACTIONS::generate_generic_extent_combination(ex->feature_space, extent_terms);
    BOOST_REQUIRE_EQUAL(triples.size(), 4);
    BOOST_REQUIRE_EQUAL(triples[0].size(), 3);
  }

  {
    const auto extent_terms = parse_full_name_interactions(*vw, "user_info|extra");
    auto pairs = INTERACTIONS::generate_generic_extent_combination(ex->feature_space, extent_terms);
    BOOST_REQUIRE_EQUAL(pairs.size(), 6);
    BOOST_REQUIRE_EQUAL(pairs[0].size(), 2);
  }
}
