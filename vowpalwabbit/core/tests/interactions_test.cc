// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/interactions.h"

#include "vw/core/constant.h"
#include "vw/core/gd_predict.h"
#include "vw/core/interactions_predict.h"
#include "vw/core/parse_args.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/scope_exit.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using namespace ::testing;

namespace std
{
std::ostream& operator<<(std::ostream& os, const std::pair<VW::namespace_index, uint64_t>& obj)
{
  return os << "_ns_char: " << obj.first << " _ns_hash: " << obj.second;
}
}  // namespace std

class eval_gen_data
{
public:
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
// use it to validate the more fast (?) analytic solution
template <VW::generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
void eval_count_of_generated_ft_naive(
    VW::workspace& all, VW::example_predict& ec, size_t& new_features_cnt, float& new_features_value)
{
  // Only makes sense to do this when not in permutations mode.
  assert(!all.permutations);

  new_features_cnt = 0;
  new_features_value = 0.;

  auto interactions = VW::details::compile_interactions<generate_func, leave_duplicate_interactions>(
      all.interactions, std::set<VW::namespace_index>(ec.indices.begin(), ec.indices.end()));

  VW::v_array<float> results;

  eval_gen_data dat(new_features_cnt, new_features_value);
  size_t ignored = 0;
  ec.interactions = &interactions;
  VW::generate_interactions<eval_gen_data, uint64_t, ft_cnt, false, nullptr>(all, ec, dat, ignored);
  ec.interactions = &all.interactions;
}

template <VW::generate_func_t<VW::extent_term> generate_func, bool leave_duplicate_interactions>
void eval_count_of_generated_ft_naive(
    VW::workspace& all, VW::example_predict& ec, size_t& new_features_cnt, float& new_features_value)
{
  // Only makes sense to do this when not in permutations mode.
  assert(!all.permutations);

  new_features_cnt = 0;
  new_features_value = 0.;
  std::set<VW::extent_term> seen_extents;
  for (auto ns_index : ec.indices)
  {
    for (const auto& extent : ec.feature_space[ns_index].namespace_extents)
    {
      seen_extents.insert({ns_index, extent.hash});
    }
  }

  auto interactions = VW::details::compile_extent_interactions<generate_func, leave_duplicate_interactions>(
      all.extent_interactions, seen_extents);

  VW::v_array<float> results;

  eval_gen_data dat(new_features_cnt, new_features_value);
  size_t ignored = 0;
  ec.extent_interactions = &interactions;
  VW::generate_interactions<eval_gen_data, uint64_t, ft_cnt, false, nullptr>(all, ec, dat, ignored);
  ec.extent_interactions = &all.extent_interactions;
}

inline void noop_func(float& /* unused_dat */, const float /* ft_weight */, const uint64_t /* ft_idx */) {}

TEST(Interactions, EvalCountOfGeneratedFtTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "-q", "::", "--noconstant"));
  auto* ex = VW::read_example(*vw, "3 |f a b c |e x y z");

  size_t naive_features_count;
  float naive_features_value;
  eval_count_of_generated_ft_naive<VW::details::generate_namespace_combinations_with_repetition<VW::namespace_index>,
      false>(*vw, *ex, naive_features_count, naive_features_value);

  auto interactions =
      VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
          vw->interactions, std::set<VW::namespace_index>(ex->indices.begin(), ex->indices.end()));
  ex->interactions = &interactions;
  ex->extent_interactions = &vw->extent_interactions;
  float fast_features_value = VW::eval_sum_ft_squared_of_generated_ft(
      vw->permutations, *ex->interactions, *ex->extent_interactions, ex->feature_space);
  ex->interactions = &vw->interactions;

  EXPECT_FLOAT_EQ(naive_features_value, fast_features_value);

  // Prediction will count the interacted features, so we can compare that too.
  vw->predict(*ex);
  EXPECT_EQ(naive_features_count, ex->num_features_from_interactions);
  VW::finish_example(*vw, *ex);
}

TEST(Interactions, EvalCountOfGeneratedFtExtentsCombinationsTest)
{
  auto vw = VW::initialize(vwtest::make_args(
      "--quiet", "--experimental_full_name_interactions", "fff|eee|gg", "ggg|gg", "gg|gg|ggg", "--noconstant"));
  auto* ex = VW::read_example(*vw, "3 |fff a b c |eee x y z |ggg a b |gg c d");

  size_t naive_features_count;
  float naive_features_value;
  eval_count_of_generated_ft_naive<VW::details::generate_namespace_combinations_with_repetition<VW::extent_term>,
      false>(*vw, *ex, naive_features_count, naive_features_value);

  float fast_features_value = VW::eval_sum_ft_squared_of_generated_ft(
      vw->permutations, *ex->interactions, *ex->extent_interactions, ex->feature_space);
  ex->interactions = &vw->interactions;

  EXPECT_FLOAT_EQ(naive_features_value, fast_features_value);

  // Prediction will count the interacted features, so we can compare that too.
  vw->predict(*ex);
  EXPECT_EQ(naive_features_count, ex->num_features_from_interactions);
  VW::finish_example(*vw, *ex);
}

