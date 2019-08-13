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
  auto& vw = *VW::initialize("--ccb_explore_adf", nullptr, false, nullptr, nullptr);
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
  VW::clear_seq_and_finish_examples(vw, actions);
  VW::finish_example(vw, *shared_ex);
  VW::finish(vw);
}

