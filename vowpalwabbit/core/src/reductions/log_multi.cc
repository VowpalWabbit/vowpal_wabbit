// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/log_multi.h"

#include "vw/config/options.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <sstream>

using namespace VW::LEARNER;
using namespace VW::config;

// TODO: This file makes extensive use of cout and partial line logging.
//       Will require some investigation on how to proceed

namespace
{
class node_pred
{
public:
  double Ehk;      // NOLINT
  float norm_Ehk;  // NOLINT
  uint32_t nk;
  uint32_t label;
  uint32_t label_count;

  bool operator==(node_pred v) const { return (label == v.label); }

  bool operator>(node_pred v) const { return label > v.label; }

  bool operator<(node_pred v) const { return label < v.label; }

  node_pred() = default;
  node_pred(uint32_t l)
  {
    label = l;
    Ehk = 0.f;
    norm_Ehk = 0;
    nk = 0;
    label_count = 0;
  }
};

static_assert(std::is_trivial<node_pred>::value, "To be used in v_array node_pred must be trivial");

class node
{
public:
  // everyone has
  uint32_t parent;               // the parent node
  VW::v_array<node_pred> preds;  // per-class state
  uint32_t
      min_count;  // the number of examples reaching this node (if it's a leaf) or the minimum reaching any grandchild.

  bool internal;  // internal or leaf

  // internal nodes have
  uint32_t base_predictor;  // id of the base predictor
  uint32_t left;            // left child
  uint32_t right;           // right child
  float norm_Eh;            // NOLINT the average margin at the node
  double Eh;                // NOLINT total margin at the node
  uint32_t n;               // total events at the node

  // leaf has
  uint32_t max_count;        // the number of samples of the most common label
  uint32_t max_count_label;  // the most common label
};

class log_multi
{
public:
  uint32_t k = 0;

  std::vector<node> nodes;

  size_t max_predictors = 0;
  size_t predictors_used = 0;

  bool progress = false;
  uint32_t swap_resist = 0;

  uint32_t nbofswaps = 0;

