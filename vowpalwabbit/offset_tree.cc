#include "offset_tree.h"
#include "offset_tree_internal.h"
#include "parse_args.h" // setup_base()
#include "learner.h"    // init_learner()

using namespace VW::config;
using namespace LEARNER;
namespace ot = offset_tree;

void learn(ot::offset_tree& tree, single_learner& base, example& ec) {
  THROW("Offset tree learn() - not yet impemented.");
}

void predict(ot::offset_tree& tree, single_learner& base, example& ec) {
  // get predictions for all internal nodes in binary tree.
  auto scores = tree.predict(base, ec);
}

base_learner* offset_tree_setup(VW::config::options_i& options, vw& all) {
  option_group_definition new_options("Offset tree Options");
  uint32_t num_actions;
  new_options.add(make_option("ot", num_actions).keep().help("Offset tree with <k> labels"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("ot"))
    return nullptr;

  // Ensure that cb_explore will be the base reduction
  if (!options.was_supplied("cb_explore")) {
    options.insert("cb_explore", "2");
  }

  auto offset_tree = scoped_calloc_or_throw<ot::offset_tree>();
  offset_tree->init(num_actions);

  base_learner* base = setup_base(options, all);

  all.delete_prediction = ACTION_SCORE::delete_action_scores;
  
  learner<ot::offset_tree, example>& l = init_learner(
    offset_tree, as_singleline(base), learn, predict, offset_tree->learner_count(), prediction_type::action_probs);

  return make_base(l);
}
