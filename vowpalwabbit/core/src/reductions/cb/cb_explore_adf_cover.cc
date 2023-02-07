// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_cover.h"

#include "vw/config/options.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/bs.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw_versions.h"
#include "vw/explore/explore.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cmath>
#include <vector>

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

using namespace VW::cb_explore_adf;

namespace
{
class cb_explore_adf_cover
{
public:
  cb_explore_adf_cover(size_t cover_size, float psi, bool nounif, float epsilon, bool epsilon_decay, bool first_only,
      VW::LEARNER::learner* cs_ldf_learner, VW::LEARNER::learner* scorer, VW::cb_type_t cb_type,
      VW::version_struct model_file_version, VW::io::logger logger);

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::learner& base, VW::multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(VW::LEARNER::learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }
  void save_load(VW::io_buf& io, bool read, bool text);

private:
  size_t _cover_size;
  float _psi;
  bool _nounif;
  float _epsilon;
  bool _epsilon_decay;
  bool _first_only;
  size_t _counter;

  VW::LEARNER::learner* _cs_ldf_learner;
  VW::details::cb_to_cs_adf gen_cs;

  VW::version_struct _model_file_version;
  VW::io::logger _logger;

  VW::v_array<VW::action_score> _action_probs;
  std::vector<float> _scores;
  VW::cs_label _cs_labels;
  VW::cs_label _cs_labels_2;
  std::vector<VW::cs_label> _prepped_cs_labels;
  std::vector<VW::cb_label> _cb_labels;
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::learner& base, VW::multi_ex& examples);
};

cb_explore_adf_cover::cb_explore_adf_cover(size_t cover_size, float psi, bool nounif, float epsilon, bool epsilon_decay,
    bool first_only, VW::LEARNER::learner* cs_ldf_learner, VW::LEARNER::learner* scorer, VW::cb_type_t cb_type,
    VW::version_struct model_file_version, VW::io::logger logger)
    : _cover_size(cover_size)
    , _psi(psi)
    , _nounif(nounif)
    , _epsilon(epsilon)
    , _epsilon_decay(epsilon_decay)
    , _first_only(first_only)
    , _counter(0)
    , _cs_ldf_learner(cs_ldf_learner)
    , _model_file_version(model_file_version)
    , _logger(std::move(logger))
{
  gen_cs.cb_type = cb_type;
  gen_cs.scorer = scorer;
}

template <bool is_learn>
void cb_explore_adf_cover::predict_or_learn_impl(VW::LEARNER::learner& base, VW::multi_ex& examples)
{
  // Redundant with the call in cb_explore_adf_base, but encapsulation means we need to do this again here
  gen_cs.known_cost = VW::get_observed_cost_or_default_cb_adf(examples);

  // Randomize over predictions from a base set of predictors
  // Use cost sensitive oracle to cover actions to form distribution.
  const bool is_mtr = gen_cs.cb_type == VW::cb_type_t::MTR;
  if (is_learn)
  {
    if (is_mtr)
    {  // use DR estimates for non-ERM policies in MTR
      VW::details::gen_cs_example_dr<true>(gen_cs, examples, _cs_labels);
    }
    else { VW::details::gen_cs_example<false>(gen_cs, examples, _cs_labels, _logger); }

    if (base.learn_returns_prediction)
    {
      // First predict() since the result of the predictions are used to learn
      // later in the reduction
      VW_DBG(examples) << "cb_explore_adf_cover: "
                          "LEARNER::multiline_learn_or_predict<false>()"
                       << std::endl;
      VW::LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
    }

    VW_DBG(examples) << "cb_explore_adf_cover: LEARNER::multiline_learn_or_predict<true>()" << std::endl;
    VW::LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
  }
  else
  {
    VW::details::gen_cs_example_ips(examples, _cs_labels, _logger);
    VW::LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
  }
  VW::v_array<VW::action_score>& preds = examples[0]->pred.a_s;
  const uint32_t num_actions = static_cast<uint32_t>(preds.size());

  float additive_probability = 1.f / static_cast<float>(_cover_size);

  float min_prob = _epsilon_decay
      ? std::min(_epsilon / num_actions, _epsilon / static_cast<float>(std::sqrt(_counter * num_actions)))
      : _epsilon / num_actions;

  _action_probs.clear();
  for (uint32_t i = 0; i < num_actions; i++) { _action_probs.push_back({i, 0.}); }
  _scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) { _scores.push_back(preds[i].score); }

  if (!_first_only)
  {
    size_t tied_actions = fill_tied(preds);
    for (size_t i = 0; i < tied_actions; ++i)
    {
      _action_probs[preds[i].action].score += additive_probability / tied_actions;
    }
  }
  else { _action_probs[preds[0].action].score += additive_probability; }

  float norm = min_prob * num_actions + (additive_probability - min_prob);
  for (size_t i = 1; i < _cover_size; i++)
  {
    // Create costs of each action based on online cover
    if (is_learn)
    {
      _cs_labels_2.costs.clear();
      /* Cover's learn policy is similar to bag in that we have multiple learners
       * The main difference here is that Cover's learners interact with each other.
       * The following code generates cost = cost + penalty where a penalty is applied
       * to any action a previous policy has already selected
       */
      for (uint32_t j = 0; j < num_actions; j++)
      {
        float pseudo_cost =
            _cs_labels.costs[j].x - _psi * min_prob / ((std::max)(_action_probs[j].score, min_prob) / norm);
        _cs_labels_2.costs.push_back({pseudo_cost, j, 0., 0.});
      }

      VW::details::cs_ldf_learn_or_predict<true>(*(_cs_ldf_learner), examples, _cb_labels, _cs_labels_2,
          _prepped_cs_labels, true, examples[0]->ft_offset, i + 1);
    }
    else
    {
      VW::details::cs_ldf_learn_or_predict<false>(*(_cs_ldf_learner), examples, _cb_labels, _cs_labels,
          _prepped_cs_labels, false, examples[0]->ft_offset, i + 1);
    }

    for (uint32_t j = 0; j < num_actions; j++) { _scores[j] += preds[j].score; }
    if (!_first_only)
    {
      size_t tied_actions = fill_tied(preds);
      const float add_prob = additive_probability / tied_actions;
      for (size_t j = 0; j < tied_actions; ++j)
      {
        if (_action_probs[preds[j].action].score < min_prob)
        {
          norm += (std::max)(0.f, add_prob - (min_prob - _action_probs[preds[j].action].score));
        }
        else { norm += add_prob; }
        _action_probs[preds[j].action].score += add_prob;
      }
    }
    else
    {
      uint32_t action = preds[0].action;
      if (_action_probs[action].score < min_prob)
      {
        norm += (std::max)(0.f, additive_probability - (min_prob - _action_probs[action].score));
      }
      else { norm += additive_probability; }
      _action_probs[action].score += additive_probability;
    }
  }

  VW::explore::enforce_minimum_probability(
      min_prob * num_actions, !_nounif, begin_scores(_action_probs), end_scores(_action_probs));

  sort_action_probs(_action_probs, _scores);
  for (size_t i = 0; i < num_actions; i++) { preds[i] = _action_probs[i]; }

  if (VW_DEBUG_LOG)
  {
    VW_DBG(examples) << "a_p[]=";
    for (auto const& ap : _action_probs) { VW_DBG_0 << ap.action << "::" << ap.score << ","; }
    VW_DBG_0 << std::endl;

    VW_DBG(examples) << "scores[]=";
    for (auto const& s : _scores) { VW_DBG_0 << s << ","; }
    VW_DBG_0 << std::endl;
  }

  if (is_learn) { ++_counter; }
}

