// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include "reductions.h"
#include "vw.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace plt_ns
{
struct node
{
  uint32_t n;  // node number
  float p;     // node probability

  bool operator<(const node& r) const { return p < r.p; }
};

struct plt
{
  vw* all;

  // tree structure
  uint32_t k;     // number of labels
  uint32_t t;     // number of tree nodes
  uint32_t ti;    // number of internal nodes
  uint32_t kary;  // kary tree

  // for training
  v_array<float> nodes_time;                    // in case of sgd, this stores individual t for each node
  std::unordered_set<uint32_t> positive_nodes;  // container for positive nodes
  std::unordered_set<uint32_t> negative_nodes;  // container for negative nodes

  // for prediction
  float threshold;
  uint32_t top_k;
  v_array<polyprediction> node_preds;  // for storing results of base.multipredict
  std::vector<node> node_queue;        // container for queue used for both types of predictions

  // for measuring predictive performance
  std::unordered_set<uint32_t> true_labels;
  v_array<uint32_t> tp_at;  // true positives at (for precision and recall at)
  uint32_t tp;
  uint32_t fp;
  uint32_t fn;
  uint32_t true_count;  // number of all true labels (for recall at)
  uint32_t ec_count;    // number of examples

  plt()
  {
    nodes_time = v_init<float>();
    node_preds = v_init<polyprediction>();
    tp_at = v_init<uint32_t>();
    tp = 0;
    fp = 0;
    fn = 0;
    ec_count = 0;
    true_count = 0;
  }

  ~plt()
  {
    nodes_time.delete_v();
    node_preds.delete_v();
    tp_at.delete_v();
  }
};

inline void learn_node(plt& p, uint32_t n, single_learner& base, example& ec)
{
  if (!p.all->weights.adaptive)
  {
    p.all->sd->t = p.nodes_time[n];
    p.nodes_time[n] += ec.weight;
  }
  base.learn(ec, n);
}

void learn(plt& p, single_learner& base, example& ec)
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
          tn = static_cast<uint32_t>(floor(static_cast<float>(tn - 1) / p.kary));
          p.positive_nodes.insert(tn);
        }
      }
    }
    if (ec.l.multilabels.label_v.last() >= p.k)
      std::cout << "label " << ec.l.multilabels.label_v.last() << " is not in {0," << p.k - 1
                << "} This won't work right." << std::endl;

    for (auto& n : p.positive_nodes)
    {
      if (n < p.ti)
      {
        for (uint32_t i = 1; i <= p.kary; ++i)
        {
          uint32_t n_child = p.kary * n + i;
          if (n_child < p.t && p.positive_nodes.find(n_child) == p.positive_nodes.end())
            p.negative_nodes.insert(n_child);
        }
      }
    }
  }
  else
    p.negative_nodes.insert(0);

  ec.l.simple = {1.f, 1.f, 0.f};
  for (auto& n : p.positive_nodes) learn_node(p, n, base, ec);

  ec.l.simple.label = -1.f;
  for (auto& n : p.negative_nodes) learn_node(p, n, base, ec);

  p.all->sd->t = t;
  p.all->sd->weighted_holdout_examples = weighted_holdout_examples;

  ec.pred.multilabels = std::move(preds);
  ec.l.multilabels = std::move(multilabels);
}

inline float predict_node(uint32_t n, single_learner& base, example& ec)
{
  ec.l.simple = {FLT_MAX, 1.f, 0.f};
  base.predict(ec, n);
  return 1.0f / (1.0f + exp(-ec.partial_prediction));
}

