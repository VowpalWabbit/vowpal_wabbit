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
  explicit cb_dro_data(double alpha, double tau, double wmax) : chisq(alpha, tau, 0, wmax) {}

  bool isValid() { return chisq.isValid(); }

  template <bool is_learn, bool is_explore>
  inline void learn_or_predict(multi_learner &base, multi_ex &examples)
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
      const auto it = std::find_if(examples.begin(), examples.end(), [](example *item) { return !item->l.cb.costs.empty(); });

      if (it != examples.end())
      {
        const CB::cb_class logged = (*it)->l.cb.costs[0];
        const uint32_t labelled_action = std::distance(examples.begin(), it);

        const auto action_scores = examples[0]->pred.a_s;

        // cb_explore_adf => want maximum probability
        // cb_adf => first action is a greedy action

        const auto maxit = is_explore
                     ? std::max_element (action_scores.begin(),
                                         action_scores.end(),
                                         [](const ACTION_SCORE::action_score& a, const ACTION_SCORE::action_score& b) { return ACTION_SCORE::score_comp(&a, &b) < 0; })
                     : action_scores.begin();
        const uint32_t chosen_action = maxit->action;

        const float w = logged.probability > 0 ? 1 / logged.probability : 0;
        const float r = -logged.cost;

        chisq.update(chosen_action == labelled_action ? w : 0, r);

        float qlb = chisq.effn() * chisq.qlb(w, r);

        // avoid pathological cases
        qlb = std::max(qlb, 0.01f);

        // save the original weights and scale the example weights
        save_weight.clear();
        save_weight.reserve(examples.size());
        std::transform(examples.cbegin(), examples.cend(), std::back_inserter(save_weight), [](example *item) { return item->weight; });
        std::for_each(examples.begin(), examples.end(), [qlb](example* item) { item->weight *= qlb; });

        // TODO: make sure descendants "do the right thing" with example->weight
        multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);

        // restore the original weights
        auto save_weight_it = save_weight.begin();
        std::for_each(examples.begin(), examples.end(), [&save_weight_it](example* item) { item->weight = *save_weight_it++; });
      }
    }
  }

 private:
  VW::distributionally_robust::ChiSquared chisq;
  std::vector<float> save_weight;
};
}  // namespace VW

template <bool is_learn, bool is_explore>
void learn_or_predict(cb_dro_data &data, multi_learner &base, multi_ex &examples)
{
  data.learn_or_predict<is_learn, is_explore>(base, examples);
}

base_learner *cb_dro_setup(options_i &options, vw &all)
{
  double alpha;
  double tau;
  double wmax;
  bool cb_dro_option = false;

  option_group_definition new_options("CB Distributionally Robust Optimization");
  new_options.add(make_option("cb_dro", cb_dro_option).keep().help("Use DRO for cb learning"))
      .add(make_option("cb_dro_alpha", alpha).default_value(0.05).keep().help("Confidence level for cb dro"))
      .add(make_option("cb_dro_tau", tau).default_value(0.999).keep().help("Time constant for count decay for cb dro"))
      .add(make_option("cb_dro_wmax", wmax).default_value(std::numeric_limits<double>::infinity()).keep().help("maximum importance weight for cb_dro"));

  options.add_and_parse(new_options);

  if (!cb_dro_option)
    return nullptr;

  if (options.was_supplied("no_predict"))
  {
    THROW("cb_dro cannot be used with no_predict");
  }

  if (!options.was_supplied("cb_adf") && !options.was_supplied("cb_explore_adf"))
  {
    THROW("cb_dro requires cb_adf or cb_explore_adf");
  }

  if (alpha <= 0 || alpha >= 1)
  {
    THROW("cb_dro_alpha must be in (0, 1)");
  }

  if (tau <= 0 || tau > 1)
  {
    THROW("cb_dro_tau must be in (0, 1]");
  }

  if (wmax <= 1)
  {
    THROW("cb_dro_wmax must exceed 1");
  }

  if (!all.quiet)
  {
    std::cerr << "Using DRO for CB learning" << std::endl;
    std::cerr << "cb_dro_alpha = " << alpha << std::endl;
    std::cerr << "cb_dro_tau = " << tau << std::endl;
    std::cerr << "cb_dro_wmax = " << wmax << std::endl;
  }

  auto data = scoped_calloc_or_throw<cb_dro_data>(alpha, tau, wmax);

  if (!data->isValid())
  {
    THROW("invalid cb_dro parameter values supplied");
  }

  if (options.was_supplied("cb_explore_adf"))
  {
    return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn_or_predict<true, true>, learn_or_predict<false, true>, 1 /* weights */, prediction_type_t::action_probs));
  }
  else
  {
    return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn_or_predict<true, false>, learn_or_predict<false, false>, 1 /* weights */, prediction_type_t::action_probs));
  }
}
