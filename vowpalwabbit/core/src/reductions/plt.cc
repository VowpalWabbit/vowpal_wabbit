// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/plt.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class node
{
public:
  uint32_t n;  // node number
  float p;     // node probability

  bool operator<(const node& r) const { return p < r.p; }
};

class plt
{
public:
  VW::workspace* all = nullptr;

  // tree structure
  uint32_t k = 0;     // number of labels
  uint32_t t = 0;     // number of tree nodes
  uint32_t ti = 0;    // number of internal nodes
  uint32_t kary = 0;  // kary tree

  // for training
  VW::v_array<float> nodes_time;                // in case of sgd, this stores individual t for each node
  std::unordered_set<uint32_t> positive_nodes;  // container for positive nodes
  std::unordered_set<uint32_t> negative_nodes;  // container for negative nodes

  // for prediction
  float threshold = 0.f;
  uint32_t top_k = 0;
  std::vector<VW::polyprediction> node_preds;  // for storing results of base.multipredict
  std::vector<node> node_queue;                // container for queue used for both types of predictions

  // for measuring predictive performance
  std::unordered_set<uint32_t> true_labels;
  VW::v_array<uint32_t> tp_at;  // true positives at (for precision and recall at)
  uint32_t tp = 0;
  uint32_t fp = 0;
  uint32_t fn = 0;
  uint32_t true_count = 0;  // number of all true labels (for recall at)
  uint32_t ec_count = 0;    // number of examples

  plt()
  {
    tp = 0;
    fp = 0;
    fn = 0;
    ec_count = 0;
    true_count = 0;
  }
};

inline void learn_node(plt& p, uint32_t n, single_learner& base, VW::example& ec)
{
  if (!p.all->weights.adaptive)
  {
    p.all->sd->t = p.nodes_time[n];
    p.nodes_time[n] += ec.weight;
  }
  base.learn(ec, n);
}

void learn(plt& p, single_learner& base, VW::example& ec)
{
  MULTILABEL::labels multilabels = std::move(ec.l.multilabels);
  MULTILABEL::labels preds = std::move(ec.pred.multilabels);

  double t = p.all->sd->t;
  double weighted_holdout_examples = p.all->sd->weighted_holdout_examples;
  p.all->sd->weighted_holdout_examples = 0;

  p.positive_nodes.clear();
  p.negative_nodes.clear();

  if (multilabels.label_v.size() > 0)
  {
    for (auto label : multilabels.label_v)
    {
      uint32_t tn = label + p.ti;
      if (tn < p.t)
      {
        p.positive_nodes.insert(tn);
        while (tn > 0)
        {
          tn = static_cast<uint32_t>(std::floor(static_cast<float>(tn - 1) / p.kary));
          p.positive_nodes.insert(tn);
        }
      }
    }
    if (multilabels.label_v.back() >= p.k)
    {
      p.all->logger.out_error(
          "label {0} is not in {{0,{1}}} This won't work right.", multilabels.label_v.back(), p.k - 1);
    }

    for (auto& n : p.positive_nodes)
    {
      if (n < p.ti)
      {
        for (uint32_t i = 1; i <= p.kary; ++i)
        {
          uint32_t n_child = p.kary * n + i;
          if (n_child < p.t && p.positive_nodes.find(n_child) == p.positive_nodes.end())
          {
            p.negative_nodes.insert(n_child);
          }
        }
      }
    }
  }
  else { p.negative_nodes.insert(0); }

  ec.l.simple = {1.f};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
  for (auto& n : p.positive_nodes) { learn_node(p, n, base, ec); }

  ec.l.simple.label = -1.f;
  for (auto& n : p.negative_nodes) { learn_node(p, n, base, ec); }

  p.all->sd->t = t;
  p.all->sd->weighted_holdout_examples = weighted_holdout_examples;

  ec.pred.multilabels = std::move(preds);
  ec.l.multilabels = std::move(multilabels);
}

