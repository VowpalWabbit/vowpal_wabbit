// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/common/random.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"

#include <sys/types.h>

#include <memory>

namespace VW
{
namespace reductions
{
namespace expreplay
{
template <VW::label_parser& lp>
class expreplay
{
public:
  VW::workspace* all = nullptr;
  std::shared_ptr<VW::rand_state> _random_state;
  size_t N = 0;                   // how big is the buffer?
  std::vector<VW::example*> buf;  // the deep copies of examples (N of them)
  std::vector<bool> filled;       // which of buf[] is filleds
  size_t replay_count = 0;  // each time er.learn() is called, how many times do we call base.learn()? default=1 (in
                            // which case we're just permuting)
  VW::LEARNER::learner* base = nullptr;

  ~expreplay()
  {
    for (auto* ex : buf) { delete ex; }
  }
};

template <VW::label_parser& lp>
void learn(expreplay<lp>& er, VW::LEARNER::learner& base, VW::example& ec)
{
  // Cannot learn if the example weight is 0.
  if (lp.get_weight(ec.l, ec.ex_reduction_features) == 0.) { return; }

  for (size_t replay = 1; replay < er.replay_count; replay++)
  {
    size_t n = (size_t)(er._random_state->get_and_update_random() * (float)er.N);
    if (er.filled[n]) { base.learn(*er.buf[n]); }
  }

  size_t n = (size_t)(er._random_state->get_and_update_random() * (float)er.N);
  if (er.filled[n]) { base.learn(*er.buf[n]); }

  er.filled[n] = true;
  VW::copy_example_data_with_label(er.buf[n], &ec);
}

template <VW::label_parser& lp>
void predict(expreplay<lp>&, VW::LEARNER::learner& base, VW::example& ec)
{
  base.predict(ec);
}

template <VW::label_parser& lp>
void multipredict(expreplay<lp>&, VW::LEARNER::learner& base, VW::example& ec, size_t count, size_t step,
    VW::polyprediction* pred, bool finalize_predictions)
{
  base.multipredict(ec, count, step, pred, finalize_predictions);
}

template <VW::label_parser& lp>
void end_pass(expreplay<lp>& er)
{  // we need to go through and learn on everyone who remains
  // also need to clean up remaining examples
  for (size_t n = 0; n < er.N; n++)
  {
    if (er.filled[n])
    {  // TODO: if er.replay_count > 1 do we need to play these more?
      er.base->learn(*er.buf[n]);
      er.filled[n] = false;
    }
  }
}
}  // namespace expreplay

template <char er_level, VW::label_parser& lp>
std::shared_ptr<VW::LEARNER::learner> expreplay_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string replay_string = "replay_";
  replay_string += er_level;
  std::string replay_count_string = replay_string;
  replay_count_string += "_count";
  uint64_t N;
  uint64_t replay_count;

  auto er = VW::make_unique<expreplay::expreplay<lp>>();
  VW::config::option_group_definition new_options("[Reduction] Experience Replay / " + replay_string);
  new_options
      .add(VW::config::make_option(replay_string, N)
               .keep()
               .necessary()
               .help("Use experience replay at a specified level [b=classification/regression, m=multiclass, c=cost "
                     "sensitive] with specified buffer size"))
      .add(VW::config::make_option(replay_count_string, replay_count)
               .default_value(1)
               .help("How many times (in expectation) should each example be played (default: 1 = permuting)"));

  if (!options.add_parse_and_check_necessary(new_options) || N == 0) { return nullptr; }

  er->N = VW::cast_to_smaller_type<size_t>(N);
  er->replay_count = VW::cast_to_smaller_type<size_t>(replay_count);
  er->all = &all;
  er->_random_state = all.get_random_state();
  for (uint64_t i = 0; i < er->N; i++)
  {
    er->buf.push_back(new VW::example);
    er->buf.back()->interactions = &all.interactions;
    er->buf.back()->extent_interactions = &all.extent_interactions;
  }
  er->filled.resize(er->N, false);

  if (!all.quiet)
  {
    *(all.trace_message) << "experience replay level=" << er_level << ", buffer=" << er->N
                         << ", replay count=" << er->replay_count << std::endl;
  }

  auto base_learner = VW::LEARNER::require_singleline(stack_builder.setup_base_learner());
  er->base = base_learner.get();
  auto l =
      make_reduction_learner(std::move(er), base_learner, expreplay::learn<lp>, expreplay::predict<lp>, replay_string)
          .set_end_pass(expreplay::end_pass<lp>)
          .build();

  return l;
}
}  // namespace reductions
}  // namespace VW