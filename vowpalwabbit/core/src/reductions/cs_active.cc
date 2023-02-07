// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cs_active.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/debug_log.h"
#include "vw/core/example.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/named_labels.h"
#include "vw/core/reductions/csoaa.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <limits>

// #define B_SEARCH_MAX_ITER 50
#define B_SEARCH_MAX_ITER 20

using namespace VW::LEARNER;
using namespace VW::config;

using std::endl;

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::CS_ACTIVE

namespace
{
class lq_data
{
public:
  // The following are used by cost-sensitive active learning
  float max_pred;            // The max cost for this label predicted by the current set of good regressors
  float min_pred;            // The min cost for this label predicted by the current set of good regressors
  bool is_range_large;       // Indicator of whether this label's cost range was large
  bool is_range_overlapped;  // Indicator of whether this label's cost range overlaps with the cost range that has the
                             // minimum max_pred
  bool query_needed;         // Used in reduction mode: tell upper-layer whether a query is needed for this label
  VW::cs_class* cl;
};

class cs_active
{
public:
  // active learning algorithm parameters
  float c0 = 0.f;        // mellowness controlling the width of the set of good functions
  float c1 = 0.f;        // multiplier on the threshold for the cost range test
  float cost_max = 0.f;  // max cost
  float cost_min = 0.f;  // min cost

  uint32_t num_classes = 0;
  size_t t = 0;

  bool print_debug_stuff = false;
  size_t min_labels = 0;
  size_t max_labels = 0;

  bool is_baseline = false;
  bool use_domination = false;

  VW::workspace* all = nullptr;  // statistics, loss
  VW::LEARNER::learner* l = nullptr;

  VW::v_array<lq_data> query_data;

  size_t num_any_queries = 0;  // examples where at least one label is queried
  size_t overlapped_and_range_small = 0;
  VW::v_array<size_t> examples_by_queries;
  size_t labels_outside_range = 0;
  float distance_to_range = 0.f;
  float range = 0.f;
};

float binary_search(float fhat, float delta, float sens, float tol)
{
  float maxw = std::min(fhat / sens, FLT_MAX);

  if (maxw * fhat * fhat <= delta) { return maxw; }

  float l = 0, u = maxw, w, v;

  for (int iter = 0; iter < B_SEARCH_MAX_ITER; iter++)
  {
    w = (u + l) / 2.f;
    v = w * (fhat * fhat - (fhat - sens * w) * (fhat - sens * w)) - delta;
    if (v > 0) { u = w; }
    else { l = w; }
    if (std::fabs(v) <= tol || u - l <= tol) { break; }
  }

  return l;
}

template <bool is_learn, bool is_simulation>
inline void inner_loop(cs_active& cs_a, learner& base, VW::example& ec, uint32_t i, float cost, uint32_t& prediction,
    float& score, float& partial_prediction, bool query_this_label, bool& query_needed)
{
  base.predict(ec, i - 1);
  if (is_learn)
  {
    VW::workspace& all = *cs_a.all;
    ec.weight = 1.;
    if (is_simulation)
    {
      // In simulation mode
      if (query_this_label)
      {
        ec.l.simple.label = cost;
        all.sd->queries += 1;
      }
      else { ec.l.simple.label = FLT_MAX; }
    }
    else
    {
      // In reduction mode.
      // If the cost of this label was previously queried, then it should be available for learning now.
      // If the cost of this label was not queried, then skip it.
      if (query_needed)
      {
        ec.l.simple.label = cost;
        if ((cost < cs_a.cost_min) || (cost > cs_a.cost_max))
        {
          cs_a.all->logger.err_warn("Cost {0} outside of cost range[{1}, {2}]", cost, cs_a.cost_min, cs_a.cost_max);
        }
      }
      else { ec.l.simple.label = FLT_MAX; }
    }

    if (ec.l.simple.label != FLT_MAX) { base.learn(ec, i - 1); }
  }
  else if (!is_simulation)
  {
    // Prediction in reduction mode could be used by upper layer to ask whether this label needs to be queried.
    // So we return that.
    query_needed = query_this_label;
  }

  partial_prediction = ec.partial_prediction;
  if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction))
  {
    score = ec.partial_prediction;
    prediction = i;
  }
  VW_ADD_PASSTHROUGH_FEATURE(ec, i, ec.partial_prediction);
}