TEST(Interactions, EvalCountOfGeneratedFtExtentsPermutationsTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "-permutations", "--experimental_full_name_interactions",
      "fff|eee|gg", "ggg|gg", "gg|gg|ggg", "--noconstant"));
  auto* ex = VW::read_example(*vw, "3 |fff a b c |eee x y z |ggg a b |gg c d");

  size_t naive_features_count;
  float naive_features_value;
  eval_count_of_generated_ft_naive<VW::details::generate_namespace_combinations_with_repetition<VW::extent_term>,
      false>(*vw, *ex, naive_features_count, naive_features_value);
  float fast_features_value = VW::eval_sum_ft_squared_of_generated_ft(
      vw->permutations, *ex->interactions, *ex->extent_interactions, ex->feature_space);

  EXPECT_FLOAT_EQ(naive_features_value, fast_features_value);

  // Prediction will count the interacted features, so we can compare that too.
  vw->predict(*ex);
  EXPECT_EQ(naive_features_count, ex->num_features_from_interactions);
  VW::finish_example(*vw, *ex);
}

// TEST(InteractionsTests, InteractionGenericExpandWildcardOnly)
// {
//   std::set<VW::namespace_index> namespaces = {'a', 'b'};
//   auto result = VW::details::generate_namespace_combinations_with_repetition(namespaces, 2);

//   std::vector<std::vector<VW::namespace_index>> compare_set = {{'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

//   std::sort(compare_set.begin(), compare_set.end());
//   std::sort(result.begin(), result.end());
//   check_vector_of_vectors_exact(result, compare_set);
// }

TEST(Interactions, InteractionGenericWithDuplicatesExpandWildcardOnly)
{
  std::set<VW::namespace_index> namespaces = {'a', 'b'};
  auto result = VW::details::generate_namespace_permutations_with_repetition(namespaces, 2);

  std::vector<std::vector<VW::namespace_index>> compare_set = {{'b', 'a'}, {'a', 'b'}, {'a', 'a'}, {'b', 'b'}};

  std::sort(compare_set.begin(), compare_set.end());
  std::sort(result.begin(), result.end());
  EXPECT_THAT(result, ContainerEq(compare_set));
}

TEST(Interactions, SortAndFilterInteractions)
{
  std::vector<std::vector<VW::namespace_index>> input = {{'b', 'a'}, {'a', 'b', 'a'}, {'a', 'a'}, {'b', 'b'}};

  size_t removed_count = 0;
  size_t sorted_count = 0;
  VW::details::sort_and_filter_duplicate_interactions(input, false, removed_count, sorted_count);

  std::vector<std::vector<VW::namespace_index>> compare_set = {{'b', 'a'}, {'a', 'a', 'b'}, {'a', 'a'}, {'b', 'b'}};
  EXPECT_THAT(input, ContainerEq(compare_set));
}

template <typename T>
void sort_all(std::vector<std::vector<T>>& interactions)
{
  for (auto& interaction : interactions) { std::sort(interaction.begin(), interaction.end()); }
  std::sort(interactions.begin(), interactions.end());
}

TEST(Interactions, CompileInteractionsQuadraticPermutationsAndCombinationsSame)
{
  std::set<VW::namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<VW::namespace_index>> interactions = {{':', 'a'}};

  // Permutations implies leave duplicate interactions (second template arg)
  auto result_perms =
      VW::details::compile_interactions<VW::details::generate_namespace_permutations_with_repetition, true>(
          interactions, indices);
  auto result_combs =
      VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
          interactions, indices);

  std::vector<std::vector<VW::namespace_index>> compare_set = {{'a', 'a'}, {'b', 'a'}, {'c', 'a'}, {'d', 'a'}};

  sort_all(compare_set);
  sort_all(result_perms);
  sort_all(result_combs);
  EXPECT_THAT(result_perms, ContainerEq(compare_set));
  EXPECT_THAT(result_combs, ContainerEq(compare_set));
}

