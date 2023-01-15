// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "gmock/gmock.h"
#include "vw/core/example.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

TEST(Ccb, ExplicitIncludedActionsNoOverlap)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "ccb shared |"));
  examples.push_back(VW::read_example(*vw, "ccb action |"));
  examples.push_back(VW::read_example(*vw, "ccb action |"));
  examples.push_back(VW::read_example(*vw, "ccb action |"));
  examples.push_back(VW::read_example(*vw, "ccb action |"));
  examples.push_back(VW::read_example(*vw, "ccb slot 0 |"));
  examples.push_back(VW::read_example(*vw, "ccb slot 3 |"));
  examples.push_back(VW::read_example(*vw, "ccb slot 1 |"));

  vw->predict(examples);

  auto& decision_scores = examples[0]->pred.decision_scores;
  EXPECT_EQ(decision_scores.size(), 3);

  EXPECT_EQ(decision_scores[0].size(), 1);
  EXPECT_EQ(decision_scores[0][0].action, 0);
  EXPECT_FLOAT_EQ(decision_scores[0][0].score, 1.f);

  EXPECT_EQ(decision_scores[1].size(), 1);
  EXPECT_EQ(decision_scores[1][0].action, 3);
  EXPECT_FLOAT_EQ(decision_scores[1][0].score, 1.f);

  EXPECT_EQ(decision_scores[2].size(), 1);
  EXPECT_EQ(decision_scores[2][0].action, 1);
  EXPECT_FLOAT_EQ(decision_scores[2][0].score, 1.f);

  vw->finish_example(examples);
}

TEST(Ccb, ExplorationReproducibilityTest)
{
  auto vw = VW::initialize(
      vwtest::make_args("--ccb_explore_adf", "--epsilon", "0.2", "--dsjson", "--chain_hash", "--no_stdin", "--quiet"));

  std::vector<uint32_t> previous;
  const size_t iterations = 10;
  const std::vector<std::string> event_ids = {"slot1", "slot2"};
  static const std::string SEED_TAG = "seed=";
  for (size_t iteration = 0; iteration < iterations; ++iteration)
  {
    const std::string json =
        R"({"GUser":{"shared_feature":"feature"},"_multi":[{"TAction":{"feature1":3.0,"feature2":"name1"}},{"TAction":{"feature1":3.0,"feature2":"name1"}},{"TAction":{"feature1":3.0,"feature2":"name1"}}],"_slots":[{"_id":"slot1"},{"_id":"slot2"}]})";
    auto examples = vwtest::parse_json(*vw, json);
    for (size_t i = 0; i < event_ids.size(); i++)
    {
      const size_t slot_example_indx = examples.size() - event_ids.size() + i;
      examples[slot_example_indx]->tag.insert(examples[slot_example_indx]->tag.end(), SEED_TAG.begin(), SEED_TAG.end());
      examples[slot_example_indx]->tag.insert(
          examples[slot_example_indx]->tag.end(), event_ids[i].begin(), event_ids[i].end());
    }

    for (auto* ex : examples) { VW::setup_example(*vw, ex); }

    vw->predict(examples);
    auto& decision_scores = examples[0]->pred.decision_scores;
    std::vector<uint32_t> current;
    current.reserve(decision_scores.size());
    for (auto& decision_score : decision_scores) { current.push_back(decision_score[0].action); }
    if (!previous.empty())
    {
      EXPECT_EQ(current.size(), previous.size());
      for (size_t i = 0; i < current.size(); ++i) { EXPECT_EQ(current[i], previous[i]); }
    }
    previous = current;
    vw->finish_example(examples);
  }
}

TEST(Ccb, InvalidExampleChecks)
{
  auto vw = VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet"));
  VW::multi_ex examples;
  examples.push_back(VW::read_example(*vw, "ccb shared |"));
  examples.push_back(VW::read_example(*vw, "ccb action |"));
  examples.push_back(VW::read_example(*vw, "ccb slot 0 |"));
  examples.push_back(VW::read_example(*vw, "ccb slot 3 |"));

  for (auto* example : examples) { VW::setup_example(*vw, example); }

  // Check that number of actions is greater than slots
  EXPECT_THROW(vw->predict(examples), VW::vw_exception);
  EXPECT_THROW(vw->learn(examples), VW::vw_exception);

  vw->finish_example(examples);
}

std::string ns_to_str(unsigned char ns)
{
  if (ns == VW::details::CONSTANT_NAMESPACE) { return "[constant]"; }
  else if (ns == VW::details::CCB_SLOT_NAMESPACE) { return "[ccbslot]"; }
  else if (ns == VW::details::CCB_ID_NAMESPACE) { return "[ccbid]"; }
  else if (ns == VW::details::WILDCARD_NAMESPACE) { return "[wild]"; }
  else if (ns == VW::details::DEFAULT_NAMESPACE) { return "[default]"; }
  else { return std::string(1, ns); }
}

std::set<std::string> interaction_vec_t_to_set(const std::vector<std::vector<VW::namespace_index>>& interactions)
{
  std::set<std::string> result;
  std::stringstream ss;
  for (const std::vector<VW::namespace_index>& v : interactions)
  {
    for (VW::namespace_index c : v) { ss << ns_to_str(c); }
    result.insert(ss.str());
    ss.clear();
    ss.str("");
  }
  return result;
}

TEST(Ccb, InsertInteractionsImplTest)
{
  auto vw =
      VW::initialize(vwtest::make_args("--ccb_explore_adf", "--quiet", "-q", "AA", "-q", "BB", "-q", "AB", "-q", "::"));

  std::set<std::string> expected_before{"AA", "AB", "BB", "[wild][wild]"};
  std::set<std::string> expected_after{
      "AA", "AA[ccbid]", "AB", "AB[ccbid]", "BB", "BB[ccbid]", "[wild][ccbid]", "[wild][wild]", "[wild][wild][ccbid]"};

  auto pre_result = interaction_vec_t_to_set(vw->interactions);
  EXPECT_THAT(pre_result, testing::ContainerEq(expected_before));

  VW::reductions::ccb::insert_ccb_interactions(vw->interactions, vw->extent_interactions);
  auto result = interaction_vec_t_to_set(vw->interactions);

  EXPECT_THAT(result, testing::ContainerEq(expected_after));
}
