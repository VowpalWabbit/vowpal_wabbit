/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <cfloat>
#include <cerrno>

#include "correctedMath.h"
#include "reductions.h"
#include "v_hashmap.h"
#include "label_dictionary.h"
#include "vw.h"
#include "gd.h"  // GD::foreach_feature() needed in subtract_example()
#include "vw_exception.h"
#include <algorithm>
#include "csoaa.h"

using namespace LEARNER;
using namespace COST_SENSITIVE;
using namespace VW::config;

namespace CSOAA
{
struct csoaa
{
  uint32_t num_classes;
  polyprediction* pred;
  ~csoaa() { free(pred); }
};

template <bool is_learn>
inline void inner_loop(single_learner& base, example& ec, uint32_t i, float cost, uint32_t& prediction, float& score,
    float& partial_prediction)
{
  if (is_learn)
  {
    ec.weight = (cost == FLT_MAX) ? 0.f : 1.f;
    ec.l.simple.label = cost;
    base.learn(ec, i - 1);
  }
  else
    base.predict(ec, i - 1);

  partial_prediction = ec.partial_prediction;
  if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction))
  {
    score = ec.partial_prediction;
    prediction = i;
  }
  add_passthrough_feature(ec, i, ec.partial_prediction);
}

#define DO_MULTIPREDICT true

template <bool is_learn>
void predict_or_learn(csoaa& c, single_learner& base, example& ec)
{
  // std::cerr << "------------- passthrough" << std::endl;
  COST_SENSITIVE::label ld = ec.l.cs;
  uint32_t prediction = 1;
  float score = FLT_MAX;
  size_t pt_start = ec.passthrough ? ec.passthrough->size() : 0;
  ec.l.simple = {0., 0., 0.};
  if (!ld.costs.empty())
  {
    for (auto& cl : ld.costs)
      inner_loop<is_learn>(base, ec, cl.class_index, cl.x, prediction, score, cl.partial_prediction);
    ec.partial_prediction = score;
  }
  else if (DO_MULTIPREDICT && !is_learn)
  {
    ec.l.simple = {FLT_MAX, 0.f, 0.f};
    base.multipredict(ec, 0, c.num_classes, c.pred, false);
    for (uint32_t i = 1; i <= c.num_classes; i++)
    {
      add_passthrough_feature(ec, i, c.pred[i - 1].scalar);
      if (c.pred[i - 1].scalar < c.pred[prediction - 1].scalar)
        prediction = i;
    }
    ec.partial_prediction = c.pred[prediction - 1].scalar;
  }
  else
  {
    float temp;
    for (uint32_t i = 1; i <= c.num_classes; i++) inner_loop<false>(base, ec, i, FLT_MAX, prediction, score, temp);
  }
  if (ec.passthrough)
  {
    uint64_t second_best = 0;
    float second_best_cost = FLT_MAX;
    for (size_t i = 0; i < ec.passthrough->size() - pt_start; i++)
    {
      float val = ec.passthrough->values[pt_start + i];
      if ((val > ec.partial_prediction) && (val < second_best_cost))
      {
        second_best_cost = val;
        second_best = ec.passthrough->indicies[pt_start + i];
      }
    }
    if (second_best_cost < FLT_MAX)
    {
      float margin = second_best_cost - ec.partial_prediction;
      add_passthrough_feature(ec, constant * 2, margin);
      add_passthrough_feature(ec, constant * 2 + 1 + second_best, 1.);
    }
    else
      add_passthrough_feature(ec, constant * 3, 1.);
  }

  ec.pred.multiclass = prediction;
  ec.l.cs = ld;
}

void finish_example(vw& all, csoaa&, example& ec) { COST_SENSITIVE::finish_example(all, ec); }