TEST(Interactions, CompileInteractionsQuadraticCombinations)
{
  std::set<VW::namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<VW::namespace_index>> interactions = {{':', ':'}};

  auto result = VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
      interactions, indices);

  std::vector<std::vector<VW::namespace_index>> compare_set = {{'a', 'a'}, {'a', 'b'}, {'a', 'c'}, {'a', 'd'},
      {'b', 'b'}, {'b', 'c'}, {'b', 'd'}, {'c', 'c'}, {'c', 'd'}, {'d', 'd'}};

  sort_all(compare_set);
  sort_all(result);
  EXPECT_THAT(result, ContainerEq(compare_set));
}

TEST(Interactions, CompileInteractionsQuadraticPermutations)
{
  std::set<VW::namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<VW::namespace_index>> interactions = {{':', ':'}};

  auto result = VW::details::compile_interactions<VW::details::generate_namespace_permutations_with_repetition, true>(
      interactions, indices);

  std::vector<std::vector<VW::namespace_index>> compare_set = {{'a', 'a'}, {'a', 'b'}, {'a', 'c'}, {'a', 'd'},
      {'b', 'a'}, {'b', 'b'}, {'b', 'c'}, {'b', 'd'}, {'c', 'a'}, {'c', 'b'}, {'c', 'c'}, {'c', 'd'}, {'d', 'a'},
      {'d', 'b'}, {'d', 'c'}, {'d', 'd'}};

  sort_all(compare_set);
  sort_all(result);
  EXPECT_THAT(result, ContainerEq(compare_set));
}