inline float predict_node(uint32_t n, single_learner& base, VW::example& ec)
{
  ec.l.simple = {FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
  base.predict(ec, n);
  return 1.0f / (1.0f + std::exp(-ec.partial_prediction));
}

template <bool threshold>
void predict(plt& p, single_learner& base, VW::example& ec)
{
  MULTILABEL::labels multilabels = std::move(ec.l.multilabels);
  MULTILABEL::labels preds = std::move(ec.pred.multilabels);
  preds.label_v.clear();

  // split labels into true and skip (those > max. label num)
  p.true_labels.clear();
  for (auto label : multilabels.label_v)
  {
    if (label < p.k) { p.true_labels.insert(label); }
    else { p.all->logger.out_error("label {0} is not in {{0,{1}}} This won't work right.", label, p.k - 1); }
  }

  p.node_queue.clear();  // clear node queue

  // prediction with threshold
  if (threshold)
  {
    float cp_root = predict_node(0, base, ec);
    if (cp_root > p.threshold)
    {
      p.node_queue.push_back({0, cp_root});  // here queue is used for dfs search
    }

    while (!p.node_queue.empty())
    {
      node node = p.node_queue.back();  // current node
      p.node_queue.pop_back();

      uint32_t n_child = p.kary * node.n + 1;
      ec.l.simple = {FLT_MAX};
      ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
      base.multipredict(ec, n_child, p.kary, p.node_preds.data(), false);

      for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
      {
        float cp_child = node.p * (1.f / (1.f + std::exp(-p.node_preds[i].scalar)));
        if (cp_child > p.threshold)
        {
          if (n_child < p.ti) { p.node_queue.push_back({n_child, cp_child}); }
          else
          {
            uint32_t l = n_child - p.ti;
            preds.label_v.push_back(l);
          }
        }
      }
    }

    if (p.true_labels.size() > 0)
    {
      uint32_t tp = 0;
      for (auto pred_label : preds.label_v)
      {
        if (p.true_labels.count(pred_label)) { ++tp; }
      }
      p.tp += tp;
      p.fp += static_cast<uint32_t>(preds.label_v.size()) - tp;
      p.fn += static_cast<uint32_t>(p.true_labels.size()) - tp;
      ++p.ec_count;
    }
  }

  // top-k prediction
  else
  {
    p.node_queue.push_back({0, predict_node(0, base, ec)});  // here queue is used as priority queue
    std::push_heap(p.node_queue.begin(), p.node_queue.end());

    while (!p.node_queue.empty())
    {
      std::pop_heap(p.node_queue.begin(), p.node_queue.end());
      node node = p.node_queue.back();
      p.node_queue.pop_back();

      if (node.n < p.ti)
      {
        uint32_t n_child = p.kary * node.n + 1;
        ec.l.simple = {FLT_MAX};
        ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

        base.multipredict(ec, n_child, p.kary, p.node_preds.data(), false);

        for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
        {
          float cp_child = node.p * (1.0f / (1.0f + std::exp(-p.node_preds[i].scalar)));
          p.node_queue.push_back({n_child, cp_child});
          std::push_heap(p.node_queue.begin(), p.node_queue.end());
        }
      }
      else
      {
        uint32_t l = node.n - p.ti;
        preds.label_v.push_back(l);
        if (preds.label_v.size() >= p.top_k) { break; }
      }
    }

    // calculate p@
    if (p.true_labels.size() > 0)
    {
      for (size_t i = 0; i < p.top_k; ++i)
      {
        if (p.true_labels.count(preds.label_v[i])) { ++p.tp_at[i]; }
      }
      ++p.ec_count;
      p.true_count += static_cast<uint32_t>(p.true_labels.size());
    }
  }

  p.node_queue.clear();

  ec.pred.multilabels = std::move(preds);
  ec.l.multilabels = std::move(multilabels);
}

void finish_example(VW::workspace& all, plt& /*p*/, VW::example& ec)
{
  MULTILABEL::output_example(all, ec);
  VW::finish_example(all, ec);
}

void finish(plt& p)
{
  // print results in test mode
  if (!p.all->training && p.ec_count > 0)
  {
    // top-k predictions
    if (p.top_k > 0)
    {
      double correct = 0;
      for (size_t i = 0; i < p.top_k; ++i)
      {
        correct += p.tp_at[i];
        // TODO: is this the correct logger?
        *(p.all->trace_message) << "p@" << i + 1 << " = " << correct / (p.ec_count * (i + 1)) << std::endl;
        *(p.all->trace_message) << "r@" << i + 1 << " = " << correct / p.true_count << std::endl;
      }
    }

    else if (p.threshold > 0)
    {
      // TODO: is this the correct logger?
      *(p.all->trace_message) << "hamming loss = " << static_cast<double>(p.fp + p.fn) / p.ec_count << std::endl;
      *(p.all->trace_message) << "precision = " << static_cast<double>(p.tp) / (p.tp + p.fp) << std::endl;
      *(p.all->trace_message) << "recall = " << static_cast<double>(p.tp) / (p.tp + p.fn) << std::endl;
    }
  }
}

void save_load_tree(plt& p, io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() > 0)
  {
    bool resume = p.all->save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&resume), sizeof(resume), read, msg, text);

    if (resume && !p.all->weights.adaptive)
    {
      for (size_t i = 0; i < p.t; ++i)
      {
        bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.nodes_time[i]), sizeof(p.nodes_time[0]), read, msg, text);
      }
    }
  }
}