base_learner* csoaa_setup(options_i& options, vw& all)
{
  auto c = scoped_calloc_or_throw<csoaa>();
  option_group_definition new_options("Cost Sensitive One Against All");
  new_options.add(make_option("csoaa", c->num_classes).keep().help("One-against-all multiclass with <k> costs"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("csoaa"))
    return nullptr;

  c->pred = calloc_or_throw<polyprediction>(c->num_classes);

  learner<csoaa, example>& l = init_learner(c, as_singleline(setup_base(*all.options, all)), predict_or_learn<true>,
      predict_or_learn<false>, c->num_classes, prediction_type::multiclass);
  all.p->lp = cs_label;
  all.label_type = label_type::cs;

  l.set_finish_example(finish_example);
  all.cost_sensitive = make_base(l);
  return all.cost_sensitive;
}

using namespace ACTION_SCORE;

// TODO: passthrough for ldf
struct ldf
{
  LabelDict::label_feature_map label_features;

  size_t read_example_this_loop;
  bool is_wap;
  bool first_pass;
  bool treat_as_classifier;
  bool is_probabilities;
  float csoaa_example_t;
  vw* all;

  bool rank;
  action_scores a_s;
  uint64_t ft_offset;

  v_array<action_scores> stored_preds;

  ~ldf()
  {
    LabelDict::free_label_features(label_features);
    a_s.delete_v();
    stored_preds.delete_v();
  }
};

bool ec_is_label_definition(example& ec)  // label defs look like "0:___" or just "label:___"
{
  if (ec.indices.empty())
    return false;
  if (ec.indices[0] != 'l')
    return false;
  v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
  for (auto const& cost : costs)
    if ((cost.class_index != 0) || (cost.x <= 0.))
      return false;
  return true;
}

bool ec_seq_is_label_definition(multi_ex& ec_seq)
{
  if (ec_seq.empty())
    return false;
  bool is_lab = ec_is_label_definition(*ec_seq[0]);
  for (size_t i = 1; i < ec_seq.size(); i++)
    if (is_lab != ec_is_label_definition(*ec_seq[i]))
      THROW("error: mixed label definition and examples in ldf data!");
  return is_lab;
}

bool ec_seq_has_label_definition(multi_ex& ec_seq)
{
  return std::any_of(ec_seq.cbegin(), ec_seq.cend(), [](example* ec) { return ec_is_label_definition(*ec); });
}

inline bool cmp_wclass_ptr(const COST_SENSITIVE::wclass* a, const COST_SENSITIVE::wclass* b) { return a->x < b->x; }

void compute_wap_values(std::vector<COST_SENSITIVE::wclass*> costs)
{
  std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
  costs[0]->wap_value = 0.;
  for (size_t i = 1; i < costs.size(); i++)
    costs[i]->wap_value = costs[i - 1]->wap_value + (costs[i]->x - costs[i - 1]->x) / (float)i;
}

// Substract a given feature from example ec.
// Rather than finding the corresponding namespace and feature in ec,
// add a new feature with opposite value (but same index) to ec to a special wap_ldf_namespace.
// This is faster and allows fast undo in unsubtract_example().
void subtract_feature(example& ec, float feature_value_x, uint64_t weight_index)
{
  ec.feature_space[wap_ldf_namespace].push_back(-feature_value_x, weight_index);
}

// Iterate over all features of ecsub including quadratic and cubic features and subtract them from ec.
void subtract_example(vw& all, example* ec, example* ecsub)
{
  features& wap_fs = ec->feature_space[wap_ldf_namespace];
  wap_fs.sum_feat_sq = 0;
  GD::foreach_feature<example&, uint64_t, subtract_feature>(all, *ecsub, *ec);
  ec->indices.push_back(wap_ldf_namespace);
  ec->num_features += wap_fs.size();
  ec->total_sum_feat_sq += wap_fs.sum_feat_sq;
}

void unsubtract_example(example* ec)
{
  if (ec->indices.empty())
  {
    std::cerr << "internal error (bug): trying to unsubtract_example, but there are no namespaces!" << std::endl;
    return;
  }

  if (ec->indices.last() != wap_ldf_namespace)
  {
    std::cerr
        << "internal error (bug): trying to unsubtract_example, but either it wasn't added, or something was added "
           "after and not removed!"
        << std::endl;
    return;
  }

  features& fs = ec->feature_space[wap_ldf_namespace];
  ec->num_features -= fs.size();
  ec->total_sum_feat_sq -= fs.sum_feat_sq;
  fs.clear();
  ec->indices.decr();
}

void make_single_prediction(ldf& data, single_learner& base, example& ec)
{
  COST_SENSITIVE::label ld = ec.l.cs;
  label_data simple_label;
  simple_label.initial = 0.;
  simple_label.label = FLT_MAX;

  LabelDict::add_example_namespace_from_memory(data.label_features, ec, ld.costs[0].class_index);

  ec.l.simple = simple_label;
  uint64_t old_offset = ec.ft_offset;
  ec.ft_offset = data.ft_offset;
  base.predict(ec);  // make a prediction
  ec.ft_offset = old_offset;
  ld.costs[0].partial_prediction = ec.partial_prediction;

  LabelDict::del_example_namespace_from_memory(data.label_features, ec, ld.costs[0].class_index);
  ec.l.cs = ld;
}

bool test_ldf_sequence(ldf& data, multi_ex& ec_seq)
{
  bool isTest;
  if (ec_seq.empty())
    isTest = true;
  else
    isTest = COST_SENSITIVE::cs_label.test_label(&ec_seq[0]->l);
  for (const auto& ec : ec_seq)
  {
    // Each sub-example must have just one cost
    assert(ec->l.cs.costs.size() == 1);

    if (COST_SENSITIVE::cs_label.test_label(&ec->l) != isTest)
    {
      isTest = true;
      data.all->trace_message << "warning: ldf example has mix of train/test data; assuming test" << std::endl;
    }
  }
  return isTest;
}

void do_actual_learning_wap(ldf& data, single_learner& base, multi_ex& ec_seq)
{
  size_t K = ec_seq.size();
  std::vector<COST_SENSITIVE::wclass*> all_costs;
  for (const auto& example : ec_seq) all_costs.push_back(&example->l.cs.costs[0]);
  compute_wap_values(all_costs);

  for (size_t k1 = 0; k1 < K; k1++)
  {
    example* ec1 = ec_seq[k1];

    // save original variables
    COST_SENSITIVE::label save_cs_label = ec1->l.cs;
    label_data& simple_label = ec1->l.simple;

    v_array<COST_SENSITIVE::wclass> costs1 = save_cs_label.costs;
    if (costs1[0].class_index == (uint32_t)-1)
      continue;

    LabelDict::add_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    for (size_t k2 = k1 + 1; k2 < K; k2++)
    {
      example* ec2 = ec_seq[k2];
      v_array<COST_SENSITIVE::wclass> costs2 = ec2->l.cs.costs;

      if (costs2[0].class_index == (uint32_t)-1)
        continue;
      float value_diff = fabs(costs2[0].wap_value - costs1[0].wap_value);
      // float value_diff = fabs(costs2[0].x - costs1[0].x);
      if (value_diff < 1e-6)
        continue;

      LabelDict::add_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);

      // learn
      simple_label.initial = 0.;
      simple_label.label = (costs1[0].x < costs2[0].x) ? -1.0f : 1.0f;
      float old_weight = ec1->weight;
      ec1->weight = value_diff;
      ec1->partial_prediction = 0.;
      subtract_example(*data.all, ec1, ec2);
      uint64_t old_offset = ec1->ft_offset;
      ec1->ft_offset = data.ft_offset;
      base.learn(*ec1);
      ec1->ft_offset = old_offset;
      ec1->weight = old_weight;
      unsubtract_example(ec1);

      LabelDict::del_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);
    }
    LabelDict::del_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    // restore original cost-sensitive label, sum of importance weights
    ec1->l.cs = save_cs_label;
    // TODO: What about partial_prediction? See do_actual_learning_oaa.
  }
}