inline void find_cost_range(cs_active& cs_a, learner& base, VW::example& ec, uint32_t i, float delta, float eta,
    float& min_pred, float& max_pred, bool& is_range_large)
{
  float tol = 1e-6f;

  base.predict(ec, i - 1);
  float sens = base.sensitivity(ec, i - 1);

  if (cs_a.t <= 1 || std::isnan(sens) || std::isinf(sens))
  {
    min_pred = cs_a.cost_min;
    max_pred = cs_a.cost_max;
    is_range_large = true;
    if (cs_a.print_debug_stuff)
    {
      cs_a.all->logger.err_info("find_cost_rangeA: i={0} pp={1} sens={2} eta={3} [{4}, {5}] = {6}", i,
          ec.partial_prediction, sens, eta, min_pred, max_pred, (max_pred - min_pred));
    }
  }
  else
  {
    // finding max_pred and min_pred by binary search
    max_pred = std::min(
        ec.pred.scalar + sens * binary_search(cs_a.cost_max - ec.pred.scalar, delta, sens, tol), cs_a.cost_max);
    min_pred = std::max(
        ec.pred.scalar - sens * binary_search(ec.pred.scalar - cs_a.cost_min, delta, sens, tol), cs_a.cost_min);
    is_range_large = (max_pred - min_pred > eta);
    if (cs_a.print_debug_stuff)
    {
      cs_a.all->logger.err_info("find_cost_rangeB: i={0} pp={1} sens={2} eta={3} [{4}, {5}] = {6}", i,
          ec.partial_prediction, sens, eta, min_pred, max_pred, (max_pred - min_pred));
    }
  }
}

template <bool is_learn, bool is_simulation>
void predict_or_learn(cs_active& cs_a, learner& base, VW::example& ec)
{
  VW::cs_label ld = ec.l.cs;

  if (cs_a.all->sd->queries >= cs_a.min_labels * cs_a.num_classes)
  {
    // save regressor
    std::stringstream filename;
    filename << cs_a.all->final_regressor_name << "." << ec.example_counter << "." << cs_a.all->sd->queries << "."
             << cs_a.num_any_queries;
    VW::save_predictor(*(cs_a.all), filename.str());
    *(cs_a.all->trace_message) << endl << "Number of examples with at least one query = " << cs_a.num_any_queries;
    // Double label query budget
    cs_a.min_labels *= 2;

    for (size_t i = 0; i < cs_a.examples_by_queries.size(); i++)
    {
      *(cs_a.all->trace_message) << endl
                                 << "examples with " << i << " labels queried = " << cs_a.examples_by_queries[i];
    }

    *(cs_a.all->trace_message) << endl << "labels outside of cost range = " << cs_a.labels_outside_range;
    *(cs_a.all->trace_message) << endl
                               << "average distance to range = "
                               << cs_a.distance_to_range / (static_cast<float>(cs_a.labels_outside_range));
    *(cs_a.all->trace_message) << endl
                               << "average range = " << cs_a.range / (static_cast<float>(cs_a.labels_outside_range));
  }

  if (cs_a.all->sd->queries >= cs_a.max_labels * cs_a.num_classes) { return; }

  uint32_t prediction = 1;
  float score = FLT_MAX;
  ec.l.simple = {0.f};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

  float min_max_cost = FLT_MAX;
  float t = static_cast<float>(cs_a.t);  // ec.example_t;  // current round
  float t_prev = t - 1.f;                // ec.weight; // last round

  float eta = cs_a.c1 * (cs_a.cost_max - cs_a.cost_min) / std::sqrt(t);  // threshold on cost range
  float delta = cs_a.c0 * std::log((cs_a.num_classes * std::max(t_prev, 1.f))) *
      static_cast<float>(std::pow(cs_a.cost_max - cs_a.cost_min, 2));  // threshold on empirical loss difference

  if (ld.costs.size() > 0)
  {
    // Create metadata structure
    for (VW::cs_class& cl : ld.costs)
    {
      lq_data f = {0.0, 0.0, 0, 0, 0, &cl};
      cs_a.query_data.push_back(f);
    }
    uint32_t n_overlapped = 0;
    for (lq_data& lqd : cs_a.query_data)
    {
      find_cost_range(cs_a, base, ec, lqd.cl->class_index, delta, eta, lqd.min_pred, lqd.max_pred, lqd.is_range_large);
      min_max_cost = std::min(min_max_cost, lqd.max_pred);
    }
    for (lq_data& lqd : cs_a.query_data)
    {
      lqd.is_range_overlapped = (lqd.min_pred <= min_max_cost);
      n_overlapped += static_cast<uint32_t>(lqd.is_range_overlapped);
      cs_a.overlapped_and_range_small += static_cast<size_t>(lqd.is_range_overlapped && !lqd.is_range_large);
      if (lqd.cl->x > lqd.max_pred || lqd.cl->x < lqd.min_pred)
      {
        cs_a.labels_outside_range++;
        cs_a.distance_to_range += std::max(lqd.cl->x - lqd.max_pred, lqd.min_pred - lqd.cl->x);
        cs_a.range += lqd.max_pred - lqd.min_pred;
      }
    }

    bool query = (n_overlapped > 1);
    size_t queries = cs_a.all->sd->queries;
    for (lq_data& lqd : cs_a.query_data)
    {
      bool query_label = ((query && cs_a.is_baseline) || (!cs_a.use_domination && lqd.is_range_large) ||
          (query && lqd.is_range_overlapped && lqd.is_range_large));
      inner_loop<is_learn, is_simulation>(cs_a, base, ec, lqd.cl->class_index, lqd.cl->x, prediction, score,
          lqd.cl->partial_prediction, query_label, lqd.query_needed);
      if (lqd.query_needed) { ec.pred.active_multiclass.more_info_required_for_classes.push_back(lqd.cl->class_index); }
      if (cs_a.print_debug_stuff)
      {
        cs_a.all->logger.err_info(
            "label={0} x={1} prediction={2} score={3} pp={4} ql={5} qn={6} ro={7} rl={8} "
            "[{9}, {10}] vs delta={11} n_overlapped={12} is_baseline={13}",
            lqd.cl->class_index /*0*/, lqd.cl->x /*1*/, prediction /*2*/, score /*3*/, lqd.cl->partial_prediction /*4*/,
            query_label /*5*/, lqd.query_needed /*6*/, lqd.is_range_overlapped /*7*/, lqd.is_range_large /*8*/,
            lqd.min_pred /*9*/, lqd.max_pred /*10*/, delta /*11*/, n_overlapped /*12*/, cs_a.is_baseline /*13*/);
      }
    }

    // Need to pop metadata
    cs_a.query_data.clear();

    if (cs_a.all->sd->queries - queries > 0) { cs_a.num_any_queries++; }

    cs_a.examples_by_queries[cs_a.all->sd->queries - queries] += 1;

    ec.partial_prediction = score;
    if (is_learn) { cs_a.t++; }
  }
  else
  {
    float temp = 0.f;
    bool temp2 = false, temp3 = false;
    for (uint32_t i = 1; i <= cs_a.num_classes; i++)
    {
      inner_loop<false, is_simulation>(cs_a, base, ec, i, FLT_MAX, prediction, score, temp, temp2, temp3);
    }
  }

  ec.pred.active_multiclass.predicted_class = prediction;
  ec.l.cs = ld;
}

