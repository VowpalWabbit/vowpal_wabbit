#include "reductions.h"
#include "cb_sample.h"
#include "explore.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

namespace VW
{
// cb_sample is used to automatically sample and swap from a cb explore pdf.
struct cb_sample_data
{
  explicit cb_sample_data(std::shared_ptr<rand_state> &random_state) : _random_state(random_state) {}
  explicit cb_sample_data(std::shared_ptr<rand_state> &&random_state) : _random_state(random_state) {}

  inline void predict(multi_learner &base, multi_ex &examples)
  {
    // Call base predict() to get action scores for all actions
    multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
    example *first_example = examples[0];
    sample(first_example);
  }

  inline void learn(multi_learner &base, multi_ex &examples)
  {
    multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);

    ptrdiff_t labeled_action = -1;
    // Find that chosen action in the learning case, skip the shared example.
    const auto it = std::find_if(examples.begin(), examples.end(), [](example *item) { return !item->l.cb.costs.empty(); });
    if (it != examples.end())
    {
      labeled_action = std::distance(examples.begin(), it);
    }

    example *first_example = examples[0];
    // If we are learning and have a label, then take that action as the chosen action. Otherwise sample the
    // distribution.
    if (labeled_action != -1)
    {
      uint32_t chosen_action = -1;
      auto action_scores = first_example->pred.a_s;

      // Find where the labeled action is in the final prediction to determine if swapping needs to occur.
      // This only matters if the prediction decided to explore, but the same output should happen for the learn case.
      const auto a_s_it = std::find_if(action_scores.begin(), action_scores.end(),
        [&](auto &a_s) { return a_s.action == labeled_action; });
      if (a_s_it != action_scores.end())
        chosen_action = uint32_t(std::distance(action_scores.begin(), a_s_it));

      auto result = exploration::swap_chosen(action_scores.begin(), action_scores.end(), chosen_action);
      assert(result == S_EXPLORATION_OK);
      _UNUSED(result);
    }
    else if (!first_example->predict_called_before_learn)
    {
      // No label to learn from.  Just sample the action_scores.
      sample(first_example);
    }
  }

 private:

  uint64_t get_random_seed(example *first_example)
  {
    // Try to get the random seed from a specific tag in the example
    if (!first_example->tag.empty())
    {
      const std::string SEED_IDENTIFIER = "seed=";
      if (strncmp(first_example->tag.begin(), SEED_IDENTIFIER.c_str(), SEED_IDENTIFIER.size()) == 0 &&
          first_example->tag.size() > SEED_IDENTIFIER.size())
      {
        substring tag_seed{first_example->tag.begin() + 5, first_example->tag.begin() + first_example->tag.size()};
        return uniform_hash(tag_seed.begin, substring_len(tag_seed), 0);
      }
    }

    // Seed not found in the example.  Get the random seed from the default provider.
    const uint64_t seed = _random_state->get_current_state();
    // Advance the random state since we are going to use the seed 
    _random_state->get_and_update_random();
    return seed;
  }

  void sample(example *first_example)
  {
    auto action_scores = first_example->pred.a_s;

    // Sample using action scores as a pmf to choose an action
    const uint64_t seed = get_random_seed(first_example);

    // Sampling is done after the base learner has generated a pdf.
    uint32_t chosen_action = -1;
    auto result = exploration::sample_after_normalizing(
        seed, ACTION_SCORE::begin_scores(action_scores), ACTION_SCORE::end_scores(action_scores), chosen_action);
    assert(result == S_EXPLORATION_OK);
    _UNUSED(result);

    result = exploration::swap_chosen(action_scores.begin(), action_scores.end(), chosen_action);
    assert(result == S_EXPLORATION_OK);
    _UNUSED(result);
  }

  std::shared_ptr<rand_state> _random_state;
};
}  // namespace VW

void predict(cb_sample_data &data, multi_learner &base, multi_ex &examples)
{
  data.predict(base, examples);
}

void learn(cb_sample_data &data, multi_learner &base, multi_ex &examples)
{
  data.learn(base, examples);
}

base_learner *cb_sample_setup(options_i &options, vw &all)
{
  bool cb_sample_option = false;

  option_group_definition new_options("CB Sample");
  new_options.add(make_option("cb_sample", cb_sample_option).keep().help("Sample from CB pdf and swap top action."));
  options.add_and_parse(new_options);

  if (!cb_sample_option)
    return nullptr;

  if (options.was_supplied("no_predict"))
  {
    THROW("cb_sample cannot be used with no_predict, as there would be no predictions to sample.");
  }

  auto data = scoped_calloc_or_throw<cb_sample_data>(all.get_random_state());
  return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn,
      predict, 1 /* weights */, prediction_type::action_probs, "cb_sample"));
}