void cb_explore_adf_cover::save_load(VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (!read || _model_file_version >= VW::version_definitions::VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG)
  {
    std::stringstream msg;
    if (!read) { msg << "cb cover adf storing example counter:  = " << _counter << "\n"; }
    VW::details::bin_text_read_write_fixed_validated(
        io, reinterpret_cast<char*>(&_counter), sizeof(_counter), read, msg, text);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_cover_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;

  bool cb_explore_adf_option = false;
  std::string type_string = "mtr";
  uint64_t cover_size = 0;
  float psi = 0.;
  bool nounif = false;
  bool first_only = false;
  float epsilon = 0.;

  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (online cover)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("cover", cover_size).keep().necessary().help("Online cover based exploration"))
      .add(make_option("psi", psi).keep().default_value(1.0f).help("Disagreement parameter for cover"))
      .add(make_option("nounif", nounif).keep().help("Do not explore uniformly on zero-probability actions in cover"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"))
      .add(make_option("cb_type", type_string)
               .keep()
               .default_value("mtr")
               .one_of({"ips", "dr", "mtr"})
               .help("Contextual bandit method to use"))
      .add(make_option("epsilon", epsilon)
               .keep()
               .allow_override()
               .default_value(0.05f)
               .help("Epsilon-greedy exploration"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Ensure serialization of cb_type in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  auto cb_type = VW::cb_type_from_string(type_string);
  switch (cb_type)
  {
    case VW::cb_type_t::DR:
    case VW::cb_type_t::IPS:
      break;
    case VW::cb_type_t::MTR:
      all.logger.err_warn("currently, mtr is only used for the first policy in cover, other policies use dr");
      break;
    case VW::cb_type_t::DM:
    case VW::cb_type_t::SM:
      all.logger.err_warn(
          "cb_type must be in {{'ips','dr','mtr'}}; resetting to mtr. Input received: {}", VW::to_string(cb_type));
      options.replace("cb_type", "mtr");
      cb_type = VW::cb_type_t::MTR;
      break;
  }

  // Set explore_type
  size_t problem_multiplier = cover_size + 1;

  // Cover is using doubly robust without the cooperation of the base reduction
  if (cb_type == VW::cb_type_t::MTR) { problem_multiplier *= 2; }

  auto base = VW::LEARNER::require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  bool epsilon_decay;
  if (options.was_supplied("epsilon"))
  {
    // fixed epsilon during learning
    epsilon_decay = false;
  }
  else
  {
    epsilon = 1.f;
    epsilon_decay = true;
  }

  auto* scorer = VW::LEARNER::require_singleline(base->get_learner_by_name_prefix("scorer"));
  auto* cost_sensitive = require_multiline(base->get_learner_by_name_prefix("cs"));

  using explore_type = cb_explore_adf_base<cb_explore_adf_cover>;
  auto data = VW::make_unique<explore_type>(all.global_metrics.are_metrics_enabled(),
      VW::cast_to_smaller_type<size_t>(cover_size), psi, nounif, epsilon, epsilon_decay, first_only, cost_sensitive,
      scorer, cb_type, all.model_file_ver, all.logger);
  auto l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_cover_setup))
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_learn_returns_prediction(true)
               .set_params_per_weight(problem_multiplier)
               .set_output_example_prediction(explore_type::output_example_prediction)
               .set_update_stats(explore_type::update_stats)
               .set_print_update(explore_type::print_update)
               .set_save_load(explore_type::save_load)
               .set_persist_metrics(explore_type::persist_metrics)
               .build();
  return l;
}
