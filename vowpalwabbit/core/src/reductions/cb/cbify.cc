// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cbify.h"

#include "vw/common/hash.h"
#include "vw/config/options.h"
#include "vw/core/debug_log.h"
#include "vw/core/prob_dist_cont.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
#include "vw/explore/explore.h"

#include <cfloat>
#include <vector>

using namespace VW::LEARNER;

using namespace VW::config;

using VW::cb_continuous::continuous_label;
using VW::cb_continuous::continuous_label_elm;

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CBIFY

namespace VW
{
namespace reductions
{
void cbify_adf_data::init_adf_data(std::size_t num_actions_, std::size_t increment_,
    std::vector<std::vector<VW::namespace_index>>& interactions,
    std::vector<std::vector<extent_term>>& extent_interactions)
{
  this->num_actions = num_actions_;
  this->increment = increment_;

  ecs.resize(num_actions_);
  for (size_t a = 0; a < num_actions_; ++a)
  {
    ecs[a] = new VW::example;
    auto& lab = ecs[a]->l.cb;
    lab.reset_to_default();
    ecs[a]->interactions = &interactions;
    ecs[a]->extent_interactions = &extent_interactions;
  }

  // cache mask for copy routine
  uint64_t total = num_actions_ * increment_;
  uint64_t power_2 = 0;

  while (total > 0)
  {
    total = total >> 1;
    power_2++;
  }

  this->custom_index_mask = (static_cast<uint64_t>(1) << power_2) - 1;
}

cbify_adf_data::~cbify_adf_data()
{
  for (auto* ex : ecs) { delete ex; }
}

void cbify_adf_data::copy_example_to_adf(parameters& weights, VW::example& ec)
{
  const uint64_t mask = weights.mask();

  for (size_t a = 0; a < num_actions; ++a)
  {
    auto& eca = *ecs[a];
    // clear label
    auto& lab = eca.l.cb;
    lab.reset_to_default();

    // copy data
    VW::copy_example_data(&eca, &ec);

    // offset indices for given action
    for (features& fs : eca)
    {
      for (feature_index& idx : fs.indices)
      {
        auto rawidx = idx;
        rawidx -= rawidx & custom_index_mask;
        rawidx += a * increment;
        idx = rawidx & mask;
      }
    }

    // avoid empty example by adding a tag (hacky)
    if (VW::example_is_newline_not_header_cb(eca) && eca.l.cb.is_test_label()) { eca.tag.push_back('n'); }
  }
}
}  // namespace reductions
}  // namespace VW

namespace
{
class cbify_reg
{
public:
  float min_value = 0.f;
  float max_value = 0.f;
  float bandwidth = 0.f;
  int num_actions = 0;
  int loss_option = 0;
  int loss_report = 0;
  float loss_01_ratio = 0.f;
  continuous_label cb_cont_label;
};

class cbify
{
public:
  VW::cb_label cb_label;
  uint64_t app_seed = 0;
  VW::action_scores a_s;
  cbify_reg regression_data;
  // used as the seed
  size_t example_counter = 0;
  VW::workspace* all = nullptr;
  bool use_adf = false;  // if true, reduce to cb_explore_adf instead of cb_explore
  VW::reductions::cbify_adf_data adf_data;
  float loss0 = 0.f;
  float loss1 = 0.f;
  bool flip_loss_sign = false;
  uint32_t chosen_action = 0;

