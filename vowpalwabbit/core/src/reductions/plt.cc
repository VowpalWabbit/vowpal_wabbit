// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/plt.h"

#include "vw/config/options.h"
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
struct node
{
  uint32_t n;  // node number
  float p;     // node probability

  bool operator<(const node& r) const { return p < r.p; }
};

struct plt
{
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
  std::vector<VW::polyprediction> node_pred;  // for storing results of base.multipredict
  std::vector<node> node_queue;               // container for queue used for both types of predictions
  bool probabilities = false;

  // for measuring predictive performance
  std::unordered_set<uint32_t> true_labels;
  VW::v_array<float> p_at;  // precision at
  VW::v_array<float> r_at;  // recall at
  uint32_t tp = 0;          // true positives
  uint32_t fp = 0;          // false positives
  uint32_t fn = 0;          // false negatives
  uint32_t ec_count = 0;    // number of examples

  plt()
  {
    tp = 0;
    fp = 0;
    fn = 0;
    ec_count = 0;
  }
};

inline float learn_node(plt& p, uint32_t n, single_learner& base, VW::example& ec)
{
  p.all->sd->t = p.nodes_time[n];
  p.nodes_time[n] += ec.weight;
  base.learn(ec, n);
  return ec.loss;
}

void learn(plt& p, single_learner& base, VW::example& ec)
{
  MULTILABEL::labels multilabels = std::move(ec.l.multilabels);
  VW::polyprediction pred = std::move(ec.pred);

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

  float loss = 0;
  ec.l.simple = {1.f};
  ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();
  for (auto& n : p.positive_nodes) { loss += learn_node(p, n, base, ec); }

  ec.l.simple.label = -1.f;
  for (auto& n : p.negative_nodes) { loss += learn_node(p, n, base, ec); }

  p.all->sd->t = t;
  p.all->sd->weighted_holdout_examples = weighted_holdout_examples;

  ec.loss = loss;
  ec.pred = std::move(pred);
  ec.l.multilabels = std::move(multilabels);
}

inline float sigmoid(float x) { return 1.0f / (1.0f + std::exp(-x)); }

inline float predict_node(uint32_t n, single_learner& base, VW::example& ec)
{
  ec.l.simple = {FLT_MAX};
  ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();
  base.predict(ec, n);
  return sigmoid(ec.partial_prediction);
}

template <bool threshold>
void predict(plt& p, single_learner& base, VW::example& ec)
{
  MULTILABEL::labels multilabels = std::move(ec.l.multilabels);
  VW::polyprediction pred = std::move(ec.pred);

  if (p.probabilities) { pred.a_s.clear(); }
  else { pred.multilabels.label_v.clear(); }

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
      ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();
      base.multipredict(ec, n_child, p.kary, p.node_pred.data(), false);

      for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
      {
        float cp_child = node.p * sigmoid(p.node_pred[i].scalar);
        if (cp_child > p.threshold)
        {
          if (n_child < p.ti) { p.node_queue.push_back({n_child, cp_child}); }
          else
          {
            uint32_t l = n_child - p.ti;
            if (p.probabilities) { ec.pred.a_s.push_back({l, cp_child}); }
            else { pred.multilabels.label_v.push_back(l); }
          }
        }
      }
    }

    // calculate evaluation measures
    if (p.true_labels.size() > 0)
    {
      uint32_t tp = 0;
      uint32_t pred_size = pred.multilabels.label_v.size();
      if (p.probabilities) { pred_size = pred.a_s.size(); }

      for (uint32_t i = 0; i < pred_size; ++i)
      {
        uint32_t pred_label;
        if (p.probabilities) { pred_label = pred.a_s[i].action; }
        else { pred_label = pred.multilabels.label_v[i]; }
        if (p.true_labels.count(pred_label)) { ++tp; }
      }
      p.tp += tp;
      p.fp += static_cast<uint32_t>(pred_size) - tp;
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
        ec._reduction_features.template get<simple_label_reduction_features>().reset_to_default();

        base.multipredict(ec, n_child, p.kary, p.node_pred.data(), false);

        for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
        {
          float cp_child = node.p * sigmoid(p.node_pred[i].scalar);
          p.node_queue.push_back({n_child, cp_child});
          std::push_heap(p.node_queue.begin(), p.node_queue.end());
        }
      }
      else
      {

        uint32_t l = node.n - p.ti;
        if (p.probabilities)
        {
          pred.a_s.push_back({l, node.p});
          if (pred.a_s.size() >= p.top_k) { break; }
        }
        else
        {
          pred.multilabels.label_v.push_back(l);
          if (pred.multilabels.label_v.size() >= p.top_k) { break; }
        }
      }
    }

    // calculate precision and recall at
    if (p.true_labels.size() > 0)
    {
      float tp_at = 0;
      for (size_t i = 0; i < p.top_k; ++i)
      {
        uint32_t pred_label;
        if (p.probabilities) { pred_label = pred.a_s[i].action; }
        else { pred_label = pred.multilabels.label_v[i]; }
        if (p.true_labels.count(pred_label)) { tp_at += 1; }
        p.p_at[i] += tp_at / (i + 1);
        p.r_at[i] += tp_at / p.true_labels.size();
      }
    }
  }

  ++p.ec_count;
  p.node_queue.clear();

  ec.pred = std::move(pred);
  ec.l.multilabels = std::move(multilabels);
}

