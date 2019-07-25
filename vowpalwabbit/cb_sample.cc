#include "reductions.h"
#include "cb_sample.h"
#include "explore.h"

#include "rand48.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

struct cb_sample_data
{
  uint64_t seed_state;
};

template <bool is_learn>
void learn_or_predict(cb_sample_data& data, multi_learner& base, multi_ex& examples)
{
  multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  auto action_scores = examples[0]->pred.a_s;
  uint32_t chosen_action = -1;

  int labelled_action = -1;
  // Find that chosen action in the learning case, skip the shared example.
  for(size_t i = 0; i < examples.size(); i++)
  {
    if(examples[i]->l.cb.costs.size() > 0)
    {
      // Must remove 1 because of shared example index.
      labelled_action = i;
    }
  }

  // If we are learning and have a label, then take that action as the chosen action. Otherwise sample the distribution.
  if (is_learn && labelled_action != -1)
  {
    // Find where the labelled action is in the final prediction to determine if swapping needs to occur.
    // This only matters if the prediction decided to explore, but the same output should happen for the learn case.
    for (size_t i = 0; i < action_scores.size(); i++)
    {
      auto& a_s = action_scores[i];
      if (a_s.action == static_cast<uint32_t>(labelled_action))
      {
        chosen_action = i;
        break;
      }
    }
  }
  else
  {
    bool tag_provided_seed = false;
    uint64_t seed = data.seed_state;
    if (examples[0]->tag.size() > 0)
    {
      const std::string SEED_IDENTIFIER = "seed=";
      if (strncmp(examples[0]->tag.begin(), SEED_IDENTIFIER.c_str(), SEED_IDENTIFIER.size()) == 0 &&
          examples[0]->tag.size() > SEED_IDENTIFIER.size())
      {
        substring tag_seed{examples[0]->tag.begin() + 5, examples[0]->tag.begin() + examples[0]->tag.size()};
        seed = uniform_hash(tag_seed.begin, substring_len(tag_seed), 0);
        tag_provided_seed = true;
      }
    }

    // Sampling is done after the base learner has generated a pdf.
    auto result = exploration::sample_after_normalizing(
        seed, ACTION_SCORE::begin_scores(action_scores), ACTION_SCORE::end_scores(action_scores), chosen_action);
    assert(result == S_EXPLORATION_OK);
    _UNUSED(result);

    // Update the seed state in place if it was used for this example.
    if (!tag_provided_seed)
    {
      merand48(data.seed_state);
    }
  }

  auto result = exploration::swap_chosen(action_scores.begin(), action_scores.end(), chosen_action);
  assert(result == S_EXPLORATION_OK);
  _UNUSED(result);
}

base_learner* cb_sample_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cb_sample_data>();
  bool cb_sample_option = false;

  option_group_definition new_options("CB Sample");
  new_options.add(make_option("cb_sample", cb_sample_option).keep().help("Sample from CB pdf and swap top action."));
  options.add_and_parse(new_options);

  if (!cb_sample_option)
    return nullptr;

  data->seed_state = all.random_seed;

  return make_base(init_learner(data, as_multiline(setup_base(options, all)), learn_or_predict<true>,
      learn_or_predict<false>, 1 /* weights */, prediction_type::action_probs));
}
