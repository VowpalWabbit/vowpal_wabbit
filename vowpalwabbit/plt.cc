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

  bool operator<(const node &r) const { return p < r.p; }
};

struct plt
{
  vw *all;

  uint32_t k;     // number of labels
  uint32_t t;     // number of tree nodes
  uint32_t ti;    // number of internal nodes
  uint32_t kary;  // kary tree

  v_array<float> nodes_t;  // in case of sgd, this keeps stores individual t for each node

  float threshold;
  uint32_t top_k;
  v_array<polyprediction> preds;  // for storing results of base.multipredict

  // for measuring predictive performance
  v_array<uint32_t> tp_at;  // true positives at (for precision and recall at)
  uint32_t tp;
  uint32_t fp;
  uint32_t fn;
  uint32_t t_count;   // number of all true labels (for recall at)
  uint32_t ec_count;  // number of examples

  plt()
  {
    nodes_t = v_init<float>();
    preds = v_init<polyprediction>();
    tp_at = v_init<uint32_t>();
    tp = 0;
    fp = 0;
    fn = 0;
    ec_count = 0;
    t_count = 0;
  }

  ~plt()
  {
    nodes_t.delete_v();
    preds.delete_v();
    tp_at.delete_v();
  }
};

inline void learn_node(plt &p, uint32_t n, single_learner &base, example &ec)
{
  if (!p.all->weights.adaptive)
  {
    p.all->sd->t = p.nodes_t[n];
    p.nodes_t[n] += ec.weight;
  }
  base.learn(ec, n);
}

void learn(plt &p, single_learner &base, example &ec)
{
  MULTILABEL::labels multilabels = ec.l.multilabels;
  MULTILABEL::labels preds = ec.pred.multilabels;

  double t = p.all->sd->t;
  double weighted_holdout_examples = p.all->sd->weighted_holdout_examples;
  p.all->sd->weighted_holdout_examples = 0;

  std::unordered_set<uint32_t> n_positive;  // positive nodes
  std::unordered_set<uint32_t> n_negative;  // negative nodes

  if (multilabels.label_v.size() > 0)
  {
    for (uint32_t i = 0; i < multilabels.label_v.size(); ++i)
    {
      uint32_t tn = multilabels.label_v[i] + p.ti;
      if (tn < p.t)
      {
        n_positive.insert(tn);
        while (tn > 0)
        {
          tn = floor(static_cast<float>(tn - 1) / p.kary);
          n_positive.insert(tn);
        }
      }
    }
    if (multilabels.label_v[multilabels.label_v.size() - 1] >= p.k)
      std::cout << "label " << multilabels.label_v[multilabels.label_v.size() - 1] << " is not in {0," << p.k - 1
                << "} This won't work right." << std::endl;

    for (auto &n : n_positive)
    {
      if (n < p.ti)
      {
        for (uint32_t i = 1; i <= p.kary; ++i)
        {
          uint32_t n_child = p.kary * n + i;
          if (n_child < p.t && n_positive.find(n_child) == n_positive.end())
            n_negative.insert(n_child);
        }
      }
    }
  }
  else
    n_negative.insert(0);

  ec.l.simple = {1.f, 1.f, 0.f};
  for (auto &n : n_positive) learn_node(p, n, base, ec);

  ec.l.simple.label = -1.f;
  for (auto &n : n_negative) learn_node(p, n, base, ec);

  p.all->sd->t = t;
  p.all->sd->weighted_holdout_examples = weighted_holdout_examples;

  ec.pred.multilabels = preds;
  ec.l.multilabels = multilabels;
}

inline float predict_node(uint32_t n, single_learner &base, example &ec)
{
  ec.l.simple = {FLT_MAX, 1.f, 0.f};
  base.predict(ec, n);
  return 1.0f / (1.0f + exp(-ec.partial_prediction));
}

