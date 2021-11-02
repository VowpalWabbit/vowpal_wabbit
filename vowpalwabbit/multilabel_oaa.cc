// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "reductions.h"
#include "vw.h"
#include "shared_data.h"

#include "io/logger.h"

using namespace VW::config;
namespace logger = VW::io::logger;

struct multi_oaa
{
  size_t k = 0;
  bool probabilities = false;
  std::string link = "";
};

template <bool is_learn>
void predict_or_learn(multi_oaa& o, VW::LEARNER::single_learner& base, example& ec)
{
  MULTILABEL::labels multilabels = ec.l.multilabels;
  MULTILABEL::labels preds = ec.pred.multilabels;
  preds.label_v.clear();

  ec.l.simple = {FLT_MAX};
  ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();
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
    else
      base.predict(ec, i);
    if ((o.link == "logistic" && ec.pred.scalar > 0.5) || (o.link != "logistic" && ec.pred.scalar > 0.0))
    { preds.label_v.push_back(i); }
    if (o.probabilities) { ec.pred.scalars.push_back(std::move(ec.pred.scalar)); }
  }
  if (is_learn)
  {
    if (multilabel_index < multilabels.label_v.size())
    {
      logger::log_error(
          "label {0} is not in {{0,{1}}} This won't work right.", multilabels.label_v[multilabel_index], o.k - 1);
    }
  }
  if (!o.probabilities)
  {
    ec.pred.multilabels = preds;
    ec.l.multilabels = multilabels;
  }
}

void finish_example(vw& all, multi_oaa& o, example& ec)
{
  if (o.probabilities)
  {
    // === Print probabilities for all classes
    std::ostringstream outputStringStream;
    for (uint32_t i = 0; i < o.k; i++)
    {
      if (i > 0) outputStringStream << ' ';
      if (all.sd->ldict) { outputStringStream << all.sd->ldict->get(i); }
      else
        outputStringStream << i;
      outputStringStream << ':' << ec.pred.scalars[i];
    }
    const auto ss_str = outputStringStream.str();
    for (auto& sink : all.final_prediction_sink) all.print_text_by_ref(sink.get(), ss_str, ec.tag);
  }
  MULTILABEL::output_example(all, ec);
  VW::finish_example(all, ec);
}

VW::LEARNER::base_learner* multilabel_oaa_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<multi_oaa>();
  option_group_definition new_options("Multilabel One Against All");
  new_options
      .add(make_option("multilabel_oaa", data->k).keep().necessary().help("One-against-all multilabel with <k> labels"))
      .add(make_option("probabilities", data->probabilities).help("predict probabilities of all classes"))
      .add(make_option("link", data->link)
               .default_value("identity")
               .keep()
               .help("Specify the link function: identity, logistic, glf1 or poisson"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

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
    pred_type = VW::prediction_type_t::scalars;
    auto loss_function_type = all.loss->getType();
    if (loss_function_type != "logistic")
      logger::log_error(
          "WARNING: --probabilities should be used only with --loss_function=logistic, currently using: {}",
          loss_function_type);
    // the three boolean template parameters are: is_learn, print_all and scores
    name_addition = "-prob";
  }
  else
  {
    name_addition = "";
    pred_type = VW::prediction_type_t::multilabels;
  }

  auto* l =
      make_reduction_learner(std::move(data), as_singleline(stack_builder.setup_base_learner()), predict_or_learn<true>,
          predict_or_learn<false>, stack_builder.get_setupfn_name(multilabel_oaa_setup) + name_addition)
          .set_params_per_weight(ws)
          .set_learn_returns_prediction(true)
          .set_input_label_type(VW::label_type_t::multilabel)
          .set_output_prediction_type(pred_type)
          .set_finish_example(finish_example)
          .build();

  all.example_parser->lbl_parser = MULTILABEL::multilabel;

  return make_base(*l);
}