void do_actual_learning_oaa(ldf& data, single_learner& base, multi_ex& ec_seq)
{
  float min_cost = FLT_MAX;
  float max_cost = -FLT_MAX;

  for (const auto& example : ec_seq)
  {
    float ec_cost = example->l.cs.costs[0].x;
    if (ec_cost < min_cost)
      min_cost = ec_cost;
    if (ec_cost > max_cost)
      max_cost = ec_cost;
  }

  for (const auto& ec : ec_seq)
  {
    // save original variables
    label save_cs_label = ec->l.cs;
    v_array<COST_SENSITIVE::wclass> costs = save_cs_label.costs;

    // build example for the base learner
    label_data simple_label;

    simple_label.initial = 0.;
    float old_weight = ec->weight;
    if (!data.treat_as_classifier)  // treat like regression
      simple_label.label = costs[0].x;
    else  // treat like classification
    {
      if (costs[0].x <= min_cost)
      {
        simple_label.label = -1.;
        ec->weight = old_weight * (max_cost - min_cost);
      }
      else
      {
        simple_label.label = 1.;
        ec->weight = old_weight * (costs[0].x - min_cost);
      }
    }
    ec->l.simple = simple_label;

    // learn
    LabelDict::add_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
    uint64_t old_offset = ec->ft_offset;
    ec->ft_offset = data.ft_offset;
    base.learn(*ec);
    ec->ft_offset = old_offset;
    LabelDict::del_example_namespace_from_memory(data.label_features, *ec, costs[0].class_index);
    ec->weight = old_weight;

    // restore original cost-sensitive label, sum of importance weights and partial_prediction
    ec->l.cs = save_cs_label;
    ec->partial_prediction = costs[0].partial_prediction;
  }
}

