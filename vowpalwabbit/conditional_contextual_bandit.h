#pragma once

#include <cstdint>
#include <vector>

#include "v_array.h"
#include "action_score.h"

struct vw;
struct example;
typedef std::vector<example*> multi_ex;

namespace CCB {
  // Each positon in outer array is implicitly the decision corresponding to that index. Each inner array is the result of CB for that call.
  typedef v_array<ACTION_SCORE::action_scores> decision_scores_t;

  struct conditional_contexual_bandit_outcome
  {
    // The cost of this class
    float cost;

    // The action chosen for this decision
    uint32_t action_id;

    // The index of the decision for this label, should this be implicit?
    uint32_t decision_id;

    // This is not well defined, the probability of the action that was chosen for this decision?
    float probability;

    // Is partial prediction required?
    //float partial_prediction;
  };

  enum example_type
  {
    shared, action, decision
  };

  struct label {
    example_type type;
    v_array<conditional_contexual_bandit_outcome> outcomes;
  };

  // TODO implement label parser for CCB

  bool ec_is_example_header(example& ec);
  void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores);
}
