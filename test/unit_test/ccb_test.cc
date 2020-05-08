#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

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
  auto shared_ex = VW::read_example(vw, std::string("ccb shared |User f"));
  multi_ex actions;
  actions.push_back(VW::read_example(vw, std::string("ccb action |Action f")));
  actions.push_back(VW::read_example(vw, std::string("ccb action |Other f |Action f")));

  std::vector<std::vector<namespace_index>> interactions;
  std::vector<std::vector<namespace_index>> compare_set = {{'U', ccb_id_namespace}, {'A', ccb_id_namespace},
      {'O', ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());
  check_vector_of_vectors_exact(interactions, compare_set);

  interactions = {{'U','A'}, {'U','O'}, {'U','O','A'}};
  compare_set = {{'U','A'}, {'U','O'}, {'U','O','A'}, {'U', 'A', ccb_id_namespace}, {'U', 'O', ccb_id_namespace},
      {'U', 'O', 'A', ccb_id_namespace}, {'U', ccb_id_namespace}, {'A', ccb_id_namespace},
      {'O', ccb_id_namespace}};
  CCB::calculate_and_insert_interactions(shared_ex, actions, interactions);
  std::sort(compare_set.begin(), compare_set.end());
  std::sort(interactions.begin(), interactions.end());

  check_vector_of_vectors_exact(interactions, compare_set);
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
