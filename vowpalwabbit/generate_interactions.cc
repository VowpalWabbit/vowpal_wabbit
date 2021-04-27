// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "reductions.h"
#include "v_array.h"

#include "io/logger.h"
#include "interactions.h"

using namespace VW::config;

namespace logger = VW::io::logger;

struct generate_interactions
{
};

void expand_generic_recur(std::vector<std::vector<namespace_index>>& result, std::vector<namespace_index>& combination, const std::vector<namespace_index>& namespaces, int offset, int k)
{
  if (k == 0) {
    result.push_back(combination);
    return;
  }
  for (int i = offset; i <= namespaces.size() - k; ++i) {
    combination.push_back(namespaces[i]);
    expand_generic_recur(result, combination, namespaces, i+1, k-1);
    combination.pop_back();
  }
}

std::vector<std::vector<namespace_index>> expand_generic(const std::vector<namespace_index>& namespaces, int k)
{
  const auto number_of_combinations = INTERACTIONS::choose(namespaces.size(), k);
  std::vector<std::vector<namespace_index>> result;
  result.reserve(number_of_combinations);
  std::vector<namespace_index> combination_store;
  expand_generic_recur(result, combination_store, namespaces, 0, k);
  return result;
}


// void expand_generic_no_duplicates(std::set<std::vector<namespace_index>>& result, std::vector<namespace_index>& combination, const std::vector<namespace_index>& namespaces, int offset, int k)
// {
//   if (k == 0) {
//     auto copy = combination;
//     std::sort(copy.begin(), copy.end());
//     result.insert(copy);
//     return;
//   }
//   for (int i = offset; i <= namespaces.size() - k; ++i) {
//     combination.push_back(namespaces[i]);
//     expand_generic_no_duplicates(result, combination, namespaces, i+1, k-1);
//     combination.pop_back();
//   }
// }



void transform_single_ex(generate_interactions& in, VW::LEARNER::single_learner& base, example& ec) {


  // if (all.interactions.quadratics_wildcard_expansion)
  // {
  //   // lock while adding interactions since reductions might also be adding their own interactions
  //   std::unique_lock<std::mutex> lock(all.interactions.mut);
  //   for (auto& ns : ae->indices)
  //   {
  //     if (ns < constant_namespace) { all.interactions.all_seen_namespaces.insert(ns); }
  //   }
  //   INTERACTIONS::expand_quadratics_wildcard_interactions(all.interactions);
  // }
}

VW::LEARNER::base_learner* generate_interactions_setup(options_i& options, vw& all)
{
  auto data = VW::make_unique<generate_interactions>();
  auto* base = as_singleline(setup_base(options, all));
  auto* l = VW::LEARNER::make_reduction_learner(std::move(data), base, transform_single_ex, transform_single_ex,
      all.get_setupfn_name(generate_interactions_setup)).build();
  return VW::LEARNER::make_base(*l);
}
