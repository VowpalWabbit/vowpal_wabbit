#include "reductions.h"
#include "cb_dro.h"
#include "distributionally_robust.h"
#include "explore.h"

#include "rand48.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

namespace VW
{
struct cb_dro_data
{
  explicit cb_dro_data(double alpha, double tau) : chisq(alpha, tau) {}

  template <bool is_learn>
  inline void learn_or_predict(multi_learner &base, multi_ex &examples)
  {
    // Some explanation required.
    //
    // VW currently is either
    //    in prediction mode with exploration (aka --cb_explore_adf)
    //    or in (off-policy) learning without exploration (aka --cb_adf)
    //
    // In practice, nobody seems to do "truly online policy learning"
    // aka "--cb_explore_adf while supplying data with labels."  Arguably
    // distributional robustness is not important for on-policy learning.
    //
    // Ergo, the following always optimizes the bound on the derived
    // deterministic argmax score policy.


    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

    if (is_learn)
      {
        auto it = std::find_if(examples.begin(), examples.end(), [](example *item) { return !item->l.cb.costs.empty(); });

        if (it != examples.end())
        {
          CB::cb_class logged = (*it)->l.cb.costs[0];
          uint32_t labelled_action = std::distance(examples.begin(), it);

          auto action_scores = examples[0]->pred.a_s;

          auto maxit = std::max_element(action_scores.begin(),
                                        action_scores.end(),
                                        [](const ACTION_SCORE::action_score& a, const ACTION_SCORE::action_score& b) { return ACTION_SCORE::score_comp(&a, &b) < 0; });
          uint32_t chosen_action = maxit->action;

          float w = logged.probability > 0 && chosen_action == labelled_action ? 1 / logged.probability : 0;
          // TODO: rmin, rmax (?)
          float r = -logged.cost;

          chisq.update(w, r);

          float qlb = chisq.qlb(w, r);

          // save the original weights and scale the example weights
          std::vector<double> save_weight;
          std::transform(examples.begin(), examples.end(), std::back_inserter(save_weight), [](example *item) { return item->weight; });
          std::for_each(examples.begin(), examples.end(), [qlb](example *item) { item->weight *= qlb; });

          // TODO: make sure descendants "do the right thing" with example->weight
          multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);

          // restore the original weights
          auto save_weight_it = save_weight.begin();
          std::for_each(examples.begin(), examples.end(), [&save_weight_it](example *item) { item->weight = *save_weight_it++; });
        }
      }
  }

 private:
  VW::distributionally_robust::ChiSquared chisq;
};
}  // namespace VW

template <bool is_learn>
void learn_or_predict(cb_dro_data &data, multi_learner &base, multi_ex &examples)
{
  data.learn_or_predict<is_learn>(base, examples);
}

base_learner *cb_dro_setup(options_i &options, vw &all)
{
  double alpha;
  double tau;
  bool cb_dro_option = false;

  option_group_definition new_options("CB Distributionally Robust Optimization");
  new_options.add(make_option("cb_dro", cb_dro_option).keep().help("Use DRO for cb learning"))
      .add(make_option("cb_dro_alpha", alpha).default_value(0.05).keep().help("Confidence level for cb dro"))
      .add(make_option("cb_dro_tau", tau).default_value(0.999).keep().help("Time constant for count decay for cb dro"));

  options.add_and_parse(new_options);

  if (!cb_dro_option)
    return nullptr;

  if (options.was_supplied("no_predict"))
  {
    THROW("cb_dro cannot be used with no_predict");
  }

  if (!options.was_supplied("cb_adf"))
  {
    THROW("cb_dro requires cb_adf");
  }

  if (alpha <= 0 || alpha >= 1)
  {
    THROW("cb_dro_alpha must be in (0, 1)");
  }

  if (tau <= 0 || tau > 1)
  {
    THROW("cb_dro_tau must be in (0, 1]");
  }

  auto data = scoped_calloc_or_throw<cb_dro_data>(alpha, tau);
  return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn_or_predict<true>,
      learn_or_predict<false>, 1 /* weights */, prediction_type::action_probs));
}
