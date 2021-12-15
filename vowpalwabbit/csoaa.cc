// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw.h"
#include "vw_exception.h"
#include "csoaa.h"

#include "io/logger.h"

namespace logger = VW::io::logger;

using namespace VW::LEARNER;
using namespace COST_SENSITIVE;
using namespace VW::config;

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::csoaa

namespace CSOAA
{
struct csoaa
{
  uint32_t num_classes = 0;
  polyprediction* pred = nullptr;
  ~csoaa() { free(pred); }
};

template <bool is_learn>
inline void inner_loop(single_learner& base, example& ec, uint32_t i, float cost, uint32_t& prediction, float& score,
    float& partial_prediction)
{
  if (is_learn)
  {
    ec.weight = (cost == FLT_MAX) ? 0.f : 1.f;
    ec.l.simple.label = cost;
    base.learn(ec, i - 1);
  }
  else
    base.predict(ec, i - 1);

  partial_prediction = ec.partial_prediction;
  if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction))
  {
    score = ec.partial_prediction;
    prediction = i;
  }
  add_passthrough_feature(ec, i, ec.partial_prediction);
}

#define DO_MULTIPREDICT true

template <bool is_learn>
void predict_or_learn(csoaa& c, single_learner& base, example& ec)
{
  COST_SENSITIVE::label ld = std::move(ec.l.cs);

  // Guard example state restore against throws
  auto restore_guard = VW::scope_exit([&ld, &ec] { ec.l.cs = std::move(ld); });

  uint32_t prediction = 1;
  float score = FLT_MAX;
  size_t pt_start = ec.passthrough ? ec.passthrough->size() : 0;
  ec.l.simple = {0.};
  ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();

  bool dont_learn = DO_MULTIPREDICT && !is_learn;

  if (!ld.costs.empty())
  {
    for (auto& cl : ld.costs)
      inner_loop<is_learn>(base, ec, cl.class_index, cl.x, prediction, score, cl.partial_prediction);
    ec.partial_prediction = score;
  }
  else if (dont_learn)
  {
    ec.l.simple = {FLT_MAX};
    ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();

    base.multipredict(ec, 0, c.num_classes, c.pred, false);
    for (uint32_t i = 1; i <= c.num_classes; i++)
    {
      add_passthrough_feature(ec, i, c.pred[i - 1].scalar);
      if (c.pred[i - 1].scalar < c.pred[prediction - 1].scalar) prediction = i;
    }
    ec.partial_prediction = c.pred[prediction - 1].scalar;
  }
  else
  {
    float temp;
    for (uint32_t i = 1; i <= c.num_classes; i++) inner_loop<false>(base, ec, i, FLT_MAX, prediction, score, temp);
  }

  if (ec.passthrough)
  {
    uint64_t second_best = 0;
    float second_best_cost = FLT_MAX;
    for (size_t i = 0; i < ec.passthrough->size() - pt_start; i++)
    {
      float val = ec.passthrough->values[pt_start + i];
      if ((val > ec.partial_prediction) && (val < second_best_cost))
      {
        second_best_cost = val;
        second_best = ec.passthrough->indicies[pt_start + i];
      }
    }
    if (second_best_cost < FLT_MAX)
    {
      float margin = second_best_cost - ec.partial_prediction;
      add_passthrough_feature(ec, constant * 2, margin);
      add_passthrough_feature(ec, constant * 2 + 1 + second_best, 1.);
    }
    else
      add_passthrough_feature(ec, constant * 3, 1.);
  }

  ec.pred.multiclass = prediction;
}

void finish_example(VW::workspace& all, csoaa&, example& ec) { COST_SENSITIVE::finish_example(all, ec); }

base_learner* csoaa_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto c = VW::make_unique<csoaa>();
  option_group_definition new_options("Cost Sensitive One Against All");
  new_options.add(
      make_option("csoaa", c->num_classes).keep().necessary().help("One-against-all multiclass with <k> costs"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  if (options.was_supplied("probabilities"))
  { THROW("Error: csoaa does not support probabilities flag, please use oaa or multilabel_oaa"); }

  c->pred = calloc_or_throw<polyprediction>(c->num_classes);
  size_t ws = c->num_classes;
  auto* l = make_reduction_learner(std::move(c), as_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(csoaa_setup))
                .set_learn_returns_prediction(
                    true) /* csoaa.learn calls gd.learn. nothing to be gained by calling csoaa.predict first */
                .set_params_per_weight(ws)
                .set_output_prediction_type(VW::prediction_type_t::multiclass)
                .set_input_label_type(VW::label_type_t::cs)
                .set_finish_example(finish_example)
                .build();

  all.example_parser->lbl_parser = cs_label;
  all.cost_sensitive = make_base(*l);
  return all.cost_sensitive;
}
}  // namespace CSOAA