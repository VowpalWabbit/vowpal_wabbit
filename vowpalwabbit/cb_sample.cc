#include "reductions.h"
#include "cb_sample.h"
#include "explore.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

struct cb_sample_data
{
  uint64_t random_seed;
  uint64_t random_seed_counter;
};

template <bool is_learn>
void learn_or_predict(cb_sample_data& data, multi_learner& base, multi_ex& examples)
{
  multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  auto action_scores = examples[0]->pred.a_s;

  uint64_t seed = data.random_seed;
  if (examples[0]->tag.size() > 0)
  {
    if (strncmp(examples[0]->tag.begin(), "seed=", 5) == 0)
    {
      substring tag_seed{examples[0]->tag.begin() + 5, examples[0]->tag.begin() + examples[0]->tag.size()};
      seed = static_cast<uint64_t>(int_of_substring(tag_seed));
    }
  }

  // Sampling is done after the base learner has generated a pdf.
  uint32_t chosen_action;
  if (exploration::sample_after_normalizing(seed + data.random_seed_counter++,
          ACTION_SCORE::begin_scores(action_scores), ACTION_SCORE::end_scores(action_scores), chosen_action))
    THROW("Failed to sample from pdf");

  if (exploration::swap_chosen(action_scores.begin(), action_scores.end(), chosen_action))
    THROW("Failed to swap top action_score chosen during sampling");
}

base_learner* cb_sample_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cb_sample_data>();
  bool cb_sample_option = false;
  uint64_t override_seed;

  option_group_definition new_options("CB Sample");
  new_options.add(make_option("cb_sample", cb_sample_option).keep().help("Sample from CB pdf and swap top action."));
  new_options.add(make_option("cb_sample_seed", override_seed)
    .help("Override seed used for sampling, if not provided overall random seed used. Overriden by tag defined seed, must be of the form 'seed=<number>'."));
  options.add_and_parse(new_options);

  if (!cb_sample_option)
    return nullptr;

  data->random_seed = options.was_supplied("cb_sample_seed") ? override_seed : all.random_seed;
  data->random_seed_counter = 0;

  return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn_or_predict<true>,
      learn_or_predict<false>, 1, prediction_type::action_probs));
}