  ~log_multi()
  {
    // save_node_stats(b);
  }
};

inline void init_leaf(node& n)
{
  n.internal = false;
  n.preds.clear();
  n.base_predictor = 0;
  n.norm_Eh = 0;
  n.Eh = 0;
  n.n = 0;
  n.max_count = 0;
  n.max_count_label = 1;
  n.left = 0;
  n.right = 0;
}

inline node init_node()
{
  node node;

  node.parent = 0;
  node.min_count = 0;
  init_leaf(node);

  return node;
}

void init_tree(log_multi& d)
{
  d.nodes.push_back(init_node());
  d.nbofswaps = 0;
}

inline uint32_t min_left_right(log_multi& b, const node& n)
{
  return std::min(b.nodes[n.left].min_count, b.nodes[n.right].min_count);
}

inline uint32_t find_switch_node(log_multi& b)
{
  uint32_t node = 0;
  while (b.nodes[node].internal)
  {
    if (b.nodes[b.nodes[node].left].min_count < b.nodes[b.nodes[node].right].min_count) { node = b.nodes[node].left; }
    else { node = b.nodes[node].right; }
  }
  return node;
}

inline void update_min_count(log_multi& b, uint32_t node)
{
  // Constant time min count update.
  while (node != 0)
  {
    uint32_t prev = node;
    node = b.nodes[node].parent;

    if (b.nodes[node].min_count == b.nodes[prev].min_count) { break; }
    else { b.nodes[node].min_count = min_left_right(b, b.nodes[node]); }
  }
}

bool children(log_multi& b, uint32_t& current, uint32_t& class_index, uint32_t label)
{
  auto& preds = b.nodes[current].preds;
  node_pred val_to_insert(label);
  auto found_it = std::lower_bound(preds.begin(), preds.end(), val_to_insert);
  if (found_it == preds.end() || !(*found_it == val_to_insert)) { found_it = preds.insert(found_it, val_to_insert); }
  class_index = static_cast<uint32_t>(std::distance(preds.begin(), found_it));
  b.nodes[current].preds[class_index].label_count++;

  if (b.nodes[current].preds[class_index].label_count > b.nodes[current].max_count)
  {
    b.nodes[current].max_count = b.nodes[current].preds[class_index].label_count;
    b.nodes[current].max_count_label = b.nodes[current].preds[class_index].label;
  }

  if (b.nodes[current].internal) { return true; }
  else if (b.nodes[current].preds.size() > 1 &&
      (b.predictors_used < b.max_predictors ||
          b.nodes[current].min_count - b.nodes[current].max_count > b.swap_resist * (b.nodes[0].min_count + 1)))
  {
    // need children and we can make them.
    uint32_t left_child;
    uint32_t right_child;
    if (b.predictors_used < b.max_predictors)
    {
      left_child = static_cast<uint32_t>(b.nodes.size());
      b.nodes.push_back(init_node());
      right_child = static_cast<uint32_t>(b.nodes.size());
      b.nodes.push_back(init_node());
      b.nodes[current].base_predictor = static_cast<uint32_t>(b.predictors_used++);
    }
    else
    {
      uint32_t swap_child = find_switch_node(b);
      uint32_t swap_parent = b.nodes[swap_child].parent;
      uint32_t swap_grandparent = b.nodes[swap_parent].parent;
      if (b.nodes[swap_child].min_count != b.nodes[0].min_count)
      {
        std::cout << "glargh " << b.nodes[swap_child].min_count << " != " << b.nodes[0].min_count << std::endl;
      }
      b.nbofswaps++;

      uint32_t nonswap_child;
      if (swap_child == b.nodes[swap_parent].right) { nonswap_child = b.nodes[swap_parent].left; }
      else { nonswap_child = b.nodes[swap_parent].right; }

      if (swap_parent == b.nodes[swap_grandparent].left) { b.nodes[swap_grandparent].left = nonswap_child; }
      else { b.nodes[swap_grandparent].right = nonswap_child; }
      b.nodes[nonswap_child].parent = swap_grandparent;
      update_min_count(b, nonswap_child);

      init_leaf(b.nodes[swap_child]);
      left_child = swap_child;
      b.nodes[current].base_predictor = b.nodes[swap_parent].base_predictor;
      init_leaf(b.nodes[swap_parent]);
      right_child = swap_parent;
    }
    b.nodes[current].left = left_child;
    b.nodes[left_child].parent = current;
    b.nodes[current].right = right_child;
    b.nodes[right_child].parent = current;

    b.nodes[left_child].min_count = b.nodes[current].min_count / 2;
    b.nodes[right_child].min_count = b.nodes[current].min_count - b.nodes[left_child].min_count;
    update_min_count(b, left_child);

    b.nodes[left_child].max_count_label = b.nodes[current].max_count_label;
    b.nodes[right_child].max_count_label = b.nodes[current].max_count_label;

    b.nodes[current].internal = true;
  }
  return b.nodes[current].internal;
}

void train_node(
    log_multi& b, learner& base, VW::example& ec, uint32_t& current, uint32_t& class_index, uint32_t /* depth */)
{
  if (b.nodes[current].norm_Eh > b.nodes[current].preds[class_index].norm_Ehk) { ec.l.simple.label = -1.f; }
  else { ec.l.simple.label = 1.f; }

  base.learn(ec, b.nodes[current].base_predictor);  // depth

  ec.l.simple.label = FLT_MAX;
  base.predict(ec, b.nodes[current].base_predictor);  // depth

  b.nodes[current].Eh += static_cast<double>(ec.partial_prediction);
  b.nodes[current].preds[class_index].Ehk += static_cast<double>(ec.partial_prediction);
  b.nodes[current].n++;
  b.nodes[current].preds[class_index].nk++;

  b.nodes[current].norm_Eh = static_cast<float>(b.nodes[current].Eh) / b.nodes[current].n;
  b.nodes[current].preds[class_index].norm_Ehk =
      static_cast<float>(b.nodes[current].preds[class_index].Ehk) / b.nodes[current].preds[class_index].nk;
}

inline uint32_t descend(node& n, float prediction)
{
  if (prediction < 0) { return n.left; }
  else { return n.right; }
}

void predict(log_multi& b, learner& base, VW::example& ec)
{
  VW::multiclass_label mc = ec.l.multi;

  ec.l.simple = {FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

  uint32_t cn = 0;
  while (b.nodes[cn].internal)
  {
    base.predict(ec, b.nodes[cn].base_predictor);  // depth
    cn = descend(b.nodes[cn], ec.pred.scalar);
  }
  ec.pred.multiclass = b.nodes[cn].max_count_label;
  ec.l.multi = mc;
}

void learn(log_multi& b, learner& base, VW::example& ec)
{
  if (ec.l.multi.label != static_cast<uint32_t>(-1))  // if training the tree
  {
    VW::multiclass_label mc = ec.l.multi;
    uint32_t start_pred = ec.pred.multiclass;

    uint32_t class_index = 0;
    ec.l.simple = {FLT_MAX};
    ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();
    uint32_t cn = 0;
    uint32_t depth = 0;
    while (children(b, cn, class_index, mc.label))
    {
      train_node(b, base, ec, cn, class_index, depth);
      cn = descend(b.nodes[cn], ec.pred.scalar);
      depth++;
    }

    b.nodes[cn].min_count++;
    update_min_count(b, cn);
    ec.pred.multiclass = start_pred;
    ec.l.multi = mc;
  }
}

void save_load_tree(log_multi& b, VW::io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() > 0)
  {
    std::stringstream msg;
    msg << "k = " << b.k;
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&b.max_predictors), sizeof(b.k), read, msg, text);