struct options_plt_v1
{
  uint32_t k;
  uint32_t kary;
  float threshold;
  uint32_t top_k;
};

std::unique_ptr<options_plt_v1> get_plt_options_instance(const VW::workspace&, VW::io::logger&, options_i& options)
{
  auto plt_opts = VW::make_unique<options_plt_v1>();
  option_group_definition new_options("[Reduction] Probabilistic Label Tree");
  new_options.add(make_option("plt", plt_opts->k).keep().necessary().help("Probabilistic Label Tree with <k> labels"))
      .add(make_option("kary_tree", plt_opts->kary).keep().default_value(2).help("Use <k>-ary tree"))
      .add(make_option("threshold", plt_opts->threshold)
               .default_value(0.5)
               .help("Predict labels with conditional marginal probability greater than <thr> threshold"))
      .add(make_option("top_k", plt_opts->top_k)
               .default_value(0)
               .help("Predict top-<k> labels instead of labels above threshold"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }
  return plt_opts;
}
}  // namespace

base_learner* VW::reductions::plt_setup(VW::setup_base_i& stack_builder)
{
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto plt_opts = get_plt_options_instance(all, all.logger, *stack_builder.get_options());
  if (plt_opts == nullptr) { return nullptr; }
  auto plt_data = VW::make_unique<plt>();

  plt_data->k = plt_opts->k;
  plt_data->kary = plt_opts->kary;
  plt_data->threshold = plt_opts->threshold;
  plt_data->top_k = plt_opts->top_k;

  if (all.loss->get_type() != "logistic")
  {
    THROW("--plt requires --loss_function=logistic, but instead found: " << all.loss->get_type());
  }

  plt_data->all = &all;

  // calculate number of tree nodes
  const double a = std::pow(plt_data->kary, std::floor(std::log(plt_data->k) / std::log(plt_data->kary)));
  const double b = plt_data->k - a;
  const double c = std::ceil(b / (plt_data->kary - 1.0));
  const double d = (plt_data->kary * a - 1.0) / (plt_data->kary - 1.0);
  const double e = plt_data->k - (a - c);
  plt_data->t = static_cast<uint32_t>(e + d);
  plt_data->ti = plt_data->t - plt_data->k;

  if (!all.quiet)
  {
    *(all.trace_message) << "PLT k = " << plt_data->k << "\nkary_tree = " << plt_data->kary << std::endl;
    if (!all.training)
    {
      if (plt_data->top_k > 0) { *(all.trace_message) << "top_k = " << plt_data->top_k << std::endl; }
      else { *(all.trace_message) << "threshold = " << plt_data->threshold << std::endl; }
    }
  }

  // resize VW::v_arrays
  plt_data->nodes_time.resize_but_with_stl_behavior(plt_data->t);
  std::fill(plt_data->nodes_time.begin(), plt_data->nodes_time.end(), all.initial_t);
  plt_data->node_preds.resize(plt_data->kary);
  if (plt_data->top_k > 0) { plt_data->tp_at.resize_but_with_stl_behavior(plt_data->top_k); }

  size_t ws = plt_data->t;
  std::string name_addition;
  void (*pred_ptr)(plt&, single_learner&, VW::example&);

  if (plt_data->top_k > 0)
  {
    name_addition = "-top_k";
    pred_ptr = predict<false>;
  }
  else
  {
    name_addition = "";
    pred_ptr = predict<true>;
  }

  auto* l = make_reduction_learner(std::move(plt_data), as_singleline(stack_builder.setup_base_learner()), learn,
      pred_ptr, stack_builder.get_setupfn_name(plt_setup) + name_addition)
                .set_params_per_weight(ws)
                .set_output_prediction_type(VW::prediction_type_t::MULTILABELS)
                .set_input_label_type(VW::label_type_t::MULTILABEL)
                .set_learn_returns_prediction(true)
                .set_finish_example(::finish_example)
                .set_finish(::finish)
                .set_save_load(save_load_tree)
                .build();

  all.example_parser->lbl_parser = MULTILABEL::multilabel;

  return make_base(*l);
}
