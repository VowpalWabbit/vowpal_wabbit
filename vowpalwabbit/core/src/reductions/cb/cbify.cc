// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cbify.h"

#include "vw/common/hash.h"
#include "vw/config/options.h"
#include "vw/core/cb_label_parser.h"
#include "vw/core/debug_log.h"
#include "vw/core/prob_dist_cont.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/vw.h"
#include "vw/explore/explore.h"

#include <cfloat>
#include <vector>

using namespace VW::LEARNER;
using namespace exploration;

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
    ecs[a] = VW::alloc_examples(1);
    auto& lab = ecs[a]->l.cb;
    CB::default_label(lab);
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
  for (auto* ex : ecs) { VW::dealloc_examples(ex, 1); }
}

void cbify_adf_data::copy_example_to_adf(parameters& weights, VW::example& ec)
{
  const uint64_t mask = weights.mask();

  for (size_t a = 0; a < num_actions; ++a)
  {
    auto& eca = *ecs[a];
    // clear label
    auto& lab = eca.l.cb;
    CB::default_label(lab);

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
    if (CB_ALGS::example_is_newline_not_header(eca) && CB::cb_label.test_label(eca.l)) { eca.tag.push_back('n'); }
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
  float max_cost = std::numeric_limits<float>::lowest();
};

class cbify
{
public:
  CB::label cb_label;
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
  std::vector<std::vector<CB::cb_class>> cb_costs;
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
void predict_or_learn_regression_discrete(cbify& data, single_learner& base, VW::example& ec)
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
  if (sample_after_normalizing(
          data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  CB::cb_class cb;
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
void predict_or_learn_regression(cbify& data, single_learner& base, VW::example& ec)
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
void predict_or_learn(cbify& data, single_learner& base, VW::example& ec)
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
  if (sample_after_normalizing(
          data.app_seed + data.example_counter++, begin_scores(ec.pred.a_s), end_scores(ec.pred.a_s), chosen_action))
    THROW("Failed to sample from pdf");

  // Create a new cb label
  const auto action = chosen_action + 1;
  const auto cost = use_cs ? loss_cs(data, csl.costs, action) : loss(data, ld.label, action);
  ec.l.cb.costs.push_back(CB::cb_class{
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
void predict_adf(cbify& data, multi_learner& base, VW::example& ec)
{
  const auto save_label = ec.l;

  data.adf_data.copy_example_to_adf(data.all->weights, ec);
  base.predict(data.adf_data.ecs);

  auto& out_ec = *data.adf_data.ecs[0];

  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
          end_scores(out_ec.pred.a_s), data.chosen_action))
    THROW("Failed to sample from pdf");

  ec.pred.multiclass = out_ec.pred.a_s[data.chosen_action].action + 1;
  ec.l = save_label;
}

template <bool use_cs>
void learn_adf(cbify& data, multi_learner& base, VW::example& ec)
{
  auto& out_ec = *data.adf_data.ecs[0];
  VW::multiclass_label ld;
  VW::cs_label csl;

  if (use_cs) { csl = ec.l.cs; }
  else { ld = ec.l.multi; }

  CB::cb_class cl;
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

void do_actual_predict_ldf(cbify& data, multi_learner& base, VW::multi_ex& ec_seq)
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

  if (sample_after_normalizing(data.app_seed + data.example_counter++, begin_scores(out_ec.pred.a_s),
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

void do_actual_learning_ldf(cbify& data, multi_learner& base, VW::multi_ex& ec_seq)
{
  CB::cb_class cl;

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

void output_example(VW::workspace& all, const VW::example& ec, bool& hit_loss, const VW::multi_ex* ec_seq)
{
  const auto& costs = ec.l.cs.costs;

  if (VW::example_is_newline(ec)) { return; }
  if (VW::is_cs_example_header(ec)) { return; }

  all.sd->total_features += ec.get_num_features();

  float loss = 0.;

  uint32_t predicted_class = ec.pred.multiclass;

  if (!VW::cs_label_parser_global.test_label(ec.l))
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

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }

  for (const auto& sink : all.final_prediction_sink)
  {
    all.print_by_ref(sink.get(), static_cast<float>(ec.pred.multiclass), 0, ec.tag, all.logger);
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
    // outputStringStream << std::endl;
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
  }

  VW::details::print_cs_update(all, VW::cs_label_parser_global.test_label(ec.l), ec, ec_seq, false, predicted_class);
}

void output_example_seq(VW::workspace& all, const VW::multi_ex& ec_seq)
{
  if (ec_seq.empty()) { return; }
  all.sd->weighted_labeled_examples += ec_seq[0]->weight;
  all.sd->example_number++;

  bool hit_loss = false;
  for (VW::example* ec : ec_seq) { output_example(all, *ec, hit_loss, &(ec_seq)); }

  if (all.raw_prediction != nullptr)
  {
    VW::v_array<char> empty;
    all.print_text_by_ref(all.raw_prediction.get(), "", empty, all.logger);
  }
}

void output_example_regression_discrete(VW::workspace& all, cbify& data, VW::example& ec)
{
  // data contains the cb vector, which store among other things, loss
  // ec contains a simple label type
  VW::simple_label& ld = ec.l.simple;
  const auto& cb_costs = data.cb_label.costs;

  // Track the max cost and report it at the end
  if (cb_costs[0].cost > data.regression_data.max_cost) { data.regression_data.max_cost = cb_costs[0].cost; }

  if (cb_costs.size() > 0)
  {
    all.sd->update(
        ec.test_only, true /*cb_costs[0].action != FLT_MAX*/, cb_costs[0].cost, ec.weight, ec.get_num_features());
  }

  if (ld.label != FLT_MAX) { all.sd->weighted_labels += static_cast<double>(cb_costs[0].action) * ec.weight; }

  VW::details::print_update(all, ec);
}

void output_example_regression(VW::workspace& all, cbify& data, VW::example& ec)
{
  // data contains the cb_cont vector, which store among other things, loss
  // ec contains a simple label type
  VW::simple_label& ld = ec.l.simple;
  const auto& cb_cont_costs = data.regression_data.cb_cont_label.costs;

  // Track the max cost and report it at the end
  if (cb_cont_costs[0].cost > data.regression_data.max_cost) { data.regression_data.max_cost = cb_cont_costs[0].cost; }

  if (cb_cont_costs.size() > 0)
  {
    all.sd->update(
        ec.test_only, cb_cont_costs[0].action != FLT_MAX, cb_cont_costs[0].cost, ec.weight, ec.get_num_features());
  }

  if (ld.label != FLT_MAX) { all.sd->weighted_labels += static_cast<double>(cb_cont_costs[0].action) * ec.weight; }

  VW::details::print_update(all, ec);
}

void output_cb_reg_predictions(
    std::vector<std::unique_ptr<VW::io::writer>>& predict_file_descriptors, continuous_label& label)
{
  std::stringstream strm;
  if (label.costs.size() == 1)
  {
    continuous_label_elm cost = label.costs[0];
    strm << cost.action << ":" << cost.cost << ":" << cost.pdf_value << std::endl;
  }
  else if (label.costs.empty()) { strm << "ERR No costs found." << std::endl; }
  else { strm << "ERR Too many costs found. Expecting one." << std::endl; }
  const std::string str = strm.str();
  for (auto& f : predict_file_descriptors) { f->write(str.c_str(), str.size()); }
}

void finish_example_cb_reg_continous(VW::workspace& all, cbify& data, VW::example& ec)
{
  // add output example
  output_example_regression(all, data, ec);
  output_cb_reg_predictions(all.final_prediction_sink, data.regression_data.cb_cont_label);
  VW::finish_example(all, ec);
}

void finish_example_cb_reg_discrete(VW::workspace& all, cbify& data, VW::example& ec)
{
  // add output VW::example
  output_example_regression_discrete(all, data, ec);
  VW::finish_example(all, ec);
}

void finish_multiline_example(VW::workspace& all, cbify&, VW::multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example_seq(all, ec_seq);
    // global_print_newline(all);
  }
  VW::finish_example(all, ec_seq);
}

struct options_cbify_v1
{

  bool use_cs;
  bool use_reg;  // todo: check
  bool use_discrete;
  uint32_t num_actions = 0;
  uint32_t cb_continuous_num_actions = 0;
  float min_value = 0.f;
  float max_value = 0.f;
  int loss_option = 0;
  int loss_report = 0;
  float loss_01_ratio = 0.f;
  float loss0;
  float loss1;
  bool flip_loss_sign;
  bool cb_explore_adf_supplied;
};

std::unique_ptr<options_cbify_v1> get_cbify_options_instance(
    const VW::workspace&, VW::io::logger&, options_i& options)
{
  auto cbify_opts = VW::make_unique<options_cbify_v1>();
  option_group_definition new_options("[Reduction] CBify");
  new_options
      .add(make_option("cbify", cbify_opts->num_actions)
               .keep()
               .necessary()
               .help("Convert multiclass on <k> classes into a contextual bandit problem"))
      .add(make_option("cbify_cs", cbify_opts->use_cs).help("Consume cost-sensitive classification examples instead of multiclass"))
      .add(make_option("cbify_reg", cbify_opts->use_reg)
               .help("Consume regression examples instead of multiclass and cost sensitive"))
      .add(make_option("cats", cbify_opts->cb_continuous_num_actions)
               .default_value(0)
               .keep()
               .help("Continuous action tree with smoothing"))
      .add(make_option("cb_discrete", cbify_opts->use_discrete)
               .keep()
               .help("Discretizes continuous space and adds cb_explore as option"))
      .add(make_option("min_value", cbify_opts->min_value).keep().help("Minimum continuous value"))
      .add(make_option("max_value", cbify_opts->max_value).keep().help("Maximum continuous value"))
      .add(make_option("loss_option", cbify_opts->loss_option)
               .default_value(0)
               .one_of({0, 1, 2})
               .help("Loss options for regression - 0:squared, 1:absolute, 2:0/1"))
      .add(make_option("loss_report", cbify_opts->loss_report)
               .default_value(0)
               .one_of({0, 1})
               .help("Loss report option - 0:normalized, 1:denormalized"))
      .add(make_option("loss_01_ratio", cbify_opts->loss_01_ratio)
               .default_value(0.1f)
               .help("Ratio of zero loss for 0/1 loss"))
      .add(make_option("loss0", cbify_opts->loss0).default_value(0.f).help("Loss for correct label"))
      .add(make_option("loss1", cbify_opts->loss1).default_value(1.f).help("Loss for incorrect label"))
      .add(make_option("flip_loss_sign", cbify_opts->flip_loss_sign)
               .keep()
               .help("Flip sign of loss (use reward instead of loss)"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  cbify_opts->cb_explore_adf_supplied = options.was_supplied("cb_explore_adf");
  if (cbify_opts->use_reg)
  {
    // Check invalid parameter combinations
    if (cbify_opts->cb_explore_adf_supplied) { THROW("Incompatible options: cb_explore_adf and cbify_reg"); }
    if (cbify_opts->use_cs) { THROW("Incompatible options: cbify_cs and cbify_reg"); }
    if (!options.was_supplied("min_value") || !options.was_supplied("max_value"))
    {
      THROW("Min and max values must be supplied with cbify_reg");
    }

    if (cbify_opts->use_discrete && options.was_supplied("cats")) { THROW("Incompatible options: cb_discrete and cats"); }
    else if (cbify_opts->use_discrete)
    {
      std::stringstream ss;
      ss << cbify_opts->num_actions;
      options.insert("cb_explore", ss.str());
    }
    else if (options.was_supplied("cats"))
    {
      if (cbify_opts->cb_continuous_num_actions != cbify_opts->num_actions)
        THROW("Different number of actions specified for cbify and cb_continuous");
    }
    else
    {
      std::stringstream ss;
      ss << cbify_opts->num_actions;
      options.insert("cats", ss.str());
    }
  }
  else
  {
    if (!options.was_supplied("cb_explore") && !cbify_opts->cb_explore_adf_supplied)
    {
      std::stringstream ss;
      ss << cbify_opts->num_actions;
      options.insert("cb_explore", ss.str());
    }
  }

  if (cbify_opts->cb_explore_adf_supplied && (options.was_supplied("regcb") || options.was_supplied("squarecb")))
  {
    options.insert("cb_min_cost", std::to_string(cbify_opts->loss0));
    options.insert("cb_max_cost", std::to_string(cbify_opts->loss1));
  }

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(cbify_opts->loss0), std::abs(cbify_opts->loss1)) / (cbify_opts->loss1 - cbify_opts->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  return cbify_opts;
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::cbify_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto cbify_opts = get_cbify_options_instance(all, all.logger, *stack_builder.get_options());
  if (cbify_opts == nullptr) { return nullptr; }

  auto cbify_data = VW::make_unique<::cbify>();

  cbify_data->regression_data.min_value = cbify_opts->min_value;
  cbify_data->regression_data.max_value = cbify_opts->max_value;
  cbify_data->regression_data.loss_option = cbify_opts->loss_option;
  cbify_data->regression_data.loss_report = cbify_opts->loss_report;
  cbify_data->regression_data.loss_01_ratio = cbify_opts->loss_01_ratio;
  cbify_data->loss0 = cbify_opts->loss0;
  cbify_data->loss1 = cbify_opts->loss1;
  cbify_data->flip_loss_sign = cbify_opts->flip_loss_sign;

  cbify_data->regression_data.num_actions = cbify_opts->num_actions;
  cbify_data->use_adf = cbify_opts->cb_explore_adf_supplied;
  cbify_data->app_seed = VW::uniform_hash("vw", 2, 0);
  cbify_data->all = &all;

  learner<cbify, VW::example>* l;
  void (*finish_ptr)(VW::workspace&, cbify&, VW::example&);
  std::string name_addition;
  VW::label_type_t in_label_type;
  VW::label_type_t out_label_type;
  VW::prediction_type_t in_pred_type;
  VW::prediction_type_t out_pred_type;

  if (cbify_data->use_adf)
  {
    out_label_type = VW::label_type_t::CB;
    in_pred_type = VW::prediction_type_t::MULTICLASS;
    out_pred_type = VW::prediction_type_t::MULTICLASS;
    void (*learn_ptr)(cbify&, multi_learner&, VW::example&);
    void (*predict_ptr)(cbify&, multi_learner&, VW::example&);
    multi_learner* base = as_multiline(stack_builder.setup_base_learner());

    if (cbify_data->use_adf)
    {
      cbify_data->adf_data.init_adf_data(cbify_opts->num_actions, base->increment, all.interactions, all.extent_interactions);
    }

    if (cbify_opts->use_cs)
    {
      in_label_type = VW::label_type_t::CS;
      learn_ptr = learn_adf<true>;
      predict_ptr = predict_adf<true>;
      finish_ptr = VW::details::finish_cs_example;
      name_addition = "-adf-cs";
      all.example_parser->lbl_parser = VW::cs_label_parser_global;
    }
    else
    {
      in_label_type = VW::label_type_t::MULTICLASS;
      learn_ptr = learn_adf<false>;
      predict_ptr = predict_adf<false>;
      finish_ptr = VW::details::finish_multiclass_example<cbify&>;
      name_addition = "-adf";
      all.example_parser->lbl_parser = VW::multiclass_label_parser_global;
    }
    l = make_reduction_learner(
        std::move(cbify_data), base, learn_ptr, predict_ptr, stack_builder.get_setupfn_name(cbify_setup) + name_addition)
            .set_input_label_type(in_label_type)
            .set_output_label_type(out_label_type)
            .set_output_prediction_type(in_pred_type)
            .set_output_prediction_type(out_pred_type)
            .set_finish_example(finish_ptr)
            .build(&all.logger);
  }
  else
  {
    void (*learn_ptr)(cbify&, single_learner&, VW::example&);
    void (*predict_ptr)(cbify&, single_learner&, VW::example&);
    single_learner* base = as_singleline(stack_builder.setup_base_learner());
    if (cbify_opts->use_reg)
    {
      in_label_type = VW::label_type_t::SIMPLE;
      out_pred_type = VW::prediction_type_t::SCALAR;
      all.example_parser->lbl_parser = simple_label_parser_global;
      if (cbify_opts->use_discrete)
      {
        out_label_type = VW::label_type_t::CB;
        in_pred_type = VW::prediction_type_t::ACTION_PROBS;
        learn_ptr = predict_or_learn_regression_discrete<true>;
        predict_ptr = predict_or_learn_regression_discrete<false>;
        finish_ptr = finish_example_cb_reg_discrete;
        name_addition = "-reg-discrete";
      }
      else
      {
        out_label_type = VW::label_type_t::CONTINUOUS;
        in_pred_type = VW::prediction_type_t::ACTION_PDF_VALUE;
        learn_ptr = predict_or_learn_regression<true>;
        predict_ptr = predict_or_learn_regression<false>;
        finish_ptr = finish_example_cb_reg_continous;
        name_addition = "-reg";
      }
    }
    else if (cbify_opts->use_cs)
    {
      in_label_type = VW::label_type_t::CS;
      out_label_type = VW::label_type_t::CB;
      in_pred_type = VW::prediction_type_t::ACTION_PROBS;
      out_pred_type = VW::prediction_type_t::MULTICLASS;
      learn_ptr = predict_or_learn<true, true>;
      predict_ptr = predict_or_learn<false, true>;
      finish_ptr = VW::details::finish_cs_example;
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
      finish_ptr = VW::details::finish_multiclass_example<cbify&>;
      name_addition = "";
      all.example_parser->lbl_parser = VW::multiclass_label_parser_global;
    }
    l = make_reduction_learner(
        std::move(cbify_data), base, learn_ptr, predict_ptr, stack_builder.get_setupfn_name(cbify_setup) + name_addition)
            .set_input_label_type(in_label_type)
            .set_output_label_type(out_label_type)
            .set_input_prediction_type(in_pred_type)
            .set_output_prediction_type(out_pred_type)
            .set_learn_returns_prediction(true)
            .set_finish_example(finish_ptr)
            .build(&all.logger);
  }

  return make_base(*l);
}

struct options_cbifyldf_v1
{
  bool cbify_ldf_option = false;
  float loss0;
  float loss1;
};

std::unique_ptr<options_cbifyldf_v1> get_cbifyldf_options_instance(
    const VW::workspace&, VW::io::logger&, options_i& options)
{
  auto cbifyldf_opts = VW::make_unique<options_cbifyldf_v1>();
  option_group_definition new_options("[Reduction] Make csoaa_ldf into Contextual Bandit");
  new_options
      .add(make_option("cbify_ldf", cbifyldf_opts->cbify_ldf_option)
               .keep()
               .necessary()
               .help("Convert csoaa_ldf into a contextual bandit problem"))
      .add(make_option("loss0", cbifyldf_opts->loss0).default_value(0.f).help("Loss for correct label"))
      .add(make_option("loss1", cbifyldf_opts->loss1).default_value(1.f).help("Loss for incorrect label"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (!options.was_supplied("cb_explore_adf")) { options.insert("cb_explore_adf", ""); }
  options.insert("cb_min_cost", std::to_string(cbifyldf_opts->loss0));
  options.insert("cb_max_cost", std::to_string(cbifyldf_opts->loss1));

  if (options.was_supplied("baseline"))
  {
    std::stringstream ss;
    ss << std::max(std::abs(cbifyldf_opts->loss0), std::abs(cbifyldf_opts->loss1)) / (cbifyldf_opts->loss1 - cbifyldf_opts->loss0);
    options.insert("lr_multiplier", ss.str());
  }

  return cbifyldf_opts;
}

VW::LEARNER::base_learner* VW::reductions::cbifyldf_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto cbifyldf_opts = get_cbifyldf_options_instance(all, all.logger, *stack_builder.get_options());
  if (cbifyldf_opts == nullptr) { return nullptr; }
  auto cbifyldf_data = VW::make_unique<cbify>();

  cbifyldf_data->loss0 = cbifyldf_opts->loss0;
  cbifyldf_data->loss1 = cbifyldf_opts->loss1;
  cbifyldf_data->app_seed = VW::uniform_hash("vw", 2, 0);
  cbifyldf_data->all = &all;
  cbifyldf_data->use_adf = true;

  multi_learner* base = as_multiline(stack_builder.setup_base_learner());
  auto* l = make_reduction_learner(std::move(cbifyldf_data), base, do_actual_learning_ldf, do_actual_predict_ldf,
      stack_builder.get_setupfn_name(cbifyldf_setup))
                .set_input_label_type(VW::label_type_t::CS)
                .set_output_label_type(VW::label_type_t::CB)
                .set_input_prediction_type(VW::prediction_type_t::ACTION_PROBS)
                .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
                .set_finish_example(finish_multiline_example)
                .build(&all.logger);
  all.example_parser->lbl_parser = VW::cs_label_parser_global;

  return make_base(*l);
}
