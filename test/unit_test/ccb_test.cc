// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "vw.h"
#include "example.h"

#include <vector>
#include "reductions/conditional_contextual_bandit.h"

namespace CCB
{
void inject_slot_features(example* shared, example* slot);
void remove_slot_features(example* shared, example* slot);
}  // namespace CCB

BOOST_AUTO_TEST_CASE(ccb_explicit_included_actions_no_overlap)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet");
  multi_ex examples;
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
  multi_ex examples;
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
