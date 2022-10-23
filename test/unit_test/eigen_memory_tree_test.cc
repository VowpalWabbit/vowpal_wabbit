// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/eigen_memory_tree.h"

#include "test_common.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/vw.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

using namespace VW::reductions::eigen_memory;

namespace eigen_memory_tree_test
{
tree* get_emt_tree(VW::workspace& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_reductions(e_r);
  if (std::find(e_r.begin(), e_r.end(), "eigen_memory_tree") == e_r.end())
  { BOOST_FAIL("Eigen memory tree not found in enabled reductions"); }

  VW::LEARNER::single_learner* emt = as_singleline(all.l->get_learner_by_name_prefix("eigen_memory_tree"));

  return (tree*)emt->get_internal_type_erased_data_pointer_test_use_only();
}

BOOST_AUTO_TEST_CASE(emt_params_test)
{
  auto vw = VW::initialize("--eigen_memory_tree --quiet");
  auto tree = get_emt_tree(*vw);

  BOOST_CHECK_EQUAL(tree->tree_bound, 0);
  BOOST_CHECK_EQUAL(tree->leaf_split, 100);
  BOOST_CHECK_EQUAL(tree->scorer_type, 3);
  BOOST_CHECK_EQUAL(tree->router_type, 1);

  VW::finish(*vw);

  vw = VW::initialize("--eigen_memory_tree --tree 20 --scorer 2 --router 3 --leaf 50 --quiet");
  tree = get_emt_tree(*vw);

  BOOST_CHECK_EQUAL(tree->tree_bound, 20);
  BOOST_CHECK_EQUAL(tree->leaf_split, 50);
  BOOST_CHECK_EQUAL(tree->scorer_type, 2);
  BOOST_CHECK_EQUAL(tree->router_type, 3);

  VW::finish(*vw);
}
BOOST_AUTO_TEST_CASE(emt_exact_match_sans_router_test)
{
  auto& vw = *VW::initialize("--eigen_memory_tree --quiet");
  auto tree = *get_emt_tree(vw);

  VW::example& ex1 = *VW::read_example(vw, "1 | 1 2 3");
  VW::example& ex2 = *VW::read_example(vw, "2 | 2 3 4");

  vw.learn(ex1);
  vw.learn(ex2);

  BOOST_CHECK_EQUAL(ex1.pred.multiclass, 0);
  BOOST_CHECK_EQUAL(ex2.pred.multiclass, 1);

  vw.predict(ex1);
  vw.predict(ex2);

  BOOST_CHECK_EQUAL(ex1.pred.multiclass, 1);
  BOOST_CHECK_EQUAL(ex2.pred.multiclass, 2);

  vw.finish_example(ex1);
  vw.finish_example(ex2);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(emt_exact_match_with_router_test)
{
  auto& vw = *VW::initialize("--eigen_memory_tree --quiet --leaf 5");
  auto tree = *get_emt_tree(vw);

  std::vector<VW::example*> examples;

  for (int i = 0; i < 10; i++)
  {
    examples.push_back(VW::read_example(vw, std::to_string(i) + " | " + std::to_string(i)));
    vw.learn(*examples[i]);
  }

  for (int i = 0; i < 10; i++)
  {
    vw.predict(*examples[i]);
    BOOST_CHECK_EQUAL(examples[i]->pred.multiclass, i);
    vw.finish_example(*examples[i]);
  }

  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(emt_bounding)
{
  auto& vw = *VW::initialize("--eigen_memory_tree --quiet --tree 5");
  auto tree = *get_emt_tree(vw);

  std::vector<VW::example*> examples;

  for (int i = 0; i < 10; i++)
  {
    examples.push_back(VW::read_example(vw, std::to_string(i) + " | " + std::to_string(i)));
    vw.learn(*examples[i]);
  }

  BOOST_CHECK_EQUAL(tree.bounder->list.size(), 5);
  BOOST_CHECK_EQUAL(tree.root->examples.size(), 5);
  BOOST_CHECK_EQUAL(tree.root->router_weights, nullptr);

  for (int i = 0; i < 10; i++) { vw.finish_example(*examples[i]); }
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(emt_split)
{
  auto& vw = *VW::initialize("--eigen_memory_tree --quiet --leaf 3 --tree 10");
  auto tree = *get_emt_tree(vw);

  std::vector<VW::example*> examples;

  for (int i = 0; i < 4; i++)
  {
    examples.push_back(VW::read_example(vw, std::to_string(i) + " | " + std::to_string(i)));
    vw.learn(*examples[i]);
  }

  BOOST_CHECK_EQUAL(tree.bounder->list.size(), 4);

  BOOST_CHECK_EQUAL(tree.root->examples.size(), 0);
  BOOST_CHECK_EQUAL(tree.root->left->examples.size(), 2);
  BOOST_CHECK_EQUAL(tree.root->right->examples.size(), 2);

  BOOST_CHECK_NE(tree.root->router_weights, nullptr);
  BOOST_CHECK_EQUAL(tree.root->right->router_weights, nullptr);
  BOOST_CHECK_EQUAL(tree.root->left->router_weights, nullptr);

  for (int i = 0; i < 4; i++) { vw.finish_example(*examples[i]); }
  VW::finish(vw);
}
}  // namespace eigen_memory_tree_test
