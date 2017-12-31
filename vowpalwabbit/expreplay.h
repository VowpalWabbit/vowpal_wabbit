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
LEARNER::base_learner* expreplay_setup(arguments& arg)
{
  std::string replay_string = "replay_";
  replay_string += er_level;
  std::string replay_count_string = replay_string;
  replay_count_string += "_count";

  expreplay& er = calloc_or_throw<expreplay>();
  if (arg.new_options("Experience Replay")
      .critical(replay_string.c_str(), er.N, (size_t)0, "use experience replay at a specified level [b=classification/regression, m=multiclass, c=cost sensitive] with specified buffer size")
      (replay_count_string.c_str(), er.replay_count, (size_t)1, "how many times (in expectation) should each example be played (default: 1 = permuting)").missing() || er.N==0)
    return free_return(&er);

  er.all = arg.all;
  er.buf = VW::alloc_examples(1, er.N);

  if (er_level == 'c')
    for (size_t n=0; n<er.N; n++)
      er.buf[n].l.cs.costs = v_init<COST_SENSITIVE::wclass>();

  er.filled = calloc_or_throw<bool>(er.N);

  if (! arg.all->quiet)
    std::cerr << "experience replay level=" << er_level << ", buffer=" << er.N << ", replay count=" << er.replay_count << std::endl;

  LEARNER::base_learner* base = setup_base(arg);
  LEARNER::learner<expreplay>* l = &init_learner(&er, base, predict_or_learn<true,lp>, predict_or_learn<false,lp>);
  l->set_finish(finish<lp>);
  l->set_end_pass(end_pass);
  er.base = base;

  return make_base(*l);
}


}
