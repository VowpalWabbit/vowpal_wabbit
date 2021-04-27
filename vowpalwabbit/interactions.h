// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "global_data.h"
#include "interactions_predict.h"

/*
 *  Interactions preprocessing and feature combinations generation
 */

namespace INTERACTIONS
{
/*
 *  Interactions preprocessing
 */

constexpr unsigned char printable_start = ' ';
constexpr unsigned char printable_end = '~';
constexpr unsigned char printable_ns_size = printable_end - printable_start;
constexpr uint64_t valid_ns_size =
    printable_end - printable_start - 1;  // -1 to skip characters ':' and '|' excluded in is_valid_ns()

inline constexpr bool is_printable_namespace(const unsigned char ns)
{
  return ns >= printable_start && ns <= printable_end;
}

// exand all wildcard namespaces in vector<string>
// req_length must be 0 if interactions of any length are allowed, otherwise contains required length
// err_msg will be printed plus exception will be thrown if req_length != 0 and mismatch interaction length.
std::vector<std::vector<namespace_index>> expand_interactions(
    const std::vector<std::vector<namespace_index>>& vec, const size_t required_length, const std::string& err_msg);

//void expand_quadratics_wildcard_interactions(namespace_interactions& interactions);

bool sort_interactions_comparator(const std::vector<namespace_index>& a, const std::vector<namespace_index>& b);

// remove duplicate interactions and sort namespaces in them (if required)
void sort_and_filter_duplicate_interactions(
    std::vector<std::vector<namespace_index>>& vec, bool filter_duplicates, size_t& removed_cnt, size_t& sorted_cnt);

/*
 *  Feature combinations generation
 */

// function estimates how many new features will be generated for example and ther sum(value^2).
void eval_count_of_generated_ft(
    const vw& all, const example_predict& ec, size_t& new_features_cnt, float& new_features_value);
void eval_count_of_generated_ft_combinations(
    const example_predict& ec, size_t& new_features_cnt, float& new_features_value);
void eval_count_of_generated_ft_permutations(
    const example_predict& ec, size_t& new_features_cnt, float& new_features_value);

template <class R, class S, void (*T)(R&, float, S), bool audit, void (*audit_func)(R&, const audit_strings*)>
inline void generate_interactions(vw& all, example_predict& ec, R& dat)
{
  if (all.weights.sparse)
    generate_interactions<R, S, T, audit, audit_func, sparse_parameters>(
        *ec.interactions, all.permutations, ec, dat, all.weights.sparse_weights);
  else
    generate_interactions<R, S, T, audit, audit_func, dense_parameters>(
        *ec.interactions, all.permutations, ec, dat, all.weights.dense_weights);
}

// this code is for C++98/03 complience as I unable to pass null function-pointer as template argument in g++-4.6
template <class R, class S, void (*T)(R&, float, S)>
inline void generate_interactions(vw& all, example_predict& ec, R& dat)
{
  if (all.weights.sparse)
    generate_interactions<R, S, T, sparse_parameters>(
        all.interactions, all.permutations, ec, dat, all.weights.sparse_weights);
  else
    generate_interactions<R, S, T, dense_parameters>(
        all.interactions, all.permutations, ec, dat, all.weights.dense_weights);
}

}  // namespace INTERACTIONS