  // for ldf inputs
  std::vector<std::vector<VW::cs_class>> cs_costs;
  std::vector<std::vector<VW::cb_class>> cb_costs;
  std::vector<VW::action_scores> cb_as;
};

float loss(const cbify& data, uint32_t label, uint32_t final_prediction)
{
  float mult = data.flip_loss_sign ? -1.f : 1.f;
  if (label != final_prediction) { return mult * data.loss1; }
  else { return mult * data.loss0; }
}

float loss_cs(const cbify& data, const std::vector<VW::cs_class>& costs, uint32_t final_prediction)
{
  float cost = 0.;
  for (const auto& wc : costs)
  {
    if (wc.class_index == final_prediction)
    {
      cost = wc.x;
      break;
    }
  }
  return data.loss0 + (data.loss1 - data.loss0) * cost;
}

float loss_csldf(const cbify& data, const std::vector<std::vector<VW::cs_class>>& cs_costs, uint32_t final_prediction)
{
  float cost = 0.;
  for (const auto& costs : cs_costs)
  {
    if (costs[0].class_index == final_prediction)
    {
      cost = costs[0].x;
      break;
    }
  }
  return data.loss0 + (data.loss1 - data.loss0) * cost;
}

float get_squared_loss(cbify& data, float chosen_action, float label)
{
  float diff = label - chosen_action;
  float range = data.regression_data.max_value - data.regression_data.min_value;
  return (diff * diff) / (range * range);
}

float get_absolute_loss(cbify& data, float chosen_action, float label)
{
  float diff = label - chosen_action;
  float range = data.regression_data.max_value - data.regression_data.min_value;
  return std::abs(diff) / range;
}

float get_01_loss(cbify& data, float chosen_action, float label)
{
  float diff = label - chosen_action;
  float range = data.regression_data.max_value - data.regression_data.min_value;
  if (std::abs(diff) <= (data.regression_data.loss_01_ratio * range)) { return 0.0f; }
  return 1.0f;
}

// discretized continuous action space predict_or_learn. Non-afd workflow only
// Receives Regression example as input, sends cb example to base learn/predict which is cb_explore
template <bool is_learn>
void predict_or_learn_regression_discrete(cbify& data, learner& base, VW::example& ec)
{
  VW_DBG(ec) << "cbify_reg: #### is_learn = " << is_learn << VW::debug::simple_label_to_string(ec)
             << VW::debug::features_to_string(ec) << std::endl;

  VW::simple_label regression_label = ec.l.simple;
  data.cb_label.costs.clear();
  ec.l.cb = data.cb_label;
  ec.pred.a_s = std::move(data.a_s);

  // Call the cb_explore algorithm. It returns a vector of probabilities for each action
  base.predict(ec);

  uint32_t chosen_action;
  if (VW::explore::sample_after_normalizing(
          data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  VW::cb_class cb;
  cb.action = chosen_action + 1;
  cb.probability = ec.pred.a_s[chosen_action].score;

  if (!cb.action) THROW("No action with non-zero probability found.");
  float continuous_range = data.regression_data.max_value - data.regression_data.min_value;
  float converted_action =
      data.regression_data.min_value + chosen_action * continuous_range / data.regression_data.num_actions;

  if (data.regression_data.loss_option == 0)
  {
    cb.cost = get_squared_loss(data, converted_action, regression_label.label);
  }
  else if (data.regression_data.loss_option == 1)
  {
    cb.cost = get_absolute_loss(data, converted_action, regression_label.label);
  }
  else if (data.regression_data.loss_option == 2)
  {
    cb.cost = get_01_loss(data, converted_action, regression_label.label);
  }

  // Create a new cb label
  data.cb_label.costs.push_back(cb);
  ec.l.cb = data.cb_label;

  if (is_learn) { base.learn(ec); }

  if (data.regression_data.loss_report == 1)
  {
    // for reporting average loss to be in the correct range (reverse normalizing)
    size_t siz = data.cb_label.costs.size();
    if (data.regression_data.loss_option == 0)
    {
      data.cb_label.costs[siz - 1].cost = cb.cost * continuous_range * continuous_range;
    }
    else if (data.regression_data.loss_option == 1) { data.cb_label.costs[siz - 1].cost = cb.cost * continuous_range; }
  }

  data.a_s = std::move(ec.pred.a_s);
  data.a_s.clear();
  ec.l.cb.costs.clear();

  ec.l.simple = regression_label;
  ec.pred.scalar = converted_action;
}

// continuous action space predict_or_learn. Non-afd workflow only
// Receives Regression example as input, sends cb_continuous example to base learn/predict
template <bool is_learn>
void predict_or_learn_regression(cbify& data, learner& base, VW::example& ec)
{
  VW_DBG(ec) << "cbify_reg: #### is_learn = " << is_learn << VW::debug::simple_label_to_string(ec)
             << VW::debug::features_to_string(ec) << std::endl;

  // Save simple label from the example just in case base.predict changes the label.
  // Technically it should not.
  const VW::simple_label regression_label = ec.l.simple;

  // Clear the prediction before getting a prediction from base
  ec.pred.pdf_value = {0.f, 0.f};

  // Get the continuous action and pdf value for the current features
  base.predict(ec);

  VW_DBG(ec) << "cbify-reg: base.predict() = " << VW::debug::simple_label_to_string(ec)
             << VW::debug::features_to_string(ec) << std::endl;
  VW_DBG(ec) << "cbify-reg: predict before learn, chosen_action=" << ec.pred.pdf_value.action << std::endl;

  // Create a label from the prediction and a cost derived from the actual
  // regression label.

  continuous_label_elm cb_cont_lbl;

  cb_cont_lbl.action = ec.pred.pdf_value.action;
  cb_cont_lbl.pdf_value = ec.pred.pdf_value.pdf_value;

  if (data.regression_data.loss_option == 0)
  {
    cb_cont_lbl.cost = get_squared_loss(data, ec.pred.pdf_value.action, regression_label.label);
  }
  else if (data.regression_data.loss_option == 1)
  {
    cb_cont_lbl.cost = get_absolute_loss(data, ec.pred.pdf_value.action, regression_label.label);
  }
  else if (data.regression_data.loss_option == 2)
  {
    cb_cont_lbl.cost = get_01_loss(data, ec.pred.pdf_value.action, regression_label.label);
  }

  data.regression_data.cb_cont_label.costs.clear();
  data.regression_data.cb_cont_label.costs.push_back(cb_cont_lbl);

  // Use the label inside the reduction data structure
  ec.l.cb_cont = data.regression_data.cb_cont_label;

  VW_DBG(ec) << "cbify-reg: before base.learn() = " << VW::to_string(ec.l.cb_cont) << VW::debug::features_to_string(ec)
             << std::endl;
  if (is_learn) { base.learn(ec); }
  VW_DBG(ec) << "cbify-reg: after base.learn() = " << VW::to_string(ec.l.cb_cont) << VW::debug::features_to_string(ec)
             << std::endl;

  // Update the label inside the reduction data structure
  data.regression_data.cb_cont_label = ec.l.cb_cont;

  if (data.regression_data.loss_report == 1)
  {
    // for reporting average loss to be in the correct range (reverse normalizing)
    const float continuous_range = data.regression_data.max_value - data.regression_data.min_value;
    const size_t cost_size = data.regression_data.cb_cont_label.costs.size();
    if (data.regression_data.loss_option == 0)
    {
      data.regression_data.cb_cont_label.costs[cost_size - 1].cost =
          cb_cont_lbl.cost * continuous_range * continuous_range;
    }
    else if (data.regression_data.loss_option == 1)
    {
      data.regression_data.cb_cont_label.costs[cost_size - 1].cost = cb_cont_lbl.cost * continuous_range;
    }
  }

  ec.l.cb_cont.costs.clear();
  ec.l.simple = regression_label;  // restore the regression label
  ec.pred.scalar = cb_cont_lbl.action;
}

template <bool is_learn, bool use_cs>
void predict_or_learn(cbify& data, learner& base, VW::example& ec)
{
  // Store the multiclass or cost-sensitive input label
  VW::multiclass_label ld;
  VW::cs_label csl;
  if (use_cs) { csl = std::move(ec.l.cs); }
  else { ld = std::move(ec.l.multi); }

  ec.l.cb.costs.clear();
  ec.pred.a_s.clear();

  // Call the cb_explore algorithm. It returns a vector of probabilities for each action
  base.predict(ec);

  uint32_t chosen_action;
  if (VW::explore::sample_after_normalizing(
          data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  // Create a new cb label
  const auto action = chosen_action + 1;
  const auto cost = use_cs ? loss_cs(data, csl.costs, action) : loss(data, ld.label, action);
  ec.l.cb.costs.push_back(VW::cb_class{
      cost,
      action,                           // action
      ec.pred.a_s[chosen_action].score  // probability
  });

  if (is_learn) { base.learn(ec); }

  if (use_cs) { ec.l.cs = std::move(csl); }
  else { ec.l.multi = std::move(ld); }

  ec.pred.multiclass = action;
  ec.l.cb.costs.clear();
}

template <bool use_cs>
void predict_adf(cbify& data, learner& base, VW::example& ec)
{
  const auto save_label = ec.l;

  data.adf_data.copy_example_to_adf(data.all->weights, ec);
  base.predict(data.adf_data.ecs);

  auto& out_ec = *data.adf_data.ecs[0];

  if (VW::explore::sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
          end_scores(out_ec.pred.a_s), data.chosen_action))
    THROW("Failed to sample from pdf");

  ec.pred.multiclass = out_ec.pred.a_s[data.chosen_action].action + 1;
  ec.l = save_label;
}

template <bool use_cs>
void learn_adf(cbify& data, learner& base, VW::example& ec)
{
  auto& out_ec = *data.adf_data.ecs[0];
  VW::multiclass_label ld;
  VW::cs_label csl;

  if (use_cs) { csl = ec.l.cs; }
  else { ld = ec.l.multi; }

  VW::cb_class cl;
  cl.action = out_ec.pred.a_s[data.chosen_action].action + 1;
  cl.probability = out_ec.pred.a_s[data.chosen_action].score;

  if (!cl.action) THROW("No action with non-zero probability found.");

  if (use_cs) { cl.cost = loss_cs(data, csl.costs, cl.action); }
  else { cl.cost = loss(data, ld.label, cl.action); }

  // add cb label to chosen action
  auto& lab = data.adf_data.ecs[cl.action - 1]->l.cb;
  lab.costs.clear();
  lab.costs.push_back(cl);

  base.learn(data.adf_data.ecs);
}

void do_actual_predict_ldf(cbify& data, learner& base, VW::multi_ex& ec_seq)
{
  // change label and pred data for cb
  if (data.cs_costs.size() < ec_seq.size()) { data.cs_costs.resize(ec_seq.size()); }
  if (data.cb_costs.size() < ec_seq.size()) { data.cb_costs.resize(ec_seq.size()); }
  if (data.cb_as.size() < ec_seq.size()) { data.cb_as.resize(ec_seq.size()); }
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    data.cs_costs[i] = ec.l.cs.costs;
    data.cb_costs[i].clear();
    ec.l.cb.costs = data.cb_costs[i];
    ec.pred.a_s = std::move(data.cb_as[i]);
    ec.pred.a_s.clear();
  }

  base.predict(ec_seq);

  auto& out_ec = *ec_seq[0];

  if (VW::explore::sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
          end_scores(out_ec.pred.a_s), data.chosen_action))
    THROW("Failed to sample from pdf");

  // Get the predicted action (adjusting for 1 based start)
  const auto predicted_action = out_ec.pred.a_s[data.chosen_action].action + 1;

  // Set cs prediction
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    data.cb_as[i] = ec.pred.a_s;  // store action_score vector for later reuse.
    if (i == predicted_action - 1) { ec.pred.multiclass = predicted_action; }
    else { ec.pred.multiclass = 0; }
  }
}

void do_actual_learning_ldf(cbify& data, learner& base, VW::multi_ex& ec_seq)
{
  VW::cb_class cl;

  cl.action = data.cb_as[0][data.chosen_action].action + 1;
  cl.probability = data.cb_as[0][data.chosen_action].score;

  if (!cl.action) { THROW("No action with non-zero probability found."); }

  cl.cost = loss_csldf(data, data.cs_costs, cl.action);

  // add cb label to chosen action
  data.cb_label.costs.clear();
  data.cb_label.costs.push_back(cl);
  data.cb_costs[cl.action - 1] = ec_seq[cl.action - 1]->l.cb.costs;
  ec_seq[cl.action - 1]->l.cb = data.cb_label;
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    ec.pred.a_s = data.cb_as[i];
  }

