// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/multilabel_oaa.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <sstream>

using namespace VW::config;

namespace
{
class multi_oaa
{
public:
  size_t k = 0;
  bool probabilities = false;
  std::string link = "";
  VW::io::logger logger;

  explicit multi_oaa(VW::io::logger logger) : logger(std::move(logger)) {}
};

template <bool is_learn>
void predict_or_learn(multi_oaa& o, VW::LEARNER::learner& base, VW::example& ec)
{
  auto multilabels = ec.l.multilabels;
  auto preds = ec.pred.multilabels;
  preds.label_v.clear();

  ec.l.simple = {FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
  uint32_t multilabel_index = 0;
  for (uint32_t i = 0; i < o.k; i++)
  {
    if (is_learn)
    {
      ec.l.simple.label = -1.f;
      if (multilabels.label_v.size() > multilabel_index && multilabels.label_v[multilabel_index] == i)
      {
        ec.l.simple.label = 1.f;
        multilabel_index++;
      }
      base.learn(ec, i);
    }
    else { base.predict(ec, i); }
    if ((o.link == "logistic" && ec.pred.scalar > 0.5) || (o.link != "logistic" && ec.pred.scalar > 0.0))
    {
      preds.label_v.push_back(i);
    }
    if (o.probabilities) { ec.pred.scalars.push_back(std::move(ec.pred.scalar)); }
  }
  if (is_learn)
  {
    if (multilabel_index < multilabels.label_v.size())
    {
      o.logger.out_error(
          "label {0} is not in {{0,{1}}} This won't work right.", multilabels.label_v[multilabel_index], o.k - 1);
    }
  }
  if (!o.probabilities)
  {
    ec.pred.multilabels = preds;
    ec.l.multilabels = multilabels;
  }
}

void update_stats_multilabel_oaa(
    const VW::workspace& all, VW::shared_data&, const multi_oaa&, const VW::example& ec, VW::io::logger&)
{
  VW::details::update_stats_multilabel(all, ec);
}

void output_example_prediction_multilabel_oaa(
    VW::workspace& all, const multi_oaa& o, const VW::example& ec, VW::io::logger&)
{
  if (o.probabilities)
  {
    // === Print probabilities for all classes
    std::ostringstream output_string_stream;
    for (uint32_t i = 0; i < o.k; i++)
    {
      if (i > 0) { output_string_stream << ' '; }
      if (all.sd->ldict) { output_string_stream << all.sd->ldict->get(i); }
      else { output_string_stream << i; }
      output_string_stream << ':' << ec.pred.scalars[i];
    }
    const auto ss_str = output_string_stream.str();
    for (auto& sink : all.final_prediction_sink) { all.print_text_by_ref(sink.get(), ss_str, ec.tag, all.logger); }
  }
  VW::details::output_example_prediction_multilabel(all, ec);
}

void print_update_multilabel_oaa(
    VW::workspace& all, VW::shared_data&, const multi_oaa&, const VW::example& ec, VW::io::logger&)
{
  VW::details::print_update_multilabel(all, ec);
}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::multilabel_oaa_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<multi_oaa>(all.logger);
  option_group_definition new_options("[Reduction] Multilabel One Against All");
  uint64_t k;
  new_options
      .add(make_option("multilabel_oaa", k).keep().necessary().help("One-against-all multilabel with <k> labels"))
      .add(make_option("probabilities", data->probabilities).help("Predict probabilities of all classes"))
      .add(make_option("link", data->link)
               .default_value("identity")
               .keep()
               .one_of({"identity", "logistic", "glf1", "poisson"})
               .help("Specify the link function"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  data->k = VW::cast_to_smaller_type<size_t>(k);
  std::string name_addition;
  VW::prediction_type_t pred_type;
  size_t ws = data->k;

  if (data->probabilities)
  {
    // Unlike oaa and csoaa_ldf (which always remove --logistic link and apply logic manually for probabilities),
    // multilabel_oaa will always add logistic link and apply logic in base (scorer) reduction
    if (data->link != "logistic")
    {
      options.replace("link", "logistic");
      data->link = "logistic";
    }
    pred_type = VW::prediction_type_t::SCALARS;
    auto loss_function_type = all.loss->get_type();
    if (loss_function_type != "logistic")
    {
      all.logger.out_warn(
          "--probabilities should be used only with --loss_function=logistic, currently using: {}", loss_function_type);
    }
    // the three boolean template parameters are: is_learn, print_all and scores
    name_addition = "-prob";
  }
  else
  {
    name_addition = "";
    pred_type = VW::prediction_type_t::MULTILABELS;
  }

  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>,
      stack_builder.get_setupfn_name(multilabel_oaa_setup) + name_addition)
               .set_params_per_weight(ws)
               .set_learn_returns_prediction(true)
               .set_input_label_type(VW::label_type_t::MULTILABEL)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(pred_type)
               .set_update_stats(update_stats_multilabel_oaa)
               .set_output_example_prediction(output_example_prediction_multilabel_oaa)
               .set_print_update(print_update_multilabel_oaa)
               .build();

  all.example_parser->lbl_parser = VW::multilabel_label_parser_global;

  return l;
}