/*
 * The begining of the multi_ex sequence may be labels.  Process those
 * and return the start index of the un-processed examples
 */
multi_ex process_labels(ldf& data, const multi_ex& ec_seq_all);

/*
 * 1) process all labels at first
 * 2) verify no labels in the middle of data
 * 3) learn_or_predict(data) with rest
 */
template <bool is_learn>
void do_actual_learning(ldf& data, single_learner& base, multi_ex& ec_seq_all)
{
  if (ec_seq_all.empty())
    return;  // nothing to do

  data.ft_offset = ec_seq_all[0]->ft_offset;

  // handle label definitions
  auto ec_seq = process_labels(data, ec_seq_all);
  if (ec_seq.empty())
    return;  // nothing more to do

  // Ensure there are no more labels
  // (can be done in existing loops later but as a side effect learning
  //    will happen with bad example)
  if (ec_seq_has_label_definition(ec_seq))
  {
    THROW("error: label definition encountered in data block");
  }

  /////////////////////// add headers
  uint32_t K = (uint32_t)ec_seq.size();

  bool isTest = test_ldf_sequence(data, ec_seq);
  /////////////////////// do prediction
  uint32_t predicted_K = 0;
  if (data.rank)
  {
    data.a_s.clear();
    data.stored_preds.clear();
    for (uint32_t k = 0; k < K; k++)
    {
      example* ec = ec_seq[k];
      data.stored_preds.push_back(ec->pred.a_s);
      make_single_prediction(data, base, *ec);
      action_score s;
      s.score = ec->partial_prediction;
      s.action = k;
      data.a_s.push_back(s);
    }

    qsort((void*)data.a_s.begin(), data.a_s.size(), sizeof(action_score), score_comp);
  }
  else
  {
    float min_score = FLT_MAX;
    for (uint32_t k = 0; k < K; k++)
    {
      example* ec = ec_seq[k];
      make_single_prediction(data, base, *ec);
      if (ec->partial_prediction < min_score)
      {
        min_score = ec->partial_prediction;
        predicted_K = k;
      }
    }
  }

  /////////////////////// learn
  if (is_learn && !isTest)
  {
    if (data.is_wap)
      do_actual_learning_wap(data, base, ec_seq);
    else
      do_actual_learning_oaa(data, base, ec_seq);
  }

  if (data.rank)
  {
    data.stored_preds[0].clear();
    for (size_t k = 0; k < K; k++)
    {
      ec_seq[k]->pred.a_s = data.stored_preds[k];
      ec_seq[0]->pred.a_s.push_back(data.a_s[k]);
    }
  }
  else
  {
    // Mark the predicted subexample with its class_index, all other with 0
    for (size_t k = 0; k < K; k++)
    {
      if (k == predicted_K)
        ec_seq[k]->pred.multiclass = ec_seq[k]->l.cs.costs[0].class_index;
      else
        ec_seq[k]->pred.multiclass = 0;
    }
  }

  ////////////////////// compute probabilities
  if (data.is_probabilities)
  {
    float sum_prob = 0;
    for (const auto& example : ec_seq)
    {
      // probability(correct_class) = 1 / (1+exp(-score)), where score is higher for better classes,
      // but partial_prediction is lower for better classes (we are predicting the cost),
      // so we need to take score = -partial_prediction,
      // thus probability(correct_class) = 1 / (1+exp(-(-partial_prediction)))
      float prob = 1.f / (1.f + correctedExp(example->partial_prediction));
      example->pred.prob = prob;
      sum_prob += prob;
    }
    // make sure that the probabilities sum up (exactly) to one
    for (const auto& example : ec_seq)
    {
      example->pred.prob /= sum_prob;
    }
  }
}