  base.learn(ec_seq);

  // reset cs costs
  for (size_t i = 0; i < ec_seq.size(); ++i)
  {
    auto& ec = *ec_seq[i];
    data.cb_as[i] = std::move(ec.pred.a_s);  // store action_score vector for later reuse.
    if (i == cl.action - 1) { data.cb_label = ec.l.cb; }
    else { data.cb_costs[i] = ec.l.cb.costs; }
    ec.l.cs.costs = data.cs_costs[i];
    if (i == cl.action - 1) { ec.pred.multiclass = cl.action; }
    else { ec.pred.multiclass = 0; }
    ec.l.cb.costs.clear();
  }
}

void update_stats_cbify_ldf(const VW::workspace& /* all */, VW::shared_data& sd, const cbify& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* logger */)
{
  if (ec_seq.empty()) { return; }

  sd.weighted_labeled_examples += ec_seq[0]->weight;
  sd.example_number++;

  bool hit_loss = false;
  for (VW::example* ec_ptr : ec_seq)
  {
    auto& ec = *ec_ptr;
    const auto& costs = ec.l.cs.costs;

    if (VW::example_is_newline(ec)) { continue; }
    if (VW::is_cs_example_header(ec)) { continue; }

    sd.total_features += ec.get_num_features();

    float loss = 0.;

    uint32_t predicted_class = ec.pred.multiclass;

    if (!ec.l.cs.is_test_label())
    {
      for (auto const& cost : costs)
      {
        if (hit_loss) { break; }
        if (predicted_class == cost.class_index)
        {
          loss = cost.x;
          hit_loss = true;
        }
      }

      sd.sum_loss += loss;
      sd.sum_loss_since_last_dump += loss;
    }
  }
}

