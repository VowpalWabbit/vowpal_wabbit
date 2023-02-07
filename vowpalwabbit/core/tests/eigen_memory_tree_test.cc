// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/eigen_memory_tree.h"

#include "vw/common/random.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace VW::reductions::eigen_memory_tree;

namespace eigen_memory_tree_test
{
emt_tree* get_emt_tree(VW::workspace& all)
{
  std::vector<std::string> e_r;
  all.l->get_enabled_learners(e_r);
  if (std::find(e_r.begin(), e_r.end(), "emt") == e_r.end())
  {
    ADD_FAILURE() << "Eigen memory tree not found in enabled learners";
  }

  VW::LEARNER::learner* emt = require_singleline(all.l->get_learner_by_name_prefix("emt"));

  return (emt_tree*)emt->get_internal_type_erased_data_pointer_test_use_only();
}

TEST(Emt, ParamsTest1)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--emt"));
  auto* tree = get_emt_tree(*vw);

  EXPECT_EQ(tree->leaf_split, 100);
  EXPECT_EQ(tree->scorer_type, emt_scorer_type::SELF_CONSISTENT_RANK);
  EXPECT_EQ(tree->router_type, emt_router_type::EIGEN);
  EXPECT_EQ(tree->bounder->max_size, 0);
}

TEST(Emt, ParamsTest2)
{
  auto args = vwtest::make_args(
      "--quiet", "--emt", "--emt_tree", "20", "--emt_scorer", "distance", "--emt_router", "random", "--emt_leaf", "50");
  auto vw = VW::initialize(std::move(args));
  auto tree = get_emt_tree(*vw);

  EXPECT_EQ(tree->leaf_split, 50);
  EXPECT_EQ(tree->scorer_type, emt_scorer_type::DISTANCE);
  EXPECT_EQ(tree->router_type, emt_router_type::RANDOM);
  EXPECT_EQ(tree->bounder->max_size, 20);
}

TEST(Emt, ExactMatchSansRouterTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--emt"));

  auto* ex1 = VW::read_example(*vw, "1 | 1 2 3");
  auto* ex2 = VW::read_example(*vw, "2 | 2 3 4");

  vw->learn(*ex1);
  vw->learn(*ex2);

  EXPECT_EQ(ex1->pred.multiclass, 0);
  EXPECT_EQ(ex2->pred.multiclass, 1);

  vw->predict(*ex1);
  vw->predict(*ex2);

  EXPECT_EQ(ex1->pred.multiclass, 1);
  EXPECT_EQ(ex2->pred.multiclass, 2);

  vw->finish_example(*ex1);
  vw->finish_example(*ex2);
}

TEST(Emt, ExactMatchWithRouterTest)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--emt", "--emt_leaf", "5"));

  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i) + " | " + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, " | " + std::to_string(i));
    vw->predict(*ex);
    EXPECT_EQ(ex->pred.multiclass, i);
    vw->finish_example(*ex);
  }
}

TEST(Emt, Bounding)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet", "--emt", "--emt_tree", "5"));
  auto* tree = get_emt_tree(*vw);

  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i) + " | " + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  EXPECT_EQ(tree->bounder->list.size(), 5);
  EXPECT_EQ(tree->root->examples.size(), 5);
  EXPECT_EQ(tree->root->router_weights.size(), 0);
}

TEST(Emt, Split)
{
  auto args = vwtest::make_args("--quiet", "--emt", "--emt_tree", "10", "--emt_leaf", "3");
  auto vw = VW::initialize(std::move(args));
  auto* tree = get_emt_tree(*vw);

  for (int i = 0; i < 4; i++)
  {
    auto* ex = VW::read_example(*vw, std::to_string(i) + " | " + std::to_string(i));
    vw->learn(*ex);
    vw->finish_example(*ex);
  }

  EXPECT_EQ(tree->bounder->list.size(), 4);

  EXPECT_EQ(tree->root->examples.size(), 0);
  EXPECT_EQ(tree->root->left->examples.size(), 2);
  EXPECT_EQ(tree->root->right->examples.size(), 2);

  EXPECT_GE(tree->root->router_weights.size(), 0);
  EXPECT_EQ(tree->root->right->router_weights.size(), 0);
  EXPECT_EQ(tree->root->left->router_weights.size(), 0);
}

TEST(Emt, Inner)
{
  emt_feats v1;
  emt_feats v2;

  EXPECT_EQ(emt_inner(v1, v2), 0);

  v1.emplace_back(1, 2);
  v2.emplace_back(2, 2);

  EXPECT_EQ(emt_inner(v1, v2), 0);

  v1.emplace_back(2, 3);

  EXPECT_EQ(emt_inner(v1, v2), 6);

  v1.emplace_back(3, 2);
  v2.emplace_back(3, 5);

  EXPECT_EQ(emt_inner(v1, v2), 16);
}

