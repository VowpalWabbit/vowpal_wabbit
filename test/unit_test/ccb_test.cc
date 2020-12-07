#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "vw.h"
#include "example.h"

#include <vector>
#include "conditional_contextual_bandit.h"

namespace CCB
{
void inject_slot_features(example* shared, example* slot);
void remove_slot_features(example* shared, example* slot);
}  // namespace CCB

BOOST_AUTO_TEST_CASE(ccb_generate_interactions)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  auto shared_ex = VW::read_example(vw, std::string("ccb shared |User f"));
  multi_ex actions;
  actions.push_back(VW::read_example(vw, std::string("ccb action |Action f")));
  actions.push_back(VW::read_example(vw, std::string("ccb action |Other f |Action f")));

  std::vector<example*> slots;
  slots.push_back(VW::read_example(vw, std::string("ccb slot 0 |SlotNamespace f1 f2")));
  for (auto* slot : slots) { CCB::inject_slot_features(shared_ex, slot); }

  std::vector<std::vector<namespace_index>> interactions;
  std::vector<std::vector<namespace_index>> compare_set = {
      {'U', ccb_id_namespace}, {'A', ccb_id_namespace}, {'O', ccb_id_namespace}, {'S', ccb_id_namespace}};

  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());
  check_vector_of_vectors_exact(interactions, compare_set);

  interactions = {{'U', 'A'}, {'U', 'O'}, {'U', 'O', 'A'}};
  compare_set = {{'U', 'A'}, {'U', 'O'}, {'U', 'O', 'A'}, {'U', 'A', ccb_id_namespace}, {'U', 'O', ccb_id_namespace},
      {'U', 'O', 'A', ccb_id_namespace}, {'U', ccb_id_namespace}, {'A', ccb_id_namespace}, {'O', ccb_id_namespace},
      {'S', ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());

  check_vector_of_vectors_exact(interactions, compare_set);

  for (auto* slot : slots)
  {
    CCB::remove_slot_features(shared_ex, slot);
    VW::finish_example(vw, *slot);
  }
  VW::finish_example(vw, actions);
  VW::finish_example(vw, *shared_ex);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(ccb_generate_interactions_w_default_slot_namespaces)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  auto shared_ex = VW::read_example(vw, std::string("ccb shared |User f"));
  multi_ex actions;
  actions.push_back(VW::read_example(vw, std::string("ccb action |Action f")));
  actions.push_back(VW::read_example(vw, std::string("ccb action |Other f |Action f")));

  std::vector<example*> slots;
  slots.push_back(VW::read_example(vw, std::string("ccb slot 0 | f1 f2")));
  for (auto* slot : slots) { CCB::inject_slot_features(shared_ex, slot); }

  std::vector<std::vector<namespace_index>> interactions;
  std::vector<std::vector<namespace_index>> compare_set = {{'U', ccb_id_namespace}, {'A', ccb_id_namespace},
      {'O', ccb_id_namespace}, {ccb_slot_namespace, ccb_slot_namespace},
      {ccb_slot_namespace, ccb_slot_namespace, ccb_id_namespace}};

  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());
  check_vector_of_vectors_exact(interactions, compare_set);

  interactions = {{'U', 'A'}, {'U', 'O'}, {'U', 'O', 'A'}};
  compare_set = {{'U', 'A'}, {'U', 'O'}, {'U', 'O', 'A'}, {'U', 'A', ccb_id_namespace}, {'U', 'O', ccb_id_namespace},
      {'U', 'O', 'A', ccb_id_namespace}, {'U', ccb_id_namespace}, {'A', ccb_id_namespace}, {'O', ccb_id_namespace},
      {ccb_slot_namespace, ccb_slot_namespace}, {ccb_slot_namespace, ccb_slot_namespace, ccb_id_namespace},
      {'U', ccb_slot_namespace}, {'U', ccb_slot_namespace, ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());

  check_vector_of_vectors_exact(interactions, compare_set);

  for (auto* slot : slots)
  {
    CCB::remove_slot_features(shared_ex, slot);
    VW::finish_example(vw, *slot);
  }
  VW::finish_example(vw, actions);
  VW::finish_example(vw, *shared_ex);
  VW::finish(vw);
}

BOOST_AUTO_TEST_CASE(ccb_explicit_included_actions_no_overlap)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet");
  multi_ex examples;
  examples.push_back(VW::read_example(vw, std::string("ccb shared |")));
  examples.push_back(VW::read_example(vw, std::string("ccb action |")));
  examples.push_back(VW::read_example(vw, std::string("ccb action |")));
  examples.push_back(VW::read_example(vw, std::string("ccb action |")));
  examples.push_back(VW::read_example(vw, std::string("ccb action |")));
  examples.push_back(VW::read_example(vw, std::string("ccb slot 0 |")));
  examples.push_back(VW::read_example(vw, std::string("ccb slot 3 |")));
  examples.push_back(VW::read_example(vw, std::string("ccb slot 1 |")));

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
      push_many(examples[slot_example_indx]->tag, SEED_TAG.c_str(), SEED_TAG.size());
      push_many(examples[slot_example_indx]->tag, event_ids[i].c_str(), event_ids[i].size());
    }

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
