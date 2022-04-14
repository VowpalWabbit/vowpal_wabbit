// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/bs.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/loss_functions.h"
#include "vw/core/rand48.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cerrno>
#include <cfloat>
#include <cmath>
#include <memory>
#include <numeric>
#include <sstream>
#include <vector>

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::reductions;

#define BS_TYPE_MEAN 0
#define BS_TYPE_VOTE 1

struct bs_data
{
  uint32_t B = 0;  // number of bootstrap rounds
  size_t bs_type = 0;
  float lb = 0.f;
  float ub = 0.f;
  std::vector<double> pred_vec;
  VW::workspace* all = nullptr;  // for raw prediction and loss
  std::shared_ptr<VW::rand_state> _random_state;
};

void bs_predict_mean(VW::workspace& all, VW::example& ec, std::vector<double>& pred_vec)
{
  ec.pred.scalar = static_cast<float>(accumulate(pred_vec.cbegin(), pred_vec.cend(), 0.0)) / pred_vec.size();
  if (ec.weight > 0 && ec.l.simple.label != FLT_MAX)
  { ec.loss = all.loss->get_loss(all.sd, ec.pred.scalar, ec.l.simple.label) * ec.weight; }
}

void bs_predict_vote(VW::example& ec, std::vector<double>& pred_vec)
{
  // majority vote in linear time
  unsigned int counter = 0;
  int current_label = 1, init_label = 1;
  // float sum_labels = 0; // uncomment for: "avg on votes" and get_loss()
  bool majority_found = false;
  bool multivote_detected = false;  // distinct(votes)>2: used to skip part of the algorithm
  std::vector<int> pred_vec_int(pred_vec.size(), 0);

  for (size_t i = 0; i < pred_vec_int.size(); i++)
  {
    pred_vec_int[i] = static_cast<int>(
        floor(pred_vec[i] + 0.5));  // could be added: link(), min_label/max_label, cutoff between true/false for binary

    if (!multivote_detected)  // distinct(votes)>2 detection bloc
    {
      if (i == 0)
      {
        init_label = pred_vec_int[i];
        current_label = pred_vec_int[i];
      }
      else if (init_label != current_label && pred_vec_int[i] != current_label && pred_vec_int[i] != init_label)
      {
        multivote_detected = true;  // more than 2 distinct votes detected
      }
    }

    if (counter == 0)
    {
      counter = 1;
      current_label = pred_vec_int[i];
    }
    else
    {
      if (pred_vec_int[i] == current_label) { counter++; }
      else
      {
        counter--;
      }
    }
  }

  if (counter > 0 && multivote_detected)  // remove this condition for: "avg on votes" and get_loss()
  {
    counter = 0;
    for (unsigned int i = 0; i < pred_vec.size(); i++)
    {
      if (pred_vec_int[i] == current_label)
      {
        counter++;
        // sum_labels += pred_vec[i]; // uncomment for: "avg on votes" and get_loss()
      }
    }
    if (counter * 2 > pred_vec.size()) { majority_found = true; }
  }

  if (multivote_detected && !majority_found)  // then find most frequent element - if tie: smallest tie label
  {
    std::sort(pred_vec_int.begin(), pred_vec_int.end());
    int tmp_label = pred_vec_int[0];
    counter = 1;
    for (unsigned int i = 1, temp_count = 1; i < pred_vec.size(); i++)
    {
      if (tmp_label == pred_vec_int[i]) { temp_count++; }
      else
      {
        if (temp_count > counter)
        {
          current_label = tmp_label;
          counter = temp_count;
        }
        tmp_label = pred_vec_int[i];
        temp_count = 1;
      }
    }
    /* uncomment for: "avg on votes" and get_loss()
    sum_labels = 0;
    for(unsigned int i=0; i<pred_vec.size(); i++)
      if(pred_vec_int[i] == current_label)
        sum_labels += pred_vec[i]; */
  }
  // ld.prediction = sum_labels/(float)counter; //replace line below for: "avg on votes" and get_loss()
  ec.pred.scalar = static_cast<float>(current_label);

  // ec.loss = all.loss->get_loss(all.sd, ld.prediction, ld.label) * ec.weight; //replace line below for: "avg on votes"
  // and get_loss()
  ec.loss = ((ec.pred.scalar == ec.l.simple.label) ? 0.f : 1.f) * ec.weight;
}

