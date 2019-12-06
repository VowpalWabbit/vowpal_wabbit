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
// cb_dro is used to automatically sample and swap from a cb explore pdf.
struct cb_dro_data
{
  explicit cb_dro_data(double alpha, double tau) : chisq(alpha, tau) {}

  template <bool is_learn>
  inline void learn_or_predict(multi_learner &base, multi_ex &examples)
  {
    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

    if (is_learn)
    {
      auto action_scores = examples[0]->pred.a_s;
      // Find that chosen action in the learning case, skip the shared example.
      auto it = std::find_if(examples.begin(), examples.end(), [](example *item) { return !item->l.cb.costs.empty(); });

      if (it != examples.end())
      {
        CB::cb_class logged = (*it)->l.cb.costs[0];
        uint32_t labelled_action = std::distance(examples.begin(), it);
        float chosen_probability = 0;
        double total = 0.0;

        // Find where the logged action is in the final prediction to determine if swapping needs to occur.
        // This only matters if the prediction decided to explore, but the same output should happen for the learn
        // case.
        for (size_t i = 0; i < action_scores.size(); i++)
        {
          auto &a_s = action_scores[i];
          total += a_s.score;

          if (a_s.action == static_cast<uint32_t>(labelled_action))
          {
            chosen_probability = a_s.score;
          }
        }

        chosen_probability = total > 0 ? chosen_probability / total : 0;

        float w = logged.probability > 0 ? chosen_probability / logged.probability : 0;
        float r = logged.cost;

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

  if (!options.was_supplied("cb_explore"))
  {
    THROW("cb_dro requires cb_explore");
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