void output_example_prediction_cbify_ldf(
    VW::workspace& all, const cbify& /* data */, const VW::multi_ex& ec_seq, VW::io::logger& logger)
{
  if (ec_seq.empty()) { return; }

  for (VW::example* ec_ptr : ec_seq)
  {
    auto& ec = *ec_ptr;
    const auto& costs = ec.l.cs.costs;

    if (VW::example_is_newline(ec)) { continue; }
    if (VW::is_cs_example_header(ec)) { continue; }

    for (const auto& sink : all.final_prediction_sink)
    {
      all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, logger);
    }

    if (all.raw_prediction != nullptr)
    {
      std::string output_string;
      std::stringstream output_string_stream(output_string);
      for (size_t i = 0; i < costs.size(); i++)
      {
        if (i > 0) { output_string_stream << ' '; }
        output_string_stream << costs[i].class_index << ':' << costs[i].partial_prediction;
      }
      all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
    }
  }

  // To output a newline.
  if (all.raw_prediction != nullptr)
  {
    VW::v_array<char> empty;
    all.print_text_by_ref(all.raw_prediction.get(), "", empty, all.logger);
  }
}

void print_update_cbify_ldf(VW::workspace& all, VW::shared_data& /* sd */, const cbify& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  if (ec_seq.empty()) { return; }

  for (VW::example* ec_ptr : ec_seq)
  {
    auto& ec = *ec_ptr;
    if (VW::example_is_newline(ec)) { continue; }
    if (VW::is_cs_example_header(ec)) { continue; }
    VW::details::print_cs_update(all, ec.l.cs.is_test_label(), ec, &ec_seq, false, ec.pred.multiclass);
  }
}

