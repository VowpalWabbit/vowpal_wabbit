#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions_fwd.h"

namespace VW
{
namespace test_red
{
VW::LEARNER::base_learner* test_red_setup(VW::setup_base_i& stack_builder);

namespace helper
{
// // for debugging purposes
// void print_interactions(example* ec)
// {
//   if (ec == nullptr) return;
//   if (ec->interactions == nullptr) return;

//   std::cerr << "p:";  // << ec->interactions;

//   for (std::vector<namespace_index> v : *(ec->interactions))
//   {
//     for (namespace_index c : v) { std::cerr << " interaction:" << c << ","; }
//   }
//   std::cerr << std::endl;
// }

// // useful to understand what namespaces are used in the examples we are given
// // this can evolve to feed in data to generate possible interactions
// void print_all_namespaces_in_examples(multi_ex& exs)
// {
//   for (example* ex : exs)
//   {
//     for (auto i : ex->indices) { std::cerr << i << ", "; }
//     std::cerr << std::endl;
//   }
// }

// void print_all_preds(example& ex, size_t i)
// {
//   const auto& preds = ex.pred.a_s;
//   std::cerr << "config_" << i << ": ";
//   for (uint32_t i = 0; i < preds.size(); i++)
//   {
//     std::cerr << preds[i].action << "(" << preds[i].score << ")"
//               << ", ";
//   }
//   std::cerr << std::endl;
// }

// add an interaction to an existing instance
void add_interaction(std::vector<std::vector<namespace_index>>&, namespace_index, namespace_index);
void fail_if_enabled(vw&, std::string);
bool cmpf(float, float, float);
void print_weights_nonzero(vw*, size_t, dense_parameters&);
}  // namespace helper
}  // namespace test_red
}  // namespace VW
