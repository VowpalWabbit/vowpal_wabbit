// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_greedy.h"

#include "vw/config/options.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/label_parser.h"
#include "vw/core/rand48.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
using namespace VW::cb_explore_adf;

namespace
{
template <bool keep_copy_adf_score>
struct cb_explore_adf_greedy
{
private:
  float _epsilon;
  bool _first_only;

public:
  ACTION_SCORE::action_scores cb_adf_a_s;
  std::unique_ptr<VW::io::writer> cb_adf_pred_sink;

  cb_explore_adf_greedy(float epsilon, bool first_only, std::unique_ptr<VW::io::writer> pred_sink)
      : _epsilon(epsilon), _first_only(first_only), cb_adf_pred_sink(std::move(pred_sink))
  {
  }
  ~cb_explore_adf_greedy() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
  {
    predict_or_learn_impl<false>(base, examples);
  }
  void learn(VW::LEARNER::multi_learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, VW::multi_ex& examples)
  {
    // Explore uniform random an epsilon fraction of the time.
    if (is_learn)
    {
      base.learn(examples);
      if (base.learn_returns_prediction) { update_example_prediction(examples); }
    }
    else
    {
      base.predict(examples);
      update_example_prediction(examples);
    }
  }
  void update_example_prediction(VW::multi_ex& examples)
  {
    ACTION_SCORE::action_scores& preds = examples[0]->pred.a_s;
    if (keep_copy_adf_score) { cb_adf_a_s = preds; };
    uint32_t num_actions = static_cast<uint32_t>(preds.size());

    auto& ep_fts = examples[0]->_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
    float actual_ep = (ep_fts.valid_epsilon_supplied()) ? ep_fts.epsilon : _epsilon;

    size_t tied_actions = fill_tied(preds);

    const float prob = actual_ep / num_actions;
    for (size_t i = 0; i < num_actions; i++) { preds[i].score = prob; }
    if (!_first_only)
    {
      for (size_t i = 0; i < tied_actions; ++i) { preds[i].score += (1.f - actual_ep) / tied_actions; }
    }
    else
    {
      preds[0].score += 1.f - actual_ep;
    }
  }
};

template <typename ExploreType>
void finish_multiline_example2(VW::workspace& all, cb_explore_adf_base<ExploreType>& data, multi_ex& ec_seq)
{
  cb_explore_adf_base<ExploreType>::print_multiline_example(all, data, ec_seq);
  if (data.explore.cb_adf_pred_sink)
  {
    auto& ec = *ec_seq[0];
    ACTION_SCORE::print_action_score(data.explore.cb_adf_pred_sink.get(), data.explore.cb_adf_a_s, ec.tag, all.logger);
  }
  VW::finish_example(all, ec_seq);
}

}  // namespace

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_greedy_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  bool first_only = false;
  std::string adf_scores_filename;

  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (greedy)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon)
               .default_value(0.05f)
               .keep()
               .allow_override()
               .help("Epsilon-greedy exploration"))
      .add(make_option("debug_write_adf_scores", adf_scores_filename)
               .help("Filename to output underlying cb adf scores to"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"));

  // This is a special case "cb_explore_adf" is needed to enable this. BUT it is only enabled when all of the other
  // "cb_explore_adf" types are disabled. This is why we don't check the return value of the
  // add_parse_and_check_necessary call like we do elsewhere.
  options.add_parse_and_check_necessary(new_options);

  // NOTE: epsilon-greedy is the default explore type.
  // This basically runs if none of the other explore strategies are used
  bool use_greedy = !(options.was_supplied("first") || options.was_supplied("bag") || options.was_supplied("cover") ||
      options.was_supplied("regcb") || options.was_supplied("regcbopt") || options.was_supplied("squarecb") ||
      options.was_supplied("rnd") || options.was_supplied("softmax") || options.was_supplied("synthcover"));

  if (!cb_explore_adf_option || !use_greedy) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
    options.insert("no_predict", "");
  }

  size_t problem_multiplier = 1;

  VW::LEARNER::multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  if (!adf_scores_filename.empty())
  {
    using explore_type = cb_explore_adf_base<cb_explore_adf_greedy<true>>;
    auto data = VW::make_unique<explore_type>(
        with_metrics, epsilon, first_only, std::move(VW::io::open_file_writer(adf_scores_filename)));

    if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
    auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
        stack_builder.get_setupfn_name(cb_explore_adf_greedy_setup))
                  .set_learn_returns_prediction(base->learn_returns_prediction)
                  .set_input_label_type(VW::label_type_t::cb)
                  .set_output_label_type(VW::label_type_t::cb)
                  .set_input_prediction_type(VW::prediction_type_t::action_scores)
                  .set_output_prediction_type(VW::prediction_type_t::action_probs)
                  .set_params_per_weight(problem_multiplier)
                  .set_finish_example(finish_multiline_example2)
                  .set_print_example(explore_type::print_multiline_example)
                  .set_persist_metrics(explore_type::persist_metrics)
                  .build(&all.logger);
    return make_base(*l);
  }
  else
  {
    using explore_type = cb_explore_adf_base<cb_explore_adf_greedy<false>>;
    auto data = VW::make_unique<explore_type>(with_metrics, epsilon, first_only, nullptr);

    if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
    auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
        stack_builder.get_setupfn_name(cb_explore_adf_greedy_setup))
                  .set_learn_returns_prediction(base->learn_returns_prediction)
                  .set_input_label_type(VW::label_type_t::cb)
                  .set_output_label_type(VW::label_type_t::cb)
                  .set_input_prediction_type(VW::prediction_type_t::action_scores)
                  .set_output_prediction_type(VW::prediction_type_t::action_probs)
                  .set_params_per_weight(problem_multiplier)
                  .set_finish_example(explore_type::finish_multiline_example)
                  .set_print_example(explore_type::print_multiline_example)
                  .set_persist_metrics(explore_type::persist_metrics)
                  .build(&all.logger);
    return make_base(*l);
  }
}