void update_stats_cbify_reg_continuous(const VW::workspace& /* all */, VW::shared_data& sd, const cbify& data,
    const VW::example& ec, VW::io::logger& /* logger */)
{
  // data contains the cb_cont vector, which store among other things, loss
  // ec contains a simple label type
  const VW::simple_label& ld = ec.l.simple;
  const auto& cb_cont_costs = data.regression_data.cb_cont_label.costs;

  if (cb_cont_costs.size() > 0)
  {
    sd.update(
        ec.test_only, cb_cont_costs[0].action != FLT_MAX, cb_cont_costs[0].cost, ec.weight, ec.get_num_features());
  }

  if (ld.label != FLT_MAX) { sd.weighted_labels += static_cast<double>(cb_cont_costs[0].action) * ec.weight; }
}

void output_example_prediction_cbify_reg_continuous(
    VW::workspace& all, const cbify& data, const VW::example& /* ec */, VW::io::logger& logger)
{
  std::stringstream strm;
  if (data.regression_data.cb_cont_label.costs.size() == 1)
  {
    continuous_label_elm cost = data.regression_data.cb_cont_label.costs[0];
    strm << cost.action << ":" << cost.cost << ":" << cost.pdf_value << std::endl;
  }
  else if (data.regression_data.cb_cont_label.costs.empty())
  {
    logger.error("No costs found when writing predictions file.");
    strm << "ERR No costs found." << std::endl;
  }
  else
  {
    logger.error("Too many costs found when writing predictions file. Expecting one. ");
    strm << "ERR Too many costs found. Expecting one." << std::endl;
  }
  const std::string str = strm.str();
  for (auto& f : all.final_prediction_sink) { f->write(str.c_str(), str.size()); }
}