void print_update_with_probabilities(VW::workspace& all, bool is_test, const VW::example& ec)
{
    if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
    {
        std::stringstream label_string;
        if (is_test) { label_string << "unknown"; }
        else
        {
            label_string << VW::to_string(ec.l.multilabels);
        }

        all.sd->print_update(*all.trace_message, all.holdout_set_off, all.current_pass, label_string.str(),
                             VW::to_string(ec.pred.a_s), ec.get_num_features(), all.progress_add, all.progress_arg);
    }
}

void output_example_with_probabilities(VW::workspace& all, const VW::example& ec)
{
    const auto& ld = ec.l.multilabels;

    float loss = 0.;
    if (ld.label_v.size() != 0)
    {
        // need to compute exact loss
        const ACTION_SCORE::action_scores& preds = ec.pred.a_s;
        const MULTILABEL::labels& given = ec.l.multilabels;

        uint32_t preds_index = 0;
        uint32_t given_index = 0;

        while (preds_index < preds.size() && given_index < given.label_v.size())
        {
            if (preds[preds_index].action < given.label_v[given_index])
            {
                preds_index++;
                loss++;
            }
            else if (preds[preds_index].action > given.label_v[given_index])
            {
                given_index++;
                loss++;
            }
            else
            {
                preds_index++;
                given_index++;
            }
        }
        loss += given.label_v.size() - given_index;
        loss += preds.size() - preds_index;
    }

    all.sd->update(ec.test_only, (ld.label_v.size() != 0), loss, 1.f, ec.get_num_features());

    for (auto& sink : all.final_prediction_sink)
    {
        if (sink != nullptr)
        {
            std::stringstream ss;

            for (size_t i = 0; i < ec.pred.a_s.size(); i++)
            {
                if (i > 0) { ss << ','; }
                ss << ec.pred.a_s[i];
            }
            ss << ' ';
            all.print_text_by_ref(sink.get(), ss.str(), ec.tag, all.logger);
        }
    }

    print_update_with_probabilities(all, (ld.label_v.size() == 0), ec);
}