TEST(Emt, ScaleAdd)
{
  emt_feats v1;
  emt_feats v2;
  emt_feats v3;

  EXPECT_EQ(emt_scale_add(1, v1, 1, v2), v3);

  v1.emplace_back(1, 2);
  v3.emplace_back(1, 2);

  EXPECT_EQ(emt_scale_add(1, v1, 1, v2), v3);

  v3.clear();
  v3.emplace_back(1, -1);

  EXPECT_EQ(emt_scale_add(-.5, v1, 1, v2), v3);

  v1.clear();
  v2.clear();
  v3.clear();
  v1.emplace_back(1, 2);
  v2.emplace_back(1, 2.5);
  v3.emplace_back(1, -.5);

  EXPECT_EQ(emt_scale_add(1, v1, -1, v2), v3);

  v1.clear();
  v2.clear();
  v3.clear();
  v1.emplace_back(1, 2);
  v2.emplace_back(1, 2.5);
  v2.emplace_back(5, 1);
  v3.emplace_back(1, -4.5);
  v3.emplace_back(5, -1);

  EXPECT_EQ(emt_scale_add(-1, v1, -1, v2), v3);
}

TEST(Emt, Abs)
{
  emt_feats v1;
  emt_feats v2;

  emt_abs(v1);
  EXPECT_EQ(v1, v2);

  v1.emplace_back(1, -3);
  v2.emplace_back(1, 3);

  emt_abs(v1);
  EXPECT_EQ(v1, v2);

  v1.emplace_back(2, -4);
  v2.emplace_back(2, 4);

  emt_abs(v1);
  EXPECT_EQ(v1, v2);
}

TEST(Emt, Normalize)
{
  emt_feats v1;
  emt_feats v2;

  emt_normalize(v1);
  EXPECT_EQ(v1, v2);

  v1.emplace_back(1, -3);
  v1.emplace_back(5, 4);

  v2.emplace_back(1, -.6);
  v2.emplace_back(5, .8);

  emt_normalize(v1);
  EXPECT_EQ(v1, v2);
}

TEST(Emt, Median)
{
  std::vector<float> v1;

  v1.push_back(3);
  v1.push_back(1);
  v1.push_back(2);

  EXPECT_EQ(emt_median(v1), 2);

  v1.clear();
  v1.push_back(8);
  v1.push_back(2);
  v1.push_back(6);
  v1.push_back(4);

  EXPECT_EQ(emt_median(v1), 5);
}

TEST(Emt, RouterEigen)
{
  VW::rand_state rng(1);

  emt_feats v1;
  emt_feats v2;
  emt_feats v3;

  v1.emplace_back(1, 2);
  v1.emplace_back(3, 3);
  v1.emplace_back(5, 2);

  v2.emplace_back(2, 5);

  v3.emplace_back(1, 4);
  v3.emplace_back(2, 10);

  std::vector<emt_feats> f;

  f.push_back(v1);
  f.push_back(v2);
  f.push_back(v3);

  auto weights = emt_router_eigen(f, rng);

  float p1 = emt_inner(v1, weights);
  float p2 = emt_inner(v2, weights);
  float p3 = emt_inner(v3, weights);

  float E2 = (p1 * p1 + p2 * p2 + p3 * p3) / 3;
  float E_2 = (p1 + p2 + p3) * (p1 + p2 + p3) / 9;
  float var = E2 - E_2;

  // top eigen is [-0.1830,-0.9244,0.2782,0,0.1855]
  // which has a projection variance of 19.5
  // our oja method gave us 19.29 variance

  EXPECT_NEAR(weights[0].second, -0.0863, .01);
  EXPECT_NEAR(weights[1].second, -0.9171, .01);
  EXPECT_NEAR(weights[2].second, 0.3237, .01);
  EXPECT_NEAR(weights[3].second, 0.2158, .01);
  EXPECT_GE(var, 19.29);
}

TEST(Emt, Shuffle)
{
  VW::rand_state rng(2);

  std::vector<int> v1{1, 2, 3};

  emt_shuffle(v1.begin(), v1.end(), rng);

  EXPECT_EQ(v1[0], 2);
  EXPECT_EQ(v1[1], 3);
  EXPECT_EQ(v1[2], 1);
}

TEST(Emt, SaveLoad)
{
  auto vw_save = VW::initialize(vwtest::make_args("--quiet", "--emt", "--emt_leaf", "5"));

  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw_save, std::to_string(i) + " | " + std::to_string(i));
    vw_save->learn(*ex);
    vw_save->finish_example(*ex);
  }

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw_save, io_writer);
  io_writer.flush();

  auto vw_load = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--preserve_performance_counters"),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  for (int i = 0; i < 10; i++)
  {
    auto* ex = VW::read_example(*vw_load, " | " + std::to_string(i));
    vw_load->predict(*ex);
    EXPECT_EQ(ex->pred.multiclass, i);
    vw_load->finish_example(*ex);
  }
}

}  // namespace eigen_memory_tree_test