void update_stats_cs_active(const VW::workspace& /* all */, VW::shared_data& sd, const cs_active& /* data */,
    const VW::example& ec, VW::io::logger& logger)
{
  const auto& label = ec.l.cs;
  const auto multiclass_prediction = ec.pred.active_multiclass.predicted_class;
  float loss = 0.;
  if (!label.is_test_label())
  {
    // need to compute exact loss
    auto pred = static_cast<size_t>(multiclass_prediction);

    float chosen_loss = FLT_MAX;
    float min = FLT_MAX;
    for (const auto& cl : label.costs)
    {
      if (cl.class_index == pred) { chosen_loss = cl.x; }
      if (cl.x < min) { min = cl.x; }
    }
    if (chosen_loss == FLT_MAX)
    {
      logger.err_warn("csoaa predicted an invalid class. Are all multi-class labels in the {{1..k}} range?");
    }

    loss = (chosen_loss - min) * ec.weight;
    // TODO(alberto): add option somewhere to allow using absolute loss instead?
    // loss = chosen_loss;
  }

  sd.update(ec.test_only, !label.is_test_label(), loss, ec.weight, ec.get_num_features());
}

void output_example_prediction_cs_active(
    VW::workspace& all, const cs_active& /* data */, const VW::example& ec, VW::io::logger& /* unused */)
{
  const auto& label = ec.l.cs;
  const auto multiclass_prediction = ec.pred.active_multiclass.predicted_class;

  for (auto& sink : all.final_prediction_sink)
  {
    if (!all.sd->ldict)
    {
      all.print_by_ref(sink.get(), static_cast<float>(multiclass_prediction), 0, ec.tag, all.logger);
    }
    else
    {
      VW::string_view sv_pred = all.sd->ldict->get(multiclass_prediction);
      all.print_text_by_ref(sink.get(), std::string{sv_pred}, ec.tag, all.logger);
    }
  }

  if (all.raw_prediction != nullptr)
  {
    std::stringstream output_string_stream;
    for (unsigned int i = 0; i < label.costs.size(); i++)
    {
      const auto& cl = label.costs[i];
      if (i > 0) { output_string_stream << ' '; }
      output_string_stream << cl.class_index << ':' << cl.partial_prediction;
    }
    all.print_text_by_ref(all.raw_prediction.get(), output_string_stream.str(), ec.tag, all.logger);
  }
}