void print_result(
    VW::io::writer* f, float res, const VW::v_array<char>& tag, float lb, float ub, VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << std::fixed << res;
  if (!tag.empty()) { ss << " " << VW::string_view{tag.begin(), tag.size()}; }
  ss << std::fixed << ' ' << lb << ' ' << ub << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
}

void output_example(VW::workspace& all, bs_data& d, const VW::example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX && !ec.test_only) { all.sd->weighted_labels += (static_cast<double>(ld.label)) * ec.weight; }

  if (!all.final_prediction_sink.empty())  // get confidence interval only when printing out predictions
  {
    d.lb = FLT_MAX;
    d.ub = -FLT_MAX;
    for (double v : d.pred_vec)
    {
      if (v > d.ub) { d.ub = static_cast<float>(v); }
      if (v < d.lb) { d.lb = static_cast<float>(v); }
    }
  }

  for (auto& sink : all.final_prediction_sink)
  { print_result(sink.get(), ec.pred.scalar, ec.tag, d.lb, d.ub, all.logger); }

  print_update(all, ec);
}

template <bool is_learn>
void predict_or_learn(bs_data& d, single_learner& base, VW::example& ec)
{
  VW::workspace& all = *d.all;
  bool shouldOutput = all.raw_prediction != nullptr;

  float weight_temp = ec.weight;

  std::stringstream outputStringStream;
  d.pred_vec.clear();

  for (size_t i = 1; i <= d.B; i++)
  {
    ec.weight = weight_temp * static_cast<float>(bs::weight_gen(d._random_state));

    if (is_learn) { base.learn(ec, i - 1); }
    else
    {
      base.predict(ec, i - 1);
    }

    d.pred_vec.push_back(ec.pred.scalar);

    if (shouldOutput)
    {
      if (i > 1) { outputStringStream << ' '; }
      outputStringStream << i << ':' << ec.partial_prediction;
    }
  }

  ec.weight = weight_temp;

  switch (d.bs_type)
  {
    case BS_TYPE_MEAN:
      bs_predict_mean(all, ec, d.pred_vec);
      break;
    case BS_TYPE_VOTE:
      bs_predict_vote(ec, d.pred_vec);
      break;
    default:
      THROW("Unknown bs_type specified: " << d.bs_type);
  }

  if (shouldOutput) { all.print_text_by_ref(all.raw_prediction.get(), outputStringStream.str(), ec.tag, all.logger); }
}

void finish_example(VW::workspace& all, bs_data& d, VW::example& ec)
{
  output_example(all, d, ec);
  VW::finish_example(all, ec);
}

base_learner* VW::reductions::bs_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<bs_data>();
  std::string type_string;
  option_group_definition new_options("[Reduction] Bootstrap");
  new_options
      .add(make_option("bootstrap", data->B).keep().necessary().help("K-way bootstrap by online importance resampling"))
      .add(make_option("bs_type", type_string)
               .keep()
               .default_value("mean")
               .one_of({"mean", "vote"})
               .help("Prediction type"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  size_t ws = data->B;
  data->ub = FLT_MAX;
  data->lb = -FLT_MAX;

  if (options.was_supplied("bs_type"))
  {
    if (type_string == "mean") { data->bs_type = BS_TYPE_MEAN; }
    else if (type_string == "vote")
    {
      data->bs_type = BS_TYPE_VOTE;
    }
    else
    {
      all.logger.err_warn("bs_type must be in {{'mean','vote'}}; resetting to mean.");
      data->bs_type = BS_TYPE_MEAN;
    }
  }
  else
  {  // by default use mean
    data->bs_type = BS_TYPE_MEAN;
  }

  data->pred_vec.reserve(data->B);
  data->all = &all;
  data->_random_state = all.get_random_state();

  auto* l = make_reduction_learner(std::move(data), as_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(bs_setup))
                .set_params_per_weight(ws)
                .set_learn_returns_prediction(true)
                .set_finish_example(::finish_example)
                .set_input_label_type(VW::label_type_t::simple)
                .set_output_prediction_type(VW::prediction_type_t::scalar)
                .build();

  return make_base(*l);
}
