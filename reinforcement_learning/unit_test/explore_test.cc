#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../../explore/explore.h"
#include "ranking_response.h"

const int NUM_ACTIONS = 10;
namespace e = exploration;
namespace r = reinforcement_learning;

BOOST_AUTO_TEST_CASE(basic_explore_test) {
  const float epsilon = 0.2f;
  const auto top_action_id = 0;
  float pdf[NUM_ACTIONS];
  auto scode = e::generate_epsilon_greedy(epsilon, top_action_id, pdf, pdf + NUM_ACTIONS);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
  uint32_t chosen_index;
  scode = e::sample_after_normalizing(7791, pdf, pdf + NUM_ACTIONS, chosen_index);
  BOOST_CHECK_EQUAL(scode, S_EXPLORATION_OK);
}

BOOST_AUTO_TEST_CASE(swap_action_test) {
  r::ranking_response resp;
  std::vector<float> scores = { 1.0f,2.0f,3.0f,4.0f,5.0f };
  for ( auto f : scores )
    resp.push_back((int)f, f);

  e::swap_chosen(std::begin(resp), std::end(resp), 2);

  std::vector<float> expected_scores = { 3.0f,2.0f,1.0f,4.0f,5.0f };
  auto idx = 0;
  for ( auto ap : resp ) {
    BOOST_CHECK_EQUAL(ap.probability, expected_scores[idx]);
    BOOST_CHECK_EQUAL(ap.action_id, (int)expected_scores[idx++]);
  }
}