void print_update_cs_active(VW::workspace& all, VW::shared_data& /* sd */, const cs_active& /* data */,
    const VW::example& ec, VW::io::logger& /* unused */)
{
  const auto& label = ec.l.cs;
  const auto multiclass_prediction = ec.pred.active_multiclass.predicted_class;

  VW::details::print_cs_update(all, label.is_test_label(), ec, nullptr, false, multiclass_prediction);
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cs_active_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<cs_active>();

  bool simulation = false;
  int32_t domination;
  uint64_t max_labels;
  uint64_t min_labels;
  option_group_definition new_options("[Reduction] Cost Sensitive Active Learning");
  new_options
      .add(make_option("cs_active", data->num_classes)
               .keep()
               .necessary()
               .help("Cost-sensitive active learning with <k> costs"))
      .add(make_option("simulation", simulation).help("Cost-sensitive active learning simulation mode"))
      .add(make_option("baseline", data->is_baseline).help("Cost-sensitive active learning baseline"))
      .add(make_option("domination", domination).default_value(1).help("Cost-sensitive active learning use domination"))
      .add(make_option("mellowness", data->c0).keep().default_value(0.1f).help("Mellowness parameter c_0"))
      .add(make_option("range_c", data->c1)
               .default_value(0.5f)
               .help("Parameter controlling the threshold for per-label cost uncertainty"))
      .add(make_option("max_labels", max_labels)
               .default_value(std::numeric_limits<uint64_t>::max())
               .help("Maximum number of label queries"))
      .add(make_option("min_labels", min_labels)
               .default_value(std::numeric_limits<uint64_t>::max())
               .help("Minimum number of label queries"))
      .add(make_option("cost_max", data->cost_max).default_value(1.f).help("Cost upper bound"))
      .add(make_option("cost_min", data->cost_min).default_value(0.f).help("Cost lower bound"))
      // TODO replace with trace and quiet
      .add(make_option("csa_debug", data->print_debug_stuff).help("Print debug stuff for cs_active"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  data->max_labels =
      max_labels == std::numeric_limits<uint64_t>::max() ? std::numeric_limits<size_t>::max() : max_labels;
  data->min_labels =
      min_labels == std::numeric_limits<uint64_t>::max() ? std::numeric_limits<size_t>::max() : min_labels;
  data->use_domination = true;
  if (options.was_supplied("domination") && !domination) { data->use_domination = false; }

  data->all = &all;
  data->t = 1;

  auto loss_function_type = all.loss->get_type();
  if (loss_function_type != "squared") THROW("non-squared loss can't be used with --cs_active");

  if (options.was_supplied("lda")) THROW("lda can't be combined with active learning");

  if (options.was_supplied("active")) THROW("--cs_active can't be used with --active");

  if (options.was_supplied("active_cover")) THROW("--cs_active can't be used with --active_cover");

  if (options.was_supplied("csoaa")) THROW("--cs_active can't be used with --csoaa");

  if (!options.was_supplied("adax")) { all.logger.err_warn("--cs_active should be used with --adax"); }

  if (all.set_minmax)
  {
    all.set_minmax(data->cost_max);
    all.set_minmax(data->cost_min);
  }

  for (uint32_t i = 0; i < data->num_classes + 1; i++) { data->examples_by_queries.push_back(0); }

  void (*learn_ptr)(cs_active & cs_a, learner & base, VW::example & ec);
  void (*predict_ptr)(cs_active & cs_a, learner & base, VW::example & ec);
  std::string name_addition;

  if (simulation)
  {
    learn_ptr = predict_or_learn<true, true>;
    predict_ptr = predict_or_learn<false, true>;
    name_addition = "-sim";
  }
  else
  {
    learn_ptr = predict_or_learn<true, false>;
    predict_ptr = predict_or_learn<false, false>;
    name_addition = "";
  }

  size_t ws = data->num_classes;
  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()), learn_ptr,
      predict_ptr, stack_builder.get_setupfn_name(cs_active_setup) + name_addition)
               .set_params_per_weight(ws)
               .set_learn_returns_prediction(true)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::ACTIVE_MULTICLASS)
               .set_input_label_type(VW::label_type_t::CS)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_output_example_prediction(output_example_prediction_cs_active)
               .set_print_update(print_update_cs_active)
               .set_update_stats(update_stats_cs_active)
               .build();

  // Label parser set to cost sensitive label parser
  all.example_parser->lbl_parser = VW::cs_label_parser_global;

  return l;
}
