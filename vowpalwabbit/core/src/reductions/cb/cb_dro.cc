#include "vw/core/reductions/cb/cb_dro.h"

#include "vw/config/options.h"
#include "vw/core/cb.h"
#include "vw/core/estimators/distributionally_robust.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"
#include "vw/io/logger.h"

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;

namespace
{
class cb_dro_data
{
public:
  explicit cb_dro_data(double alpha, double tau, double wmax) : _chisq(alpha, tau, 0, wmax) {}

  bool is_valid() { return _chisq.is_valid(); }

  template <bool is_learn, bool is_explore>
  inline void learn_or_predict(learner& base, VW::multi_ex& examples)
  {
    // Some explanation required.
    //
    // VW currently is either
    //    in prediction mode with exploration (aka --cb_explore_adf)
    //    or in (off-policy) learning without exploration (aka --cb_adf)
    //
    // In practice, nobody seems to do
    // "off policy learning of the stochastic exploration policy".
    //
    // Ergo, the following always optimizes the bound on the associated
    // deterministic argmax score policy.

    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

    if (is_learn)
    {
      const auto it = std::find_if(
          examples.begin(), examples.end(), [](const VW::example* item) { return !item->l.cb.costs.empty(); });

      if (it != examples.end())
      {
        const VW::cb_class logged = (*it)->l.cb.costs[0];
        const uint32_t labelled_action = static_cast<uint32_t>(std::distance(examples.begin(), it));

        const auto& action_scores = examples[0]->pred.a_s;

        // cb_explore_adf => want maximum probability
        // cb_adf => first action is a greedy action

        const auto maxit =
            is_explore ? std::max_element(action_scores.begin(), action_scores.end()) : action_scores.begin();
        const uint32_t chosen_action = maxit->action;

        const float w = logged.probability > 0 ? 1 / logged.probability : 0;
        const float r = -logged.cost;

        _chisq.update(chosen_action == labelled_action ? w : 0, r);

        float qlb = static_cast<float>(w > 0 ? _chisq.effn() * _chisq.qlb(w, r, 1) / w : 1);

        // avoid pathological cases
        qlb = std::max(qlb, 0.01f);

        // save the original weights and scale the example weights
        _save_weight.clear();
        _save_weight.reserve(examples.size());
        std::transform(examples.cbegin(), examples.cend(), std::back_inserter(_save_weight),
            [](const VW::example* item) { return item->weight; });
        std::for_each(examples.begin(), examples.end(), [qlb](VW::example* item) { item->weight *= qlb; });

        // TODO: make sure descendants "do the right thing" with example->weight
        multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);

        // restore the original weights
        auto save_weight_it = _save_weight.begin();
        std::for_each(examples.begin(), examples.end(),
            [&save_weight_it](VW::example* item) { item->weight = *save_weight_it++; });
      }
    }
  }

private:
  VW::estimators::chi_squared _chisq;
  std::vector<float> _save_weight;
};
template <bool is_learn, bool is_explore>
void learn_or_predict(cb_dro_data& data, learner& base, VW::multi_ex& examples)
{
  data.learn_or_predict<is_learn, is_explore>(base, examples);
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_dro_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  float alpha;
  float tau;
  float wmax;
  bool cb_dro_option = false;

  option_group_definition new_options("[Reduction] CB Distributionally Robust Optimization");
  new_options.add(make_option("cb_dro", cb_dro_option).keep().necessary().help("Use DRO for cb learning"))
      .add(make_option("cb_dro_alpha", alpha).default_value(0.05f).keep().help("Confidence level for cb dro"))
      .add(make_option("cb_dro_tau", tau)
               .default_value(VW::details::BASELINE_DEFAULT_TAU)
               .keep()
               .help("Time constant for count decay for cb dro"))
      .add(make_option("cb_dro_wmax", wmax)
               .default_value(std::numeric_limits<float>::infinity())
               .keep()
               .help("Maximum importance weight for cb_dro"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (options.was_supplied("no_predict")) { THROW("cb_dro cannot be used with no_predict"); }

  if (!options.was_supplied("cb_adf") && !options.was_supplied("cb_explore_adf"))
  {
    THROW("cb_dro requires cb_adf or cb_explore_adf");
  }

  if (alpha <= 0 || alpha >= 1) { THROW("cb_dro_alpha must be in (0, 1)"); }

  if (tau <= 0 || tau > 1) { THROW("cb_dro_tau must be in (0, 1]"); }

  if (wmax <= 1) { THROW("cb_dro_wmax must exceed 1"); }

  if (!all.quiet)
  {
    *(all.trace_message) << "Using DRO for CB learning" << std::endl;
    *(all.trace_message) << "cb_dro_alpha = " << alpha << std::endl;
    *(all.trace_message) << "cb_dro_tau = " << tau << std::endl;
    *(all.trace_message) << "cb_dro_wmax = " << wmax << std::endl;
  }

  auto data = VW::make_unique<cb_dro_data>(alpha, tau, wmax);

  if (!data->is_valid()) { THROW("invalid cb_dro parameter values supplied"); }

  void (*learn_ptr)(cb_dro_data&, learner&, VW::multi_ex&);
  void (*pred_ptr)(cb_dro_data&, learner&, VW::multi_ex&);
  std::string name_addition;
  VW::prediction_type_t pred_type;
  if (options.was_supplied("cb_explore_adf"))
  {
    learn_ptr = learn_or_predict<true, true>;
    pred_ptr = learn_or_predict<false, true>;
    name_addition = "-cb_explore_adf";
    pred_type = VW::prediction_type_t::ACTION_PROBS;
  }
  else
  {
    learn_ptr = learn_or_predict<true, false>;
    pred_ptr = learn_or_predict<false, false>;
    name_addition = "";
    pred_type = VW::prediction_type_t::ACTION_SCORES;
  }

  auto base = stack_builder.setup_base_learner();
  auto l = make_reduction_learner(std::move(data), require_multiline(base), learn_ptr, pred_ptr,
      stack_builder.get_setupfn_name(cb_dro_setup) + name_addition)
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(pred_type)
               .set_output_prediction_type(pred_type)
               .build();
  return l;
}