TEST(Interactions, CompileInteractionsCubicCombinations)
{
  std::set<VW::namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<VW::namespace_index>> interactions = {{':', ':', ':'}};

  auto result = VW::details::compile_interactions<VW::details::generate_namespace_combinations_with_repetition, false>(
      interactions, indices);

  std::vector<std::vector<VW::namespace_index>> compare_set = {
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
  EXPECT_THAT(result, ContainerEq(compare_set));
}

TEST(Interactions, CompileInteractionsCubicPermutations)
{
  std::set<VW::namespace_index> indices = {'a', 'b', 'c', 'd'};
  std::vector<std::vector<VW::namespace_index>> interactions = {{':', ':', ':'}};

  auto result = VW::details::compile_interactions<VW::details::generate_namespace_permutations_with_repetition, true>(
      interactions, indices);

  std::vector<std::vector<VW::namespace_index>> compare_set = {
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
  EXPECT_THAT(result, ContainerEq(compare_set));
}

TEST(Interactions, ParseFullNameInteractionsTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  {
    auto a = VW::details::parse_full_name_interactions(*vw, "a|b");
    std::vector<VW::extent_term> expected = {
        VW::extent_term{'a', VW::hash_space(*vw, "a")}, VW::extent_term{'b', VW::hash_space(*vw, "b")}};
    EXPECT_THAT(a, ContainerEq(expected));
  }

  {
    auto a = VW::details::parse_full_name_interactions(*vw, "art|bat|and");
    std::vector<VW::extent_term> expected = {VW::extent_term{'a', VW::hash_space(*vw, "art")},
        VW::extent_term{'b', VW::hash_space(*vw, "bat")}, VW::extent_term{'a', VW::hash_space(*vw, "and")}};
    EXPECT_THAT(a, ContainerEq(expected));
  }

  {
    auto a = VW::details::parse_full_name_interactions(*vw, "art|:|and");
    std::vector<VW::extent_term> expected = {VW::extent_term{'a', VW::hash_space(*vw, "art")},
        VW::extent_term{VW::details::WILDCARD_NAMESPACE, VW::details::WILDCARD_NAMESPACE},
        VW::extent_term{'a', VW::hash_space(*vw, "and")}};
    EXPECT_THAT(a, ContainerEq(expected));
  }

  EXPECT_THROW(VW::details::parse_full_name_interactions(*vw, "||"), VW::vw_exception);
  EXPECT_THROW(VW::details::parse_full_name_interactions(*vw, "||||"), VW::vw_exception);
  EXPECT_THROW(VW::details::parse_full_name_interactions(*vw, "|a|||b"), VW::vw_exception);
  EXPECT_THROW(VW::details::parse_full_name_interactions(*vw, "abc|::"), VW::vw_exception);
}

TEST(Interactions, ExtentVsCharInteractions)
{
  auto vw_char_inter = VW::initialize(vwtest::make_args("--quiet", "-q", "AB"));
  auto vw_extent_inter =
      VW::initialize(vwtest::make_args("--quiet", "--experimental_full_name_interactions", "group1|group2"));

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
  EXPECT_EQ(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group1 c |group1 a b |group2 a b c d");
  EXPECT_EQ(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group2 a d |group1 c |group1 a b |group2 b c");
  EXPECT_EQ(num_char_fts, num_extent_fts);
}

TEST(Interactions, ExtentInteractionExpansionTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  auto* ex = VW::read_example(*vw,
      "|user_info a b c |user_geo a b c d |user_info a b |another a b c |extra a b |extra_filler a |extra a b "
      "|extra_filler a |extra a b");
  auto cleanup = VW::scope_exit([&]() { VW::finish_example(*vw, *ex); });

  VW::details::generate_interactions_object_cache cache;

  {
    const auto extent_terms = VW::details::parse_full_name_interactions(*vw, "user_info|user_info");
    size_t counter = 0;
    VW::details::generate_generic_extent_combination_iterative(
        ex->feature_space, extent_terms,
        [&](const std::vector<VW::details::features_range_t>& combination)
        {
          counter++;
          EXPECT_EQ(combination.size(), 2);
        },
        cache.in_process_frames, cache.frame_pool);
    EXPECT_EQ(counter, 3);
  }

  {
    const auto extent_terms = VW::details::parse_full_name_interactions(*vw, "user_info|user_info|user_info");
    size_t counter = 0;
    VW::details::generate_generic_extent_combination_iterative(
        ex->feature_space, extent_terms,
        [&](const std::vector<VW::details::features_range_t>& combination)
        {
          counter++;
          EXPECT_EQ(combination.size(), 3);
        },
        cache.in_process_frames, cache.frame_pool);
    EXPECT_EQ(counter, 4);
  }

  {
    const auto extent_terms = VW::details::parse_full_name_interactions(*vw, "user_info|extra");
    size_t counter = 0;
    VW::details::generate_generic_extent_combination_iterative(
        ex->feature_space, extent_terms,
        [&](const std::vector<VW::details::features_range_t>& combination)
        {
          counter++;
          EXPECT_EQ(combination.size(), 2);
        },
        cache.in_process_frames, cache.frame_pool);
    EXPECT_EQ(counter, 6);
  }
}

void do_interaction_feature_count_test(bool add_quadratic, bool add_cubic, bool combinations, bool no_constant)
{
  std::vector<std::string> char_cmd_line{"--quiet"};
  std::vector<std::string> extent_cmd_line{"--quiet"};
  if (add_quadratic)
  {
    char_cmd_line.emplace_back("--quadratic=::");
    extent_cmd_line.emplace_back("--experimental_full_name_interactions=:|:");
  }
  if (add_cubic)
  {
    char_cmd_line.emplace_back("--cubic=:::");
    extent_cmd_line.emplace_back("--experimental_full_name_interactions=:|:|:");
  }
  if (!combinations)
  {
    char_cmd_line.emplace_back("--leave_duplicate_interactions");
    extent_cmd_line.emplace_back("--leave_duplicate_interactions");
  }
  if (no_constant)
  {
    char_cmd_line.emplace_back("--noconstant");
    extent_cmd_line.emplace_back("--noconstant");
  }
  auto vw_char_inter = VW::initialize(VW::make_unique<VW::config::options_cli>(char_cmd_line));
  auto vw_extent_inter = VW::initialize(VW::make_unique<VW::config::options_cli>(extent_cmd_line));
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
  EXPECT_EQ(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group1 c |group1 a b |group2 a b c d");
  EXPECT_EQ(num_char_fts, num_extent_fts);

  std::tie(num_char_fts, num_extent_fts) =
      parse_and_return_num_fts("|A a b c |B a b c d", "|group2 a d |group1 c |group1 a b |group2 b c");
  EXPECT_EQ(num_char_fts, num_extent_fts);
}

TEST(Interactions, extentVsCharInteractionsWildcard) { do_interaction_feature_count_test(true, false, true, true); }
TEST(Interactions, ExtentVsCharInteractionsCubicWildcard) { do_interaction_feature_count_test(true, true, true, true); }

TEST(Interactions, ExtentVsCharInteractionsWildcardPermutations)
{
  do_interaction_feature_count_test(true, false, false, true);
}
TEST(Interactions, ExtentVsCharInteractionsCubicWildcardPermutations)
{
  do_interaction_feature_count_test(true, true, false, true);
}

TEST(Interactions, ExtentVsCharInteractionsCubicWildcardPermutationsConstant)
{
  do_interaction_feature_count_test(true, true, false, false);
}

TEST(Interactions, ExtentVsCharInteractionsCubicWildcardPermutationsCombinationsConstant)
{
  do_interaction_feature_count_test(true, true, true, false);
}