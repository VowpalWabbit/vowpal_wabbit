#pragma once

#include <cstdint>
#include <vector>

#include "label_parser.h"
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

    //// The index of the decision for this label, should this be implicit?
    //uint32_t decision_id;

    // Either probability for top action or for all actions in action set.
    ACTION_SCORE::action_scores probabilities;
  };

  enum example_type : uint8_t
  {
    unset = 0,
    shared = 1,
    action = 2,
    decision = 3
  };

  struct label {
    example_type type;
    // Outcome may be unset.
    conditional_contexual_bandit_outcome* outcome;
    v_array<uint32_t> explicit_included_actions;
  };

  extern label_parser ccb_label_parser;

  bool ec_is_example_header(example& ec);
  void print_update(vw& all, bool is_test, example& ec, multi_ex* ec_seq, bool action_scores);
}
