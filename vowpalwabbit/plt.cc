/*
  Probabilistic Label Tree
  Implementation by Marek Wydmuch and Kalina Jasińska.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <queue>

#include "reductions.h"
#include "vw.h"

using namespace std;
using namespace LEARNER;

struct node
{ uint32_t n;
  float p;

  bool operator<(const node &r) const { return p < r.p; }
};

struct plt
{ vw *all;

  uint32_t k; // number of labels
  uint32_t t; // number of tree nodes
  uint32_t ti; // number of internal nodes
  uint32_t kary;

  float threshold; // inner threshold
  uint32_t top_k;
  float *nodes_t;

  float* precision;
  uint32_t prediction_count;
};

inline void learn_node(plt &p, uint32_t n, base_learner &base, example &ec)
{ if(!p.all->adaptive)
  { p.all->sd->t = p.nodes_t[n];
    p.nodes_t[n] += ec.weight;
  }
  base.learn(ec, n);
}

void learn(plt &p, base_learner &base, example &ec)
{ MULTILABEL::labels multilabels = ec.l.multilabels;
  MULTILABEL::labels preds = ec.pred.multilabels;

  double t = p.all->sd->t;
  double weighted_holdout_examples = p.all->sd->weighted_holdout_examples;
  p.all->sd->weighted_holdout_examples = 0;

  unordered_set<uint32_t> n_positive; // positive nodes
  unordered_set<uint32_t> n_negative; // negative nodes

  if (multilabels.label_v.size() > 0)
  { for (uint32_t i = 0; i < multilabels.label_v.size(); ++i)
    { uint32_t tn = multilabels.label_v[i] + p.ti;
      n_positive.insert(tn);
      while (tn > 0) {
        tn = floor(static_cast<float>(tn - 1) / p.kary);
        n_positive.insert(tn);
      }
    }
    if (multilabels.label_v[multilabels.label_v.size()-1] >= p.k)
      cerr << "label " << multilabels.label_v[multilabels.label_v.size()-1] << " is not in {0," << p.k - 1 << "} This won't work right." << endl;

    queue <uint32_t> n_queue; // nodes queue
    n_queue.push(0);

    while (!n_queue.empty()) {
      uint32_t n = n_queue.front(); // current node index
      n_queue.pop();

      if (n < p.ti) {
        for (uint32_t i = 1; i <= p.kary; ++i) {
          uint32_t n_child = p.kary * n + i;

          if (n_child < p.t) {
            if (n_positive.find(n_child) != n_positive.end())
              n_queue.push(n_child);
            else
              n_negative.insert(n_child);
          }
        }
      }
    }
  } else
    n_negative.insert(0);

  ec.l.simple = {1.f, 1.f, 0.f};
  for (auto &n : n_positive)
    learn_node(p, n, base, ec);

  ec.l.simple.label = -1.f;
  for (auto &n : n_negative)
    learn_node(p, n, base, ec);

  p.all->sd->t = t;
  p.all->sd->weighted_holdout_examples = weighted_holdout_examples;

  ec.pred.multilabels = preds;
  ec.l.multilabels = multilabels;
}

inline float predict_node(plt &p, uint32_t n, base_learner &base, example &ec)
{ ec.l.simple = {FLT_MAX, 1.f, 0.f};
  base.predict(ec, n);
  return 1.0f / (1.0f + exp(-ec.partial_prediction));
}

template<bool threshold>
void predict(plt &p, base_learner &base, example &ec)
{ MULTILABEL::labels multilabels = ec.l.multilabels;
  MULTILABEL::labels preds = ec.pred.multilabels;
  preds.label_v.erase();

  // threshold prediction
  if (threshold)
  { vector <node> positive_labels;
    queue <node> node_queue;
    node_queue.push({0, 1.0f});

    while (!node_queue.empty())
    { node node = node_queue.front(); // current node
      node_queue.pop();

      float cp = node.p * predict_node(p, node.n, base, ec);

      if (cp > p.threshold)
      { if (node.n < p.ti)
        { for (uint32_t i = 1; i <= p.kary; ++i)
          { uint32_t n_child = p.kary * node.n + i;
            node_queue.push({n_child, cp});
          }
        }
        else {
          uint32_t l = node.n - p.ti;
          positive_labels.push_back({l, cp});
        }
      }
    }

    sort(positive_labels.rbegin(), positive_labels.rend());
    for (auto p : positive_labels)
      preds.label_v.push_back(p.n);

    vector<uint32_t> true_labels;
    for (uint32_t i = 0; i < multilabels.label_v.size(); ++i)
      true_labels.push_back(multilabels.label_v[i]);

    if (true_labels.size() > 0)
    { for (size_t i = 0; i < preds.label_v.size(); ++i)
      { if (find(true_labels.begin(), true_labels.end(), preds.label_v[i]) != true_labels.end())
          p.precision[0] += 1.0f;
      }
    }

    p.prediction_count += preds.label_v.size();
  }

  // top-k predictions
  else
  { std::unordered_set<uint32_t> found_leaves;
    priority_queue <node> node_queue;
    node_queue.push({0, 1.0f});

    while (!node_queue.empty())
    { node node = node_queue.top();
      node_queue.pop();

      auto found_leaves_it = found_leaves.find(node.n);
      if (found_leaves_it != found_leaves.end())
      { uint32_t l = node.n - p.ti;
        preds.label_v.push_back(l);
        if (preds.label_v.size() >= p.top_k)
          break;
      }
      else
      { float cp = node.p * predict_node(p, node.n, base, ec);

        if (node.n < p.ti)
        { for (uint32_t i = 1; i <= p.kary; ++i) {
            uint32_t n_child = p.kary * node.n + i;
            node_queue.push({n_child, cp});
          }
        }
        else
        { found_leaves.emplace_hint(found_leaves_it, node.n);
          node_queue.push({node.n, cp});
        }
      }
    }

    std::unordered_set<uint32_t> true_labels(multilabels.label_v.begin(), multilabels.label_v.end());

    if (p.top_k > 0 && true_labels.size() > 0)
    { for (size_t i = 0; i < p.top_k; ++i)
      { if (find(true_labels.begin(), true_labels.end(), preds.label_v[i]) != true_labels.end())
          p.precision[i] += 1.0f;
      }
    }

    ++p.prediction_count;
  }

  // for multilabel loss
  //sort(preds.label_v.begin(), preds.label_v.end());

  ec.pred.multilabels = preds;
  ec.l.multilabels = multilabels;
}

void finish_example(vw& all, plt &p, example& ec)
{ MULTILABEL::output_example(all, ec);
  VW::finish_example(all, &ec);
}

void finish(plt &p)
{ // threshold prediction
  if (p.threshold > 0 && !p.all->training) {
    if (p.prediction_count > 0)
      cerr << "Precision = " << p.precision[0] / p.prediction_count << "\n";
    else
      cerr << "Precision unknown - nothing predicted" << endl;

  // top-k predictions
  } else if(p.top_k > 0 && !p.all->training)
  { float correct = 0;
    for (size_t i = 0; i < p.top_k; ++i)
    { correct += p.precision[i];
      cerr << "P@" << i + 1 << " = " << correct / (p.prediction_count * (i + 1)) << "\n";
    }
  }

  free(p.nodes_t);
  free(p.precision);
}

void save_load_nodes(plt &p, io_buf &model_file, bool read, bool text)
{ if (model_file.files.size() > 0)
  { bool resume = p.all->save_resume;
    stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, (char *) &resume, sizeof(resume), "", read, msg, text);

    if (resume)
      for (size_t i = 0; i < p.t; ++i)
        bin_text_read_write_fixed(model_file, (char *) &p.nodes_t[i], sizeof(p.nodes_t[0]), "", read, msg, text);
  }
}

LEARNER::base_learner* plt_setup(vw &all) //learner setup
{ if (missing_option<size_t, true>(all, "plt", "Probabilistic Label Tree with <k> labels"))
    return nullptr;
  new_options(all, "plt options")
  ("kary_tree", po::value<uint32_t>(), "tree in which each node has no more than k children")
  ("top_k", po::value<uint32_t>(), "top_k labels (default 1)")
  ("threshold", po::value<float>(), "threshold for positive label (default 0.15)");
  add_options(all);

  plt &data = calloc_or_throw<plt>();
  data.k = (uint32_t) all.vm["plt"].as<size_t>();
  data.threshold = -1;
  data.top_k = 1;
  data.all = &all;

  LEARNER::learner<plt>* l;

  // calculate number of nodes
  if (all.vm.count("kary_tree")) {
    data.kary = all.vm["kary_tree"].as<uint32_t>();

    double a = pow(data.kary, floor(log(data.k) / log(data.kary)));
    double b = data.k - a;
    double c = ceil(b / (data.kary - 1.0));
    double d = (data.kary * a - 1.0) / (data.kary - 1.0);
    double e = data.k - (a - c);
    data.t = static_cast<uint32_t>(e + d);
  } else {
    data.kary = 2;
    data.t = 2 * data.k - 1;
  }
  data.ti = data.t - data.k;
  *(all.file_options) << " --kary_tree " << data.kary;

  if (all.vm.count("top_k"))
    data.top_k = all.vm["top_k"].as<uint32_t>();
  if (all.vm.count("threshold"))
    data.threshold = all.vm["threshold"].as<float>();
  data.precision = calloc_or_throw<float>(data.top_k > 0 ? data.top_k : 1);

  if (data.threshold >= 0)
    l = &init_learner(&data, setup_base(all), learn, predict<true>, data.t, prediction_type::multilabels);
  else
    l = &init_learner(&data, setup_base(all), learn, predict<false>, data.t, prediction_type::multilabels);

  all.p->lp = MULTILABEL::multilabel;
  all.label_type = label_type::multi;
  all.delete_prediction = MULTILABEL::multilabel.delete_label;

  // force logistic loss for base classifier
  string loss_function = "logistic";
  delete(all.loss);
  all.loss = getLossFunction(all, loss_function);

  data.nodes_t = calloc_or_throw<float>(data.t);
  for (size_t i = 0; i < data.t; ++i)
    data.nodes_t[i] = all.initial_t;

  l->set_finish_example(finish_example);
  l->set_finish(finish);
  l->set_save_load(save_load_nodes);

  // turn off stop based on holdout loss
  all.holdout_set_off = true;

  return make_base(*l);
}
