#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "vw.h"
#include "example.h"

#include <vector>
#include "conditional_contextual_bandit.h"

BOOST_AUTO_TEST_CASE(ccb_generate_interactions)
{
  auto& vw = *VW::initialize("--ccb_explore_adf --quiet", nullptr, false, nullptr, nullptr);
  auto shared_ex = VW::read_example(vw, "ccb shared |User f");
  multi_ex actions;
  actions.push_back(VW::read_example(vw, "ccb action |Action f"));
  actions.push_back(VW::read_example(vw, "ccb action |Other f |Action f"));

  std::vector<std::string> interactions;
  std::vector<std::string> compare_set = {{'U', (char)ccb_id_namespace}, {'A', (char)ccb_id_namespace},
      {'O', (char)ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());
  check_vectors(interactions, compare_set);

  interactions = {"UA", "UO", "UOA"};
  compare_set = {"UA", "UO", "UOA", {'U', 'A', (char)ccb_id_namespace}, {'U', 'O', (char)ccb_id_namespace},
      {'U', 'O', 'A', (char)ccb_id_namespace}, {'U', (char)ccb_id_namespace}, {'A', (char)ccb_id_namespace},
      {'O', (char)ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());

  check_vectors(interactions, compare_set);
  VW::finish_example(vw, actions);
  VW::finish_example(vw, *shared_ex);
  VW::finish(vw);
}


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
