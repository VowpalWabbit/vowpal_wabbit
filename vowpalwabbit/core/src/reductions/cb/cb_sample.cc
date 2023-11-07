// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_sample.h"

#include "vw/common/random.h"
#include "vw/common/string_view.h"
#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/tag_utils.h"
#include "vw/explore/explore.h"

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CB_SAMPLE

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;

namespace
{
// cb_sample is used to automatically sample and swap from a cb explore pdf.
class cb_sample_data
{
public:
  explicit cb_sample_data(std::shared_ptr<VW::rand_state>& random_state) : _random_state(random_state) {}
  explicit cb_sample_data(std::shared_ptr<VW::rand_state>&& random_state) : _random_state(random_state) {}

  template <bool is_learn>
  inline void learn_or_predict(learner& base, VW::multi_ex& examples)
  {
    // If base.learn() does not return prediction then we need to predict first
    // so that there is something to sample from
    VW_WARNING_STATE_PUSH
    VW_WARNING_DISABLE_COND_CONST_EXPR
    if (is_learn && !base.learn_returns_prediction)
    {
      multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
    }
    VW_WARNING_STATE_POP

    multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

    auto& action_scores = examples[0]->pred.a_s;

    uint32_t chosen_action = 0;
    int64_t maybe_labelled_action = -1;

    // Find that chosen action in the learning case, skip the shared example.
    auto it =
        std::find_if(examples.begin(), examples.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });
    if (it != examples.end()) { maybe_labelled_action = static_cast<int64_t>(std::distance(examples.begin(), it)); }

    // If we are learning and have a label, then take that action as the chosen action. Otherwise sample the
    // distribution.
    bool learn = is_learn && maybe_labelled_action >= 0;
    if (learn)
    {
      // Find where the labelled action is in the final prediction to determine if swapping needs to occur.
      // This only matters if the prediction decided to explore, but the same output should happen for the learn case.
      for (size_t i = 0; i < action_scores.size(); i++)
      {
        auto& a_s = action_scores[i];
        if (a_s.action == static_cast<uint32_t>(maybe_labelled_action))
        {
          chosen_action = static_cast<uint32_t>(i);
          break;
        }
      }
    }
    else
    {
      uint64_t seed = _random_state->get_current_state();

      VW::string_view tag_seed;
      const bool tag_provided_seed = try_extract_random_seed(*examples[0], tag_seed);
      if (tag_provided_seed) { seed = VW::uniform_hash(tag_seed.data(), tag_seed.size(), 0); }

      // Sampling is done after the base learner has generated a pdf.
      auto result = VW::explore::sample_after_normalizing(
          seed, VW::begin_scores(action_scores), VW::end_scores(action_scores), chosen_action);
      assert(result == S_EXPLORATION_OK);
      _UNUSED(result);

      // Update the seed state in place if it was used for this example.
      if (!tag_provided_seed) { _random_state->get_and_update_random(); }
    }

    auto result = VW::explore::swap_chosen(action_scores.begin(), action_scores.end(), chosen_action);
    assert(result == S_EXPLORATION_OK);

    VW_DBG(examples) << "cb " << cb_decision_to_string(examples[0]->pred.a_s)
                     << " rnd:" << _random_state->get_current_state() << std::endl;

    _UNUSED(result);
  }

  static std::string cb_decision_to_string(const VW::action_scores& action_scores)
  {
    std::ostringstream ostrm;
    if (action_scores.empty()) { return ""; }
    ostrm << "chosen" << action_scores[0] << action_scores;
    return ostrm.str();
  }

private:
  std::shared_ptr<VW::rand_state> _random_state;
};
template <bool is_learn>
void learn_or_predict(cb_sample_data& data, learner& base, VW::multi_ex& examples)
{
  data.learn_or_predict<is_learn>(base, examples);
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_sample_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  bool cb_sample_option = false;

  option_group_definition new_options("[Reduction] CB Sample");
  new_options.add(
      make_option("cb_sample", cb_sample_option).keep().necessary().help("Sample from CB pdf and swap top action"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto data = VW::make_unique<cb_sample_data>(all.get_random_state());

  auto l = make_reduction_learner(std::move(data), require_multiline(stack_builder.setup_base_learner()),
      learn_or_predict<true>, learn_or_predict<false>, stack_builder.get_setupfn_name(cb_sample_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_learn_returns_prediction(true)
               .build();
  return l;
}