    msg << "nodes = " << b.nodes.size() << " ";
    uint32_t temp = static_cast<uint32_t>(b.nodes.size());
    VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&temp), sizeof(temp), read, msg, text);
    if (read)
    {
      for (uint32_t j = 1; j < temp; j++) { b.nodes.push_back(init_node()); }
    }

    msg << "max predictors = " << b.max_predictors << " ";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&b.max_predictors), sizeof(b.max_predictors), read, msg, text);

    msg << "predictors_used = " << b.predictors_used << " ";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&b.predictors_used), sizeof(b.predictors_used), read, msg, text);

    msg << "progress = " << b.progress << " ";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&b.progress), sizeof(b.progress), read, msg, text);

    msg << "swap_resist = " << b.swap_resist << "\n";
    VW::details::bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&b.swap_resist), sizeof(b.swap_resist), read, msg, text);

    for (size_t j = 0; j < b.nodes.size(); j++)
    {
      // Need to read or write nodes.
      node& n = b.nodes[j];

      msg << " parent = " << n.parent;
      VW::details::bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&n.parent), sizeof(n.parent), read, msg, text);

      temp = static_cast<uint32_t>(n.preds.size());

      msg << " preds = " << temp;
      VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&temp), sizeof(temp), read, msg, text);
      if (read)
      {
        for (uint32_t k = 0; k < temp; k++) { n.preds.push_back(node_pred(1)); }
      }

      msg << " min_count = " << n.min_count;
      VW::details::bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&n.min_count), sizeof(n.min_count), read, msg, text);

      msg << " internal = " << n.internal;
      VW::details::bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&n.internal), sizeof(n.internal), read, msg, text);

      if (n.internal)
      {
        msg << " base_predictor = " << n.base_predictor;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.base_predictor), sizeof(n.base_predictor), read, msg, text);

        msg << " left = " << n.left;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.left), sizeof(n.left), read, msg, text);

        msg << " right = " << n.right;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.right), sizeof(n.right), read, msg, text);

        msg << " norm_Eh = " << n.norm_Eh;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.norm_Eh), sizeof(n.norm_Eh), read, msg, text);

        msg << " Eh = " << n.Eh;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.Eh), sizeof(n.Eh), read, msg, text);

        msg << " n = " << n.n << "\n";
        VW::details::bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&n.n), sizeof(n.n), read, msg, text);
      }
      else
      {
        msg << " max_count = " << n.max_count;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.max_count), sizeof(n.max_count), read, msg, text);
        msg << " max_count_label = " << n.max_count_label << "\n";
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&n.max_count_label), sizeof(n.max_count_label), read, msg, text);
      }

      for (size_t k = 0; k < n.preds.size(); k++)
      {
        node_pred& p = n.preds[k];

        msg << "  Ehk = " << p.Ehk;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.Ehk), sizeof(p.Ehk), read, msg, text);

        msg << " norm_Ehk = " << p.norm_Ehk;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.norm_Ehk), sizeof(p.norm_Ehk), read, msg, text);

        msg << " nk = " << p.nk;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.nk), sizeof(p.nk), read, msg, text);

        msg << " label = " << p.label;
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.label), sizeof(p.label), read, msg, text);

        msg << " label_count = " << p.label_count << "\n";
        VW::details::bin_text_read_write_fixed(
            model_file, reinterpret_cast<char*>(&p.label_count), sizeof(p.label_count), read, msg, text);
      }
    }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::log_multi_setup(VW::setup_base_i& stack_builder)  // learner setup
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  auto data = VW::make_unique<log_multi>();
  option_group_definition new_options("[Reduction] Logarithmic Time Multiclass Tree");
  new_options.add(make_option("log_multi", data->k).keep().necessary().help("Use online tree for multiclass"))
      .add(make_option("no_progress", data->progress).help("Disable progressive validation"))
      .add(make_option("swap_resistance", data->swap_resist)
               .default_value(4)
               .help("Higher = more resistance to swap, default=4"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  data->progress = !data->progress;

  std::string loss_function = "quantile";
  float loss_parameter = 0.5;
  all.loss = get_loss_function(all, loss_function, loss_parameter);

  data->max_predictors = data->k - 1;
  init_tree(*data.get());

  size_t ws = data->max_predictors;
  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()), learn,
      predict, stack_builder.get_setupfn_name(log_multi_setup))
               .set_params_per_weight(ws)
               .set_update_stats(VW::details::update_stats_multiclass_label<log_multi>)
               .set_output_example_prediction(VW::details::output_example_prediction_multiclass_label<log_multi>)
               .set_print_update(VW::details::print_update_multiclass_label<log_multi>)
               .set_save_load(save_load_tree)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_input_label_type(VW::label_type_t::MULTICLASS)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .build();

  all.example_parser->lbl_parser = VW::multiclass_label_parser_global;

  return l;
}