void global_print_newline(vw& all)
{
  char temp[1];
  temp[0] = '\n';
  for (int f : all.final_prediction_sink)
  {
    ssize_t t;
    t = io_buf::write_file_or_socket(f, temp, 1);
    if (t != 1)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void output_example(vw& all, example& ec, bool& hit_loss, multi_ex* ec_seq, ldf& data)
{
  label& ld = ec.l.cs;
  v_array<COST_SENSITIVE::wclass> costs = ld.costs;

  if (example_is_newline(ec))
    return;
  if (ec_is_label_definition(ec))
    return;

  all.sd->total_features += ec.num_features;

  float loss = 0.;

  uint32_t predicted_class;
  if (data.is_probabilities)
  {
    // predicted_K was already computed in do_actual_learning(),
    // but we cannot store it in ec.pred union because we store ec.pred.prob there.
    // So we must compute it again.
    uint32_t predicted_K = 0;
    float min_score = FLT_MAX;
    for (size_t k = 0; k < ec_seq->size(); k++)
    {
      example* ec_k = (*ec_seq)[k];
      if (ec_k->partial_prediction < min_score)
      {
        min_score = ec_k->partial_prediction;
        predicted_K = (uint32_t)k;
      }
    }
    predicted_class = (*ec_seq)[predicted_K]->l.cs.costs[0].class_index;
  }
  else
    predicted_class = ec.pred.multiclass;

  if (!COST_SENSITIVE::cs_label.test_label(&ec.l))
  {
    for (auto const& cost : costs)
    {
      if (hit_loss)
        break;
      if (predicted_class == cost.class_index)
      {
        loss = cost.x;
        hit_loss = true;
      }
    }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }

  for (int sink : all.final_prediction_sink)
    all.print(sink, data.is_probabilities ? ec.pred.prob : (float)ec.pred.multiclass, 0, ec.tag);

  if (all.raw_prediction > 0)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::cs_label.test_label(&ec.l), ec, ec_seq, false, predicted_class);
}

void output_rank_example(vw& all, example& head_ec, bool& hit_loss, multi_ex* ec_seq)
{
  label& ld = head_ec.l.cs;
  v_array<COST_SENSITIVE::wclass> costs = ld.costs;

  if (example_is_newline(head_ec))
    return;
  if (ec_is_label_definition(head_ec))
    return;

  all.sd->total_features += head_ec.num_features;

  float loss = 0.;
  v_array<action_score>& preds = head_ec.pred.a_s;

  if (!COST_SENSITIVE::cs_label.test_label(&head_ec.l))
  {
    size_t idx = 0;
    for (example* ex : *ec_seq)
    {
      if (hit_loss)
        break;
      if (preds[0].action == idx)
      {
        loss = ex->l.cs.costs[0].x;
        hit_loss = true;
      }
      idx++;
    }
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
    assert(loss >= 0);
  }

  for (int sink : all.final_prediction_sink) print_action_score(sink, head_ec.pred.a_s, head_ec.tag);

  if (all.raw_prediction > 0)
  {
    std::string outputString;
    std::stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    // outputStringStream << std::endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), head_ec.tag);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::cs_label.test_label(&head_ec.l), head_ec, ec_seq, true, 0);
}

void output_example_seq(vw& all, ldf& data, multi_ex& ec_seq)
{
  size_t K = ec_seq.size();
  if ((K > 0) && !ec_seq_is_label_definition(ec_seq))
  {
    if (test_ldf_sequence(data, ec_seq))
      all.sd->weighted_unlabeled_examples += ec_seq[0]->weight;
    else
      all.sd->weighted_labeled_examples += ec_seq[0]->weight;
    all.sd->example_number++;

    bool hit_loss = false;
    if (data.rank)
      output_rank_example(all, **(ec_seq.begin()), hit_loss, &(ec_seq));
    else
      for (example* ec : ec_seq) output_example(all, *ec, hit_loss, &(ec_seq), data);

    if (all.raw_prediction > 0)
    {
      v_array<char> empty = {nullptr, nullptr, nullptr, 0};
      all.print_text(all.raw_prediction, "", empty);
    }

    if (data.is_probabilities)
    {
      float min_cost = FLT_MAX;
      size_t correct_class_k = 0;

      for (size_t k = 0; k < K; k++)
      {
        float ec_cost = ec_seq[k]->l.cs.costs[0].x;
        if (ec_cost < min_cost)
        {
          min_cost = ec_cost;
          correct_class_k = k;
        }
      }

      float multiclass_log_loss = 999;  // -log(0) = plus infinity
      float correct_class_prob = ec_seq[correct_class_k]->pred.prob;
      if (correct_class_prob > 0)
        multiclass_log_loss = -log(correct_class_prob);

      // TODO: How to detect if we should update holdout or normal loss?
      // (ec.test_only) OR (COST_SENSITIVE::example_is_test(ec))
      // What should be the "ec"? data.ec_seq[0]?
      // Based on parse_args.cc (where "average multiclass log loss") is printed,
      // I decided to try yet another way: (!all.holdout_set_off).
      if (!all.holdout_set_off)
        all.sd->holdout_multiclass_log_loss += multiclass_log_loss;
      else
        all.sd->multiclass_log_loss += multiclass_log_loss;
    }
  }
}

void end_pass(ldf& data) { data.first_pass = false; }

void finish_multiline_example(vw& all, ldf& data, multi_ex& ec_seq)
{
  if (!ec_seq.empty())
  {
    output_example_seq(all, data, ec_seq);
    global_print_newline(all);
  }
  VW::finish_example(all, ec_seq);
}

/*
 * Process a single example as a label.
 * Note: example should already be confirmed as a label
 */
void inline process_label(ldf& data, example* ec)
{
  auto new_fs = ec->feature_space[ec->indices[0]];
  auto& costs = ec->l.cs.costs;
  for (auto const& cost : costs)
  {
    const auto lab = (size_t)cost.x;
    LabelDict::set_label_features(data.label_features, lab, new_fs);
  }
}

/*
 * The begining of the multi_ex sequence may be labels.  Process those
 * and return the start index of the un-processed examples
 */
multi_ex process_labels(ldf& data, const multi_ex& ec_seq_all)
{
  example* ec = ec_seq_all[0];

  // check the first element, if it's not a label, return
  if (!ec_is_label_definition(*ec))
    return ec_seq_all;

  // process the first element as a label
  process_label(data, ec);

  multi_ex ret;
  size_t i = 1;
  // process the rest of the elements that are labels
  for (; i < ec_seq_all.size(); i++)
  {
    ec = ec_seq_all[i];
    if (!ec_is_label_definition(*ec))
    {
      for (size_t j = i; j < ec_seq_all.size(); j++) ret.push_back(ec_seq_all[j]);
      // return index of the first element that is not a label
      return ret;
    }

    process_label(data, ec);
  }

  // all examples were labels return size
  return ret;
}

base_learner* csldf_setup(options_i& options, vw& all)
{
  auto ld = scoped_calloc_or_throw<ldf>();

  std::string csoaa_ldf;
  std::string ldf_override;
  std::string wap_ldf;

  option_group_definition csldf_outer_options("Cost Sensitive One Against All with Label Dependent Features");
  csldf_outer_options.add(make_option("csoaa_ldf", csoaa_ldf)
                              .keep()
                              .help("Use one-against-all multiclass learning with label dependent features."));
  csldf_outer_options.add(
      make_option("ldf_override", ldf_override)
          .help("Override singleline or multiline from csoaa_ldf or wap_ldf, eg if stored in file"));
  csldf_outer_options.add(make_option("csoaa_rank", ld->rank).keep().help("Return actions sorted by score order"));
  csldf_outer_options.add(
      make_option("probabilities", ld->is_probabilities).keep().help("predict probabilites of all classes"));

  option_group_definition csldf_inner_options("Cost Sensitive One Against All with Label Dependent Features");
  csldf_inner_options.add(make_option("wap_ldf", wap_ldf)
                              .keep()
                              .help("Use weighted all-pairs multiclass learning with label dependent features.  "
                                    "Specify singleline or multiline."));

  options.add_and_parse(csldf_outer_options);
  if (!options.was_supplied("csoaa_ldf"))
  {
    options.add_and_parse(csldf_inner_options);
    if (!options.was_supplied("wap_ldf"))
    {
      return nullptr;
    }
  }

  ld->all = &all;
  ld->first_pass = true;

  std::string ldf_arg;

  if (options.was_supplied("csoaa_ldf"))
    ldf_arg = csoaa_ldf;
  else
  {
    ldf_arg = wap_ldf;
    ld->is_wap = true;
  }
  if (options.was_supplied("ldf_override"))
    ldf_arg = ldf_override;
  if (ld->rank)
    all.delete_prediction = delete_action_scores;

  all.p->lp = COST_SENSITIVE::cs_label;
  all.label_type = label_type::cs;

  ld->treat_as_classifier = false;
  if (ldf_arg == "multiline" || ldf_arg == "m")
    ld->treat_as_classifier = false;
  else if (ldf_arg == "multiline-classifier" || ldf_arg == "mc")
    ld->treat_as_classifier = true;
  else
  {
    if (all.training)
      THROW("ldf requires either m/multiline or mc/multiline-classifier");
    if ((ldf_arg == "singleline" || ldf_arg == "s") || (ldf_arg == "singleline-classifier" || ldf_arg == "sc"))
      THROW(
          "ldf requires either m/multiline or mc/multiline-classifier.  s/sc/singleline/singleline-classifier is no "
          "longer supported");
  }

  if (ld->is_probabilities)
  {
    all.sd->report_multiclass_log_loss = true;
    auto loss_function_type = all.loss->getType();
    if (loss_function_type != "logistic")
      all.trace_message << "WARNING: --probabilities should be used only with --loss_function=logistic" << std::endl;
    if (!ld->treat_as_classifier)
      all.trace_message << "WARNING: --probabilities should be used with --csoaa_ldf=mc (or --oaa)" << std::endl;
  }

  all.p->emptylines_separate_examples = true;  // TODO: check this to be sure!!!  !ld->is_singleline;

  features fs;
  ld->label_features.init(256, fs, LabelDict::size_t_eq);
  ld->label_features.get(1, 94717244);  // TODO: figure this out
  prediction_type::prediction_type_t pred_type;

  if (ld->rank)
    pred_type = prediction_type::action_scores;
  else if (ld->is_probabilities)
    pred_type = prediction_type::prob;
  else
    pred_type = prediction_type::multiclass;

  ld->read_example_this_loop = 0;
  learner<ldf, multi_ex>& l = init_learner(ld, as_singleline(setup_base(*all.options, all)), do_actual_learning<true>,
      do_actual_learning<false>, 1, pred_type);
  l.set_finish_example(finish_multiline_example);
  l.set_end_pass(end_pass);
  all.cost_sensitive = make_base(l);
  return all.cost_sensitive;
}
}  // namespace CSOAA