void update_stats_cbify_reg_discrete(const VW::workspace& /* all */, VW::shared_data& sd, const cbify& data,
    const VW::example& ec, VW::io::logger& /* logger */)
{
  // data contains the cb vector, which store among other things, loss
  // ec contains a simple label type
  const auto& ld = ec.l.simple;
  const auto& cb_costs = data.cb_label.costs;

  if (cb_costs.size() > 0)
  {
    sd.update(ec.test_only, true /*cb_costs[0].action != FLT_MAX*/, cb_costs[0].cost, ec.weight, ec.get_num_features());
  }
  if (ld.label != FLT_MAX) { sd.weighted_labels += static_cast<double>(cb_costs[0].action) * ec.weight; }
}

}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cbify_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  uint32_t num_actions = 0;
  uint32_t cb_continuous_num_actions = 0;
  auto data = VW::make_unique<::cbify>();
  bool use_cs;
  bool use_reg;  // todo: check
  bool use_discrete;

  option_group_definition new_options("[Reduction] CBify");
  new_options
      .add(make_option("cbify", num_actions)
               .keep()
               .necessary()
               .help("Convert multiclass on <k> classes into a contextual bandit problem"))
      .add(make_option("cbify_cs", use_cs).help("Consume cost-sensitive classification examples instead of multiclass"))
      .add(make_option("cbify_reg", use_reg)
               .help("Consume regression examples instead of multiclass and cost sensitive"))
      .add(make_option("cats", cb_continuous_num_actions)
               .default_value(0)
               .keep()
               .help("Continuous action tree with smoothing"))
      .add(make_option("cb_discrete", use_discrete)
               .keep()
               .help("Discretizes continuous space and adds cb_explore as option"))
      .add(make_option("min_value", data->regression_data.min_value).keep().help("Minimum continuous value"))
      .add(make_option("max_value", data->regression_data.max_value).keep().help("Maximum continuous value"))
      .add(make_option("loss_option", data->regression_data.loss_option)
               .default_value(0)
               .one_of({0, 1, 2})
               .help("Loss options for regression - 0:squared, 1:absolute, 2:0/1"))
      .add(make_option("loss_report", data->regression_data.loss_report)
               .default_value(0)
               .one_of({0, 1})
               .help("Loss report option - 0:normalized, 1:denormalized"))
      .add(make_option("loss_01_ratio", data->regression_data.loss_01_ratio)
               .default_value(0.1f)
               .help("Ratio of zero loss for 0/1 loss"))
      .add(make_option("loss0", data->loss0).default_value(0.f).help("Loss for correct label"))
      .add(make_option("loss1", data->loss1).default_value(1.f).help("Loss for incorrect label"))
      .add(make_option("flip_loss_sign", data->flip_loss_sign)
               .keep()
               .help("Flip sign of loss (use reward instead of loss)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  data->regression_data.num_actions = num_actions;
  data->use_adf = options.was_supplied("cb_explore_adf");
  data->app_seed = VW::uniform_hash("vw", 2, 0);
  data->all = &all;

  if (use_reg)
  {
    // Check invalid parameter combinations
    if (data->use_adf) { THROW("Incompatible options: cb_explore_adf and cbify_reg"); }
    if (use_cs) { THROW("Incompatible options: cbify_cs and cbify_reg"); }
    if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
    {
      THROW("Min and max values must be supplied with cbify_reg");
    }

    if (use_discrete && options.was_supplied("cats")) { THROW("Incompatible options: cb_discrete and cats"); }
    else if (use_discrete)
    {
      std::stringstream ss;
      ss << num_actions;
      options.insert("cb_explore", ss.str());
    }
    else if (options.was_supplied("cats"))
    {
      if (cb_continuous_num_actions != num_actions)
        THROW("Different number of actions specified for cbify and cb_continuous");
    }
    else
    {
      std::stringstream ss;
      ss << num_actions;
      options.insert("cats", ss.str());
    }
  }
  else
  {
    if (!options.was_supplied("cb_explore") && !data->use_adf)
    {
      std::stringstream ss;
      ss << num_actions;
      options.insert("cb_explore", ss.str());
    }
  }

  if (data->use_adf && (options.was_supplied("regcb") || options.was_supplied("squarecb")))
  {
    options.insert("cb_min_cost", std::to_string(data->loss0));
    options.insert("cb_max_cost", std::to_string(data->loss1));
  }

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(data->loss0), std::abs(data->loss1)) / (data->loss1 - data->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  std::shared_ptr<learner> l;
  VW::learner_update_stats_func<cbify, VW::example>* update_stats_func = nullptr;
  VW::learner_output_example_prediction_func<cbify, VW::example>* output_example_prediction_func = nullptr;
  VW::learner_print_update_func<cbify, VW::example>* print_update_func = nullptr;
  std::string name_addition;
  VW::label_type_t in_label_type;
  VW::label_type_t out_label_type;
  VW::prediction_type_t in_pred_type;
  VW::prediction_type_t out_pred_type;

  if (data->use_adf)
  {
    out_label_type = VW::label_type_t::CB;
    in_pred_type = VW::prediction_type_t::MULTICLASS;
    out_pred_type = VW::prediction_type_t::MULTICLASS;
    void (*learn_ptr)(cbify&, learner&, VW::example&);
    void (*predict_ptr)(cbify&, learner&, VW::example&);
    auto base = require_multiline(stack_builder.setup_base_learner());

    if (data->use_adf)
    {
      data->adf_data.init_adf_data(num_actions, base->increment, all.interactions, all.extent_interactions);
    }

    if (use_cs)
    {
      in_label_type = VW::label_type_t::CS;
      learn_ptr = learn_adf<true>;
      predict_ptr = predict_adf<true>;
      update_stats_func = VW::details::update_stats_cs_label<cbify>;
      output_example_prediction_func = VW::details::output_example_prediction_cs_label<cbify>;
      print_update_func = VW::details::print_update_cs_label<cbify>;
      name_addition = "-adf-cs";
      all.example_parser->lbl_parser = VW::cs_label_parser_global;
    }
    else
    {
      in_label_type = VW::label_type_t::MULTICLASS;
      learn_ptr = learn_adf<false>;
      predict_ptr = predict_adf<false>;
      update_stats_func = VW::details::update_stats_multiclass_label<cbify>;
      output_example_prediction_func = VW::details::output_example_prediction_multiclass_label<cbify>;
      print_update_func = VW::details::print_update_multiclass_label<cbify>;
      name_addition = "-adf";
      all.example_parser->lbl_parser = VW::multiclass_label_parser_global;
    }
    l = make_reduction_learner(
        std::move(data), base, learn_ptr, predict_ptr, stack_builder.get_setupfn_name(cbify_setup) + name_addition)
            .set_input_label_type(in_label_type)
            .set_output_label_type(out_label_type)
            .set_output_prediction_type(in_pred_type)
            .set_output_prediction_type(out_pred_type)
            .set_update_stats(update_stats_func)
            .set_output_example_prediction(output_example_prediction_func)
            .set_print_update(print_update_func)
            .build();
  }
  else
  {
    void (*learn_ptr)(cbify&, learner&, VW::example&);
    void (*predict_ptr)(cbify&, learner&, VW::example&);
    auto base = require_singleline(stack_builder.setup_base_learner());
    if (use_reg)
    {
      in_label_type = VW::label_type_t::SIMPLE;
      out_pred_type = VW::prediction_type_t::SCALAR;
      all.example_parser->lbl_parser = simple_label_parser_global;
      if (use_discrete)
      {
        out_label_type = VW::label_type_t::CB;
        in_pred_type = VW::prediction_type_t::ACTION_PROBS;
        learn_ptr = predict_or_learn_regression_discrete<true>;
        predict_ptr = predict_or_learn_regression_discrete<false>;
        update_stats_func = update_stats_cbify_reg_discrete;
        // Prediction value output is not supported.
        output_example_prediction_func = nullptr;
        // FIXME: this should not use the simple label version since the label types dont match.
        print_update_func = VW::details::print_update_simple_label<cbify>;
        name_addition = "-reg-discrete";
      }
      else
      {
        out_label_type = VW::label_type_t::CONTINUOUS;
        in_pred_type = VW::prediction_type_t::ACTION_PDF_VALUE;
        learn_ptr = predict_or_learn_regression<true>;
        predict_ptr = predict_or_learn_regression<false>;
        update_stats_func = update_stats_cbify_reg_continuous;
        output_example_prediction_func = output_example_prediction_cbify_reg_continuous;
        // FIXME: this should not use the simple label version since the label types dont match.
        print_update_func = VW::details::print_update_simple_label<cbify>;

        name_addition = "-reg";
      }
    }
    else if (use_cs)
    {
      in_label_type = VW::label_type_t::CS;
      out_label_type = VW::label_type_t::CB;
      in_pred_type = VW::prediction_type_t::ACTION_PROBS;
      out_pred_type = VW::prediction_type_t::MULTICLASS;
      learn_ptr = predict_or_learn<true, true>;
      predict_ptr = predict_or_learn<false, true>;
      update_stats_func = VW::details::update_stats_cs_label<cbify>;
      output_example_prediction_func = VW::details::output_example_prediction_cs_label<cbify>;
      print_update_func = VW::details::print_update_cs_label<cbify>;
      name_addition = "-cs";
      all.example_parser->lbl_parser = VW::cs_label_parser_global;
    }
    else
    {
      in_label_type = VW::label_type_t::MULTICLASS;
      out_label_type = VW::label_type_t::CB;
      in_pred_type = VW::prediction_type_t::ACTION_PROBS;
      out_pred_type = VW::prediction_type_t::MULTICLASS;
      learn_ptr = predict_or_learn<true, false>;
      predict_ptr = predict_or_learn<false, false>;
      update_stats_func = VW::details::update_stats_multiclass_label<cbify>;
      output_example_prediction_func = VW::details::output_example_prediction_multiclass_label<cbify>;
      print_update_func = VW::details::print_update_multiclass_label<cbify>;
      name_addition = "";
      all.example_parser->lbl_parser = VW::multiclass_label_parser_global;
    }
    auto builder = make_reduction_learner(
        std::move(data), base, learn_ptr, predict_ptr, stack_builder.get_setupfn_name(cbify_setup) + name_addition)
                       .set_input_label_type(in_label_type)
                       .set_output_label_type(out_label_type)
                       .set_input_prediction_type(in_pred_type)
                       .set_output_prediction_type(out_pred_type)
                       .set_learn_returns_prediction(true)
                       .set_update_stats(update_stats_func)
                       .set_print_update(print_update_func);

    // output_example_prediction_func is not used in the case of use_discrete
    if (output_example_prediction_func != nullptr)
    {
      builder.set_output_example_prediction(output_example_prediction_func);
    }
    l = builder.build();
  }

  return l;
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cbifyldf_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<cbify>();
  bool cbify_ldf_option = false;

  option_group_definition new_options("[Reduction] Make csoaa_ldf into Contextual Bandit");
  new_options
      .add(make_option("cbify_ldf", cbify_ldf_option)
               .keep()
               .necessary()
               .help("Convert csoaa_ldf into a contextual bandit problem"))
      .add(make_option("loss0", data->loss0).default_value(0.f).help("Loss for correct label"))
      .add(make_option("loss1", data->loss1).default_value(1.f).help("Loss for incorrect label"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  data->app_seed = VW::uniform_hash("vw", 2, 0);
  data->all = &all;
  data->use_adf = true;

  if (!options.was_supplied("cb_explore_adf")) { options.insert("cb_explore_adf", ""); }
  options.insert("cb_min_cost", std::to_string(data->loss0));
  options.insert("cb_max_cost", std::to_string(data->loss1));

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(data->loss0), std::abs(data->loss1)) / (data->loss1 - data->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  auto base = require_multiline(stack_builder.setup_base_learner());
  auto l = make_reduction_learner(std::move(data), base, do_actual_learning_ldf, do_actual_predict_ldf,
      stack_builder.get_setupfn_name(cbifyldf_setup))
               .set_input_label_type(VW::label_type_t::CS)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_output_example_prediction(output_example_prediction_cbify_ldf)
               .set_print_update(print_update_cbify_ldf)
               .set_update_stats(update_stats_cbify_ldf)
               .build();
  all.example_parser->lbl_parser = VW::cs_label_parser_global;

  return l;
}
