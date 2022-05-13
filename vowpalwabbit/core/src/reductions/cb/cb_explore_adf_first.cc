// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_first.h"

#include "vw/config/options.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/label_parser.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/rand48.h"
#include "vw/core/reductions/bs.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/core/version.h"
#include "vw/core/vw_versions.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace VW::LEARNER;
using namespace VW::cb_explore_adf;

namespace
{
struct cb_explore_adf_first
{
private:
  size_t _tau;
  float _epsilon;

  VW::version_struct _model_file_version;

public:
  cb_explore_adf_first(size_t tau, float epsilon, VW::version_struct model_file_version);
  ~cb_explore_adf_first() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(multi_learner& base, VW::multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(multi_learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }
  void save_load(io_buf& io, bool read, bool text);

private:
  template <bool is_learn>
  void predict_or_learn_impl(multi_learner& base, VW::multi_ex& examples);
};

cb_explore_adf_first::cb_explore_adf_first(size_t tau, float epsilon, VW::version_struct model_file_version)
    : _tau(tau), _epsilon(epsilon), _model_file_version(model_file_version)
{
}

template <bool is_learn>
void cb_explore_adf_first::predict_or_learn_impl(multi_learner& base, VW::multi_ex& examples)
{
  // Explore tau times, then act according to optimal.
  if (is_learn) { multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset); }
  else
  {
    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
  }

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = static_cast<uint32_t>(preds.size());

  if (_tau)
  {
    float prob = 1.f / static_cast<float>(num_actions);
    for (size_t i = 0; i < num_actions; i++) { preds[i].score = prob; }
    if (is_learn) { _tau--; }
  }
  else
  {
    for (size_t i = 1; i < num_actions; i++) { preds[i].score = 0.; }
    preds[0].score = 1.0;
  }

  exploration::enforce_minimum_probability(_epsilon, true, begin_scores(preds), end_scores(preds));
}

void cb_explore_adf_first::save_load(io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (!read || _model_file_version >= VW::version_definitions::VERSION_FILE_WITH_FIRST_SAVE_RESUME)
  {
    std::stringstream msg;
    if (!read) { msg << "cb first adf storing example counter:  = " << _tau << "\n"; }
    bin_text_read_write_fixed_validated(io, reinterpret_cast<char*>(&_tau), sizeof(_tau), read, msg, text);
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::cb_explore_adf_first_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  uint64_t tau = 0;
  float epsilon = 0.;
  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (tau-first)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("first", tau).keep().necessary().help("Tau-first exploration"))
      .add(make_option("epsilon", epsilon)
               .default_value(0.f)
               .keep()
               .allow_override()
               .help("Epsilon-greedy exploration"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  size_t problem_multiplier = 1;

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = CB::cb_label;

  bool with_metrics = options.was_supplied("extra_metrics");

  using explore_type = cb_explore_adf_base<cb_explore_adf_first>;
  auto data =
      VW::make_unique<explore_type>(with_metrics, VW::cast_to_smaller_type<size_t>(tau), epsilon, all.model_file_ver);

  if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
  auto* l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_first_setup))
                .set_input_label_type(VW::label_type_t::cb)
                .set_output_label_type(VW::label_type_t::cb)
                .set_input_prediction_type(VW::prediction_type_t::action_scores)
                .set_output_prediction_type(VW::prediction_type_t::action_probs)
                .set_params_per_weight(problem_multiplier)
                .set_finish_example(explore_type::finish_multiline_example)
                .set_print_example(explore_type::print_multiline_example)
                .set_persist_metrics(explore_type::persist_metrics)
                .set_save_load(explore_type::save_load)
                .build(&all.logger);
  return make_base(*l);
}
