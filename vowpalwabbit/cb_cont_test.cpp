#include "reductions.h"
//#include "cb_sample.h"
#include "cb_continuous.h"
#include "explore.h"

#include "rand48.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

struct cb_continuous
{
  uint32_t num_actions;
  uint32_t bandwidth;
};


template <bool is_learn>
void learn_or_predict(cb_continuous& data, single_learner& base, multi_ex& examples)
{
  multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  auto action_scores = examples[0]->pred.a_s;
  uint32_t chosen_action = -1;
}

base_learner* cb_sample_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<cb_continuous>();
  option_group_definition new_options("CB Continous");
  new_options
      .add(make_option("cb_continuous", data->num_actions)
               .default_value(0)
               .keep()
               .help("Convert discrete PDF into continuous PDF."))
      .add(make_option("bandwidth", data->bandwidth)
               .keep()
               .help("Bandwidth of randomization around discrete actions in number of actions."));
  options.add_and_parse(new_options);

  if (data->num_actions == 0)
    return nullptr;

  // todo: enforce the right kind of base reduction

  return make_base(init_learner(data, as_singleline(setup_base(options, all)), learn_or_predict<true>,
      learn_or_predict<false>, data->num_actions /* weights */, prediction_type::action_probs));
}
