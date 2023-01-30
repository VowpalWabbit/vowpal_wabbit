// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/bs.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"

#include <algorithm>
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

namespace
{
#define BS_TYPE_MEAN 0
#define BS_TYPE_VOTE 1

class bs_data
{
public:
  uint32_t num_bootstrap_rounds = 0;  // number of bootstrap rounds
  size_t bs_type = 0;
  std::vector<double> pred_vec;
  VW::workspace* all = nullptr;  // for raw prediction and loss
  std::shared_ptr<VW::rand_state> random_state;
};

void bs_predict_mean(const VW::workspace& all, VW::example& ec, const std::vector<double>& pred_vec)
{
  ec.pred.scalar = static_cast<float>(std::accumulate(pred_vec.cbegin(), pred_vec.cend(), 0.0)) / pred_vec.size();
  if (ec.weight > 0 && ec.l.simple.label != FLT_MAX)
  {
    ec.loss = all.loss->get_loss(all.sd.get(), ec.pred.scalar, ec.l.simple.label) * ec.weight;
  }
}

void bs_predict_vote(VW::example& ec, const std::vector<double>& pred_vec)
{
  // majority vote in linear time
  unsigned int counter = 0;
  int current_label = 1;
  int init_label = 1;
  // float sum_labels = 0; // uncomment for: "avg on votes" and get_loss()
  bool majority_found = false;
  bool multivote_detected = false;  // distinct(votes)>2: used to skip part of the algorithm
  std::vector<int> pred_vec_int(pred_vec.size(), 0);

  for (size_t i = 0; i < pred_vec_int.size(); i++)
  {
    pred_vec_int[i] = static_cast<int>(std::floor(
        pred_vec[i] + 0.5));  // could be added: link(), min_label/max_label, cutoff between true/false for binary

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
      else { counter--; }
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

void print_result(VW::io::writer* f, float res, const VW::v_array<char>& tag, float lower_bound, float upper_bound,
    VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << std::fixed << res;
  if (!tag.empty()) { ss << " " << VW::string_view{tag.begin(), tag.size()}; }
  ss << std::fixed << ' ' << lower_bound << ' ' << upper_bound << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
}

void output_example_prediction_bs(
    VW::workspace& all, const bs_data& data, const VW::example& ec, VW::io::logger& logger)
{
  if (!all.final_prediction_sink.empty())
  {
    // get confidence interval only when printing out predictions
    const auto min_max = std::minmax_element(data.pred_vec.begin(), data.pred_vec.end());
    for (auto& sink : all.final_prediction_sink)
    {
      print_result(sink.get(), ec.pred.scalar, ec.tag, *min_max.first, *min_max.second, logger);
    }
  }
}

template <bool is_learn>
void predict_or_learn(bs_data& d, learner& base, VW::example& ec)
{
  VW::workspace& all = *d.all;
  bool should_output = all.raw_prediction != nullptr;

  float weight_temp = ec.weight;

  std::stringstream output_string_stream;
  d.pred_vec.clear();

  for (size_t i = 1; i <= d.num_bootstrap_rounds; i++)
  {
    ec.weight = weight_temp * static_cast<float>(bs::weight_gen(*d.random_state));

    if (is_learn) { base.learn(ec, i - 1); }
    else { base.predict(ec, i - 1); }

    d.pred_vec.push_back(ec.pred.scalar);

    if (should_output)
    {
      if (i > 1) { output_string_stream << ' '; }
      output_string_stream << i << ':' << ec.partial_prediction;
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

  if (should_output)
  {
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::bs_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<bs_data>();
  std::string type_string;
  option_group_definition new_options("[Reduction] Bootstrap");
  new_options
      .add(make_option("bootstrap", data->num_bootstrap_rounds)
               .keep()
               .necessary()
               .help("K-way bootstrap by online importance resampling"))
      .add(make_option("bs_type", type_string)
               .keep()
               .default_value("mean")
               .one_of({"mean", "vote"})
               .help("Prediction type"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  size_t ws = data->num_bootstrap_rounds;

  if (options.was_supplied("bs_type"))
  {
    if (type_string == "mean") { data->bs_type = BS_TYPE_MEAN; }
    else if (type_string == "vote") { data->bs_type = BS_TYPE_VOTE; }
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

  data->pred_vec.reserve(data->num_bootstrap_rounds);
  data->all = &all;
  data->random_state = all.get_random_state();

  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(bs_setup))
               .set_params_per_weight(ws)
               .set_learn_returns_prediction(true)
               .set_output_example_prediction(output_example_prediction_bs)
               .set_update_stats(VW::details::update_stats_simple_label<bs_data>)
               .set_print_update(VW::details::print_update_simple_label<bs_data>)
               .set_input_label_type(VW::label_type_t::SIMPLE)
               .set_output_prediction_type(VW::prediction_type_t::SCALAR)
               .build();

  return l;
}
