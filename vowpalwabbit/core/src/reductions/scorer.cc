// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/scorer.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/correctedMath.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/setup_base.h"

#include <cfloat>

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::SCORER

using namespace VW::config;

namespace
{
class scorer
{
public:
  scorer(VW::workspace* all) : all(all) {}
  VW::workspace* all;
};  // for set_minmax, loss

template <bool is_learn, float (*link)(float in)>
void predict_or_learn(scorer& s, VW::LEARNER::learner& base, VW::example& ec)
{
  // Predict does not need set_minmax
  if (is_learn && s.all->set_minmax) { s.all->set_minmax(ec.l.simple.label); }

  bool learn = is_learn && ec.l.simple.label != FLT_MAX && ec.weight > 0;
  if (learn) { base.learn(ec); }
  else { base.predict(ec); }

  if (ec.weight > 0 && ec.l.simple.label != FLT_MAX)
  {
    ec.loss = s.all->loss->get_loss(s.all->sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  }

  ec.pred.scalar = link(ec.pred.scalar);
  VW_DBG(ec) << "ex#= " << ec.example_counter << ", offset=" << ec.ft_offset << ", lbl=" << ec.l.simple.label
             << ", pred= " << ec.pred.scalar << ", wt=" << ec.weight << ", gd.raw=" << ec.partial_prediction
             << ", loss=" << ec.loss << std::endl;
}

template <float (*link)(float in)>
inline void multipredict(scorer& /*unused*/, VW::LEARNER::learner& base, VW::example& ec, size_t count,
    size_t /*unused*/, VW::polyprediction* pred, bool finalize_predictions)
{
  base.multipredict(ec, 0, count, pred, finalize_predictions);  // TODO: need to thread step through???
  for (size_t c = 0; c < count; c++) { pred[c].scalar = link(pred[c].scalar); }
}

void update(scorer& s, VW::LEARNER::learner& base, VW::example& ec)
{
  if (s.all->set_minmax) { s.all->set_minmax(ec.l.simple.label); }
  base.update(ec);
  VW_DBG(ec) << "ex#= " << ec.example_counter << ", offset=" << ec.ft_offset << ", lbl=" << ec.l.simple.label
             << ", pred= " << ec.pred.scalar << ", wt=" << ec.weight << ", gd.raw=" << ec.partial_prediction
             << ", loss=" << ec.loss << std::endl;
}

// y = f(x) -> [0, 1]
inline float logistic(float in) { return 1.f / (1.f + VW::details::correctedExp(-in)); }

// http://en.wikipedia.org/wiki/Generalized_logistic_curve
// where the lower & upper asymptotes are -1 & 1 respectively
// 'glf1' stands for 'Generalized Logistic Function with [-1,1] range'
//    y = f(x) -> [-1, 1]
inline float glf1(float in) { return 2.f / (1.f + VW::details::correctedExp(-in)) - 1.f; }

inline float id(float in) { return in; }
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::scorer_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string link;
  option_group_definition new_options("[Reduction] Scorer");
  new_options.add(make_option("link", link)
                      .default_value("identity")
                      .keep()
                      .one_of({"identity", "logistic", "glf1", "poisson"})
                      .help("Specify the link function"));
  options.add_and_parse(new_options);

  using predict_or_learn_fn_t = void (*)(scorer&, VW::LEARNER::learner&, VW::example&);
  using multipredict_fn_t =
      void (*)(scorer&, VW::LEARNER::learner&, VW::example&, size_t, size_t, VW::polyprediction*, bool);
  multipredict_fn_t multipredict_f = multipredict<id>;
  predict_or_learn_fn_t learn_fn;
  predict_or_learn_fn_t predict_fn;
  std::string name = stack_builder.get_setupfn_name(scorer_setup);

  if (link == "identity")
  {
    learn_fn = predict_or_learn<true, id>;
    predict_fn = predict_or_learn<false, id>;
    name += "-identity";
  }
  else if (link == "logistic")
  {
    learn_fn = predict_or_learn<true, logistic>;
    predict_fn = predict_or_learn<false, logistic>;
    name += "-logistic";
    multipredict_f = multipredict<logistic>;
  }
  else if (link == "glf1")
  {
    learn_fn = predict_or_learn<true, glf1>;
    predict_fn = predict_or_learn<false, glf1>;
    name += "-glf1";
    multipredict_f = multipredict<glf1>;
  }
  else if (link == "poisson")
  {
    learn_fn = predict_or_learn<true, expf>;
    predict_fn = predict_or_learn<false, expf>;
    name += "-poisson";
    multipredict_f = multipredict<expf>;
  }
  else { THROW("Unknown link function: " << link); }

  auto s = VW::make_unique<scorer>(&all);
  // This always returns a learner.
  auto base = require_singleline(stack_builder.setup_base_learner());
  auto l = make_reduction_learner(std::move(s), base, learn_fn, predict_fn, name)
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .set_input_label_type(VW::label_type_t::SIMPLE)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::SCALAR)
               .set_multipredict(multipredict_f)
               .set_update(update)
               .build();

  return l;
}