void finish_example(VW::workspace& all, plt& p, VW::example& ec)
{
  if (p.probabilities)
  { // Since the input is multi-label and output is action-score, lets use custom output_example
      output_example_with_probabilities(all, ec);
  }
  else { MULTILABEL::output_example(all, ec); }
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
      for (size_t i = 0; i < p.top_k; ++i)
      {
        // TODO: is this the correct logger?
        *(p.all->trace_message) << "p@" << i + 1 << " = " << p.p_at[i] / p.ec_count << std::endl;
        *(p.all->trace_message) << "r@" << i + 1 << " = " << p.r_at[i] / p.ec_count << std::endl;
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
  std::stringstream msg;
  if (model_file.num_files() > 0)
  {
    for (size_t i = 0; i < p.t; ++i)
    {
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&p.nodes_time[i]), sizeof(p.nodes_time[0]), read, msg, text);
    }
  }
}
}  // namespace

base_learner* VW::reductions::plt_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto tree = VW::make_unique<plt>();
  option_group_definition new_options("[Reduction] Probabilistic Label Tree");
  new_options.add(make_option("plt", tree->k).keep().necessary().help("Probabilistic Label Tree with <k> labels"))
      .add(make_option("kary_tree", tree->kary).keep().default_value(2).help("Use <k>-ary tree"))
      .add(make_option("threshold", tree->threshold)
               .default_value(0.5)
               .help("Predict labels with conditional marginal probability greater than <thr> threshold"))
      .add(make_option("top_k", tree->top_k)
               .default_value(0)
               .help("Predict top-<k> labels instead of labels above threshold"))
      .add(make_option("probabilities", tree->probabilities).help("Predict probabilities for the predicted labels"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }


  if (all.loss->get_type() != "logistic")
  {
    THROW("--plt requires --loss_function=logistic, but instead found: " << all.loss->get_type());
  }

  tree->all = &all;

  // calculate number of tree nodes
  const double a = std::pow(tree->kary, std::floor(std::log(tree->k) / std::log(tree->kary)));
  const double b = tree->k - a;
  const double c = std::ceil(b / (tree->kary - 1.0));
  const double d = (tree->kary * a - 1.0) / (tree->kary - 1.0);
  const double e = tree->k - (a - c);
  tree->t = static_cast<uint32_t>(e + d);
  tree->ti = tree->t - tree->k;

  if (!all.quiet)
  {
    *(all.trace_message) << "PLT k = " << tree->k << "\nkary_tree = " << tree->kary << std::endl;
    if (!all.training)
    {
      if (tree->top_k > 0) { *(all.trace_message) << "top_k = " << tree->top_k << std::endl; }
      else { *(all.trace_message) << "threshold = " << tree->threshold << std::endl; }
    }
  }

  // resize VW::v_arrays
  tree->nodes_time.resize_but_with_stl_behavior(tree->t);
  std::fill(tree->nodes_time.begin(), tree->nodes_time.end(), all.initial_t);
  tree->node_pred.resize(tree->kary);
  if (tree->top_k > 0)
  {
    tree->p_at.resize_but_with_stl_behavior(tree->top_k);
    tree->r_at.resize_but_with_stl_behavior(tree->top_k);
  }

  size_t ws = tree->t;
  std::string name_addition = "";
  VW::prediction_type_t pred_type;
  void (*pred_ptr)(plt&, single_learner&, VW::example&);

  if (tree->top_k > 0)
  {
    name_addition = " -top_k";
    pred_ptr = predict<false>;
  }
  else { pred_ptr = predict<true>; }

  if (tree->probabilities)
  {
    name_addition += " -prob";
    pred_type = VW::prediction_type_t::action_probs;
  }
  else { pred_type = VW::prediction_type_t::multilabels; }

  auto* l = make_reduction_learner(std::move(tree), as_singleline(stack_builder.setup_base_learner()), learn, pred_ptr,
      stack_builder.get_setupfn_name(plt_setup) + name_addition)
                .set_params_per_weight(ws)
                .set_output_prediction_type(pred_type)
                .set_input_label_type(VW::label_type_t::multilabel)
                .set_learn_returns_prediction(false)
                .set_finish_example(::finish_example)
                .set_finish(::finish)
                .set_save_load(save_load_tree)
                .build();

  all.example_parser->lbl_parser = MULTILABEL::multilabel;

  return make_base(*l);
}
