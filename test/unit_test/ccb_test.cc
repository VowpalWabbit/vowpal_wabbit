// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "test_common.h"
#include "vw/core/example.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

namespace CCB
{
void inject_slot_features(VW::example* shared, VW::example* slot);
void remove_slot_features(VW::example* shared, VW::example* slot);
}  // namespace CCB

BOOST_AUTO_TEST_CASE(ccb_explicit_included_actions_no_overlap)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet");
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "ccb shared |"));
  examples.push_back(VW::read_example(vw, "ccb action |"));
  examples.push_back(VW::read_example(vw, "ccb action |"));
  examples.push_back(VW::read_example(vw, "ccb action |"));
  examples.push_back(VW::read_example(vw, "ccb action |"));
  examples.push_back(VW::read_example(vw, "ccb slot 0 |"));
  examples.push_back(VW::read_example(vw, "ccb slot 3 |"));
  examples.push_back(VW::read_example(vw, "ccb slot 1 |"));

  vw.predict(examples);

  auto& decision_scores = examples[0]->pred.decision_scores;
  BOOST_CHECK_EQUAL(decision_scores.size(), 3);

  BOOST_CHECK_EQUAL(decision_scores[0].size(), 1);
  BOOST_CHECK_EQUAL(decision_scores[0][0].action, 0);
  BOOST_CHECK_CLOSE(decision_scores[0][0].score, 1.f, FLOAT_TOL);

  BOOST_CHECK_EQUAL(decision_scores[1].size(), 1);
  BOOST_CHECK_EQUAL(decision_scores[1][0].action, 3);
  BOOST_CHECK_CLOSE(decision_scores[1][0].score, 1.f, FLOAT_TOL);

  BOOST_CHECK_EQUAL(decision_scores[2].size(), 1);
  BOOST_CHECK_EQUAL(decision_scores[2][0].action, 1);
  BOOST_CHECK_CLOSE(decision_scores[2][0].score, 1.f, FLOAT_TOL);

  vw.finish_example(examples);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(ccb_exploration_reproducibility_test)
{
  auto vw = VW::initialize(
      "--ccb_explore_adf --epsilon 0.2 --dsjson --chain_hash --no_stdin --quiet", nullptr, false, nullptr, nullptr);

  std::vector<uint32_t> previous;
  const size_t iterations = 10;
  const std::vector<std::string> event_ids = {"slot1", "slot2"};
  const std::string SEED_TAG = "seed=";
  for (size_t iteration = 0; iteration < iterations; ++iteration)
  {
    const std::string json =
        R"({"GUser":{"shared_feature":"feature"},"_multi":[{"TAction":{"feature1":3.0,"feature2":"name1"}},{"TAction":{"feature1":3.0,"feature2":"name1"}},{"TAction":{"feature1":3.0,"feature2":"name1"}}],"_slots":[{"_id":"slot1"},{"_id":"slot2"}]})";
    auto examples = parse_json(*vw, json);
    for (int i = 0; i < event_ids.size(); i++)
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
    for (size_t i = 0; i < decision_scores.size(); ++i) { current.push_back(decision_scores[i][0].action); }
    if (!previous.empty())
    {
      BOOST_CHECK_EQUAL(current.size(), previous.size());
      for (size_t i = 0; i < current.size(); ++i) { BOOST_CHECK_EQUAL(current[i], previous[i]); }
    }
    previous = current;
    vw->finish_example(examples);
  }
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(ccb_invalid_example_checks)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet");
  VW::multi_ex examples;
  examples.push_back(VW::read_example(vw, "ccb shared |"));
  examples.push_back(VW::read_example(vw, "ccb action |"));
  examples.push_back(VW::read_example(vw, "ccb slot 0 |"));
  examples.push_back(VW::read_example(vw, "ccb slot 3 |"));

  for (auto* example : examples) { VW::setup_example(vw, example); }

  // Check that number of actions is greater than slots
  BOOST_REQUIRE_THROW(vw.predict(examples), VW::vw_exception);
  BOOST_REQUIRE_THROW(vw.learn(examples), VW::vw_exception);

  vw.finish_example(examples);
  VW::finish(vw);
}

std::string ns_to_str(unsigned char ns)
{
  if (ns == constant_namespace)
    return "[constant]";
  else if (ns == ccb_slot_namespace)
    return "[ccbslot]";
  else if (ns == ccb_id_namespace)
    return "[ccbid]";
  else if (ns == wildcard_namespace)
    return "[wild]";
  else if (ns == default_namespace)
    return "[default]";
  else
    return std::string(1, ns);
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

BOOST_AUTO_TEST_CASE(ccb_insert_interactions_impl_test)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet -q AA -q BB -q AB -q ::");

  std::set<std::string> expected_before{"AA", "AB", "BB", "[wild][wild]"};
  std::set<std::string> expected_after{
      "AA", "AA[ccbid]", "AB", "AB[ccbid]", "BB", "BB[ccbid]", "[wild][ccbid]", "[wild][wild]", "[wild][wild][ccbid]"};

  auto pre_result = interaction_vec_t_to_set(vw.interactions);
  BOOST_CHECK_EQUAL_COLLECTIONS(expected_before.begin(), expected_before.end(), pre_result.begin(), pre_result.end());

  VW::reductions::ccb::insert_ccb_interactions(vw.interactions, vw.extent_interactions);
  auto result = interaction_vec_t_to_set(vw.interactions);

  BOOST_CHECK_EQUAL_COLLECTIONS(expected_after.begin(), expected_after.end(), result.begin(), result.end());

  VW::finish(vw);
}