template <bool threshold>
void predict(plt& p, single_learner& base, example& ec)
{
  MULTILABEL::labels multilabels = std::move(ec.l.multilabels);
  MULTILABEL::labels preds = std::move(ec.pred.multilabels);
  preds.label_v.clear();

  // split labels into true and skip (those > max. label num)
  p.true_labels.clear();
  for (auto label : ec.l.multilabels.label_v)
  {
    if (label < p.k) p.true_labels.insert(label);
    else
      std::cout << "label " << label << " is not in {0," << p.k - 1 << "} Model can't predict it." << std::endl;
  }

  p.node_queue.clear();  // clear node queue

  // prediction with threshold
  if (threshold)
  {
    float cp_root = predict_node(0, base, ec);
    if (cp_root > p.threshold) p.node_queue.push_back({0, cp_root});  // here queue is used for dfs search

    while (!p.node_queue.empty())
    {
      node node = p.node_queue.back();  // current node
      p.node_queue.pop_back();

      uint32_t n_child = p.kary * node.n + 1;
      ec.l.simple = {FLT_MAX, 1.f, 0.f};
      base.multipredict(ec, n_child, p.kary, p.node_preds.begin(), false);

      for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
      {
        float cp_child = node.p * (1.f / (1.f + exp(-p.node_preds[i].scalar)));
        if (cp_child > p.threshold)
        {
          if (n_child < p.ti) p.node_queue.push_back({n_child, cp_child});
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
        if (p.true_labels.count(pred_label)) ++tp;
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
        ec.l.simple = {FLT_MAX, 1.f, 0.f};
        base.multipredict(ec, n_child, p.kary, p.node_preds.begin(), false);

        for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
        {
          float cp_child = node.p * (1.0f / (1.0f + exp(-p.node_preds[i].scalar)));
          p.node_queue.push_back({n_child, cp_child});
          std::push_heap(p.node_queue.begin(), p.node_queue.end());
        }
      }
      else
      {
        uint32_t l = node.n - p.ti;
        preds.label_v.push_back(l);
        if (preds.label_v.size() >= p.top_k) break;
      }
    }

    // calculate p@
    if (p.true_labels.size() > 0)
    {
      for (size_t i = 0; i < p.top_k; ++i)
      {
        if (p.true_labels.count(preds.label_v[i])) ++p.tp_at[i];
      }
      ++p.ec_count;
      p.true_count += static_cast<uint32_t>(p.true_labels.size());
    }
  }

  p.node_queue.clear();

  ec.pred.multilabels = std::move(preds);
  ec.l.multilabels = std::move(multilabels);
}

void finish_example(vw& all, plt& /*p*/, example& ec)
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
        std::cerr << "p@" << i + 1 << " = " << correct / (p.ec_count * (i + 1)) << std::endl;
        std::cerr << "r@" << i + 1 << " = " << correct / p.true_count << std::endl;
      }
    }

    else if (p.threshold > 0)
    {
      std::cerr << "hamming loss = " << static_cast<double>(p.fp + p.fn) / p.ec_count << std::endl;
      std::cerr << "precision = " << static_cast<double>(p.tp) / (p.tp + p.fp) << std::endl;
      std::cerr << "recall = " << static_cast<double>(p.tp) / (p.tp + p.fn) << std::endl;
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
    bin_text_read_write_fixed(model_file, (char*)&resume, sizeof(resume), "", read, msg, text);

    if (resume && !p.all->weights.adaptive)
    {
      for (size_t i = 0; i < p.t; ++i)
        bin_text_read_write_fixed(model_file, (char*)&p.nodes_time[i], sizeof(p.nodes_time[0]), "", read, msg, text);
    }
  }
}
}  // namespace plt_ns

using namespace plt_ns;

base_learner* plt_setup(options_i& options, vw& all)
{
  auto tree = scoped_calloc_or_throw<plt>();
  option_group_definition new_options("Probabilistic Label Tree ");
  new_options.add(make_option("plt", tree->k).keep().help("Probabilistic Label Tree with <k> labels"))
      .add(make_option("kary_tree", tree->kary).keep().default_value(2).help("use <k>-ary tree"))
      .add(make_option("threshold", tree->threshold)
               .default_value(0.5)
               .help("predict labels with conditional marginal probability greater than <thr> threshold"))
      .add(make_option("top_k", tree->top_k)
               .default_value(0)
               .help("predict top-<k> labels instead of labels above threshold"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("plt")) return nullptr;

  tree->all = &all;

  // calculate number of tree nodes
  const double a = std::pow(tree->kary, std::floor(std::log(tree->k) / std::log(tree->kary)));
  const double b = tree->k - a;
  const double c = std::ceil(b / (tree->kary - 1.0));
  const double d = (tree->kary * a - 1.0) / (tree->kary - 1.0);
  const double e = tree->k - (a - c);
  tree->t = static_cast<uint32_t>(e + d);
  tree->ti = tree->t - tree->k;

  if (!all.logger.quiet)
  {
    all.trace_message << "PLT k = " << tree->k << "\nkary_tree = " << tree->kary << std::endl;
    if (!all.training)
    {
      if (tree->top_k > 0) { all.trace_message << "top_k = " << tree->top_k << std::endl; }
      else
      {
        all.trace_message << "threshold = " << tree->threshold << std::endl;
      }
    }
  }

  // resize v_arrays
  tree->nodes_time.resize(tree->t);
  std::fill(tree->nodes_time.begin(), tree->nodes_time.end(), all.initial_t);
  tree->node_preds.resize(tree->kary);
  if (tree->top_k > 0) tree->tp_at.resize(tree->top_k);

  learner<plt, example>* l;
  if (tree->top_k > 0)
    l = &init_learner(
        tree, as_singleline(setup_base(options, all)), learn, predict<false>, tree->t, prediction_type_t::multilabels, "plt", false);
  else
    l = &init_learner(
        tree, as_singleline(setup_base(options, all)), learn, predict<true>, tree->t, prediction_type_t::multilabels, "plt", false);

  all.example_parser->lbl_parser = MULTILABEL::multilabel;
  all.label_type = label_type_t::multi;
  all.delete_prediction = MULTILABEL::multilabel.delete_label;

  // force logistic loss for base classifiers
  delete (all.loss);
  all.loss = getLossFunction(all, "logistic");

  l->set_finish_example(finish_example);
  l->set_finish(finish);
  l->set_save_load(save_load_tree);

  return make_base(*l);
}