template <bool threshold>
void predict(plt &p, single_learner &base, example &ec)
{
  MULTILABEL::labels multilabels = ec.l.multilabels;
  MULTILABEL::labels preds = ec.pred.multilabels;
  preds.label_v.clear();

  // split labels into true and skip (those > max. label num)
  std::unordered_set<uint32_t> true_labels;
  std::unordered_set<uint32_t> skip_labels;
  for (auto label : multilabels.label_v)
  {
    if (label < p.k)
      true_labels.insert(label);
    else
      skip_labels.insert(label - p.k);
  }

  // prediction with threshold
  if (threshold)
  {
    std::queue<node> node_queue;
    float cp_root = predict_node(0, base, ec);
    if (cp_root > p.threshold)
      node_queue.push({0, cp_root});

    while (!node_queue.empty())
    {
      node node = node_queue.front();  // current node
      node_queue.pop();

      uint32_t n_child = p.kary * node.n + 1;
      ec.l.simple = {FLT_MAX, 1.f, 0.f};
      base.multipredict(ec, n_child, p.kary, p.preds.begin(), false);

      for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
      {
        float cp_child = node.p * (1.f / (1.f + exp(-p.preds[i].scalar)));
        if (cp_child > p.threshold)
        {
          if (n_child < p.ti)
            node_queue.push({n_child, cp_child});
          else
          {
            uint32_t l = n_child - p.ti;
            preds.label_v.push_back(l);
          }
        }
      }
    }

    if (true_labels.size() > 0)
    {
      uint32_t tp = 0;
      for (size_t i = 0; i < preds.label_v.size(); ++i)
      {
        if (true_labels.count(preds.label_v[i]))
          ++tp;
      }
      p.tp += tp;
      p.fp += preds.label_v.size() - tp;
      p.fn += true_labels.size() - tp;
      ++p.ec_count;
    }
  }

  // top-k prediction
  else
  {
    std::priority_queue<node> node_queue;
    node_queue.push({0, predict_node(0, base, ec)});

    while (!node_queue.empty())
    {
      node node = node_queue.top();
      node_queue.pop();

      if (node.n < p.ti)
      {
        uint32_t n_child = p.kary * node.n + 1;
        ec.l.simple = {FLT_MAX, 1.f, 0.f};
        base.multipredict(ec, n_child, p.kary, p.preds.begin(), false);

        for (uint32_t i = 0; i < p.kary; ++i, ++n_child)
        {
          float cp_child = node.p * (1.0f / (1.0f + exp(-p.preds[i].scalar)));
          node_queue.push({n_child, cp_child});
        }
      }
      else
      {
        uint32_t l = node.n - p.ti;
        preds.label_v.push_back(l);
        if (preds.label_v.size() >= p.top_k)
          break;
      }
    }

    // calculate p@
    if (true_labels.size() > 0)
    {
      for (size_t i = 0; i < p.top_k; ++i)
      {
        if (true_labels.count(preds.label_v[i]))
          ++p.tp_at[i];
      }
      ++p.ec_count;
      p.t_count += true_labels.size();
    }
  }

  ec.l.multilabels = multilabels;
  ec.pred.multilabels = preds;
}

void finish_example(vw &all, plt &p, example &ec)
{
  MULTILABEL::output_example(all, ec);
  VW::finish_example(all, ec);
}

void finish(plt &p)
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
        std::cerr << "r@" << i + 1 << " = " << correct / p.t_count << std::endl;
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

void save_load_tree(plt &p, io_buf &model_file, bool read, bool text)
{
  if (model_file.num_files() > 0)
  {
    bool resume = p.all->save_resume;
    std::stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, (char *)&resume, sizeof(resume), "", read, msg, text);

    if (resume && !p.all->weights.adaptive)
    {
      for (size_t i = 0; i < p.t; ++i)
        bin_text_read_write_fixed(model_file, (char *)&p.nodes_t[i], sizeof(p.nodes_t[0]), "", read, msg, text);
    }
  }
}
}  // namespace plt_ns

using namespace plt_ns;

LEARNER::base_learner *plt_setup(options_i &options, vw &all)
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

  if (!options.was_supplied("plt"))
    return nullptr;

  tree->all = &all;

  // calculate number of tree nodes
  double a = std::pow(tree->kary, std::floor(std::log(tree->k) / std::log(tree->kary)));
  double b = tree->k - a;
  double c = std::ceil(b / (tree->kary - 1.0));
  double d = (tree->kary * a - 1.0) / (tree->kary - 1.0);
  double e = tree->k - (a - c);
  tree->t = static_cast<uint32_t>(e + d);
  tree->ti = tree->t - tree->k;

  if (!all.logger.quiet)
  {
    all.trace_message << "PLT k = " << tree->k << "\nkary_tree = " << tree->kary << std::endl;
    if (!all.training)
      if (tree->top_k > 0)
        all.trace_message << "top_k = " << tree->top_k << std::endl;
      else
        all.trace_message << "threshold = " << tree->threshold << std::endl;
  }

  // resize v_arrays
  if (!all.weights.adaptive)
  {
    tree->nodes_t.resize(tree->t);
    std::fill(tree->nodes_t.begin(), tree->nodes_t.end(), all.initial_t);
  }
  tree->preds.resize(tree->kary);
  tree->tp_at.resize(tree->top_k);

  learner<plt, example> *l;
  if (tree->top_k > 0)
    l = &init_learner(
        tree, as_singleline(setup_base(options, all)), learn, predict<false>, tree->t, prediction_type_t::multilabels);
  else
    l = &init_learner(
        tree, as_singleline(setup_base(options, all)), learn, predict<true>, tree->t, prediction_type_t::multilabels);

  all.p->lp = MULTILABEL::multilabel;
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
