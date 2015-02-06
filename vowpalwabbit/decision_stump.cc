#include "reductions.h"
#include "gd.h"
#include <float.h>

struct decision_stump {
  std::vector<double> losses; //length = 2^bits
  example synth;
  vw* all;
};

struct loop_data {
  double best_loss;
  float pred;
  uint32_t best_feature;
  LEARNER::base_learner& base;
  decision_stump& s;
};

void inner(loop_data& ld, float feature_value, uint32_t feature_index)
  {
    ld.s.synth.atomics[0].erase();
    feature f = {feature_value, feature_index};
    ld.s.synth.atomics[0].push_back(f);
    ld.base.learn(ld.s.synth);

    uint32_t feature_id = feature_index >> ld.s.all->reg.stride_shift;

    if (ld.s.losses[feature_id] < ld.best_loss)
      {
	ld.best_loss = ld.s.losses[feature_id];
	ld.pred = ld.s.synth.pred.scalar;
	ld.best_feature = feature_index;
      }
    ld.s.losses[feature_id] 
      += ld.s.all->loss->getLoss(ld.s.all->sd, ld.s.synth.pred.scalar, ld.s.synth.l.simple.label) * ld.s.synth.l.simple.weight;
}

void inner_predict(loop_data& ld, float feature_value, uint32_t feature_index)
{
    if (ld.s.losses[feature_index] < ld.best_loss) {
       ld.s.synth.atomics[0].erase();
       feature f = {feature_value, feature_index};
       ld.s.synth.atomics[0].push_back(f);
       ld.base.predict(ld.s.synth);
       ld.pred = ld.s.synth.pred.scalar;
       ld.best_loss = ld.s.losses[feature_index];
    }
}

template <bool is_learn>
void predict_or_learn(decision_stump& s, LEARNER::base_learner& base, example& ec)
{
  s.synth.l = ec.l;
  loop_data ld = {FLT_MAX, 0., 0, base, s};

  if (is_learn) {
    GD::foreach_feature<loop_data, uint32_t, inner>(*(s.all), ec, ld);
  } else {
    GD::foreach_feature<loop_data, uint32_t, inner_predict>(*(s.all), ec, ld);
  }
  ec.pred.scalar = ld.pred;
}

void finish(decision_stump& s)
{
  s.losses.~vector();
  s.synth.atomics[0].delete_v();
  s.synth.indices.delete_v();
}

LEARNER::base_learner* decision_stump_setup(vw& all)
{
  if (missing_option(all,false,"decision_stump", "Learn with decision stumps"))
    return NULL;

  decision_stump& data = calloc_or_die<decision_stump>();

  uint32_t length = 1 << all.num_bits;
  data.losses = std::vector<double>(length,0);

  data.all = &all;
  data.synth.in_use=true;
  data.synth.indices.push_back(0);
  
  LEARNER::learner<decision_stump>& ret = 
    init_learner(&data, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);
  ret.set_finish(finish);

  return make_base(ret);
}
