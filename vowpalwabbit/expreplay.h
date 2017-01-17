#pragma once
#include "learner.h"
#include "vw.h"
#include "parse_args.h"
#include "rand48.h"

namespace ExpReplay
{

struct expreplay
{ vw* all;
  size_t N;      //how big is the buffer?
  example* buf;  //the deep copies of examples (N of them)
  bool* filled;  //which of buf[] is filled
  size_t replay_count; //each time er.learn() is called, how many times do we call base.learn()? default=1 (in which case we're just permuting)
  LEARNER::base_learner* base;
};

template<bool is_learn, label_parser& lp>
void predict_or_learn(expreplay& er, LEARNER::base_learner& base, example& ec)
{ // regardless of what happens, we must predict
  base.predict(ec);
  // if we're not learning, that's all that has to happen
  if (!is_learn || lp.get_weight(&ec.l) == 0.) return;

  for (size_t replay=1; replay<er.replay_count; replay++)
  { size_t n = (size_t)(merand48(er.all->random_state) * (float)er.N);
    if (er.filled[n])
      base.learn(er.buf[n]);
  }

  size_t n = (size_t)(merand48(er.all->random_state) * (float)er.N);
  if (er.filled[n])
    base.learn(er.buf[n]);

  er.filled[n] = true;
  VW::copy_example_data(er.all->audit, &er.buf[n], &ec); // don't copy the label
  if (lp.copy_label)
    lp.copy_label(&er.buf[n].l, &ec.l);
  else
    er.buf[n].l = ec.l;
}

void multipredict(expreplay&, LEARNER::base_learner& base, example& ec, size_t count, size_t step, polyprediction*pred, bool finalize_predictions)
{ base.multipredict(ec, count, step, pred, finalize_predictions);
}

void end_pass(expreplay& er)
{ // we need to go through and learn on everyone who remains
  // also need to clean up remaining examples
  for (size_t n = 0; n < er.N; n++)
    if (er.filled[n])
    { // TODO: if er.replay_count > 1 do we need to play these more?
      er.base->learn(er.buf[n]);
      er.filled[n] = false;
    }
}

template<label_parser& lp>
void finish(expreplay& er)
{ for (size_t n=0; n<er.N; n++)
  { lp.delete_label(&er.buf[n].l);
    VW::dealloc_example(NULL, er.buf[n], NULL);  // TODO: need to free label
  }
  free(er.buf);
  free(er.filled);
}

template<char er_level, label_parser& lp>
LEARNER::base_learner* expreplay_setup(vw& all)
{ std::string replay_string = "replay_"; replay_string += er_level;
  if (missing_option<size_t, true>(all, replay_string.c_str(), "use experience replay at a specified level [b=classification/regression, m=multiclass, c=cost sensitive] with specified buffer size"))
    return nullptr;

  po::variables_map& vm = all.vm;

  size_t N  = vm[replay_string].as<size_t>();

  std::string replay_count_string = replay_string;
  replay_count_string += "_count";

  size_t rc = 1;
  new_options(all, "Experience Replay options")
  (replay_count_string.c_str(), po::value<size_t>(&rc)->default_value(1), "how many times (in expectation) should each example be played (default: 1 = permuting)");
  add_options(all);

  if (N == 0) return nullptr;

  expreplay& er = calloc_or_throw<expreplay>();
  er.all = &all;
  er.N   = N;
  er.buf = VW::alloc_examples(1, er.N);

  if (er_level == 'c')
    for (size_t n=0; n<er.N; n++)
      er.buf[n].l.cs.costs = v_init<COST_SENSITIVE::wclass>();

  er.filled = calloc_or_throw<bool>(er.N);
  er.replay_count = rc;

  if (! all.quiet)
    std::cerr << "experience replay level=" << er_level << ", buffer=" << er.N << ", replay count=" << er.replay_count << std::endl;

  LEARNER::base_learner* base = setup_base(all);
  LEARNER::learner<expreplay>* l = &init_learner(&er, base, predict_or_learn<true,lp>, predict_or_learn<false,lp>);
  l->set_finish(finish<lp>);
  l->set_end_pass(end_pass);
  er.base = base;

  return make_base(*l);
}


}
