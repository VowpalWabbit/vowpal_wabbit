// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include "correctedMath.h"
#include "reductions.h"
#include "vw_exception.h"

using namespace VW::config;

struct scorer
{
  vw* all;
};  // for set_minmax, loss

template <bool is_learn, float (*link)(float in)>
void predict_or_learn(scorer& s, LEARNER::single_learner& base, example& ec)
{
  s.all->set_minmax(s.all->sd, ec.l.simple.label);
  if (is_learn && ec.l.simple.label != FLT_MAX && ec.weight > 0)
    base.learn(ec);
  else
    base.predict(ec);

  if (ec.weight > 0 && ec.l.simple.label != FLT_MAX)
    ec.loss = s.all->loss->getLoss(s.all->sd, ec.pred.scalar, ec.l.simple.label) * ec.weight;

  ec.pred.scalar = link(ec.pred.scalar);
}

template <float (*link)(float in)>
inline void multipredict(scorer&, LEARNER::single_learner& base, example& ec, size_t count, size_t,
    polyprediction* pred, bool finalize_predictions)
{
  base.multipredict(ec, 0, count, pred, finalize_predictions);  // TODO: need to thread step through???
  for (size_t c = 0; c < count; c++) pred[c].scalar = link(pred[c].scalar);
}

void update(scorer& s, LEARNER::single_learner& base, example& ec)
{
  s.all->set_minmax(s.all->sd, ec.l.simple.label);
  base.update(ec);
}

// y = f(x) -> [0, 1]
inline float logistic(float in) { return 1.f / (1.f + correctedExp(-in)); }

// http://en.wikipedia.org/wiki/Generalized_logistic_curve
// where the lower & upper asymptotes are -1 & 1 respectively
// 'glf1' stands for 'Generalized Logistic Function with [-1,1] range'
//    y = f(x) -> [-1, 1]
inline float glf1(float in) { return 2.f / (1.f + correctedExp(-in)) - 1.f; }

inline float id(float in) { return in; }

LEARNER::base_learner* scorer_setup(options_i& options, vw& all)
{
  auto s = scoped_calloc_or_throw<scorer>();
  std::string link;
  option_group_definition new_options("scorer options");
  new_options.add(make_option("link", link)
                      .default_value("identity")
                      .keep()
                      .help("Specify the link function: identity, logistic, glf1 or poisson"));
  options.add_and_parse(new_options);

  // This always returns a base_learner.

  s->all = &all;

  auto base = as_singleline(setup_base(options, all));
  LEARNER::learner<scorer, example>* l;
  void (*multipredict_f)(scorer&, LEARNER::single_learner&, example&, size_t, size_t, polyprediction*, bool) =
      multipredict<id>;

  if (link == "identity")
    l = &init_learner(s, base, predict_or_learn<true, id>, predict_or_learn<false, id>);
  else if (link == "logistic")
  {
    l = &init_learner(s, base, predict_or_learn<true, logistic>, predict_or_learn<false, logistic>);
    multipredict_f = multipredict<logistic>;
  }
  else if (link == "glf1")
  {
    l = &init_learner(s, base, predict_or_learn<true, glf1>, predict_or_learn<false, glf1>);
    multipredict_f = multipredict<glf1>;
  }
  else if (link == "poisson")
  {
    l = &init_learner(s, base, predict_or_learn<true, expf>, predict_or_learn<false, expf>);
    multipredict_f = multipredict<expf>;
  }
  else
    THROW("Unknown link function: " << link);

  l->set_multipredict(multipredict_f);
  l->set_update(update);
  all.scorer = LEARNER::as_singleline(l);

  return make_base(*all.scorer);
}
