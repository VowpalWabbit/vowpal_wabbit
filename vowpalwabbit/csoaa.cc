/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <errno.h>

#include "reductions.h"
#include "v_hashmap.h"
#include "label_dictionary.h"
#include "vw.h"
#include "gd.h" // GD::foreach_feature() needed in subtract_example()
#include "vw_exception.h"

using namespace std;
using namespace LEARNER;
using namespace COST_SENSITIVE;

struct csoaa
{ uint32_t num_classes;
  polyprediction* pred;
};

template<bool is_learn>
inline void inner_loop(base_learner& base, example& ec, uint32_t i, float cost,
                       uint32_t& prediction, float& score, float& partial_prediction)
{ if (is_learn)
    { ec.weight = (cost == FLT_MAX) ? 0.f : 1.f;
      ec.l.simple.label = cost;
      base.learn(ec, i-1);
    }
  else
    base.predict(ec, i-1);

  partial_prediction = ec.partial_prediction;
  if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction))
  { score = ec.partial_prediction;
    prediction = i;
  }
  add_passthrough_feature(ec, i, ec.partial_prediction);
}

#define DO_MULTIPREDICT true

template <bool is_learn>
void predict_or_learn(csoaa& c, base_learner& base, example& ec)
{ //cerr << "------------- passthrough" << endl;
  COST_SENSITIVE::label ld = ec.l.cs;
  uint32_t prediction = 1;
  float score = FLT_MAX;
  size_t pt_start = ec.passthrough ? ec.passthrough->size() : 0;
  ec.l.simple = { 0., 0., 0. };
  if (ld.costs.size() > 0)
  { for (auto& cl : ld.costs)
      inner_loop<is_learn>(base, ec, cl.class_index, cl.x, prediction, score, cl.partial_prediction);
    ec.partial_prediction = score;
  }
  else if (DO_MULTIPREDICT && !is_learn)
  { ec.l.simple = { FLT_MAX, 0.f, 0.f };
    base.multipredict(ec, 0, c.num_classes, c.pred, false);
    for (uint32_t i = 1; i <= c.num_classes; i++)
    { add_passthrough_feature(ec, i, c.pred[i-1].scalar);
      if (c.pred[i-1].scalar < c.pred[prediction-1].scalar)
        prediction = i;
    }
    ec.partial_prediction = c.pred[prediction-1].scalar;
  }
  else
  { float temp;
    for (uint32_t i = 1; i <= c.num_classes; i++)
      inner_loop<false>(base, ec, i, FLT_MAX, prediction, score, temp);
  }
  if (ec.passthrough)
  { uint64_t second_best = 0;
    float    second_best_cost = FLT_MAX;
    for (size_t i=0; i<ec.passthrough->size() - pt_start; i++)
    { float  val = ec.passthrough->values[pt_start + i];
      if ((val > ec.partial_prediction) && (val < second_best_cost))
      { second_best_cost = val;
        second_best = ec.passthrough->indicies[pt_start + i];
      }
    }
    if (second_best_cost < FLT_MAX)
    { float margin = second_best_cost - ec.partial_prediction;
      add_passthrough_feature(ec, constant*2, margin);
      add_passthrough_feature(ec, constant*2+1 + second_best, 1.);
    }
    else
      add_passthrough_feature(ec, constant*3, 1.);
  }

  ec.pred.multiclass = prediction;
  ec.l.cs = ld;
}

void finish_example(vw& all, csoaa&, example& ec)
{ output_example(all, ec);
  VW::finish_example(all, &ec);
}

void finish(csoaa& c)
{ free(c.pred);
}


base_learner* csoaa_setup(vw& all)
{ if (missing_option<size_t, true>(all, "csoaa", "One-against-all multiclass with <k> costs"))
    return nullptr;

  csoaa& c = calloc_or_throw<csoaa>();
  c.num_classes = (uint32_t)all.vm["csoaa"].as<size_t>();
  c.pred = calloc_or_throw<polyprediction>(c.num_classes);

  learner<csoaa>& l = init_learner(&c, setup_base(all), predict_or_learn<true>,
                                   predict_or_learn<false>, c.num_classes, prediction_type::multiclass);
  all.p->lp = cs_label;
  all.label_type = label_type::cs;

  l.set_finish_example(finish_example);
  l.set_finish(finish);
  all.cost_sensitive = make_base(l);
  return all.cost_sensitive;
}

using namespace ACTION_SCORE;

// TODO: passthrough for ldf
struct ldf
{ v_array<example*> ec_seq;
  LabelDict::label_feature_map label_features;

  size_t read_example_this_loop;
  bool need_to_clear;
  bool is_wap;
  bool first_pass;
  bool treat_as_classifier;
  bool is_singleline;
  bool is_probabilities;
  float csoaa_example_t;
  vw* all;

  bool rank;
  action_scores a_s;
  uint64_t ft_offset;

  v_array<action_scores > stored_preds;
};

bool ec_is_label_definition(example& ec) // label defs look like "0:___" or just "label:___"
{ if (ec.indices.size() < 1) return false;
  if (ec.indices[0] != 'l') return false;
  v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
  for (size_t j=0; j<costs.size(); j++)
    if ((costs[j].class_index != 0) || (costs[j].x <= 0.)) return false;
  return true;
}

bool ec_seq_is_label_definition(v_array<example*>ec_seq)
{ if (ec_seq.size() == 0) return false;
  bool is_lab = ec_is_label_definition(*ec_seq[0]);
  for (size_t i=1; i<ec_seq.size(); i++)
  { if (is_lab != ec_is_label_definition(*ec_seq[i]))
    { if (!((i == ec_seq.size()-1) && (example_is_newline(*ec_seq[i]))))
        THROW("error: mixed label definition and examples in ldf data!");
    }
  }
  return is_lab;
}

inline bool cmp_wclass_ptr(const COST_SENSITIVE::wclass* a, const COST_SENSITIVE::wclass* b) { return a->x < b->x; }

void compute_wap_values(vector<COST_SENSITIVE::wclass*> costs)
{ std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
  costs[0]->wap_value = 0.;
  for (size_t i=1; i<costs.size(); i++)
    costs[i]->wap_value = costs[i-1]->wap_value + (costs[i]->x - costs[i-1]->x) / (float)i;
}

// Substract a given feature from example ec.
// Rather than finding the corresponding namespace and feature in ec,
// add a new feature with opposite value (but same index) to ec to a special wap_ldf_namespace.
// This is faster and allows fast undo in unsubtract_example().
void subtract_feature(example& ec, float feature_value_x, uint64_t weight_index)
{ ec.feature_space[wap_ldf_namespace].push_back(-feature_value_x, weight_index); }

// Iterate over all features of ecsub including quadratic and cubic features and subtract them from ec.
void subtract_example(vw& all, example *ec, example *ecsub)
{ features& wap_fs = ec->feature_space[wap_ldf_namespace];
  wap_fs.sum_feat_sq = 0;
  GD::foreach_feature<example&, uint64_t, subtract_feature>(all, *ecsub, *ec);
  ec->indices.push_back(wap_ldf_namespace);
  ec->num_features += wap_fs.size();
  ec->total_sum_feat_sq += wap_fs.sum_feat_sq;
}

void unsubtract_example(example *ec)
{ if (ec->indices.size() == 0)
  { cerr << "internal error (bug): trying to unsubtract_example, but there are no namespaces!" << endl;
    return;
  }

  if (ec->indices.last() != wap_ldf_namespace)
  { cerr << "internal error (bug): trying to unsubtract_example, but either it wasn't added, or something was added after and not removed!" << endl;
    return;
  }

  features& fs = ec->feature_space[wap_ldf_namespace];
  ec->num_features -= fs.size();
  ec->total_sum_feat_sq -= fs.sum_feat_sq;
  fs.erase();
  ec->indices.decr();
}

void make_single_prediction(ldf& data, base_learner& base, example& ec)
{ COST_SENSITIVE::label ld = ec.l.cs;
  label_data simple_label;
  simple_label.initial = 0.;
  simple_label.label = FLT_MAX;

  LabelDict::add_example_namespace_from_memory(data.label_features, ec, ld.costs[0].class_index);

  ec.l.simple = simple_label;
  uint64_t old_offset = ec.ft_offset;
  ec.ft_offset = data.ft_offset;
  base.predict(ec); // make a prediction
  ec.ft_offset = old_offset;
  ld.costs[0].partial_prediction = ec.partial_prediction;

  LabelDict::del_example_namespace_from_memory(data.label_features, ec, ld.costs[0].class_index);
  ec.l.cs = ld;
}

bool test_ldf_sequence(ldf& data, size_t start_K)
{ bool isTest;
  if (start_K == data.ec_seq.size())
    isTest = true;
  else
    isTest = COST_SENSITIVE::example_is_test(*data.ec_seq[start_K]);
  for (size_t k=start_K; k<data.ec_seq.size(); k++)
  { example *ec = data.ec_seq[k];
    // Each sub-example must have just one cost
    assert(ec->l.cs.costs.size()==1);

    if (COST_SENSITIVE::example_is_test(*ec) != isTest)
    { isTest = true;
      data.all->trace_message << "warning: ldf example has mix of train/test data; assuming test" << endl;
    }
    if (ec_is_example_header(*ec))
      THROW("warning: example headers at position " << k << ": can only have in initial position!");
  }
  return isTest;
}

void do_actual_learning_wap(ldf& data, base_learner& base, size_t start_K)
{ size_t K = data.ec_seq.size();
  vector<COST_SENSITIVE::wclass*> all_costs;
  for (size_t k=start_K; k<K; k++)
    all_costs.push_back(&data.ec_seq[k]->l.cs.costs[0]);
  compute_wap_values(all_costs);

  for (size_t k1=start_K; k1<K; k1++)
  { example *ec1 = data.ec_seq[k1];

    // save original variables
    COST_SENSITIVE::label   save_cs_label = ec1->l.cs;
    label_data& simple_label = ec1->l.simple;

    v_array<COST_SENSITIVE::wclass> costs1 = save_cs_label.costs;
    if (costs1[0].class_index == (uint32_t)-1) continue;

    LabelDict::add_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    for (size_t k2=k1+1; k2<K; k2++)
    { example *ec2 = data.ec_seq[k2];
      v_array<COST_SENSITIVE::wclass> costs2 = ec2->l.cs.costs;

      if (costs2[0].class_index == (uint32_t)-1) continue;
      float value_diff = fabs(costs2[0].wap_value - costs1[0].wap_value);
      //float value_diff = fabs(costs2[0].x - costs1[0].x);
      if (value_diff < 1e-6)
        continue;

      LabelDict::add_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);

      // learn
      simple_label.initial = 0.;
      simple_label.label = (costs1[0].x < costs2[0].x) ? -1.0f : 1.0f;
      ec1->weight = value_diff;
      ec1->partial_prediction = 0.;
      subtract_example(*data.all, ec1, ec2);
      uint64_t old_offset = ec1->ft_offset;
      ec1->ft_offset = data.ft_offset;
      base.learn(*ec1);
      ec1->ft_offset = old_offset;
      unsubtract_example(ec1);

      LabelDict::del_example_namespace_from_memory(data.label_features, *ec2, costs2[0].class_index);
    }
    LabelDict::del_example_namespace_from_memory(data.label_features, *ec1, costs1[0].class_index);

    // restore original cost-sensitive label, sum of importance weights
    ec1->l.cs = save_cs_label;
    // TODO: What about partial_prediction? See do_actual_learning_oaa.
  }
}

void do_actual_learning_oaa(ldf& data, base_learner& base, size_t start_K)
{ size_t K = data.ec_seq.size();
  float  min_cost  = FLT_MAX;
  float  max_cost  = -FLT_MAX;

  for (size_t k=start_K; k<K; k++)
  { float ec_cost = data.ec_seq[k]->l.cs.costs[0].x;
    if (ec_cost < min_cost) min_cost = ec_cost;
    if (ec_cost > max_cost) max_cost = ec_cost;
  }

  for (size_t k=start_K; k<K; k++)
  { example *ec = data.ec_seq[k];

    // save original variables
    label save_cs_label = ec->l.cs;
    v_array<COST_SENSITIVE::wclass> costs = save_cs_label.costs;

    // build example for the base learner
    label_data simple_label;

    simple_label.initial = 0.;
    float old_weight = ec->weight;
    if (!data.treat_as_classifier)   // treat like regression
      simple_label.label = costs[0].x;
    else     // treat like classification
    { if (costs[0].x <= min_cost)
      { simple_label.label = -1.;
        ec->weight = old_weight * (max_cost - min_cost);
      }
      else
      { simple_label.label = 1.;
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

template <bool is_learn>
void do_actual_learning(ldf& data, base_learner& base)
{ if (data.ec_seq.size() <= 0) return;  // nothing to do
  /////////////////////// handle label definitions

  if (ec_seq_is_label_definition(data.ec_seq))
  { for (size_t i=0; i<data.ec_seq.size(); i++)
    { features new_fs = data.ec_seq[i]->feature_space[data.ec_seq[i]->indices[0]];

      v_array<COST_SENSITIVE::wclass>& costs = data.ec_seq[i]->l.cs.costs;
      for (size_t j=0; j<costs.size(); j++)
      { size_t lab = (size_t)costs[j].x;
        LabelDict::set_label_features(data.label_features, lab, new_fs);
      }
    }
    return;
  }

  /////////////////////// add headers
  uint32_t K = (uint32_t)data.ec_seq.size();
  uint32_t start_K = 0;

  if (ec_is_example_header(*data.ec_seq[0]))
  { start_K = 1;
    for (uint32_t k=1; k<K; k++)
      LabelDict::add_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);
  }
  bool isTest = test_ldf_sequence(data, start_K);
  /////////////////////// do prediction
  uint32_t predicted_K = start_K;
  if(data.rank)
  { data.a_s.erase();
    data.stored_preds.erase();
    if (start_K > 0)
      data.stored_preds.push_back(data.ec_seq[0]->pred.a_s);
    for (uint32_t k=start_K; k<K; k++)
    { data.stored_preds.push_back(data.ec_seq[k]->pred.a_s);
      example *ec = data.ec_seq[k];
      make_single_prediction(data, base, *ec);
      action_score s;
      s.score = ec->partial_prediction;
      s.action = k - start_K;
      data.a_s.push_back(s);
    }

    qsort((void*) data.a_s.begin(), data.a_s.size(), sizeof(action_score), score_comp);
  }
  else
  { float  min_score = FLT_MAX;
    for (uint32_t k=start_K; k<K; k++)
    { example *ec = data.ec_seq[k];
      make_single_prediction(data, base, *ec);
      if (ec->partial_prediction < min_score)
      { min_score = ec->partial_prediction;
        predicted_K = k;
      }
    }
  }

  /////////////////////// learn
  if (is_learn && !isTest)
  { if (data.is_wap) do_actual_learning_wap(data, base, start_K);
    else             do_actual_learning_oaa(data, base, start_K);
  }

  if(data.rank)
  { data.stored_preds[0].erase();
    if (start_K > 0)
    { data.ec_seq[0]->pred.a_s = data.stored_preds[0];
    }
    for (size_t k=start_K; k<K; k++)
    { data.ec_seq[k]->pred.a_s = data.stored_preds[k];
      data.ec_seq[0]->pred.a_s.push_back(data.a_s[k-start_K]);
    }
  }
  else
  { // Mark the predicted subexample with its class_index, all other with 0
    for (size_t k=start_K; k<K; k++)
      { if (k == predicted_K)
	  data.ec_seq[k]->pred.multiclass =  data.ec_seq[k]->l.cs.costs[0].class_index;
      else
        data.ec_seq[k]->pred.multiclass =  0;
    }
  }
  /////////////////////// remove header
  if (start_K > 0)
    for (size_t k=1; k<K; k++)
      LabelDict::del_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);

  ////////////////////// compute probabilities
  if (data.is_probabilities)
  { float sum_prob = 0;
    for (size_t k=start_K; k<K; k++)
    { // probability(correct_class) = 1 / (1+exp(-score)), where score is higher for better classes,
      // but partial_prediction is lower for better classes (we are predicting the cost),
      // so we need to take score = -partial_prediction,
      // thus probability(correct_class) = 1 / (1+exp(-(-partial_prediction)))
      float prob = 1.f / (1.f + exp(data.ec_seq[k]->partial_prediction));
      data.ec_seq[k]->pred.prob = prob;
      sum_prob += prob;
    }
    // make sure that the probabilities sum up (exactly) to one
    for (size_t k=start_K; k<K; k++)
    { data.ec_seq[k]->pred.prob /= sum_prob;
    }
  }
}

void global_print_newline(vw& all)
{ char temp[1];
  temp[0] = '\n';
  for (size_t i=0; i<all.final_prediction_sink.size(); i++)
  { int f = all.final_prediction_sink[i];
    ssize_t t;
    t = io_buf::write_file_or_socket(f, temp, 1);
    if (t != 1)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void output_example(vw& all, example& ec, bool& hit_loss, v_array<example*>* ec_seq, ldf& data)
{ label& ld = ec.l.cs;
  v_array<COST_SENSITIVE::wclass> costs = ld.costs;

  if (example_is_newline(ec)) return;
  if (ec_is_example_header(ec)) return;
  if (ec_is_label_definition(ec)) return;

  all.sd->total_features += ec.num_features;

  float loss = 0.;

  uint32_t predicted_class;
  if (data.is_probabilities)
  { // predicted_K was already computed in do_actual_learning(),
    // but we cannot store it in ec.pred union because we store ec.pred.prob there.
    // So we must compute it again.
    size_t start_K = 0;
    size_t K = ec_seq->size();
    if (ec_is_example_header(*ec_seq->get(0)))
      start_K = 1;
    uint32_t predicted_K = (uint32_t)start_K;
    float  min_score = FLT_MAX;
    for (size_t k=start_K; k<K; k++)
    { example *ec = ec_seq->get(k);
      if (ec->partial_prediction < min_score)
      { min_score = ec->partial_prediction;
        predicted_K = (uint32_t)k;
      }
    }
    predicted_class = ec_seq->get(predicted_K)->l.cs.costs[0].class_index;
  }
  else
    predicted_class = ec.pred.multiclass;

  if (!COST_SENSITIVE::example_is_test(ec))
  { for (size_t j=0; j<costs.size(); j++)
    { if (hit_loss) break;
      if (predicted_class == costs[j].class_index)
      { loss = costs[j].x;
        hit_loss = true;
      }
    }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }

  for (int sink : all.final_prediction_sink)
    all.print(sink, data.is_probabilities ? ec.pred.prob : (float)ec.pred.multiclass, 0, ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    //outputStringStream << endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::example_is_test(ec), ec, ec_seq, false, predicted_class);
}

void output_rank_example(vw& all, example& head_ec, bool& hit_loss, v_array<example*>* ec_seq)
{ label& ld = head_ec.l.cs;
  v_array<COST_SENSITIVE::wclass> costs = ld.costs;

  if (example_is_newline(head_ec)) return;
  if (ec_is_label_definition(head_ec)) return;

  all.sd->total_features += head_ec.num_features;

  float loss = 0.;
  v_array<action_score>& preds = head_ec.pred.a_s;

  if (!COST_SENSITIVE::example_is_test(head_ec))
  { size_t idx = 0;
    for (example* ex : *ec_seq)
    { if(ec_is_example_header(*ex)) continue;
      if (hit_loss) break;
      if (preds[0].action == idx)
      { loss = ex->l.cs.costs[0].x;
        hit_loss = true;
      }
      idx++;
    }
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
    assert(loss >= 0);
  }

  for (int sink : all.final_prediction_sink)
    print_action_score(sink, head_ec.pred.a_s, head_ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
    }
    //outputStringStream << endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), head_ec.tag);
  }

  COST_SENSITIVE::print_update(all, COST_SENSITIVE::example_is_test(head_ec), head_ec, ec_seq, true, 0);
}

void output_example_seq(vw& all, ldf& data)
{ size_t K = data.ec_seq.size();
  if ((K > 0) && !ec_seq_is_label_definition(data.ec_seq))
  {
    size_t start_K = 0;
    if (ec_is_example_header(*(data.ec_seq[0])))
      start_K = 1;
    if (test_ldf_sequence(data, start_K))
      all.sd->weighted_unlabeled_examples += 1;
    else
      all.sd->weighted_labeled_examples += 1;
    all.sd->example_number++;

    bool hit_loss = false;
    if(data.rank)
      output_rank_example(all, **(data.ec_seq.begin()), hit_loss, &(data.ec_seq));
    else
      for (example* ec : data.ec_seq)
        output_example(all, *ec, hit_loss, &(data.ec_seq), data);

    if (!data.is_singleline && (all.raw_prediction > 0))
    { v_array<char> empty = { nullptr, nullptr, nullptr, 0 };
      all.print_text(all.raw_prediction, "", empty);
    }

    if (data.is_probabilities)
    { size_t start_K = ec_is_example_header(*data.ec_seq[0]) ? 1 : 0;
      float  min_cost = FLT_MAX;
      size_t correct_class_k = start_K;

      for (size_t k=start_K; k<K; k++)
      { float ec_cost = data.ec_seq[k]->l.cs.costs[0].x;
        if (ec_cost < min_cost)
        { min_cost = ec_cost;
          correct_class_k = k;
        }
      }

      float multiclass_log_loss = 999; // -log(0) = plus infinity
      float correct_class_prob = data.ec_seq[correct_class_k]->pred.prob;
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

void clear_seq_and_finish_examples(vw& all, ldf& data)
{ if (data.ec_seq.size() > 0)
    for (auto ec : data.ec_seq)
      if (ec->in_use)
        VW::finish_example(all, ec);
  data.ec_seq.erase();
}

void end_pass(ldf& data)
{ data.first_pass = false;
}

void finish_singleline_example(vw& all, ldf& data, example& ec)
{ if (! ec_is_label_definition(ec))
  {
    if (COST_SENSITIVE::example_is_test(ec))
      all.sd->weighted_unlabeled_examples += ec.weight;
    else
      all.sd->weighted_labeled_examples += ec.weight;
    all.sd->example_number++;
  }
  bool hit_loss = false;
  output_example(all, ec, hit_loss, nullptr, data);
  VW::finish_example(all, &ec);
}

void finish_multiline_example(vw& all, ldf& data, example& ec)
{ if (data.need_to_clear)
  { if (data.ec_seq.size() > 0)
    { output_example_seq(all, data);
      global_print_newline(all);
    }
    clear_seq_and_finish_examples(all, data);
    data.need_to_clear = false;
    if (ec.in_use) VW::finish_example(all, &ec);
  }
}

void end_examples(ldf& data)
{ if (data.need_to_clear)
    data.ec_seq.erase();
}


void finish(ldf& data)
{ data.ec_seq.delete_v();
  LabelDict::free_label_features(data.label_features);
  data.a_s.delete_v();
  data.stored_preds.delete_v();
}

template <bool is_learn>
void predict_or_learn(ldf& data, base_learner& base, example &ec)
{ vw* all = data.all;
  data.ft_offset = ec.ft_offset;
  bool is_test_ec = COST_SENSITIVE::example_is_test(ec);
  bool need_to_break = data.ec_seq.size() >= all->p->ring_size - 2;

  // singleline is used by library/ezexample_predict
  if (data.is_singleline)
  { assert(is_test_ec); // Only test examples are supported with singleline
    assert(ec.l.cs.costs.size() > 0); // headers not allowed with singleline
    make_single_prediction(data, base, ec);
  }
  else if (ec_is_label_definition(ec))
  { if (data.ec_seq.size() > 0)
      THROW("error: label definition encountered in data block");

    data.ec_seq.push_back(&ec);
    do_actual_learning<is_learn>(data, base);
    data.need_to_clear = true;
  }
  else if ((example_is_newline(ec) && is_test_ec) || need_to_break)
  { if (need_to_break && data.first_pass)
      data.all->trace_message << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << endl;
    do_actual_learning<is_learn>(data, base);
    data.need_to_clear = true;
  }
  else
  { if (data.need_to_clear)    // should only happen if we're NOT driving
    { data.ec_seq.erase();
      data.need_to_clear = false;
    }
    data.ec_seq.push_back(&ec);
  }
}

base_learner* csldf_setup(vw& all)
{ if (missing_option<string, true>(all, "csoaa_ldf", "Use one-against-all multiclass learning with label dependent features.  Specify singleline or multiline.")
      && missing_option<string, true>(all, "wap_ldf", "Use weighted all-pairs multiclass learning with label dependent features.  Specify singleline or multiline."))
    return nullptr;
  new_options(all, "LDF Options")
  ("ldf_override", po::value<string>(), "Override singleline or multiline from csoaa_ldf or wap_ldf, eg if stored in file")
  ("csoaa_rank", "Return actions sorted by score order")
  ("probabilities", "predict probabilites of all classes");
  add_options(all);

  po::variables_map& vm = all.vm;
  ldf& ld = calloc_or_throw<ldf>();

  ld.all = &all;
  ld.need_to_clear = true;
  ld.first_pass = true;

  string ldf_arg;

  if( vm.count("csoaa_ldf") )
  { ldf_arg = vm["csoaa_ldf"].as<string>();
  }
  else
  { ldf_arg = vm["wap_ldf"].as<string>();
    ld.is_wap = true;
  }
  if ( vm.count("ldf_override") )
    ldf_arg = vm["ldf_override"].as<string>();
  if (vm.count("csoaa_rank"))
  { ld.rank = true;
    *all.file_options << " --csoaa_rank";
    all.delete_prediction = delete_action_scores;
  }

  all.p->lp = COST_SENSITIVE::cs_label;
  all.label_type = label_type::cs;

  ld.treat_as_classifier = false;
  ld.is_singleline = false;
  if (ldf_arg.compare("multiline") == 0 || ldf_arg.compare("m") == 0)
  { ld.treat_as_classifier = false;
  }
  else if (ldf_arg.compare("multiline-classifier") == 0 || ldf_arg.compare("mc") == 0)
  { ld.treat_as_classifier = true;
  }
  else
  { if (all.training)
    { free(&ld);
      THROW("ldf requires either m/multiline or mc/multiline-classifier, except in test-mode which can be s/sc/singleline/singleline-classifier");
    }

    if (ldf_arg.compare("singleline") == 0 || ldf_arg.compare("s") == 0)
    { ld.treat_as_classifier = false;
      ld.is_singleline = true;
    }
    else if (ldf_arg.compare("singleline-classifier") == 0 || ldf_arg.compare("sc") == 0)
    { ld.treat_as_classifier = true;
      ld.is_singleline = true;
    }
  }

  if( vm.count("probabilities") )
  { ld.is_probabilities = true;
    all.sd->report_multiclass_log_loss = true;
    *all.file_options << " --probabilities";
    if (!vm.count("loss_function") || vm["loss_function"].as<string>() != "logistic" )
      all.trace_message << "WARNING: --probabilities should be used only with --loss_function=logistic" << endl;
    if (!ld.treat_as_classifier)
      all.trace_message << "WARNING: --probabilities should be used with --csoaa_ldf=mc (or --oaa)" << endl;
  }
  else
  { ld.is_probabilities = false;
  }

  all.p->emptylines_separate_examples = true; // TODO: check this to be sure!!!  !ld.is_singleline;

  /*if (all.add_constant) {
    all.add_constant = false;
    }*/
  features fs;
  ld.label_features.init(256, fs, LabelDict::size_t_eq);
  ld.label_features.get(1, 94717244); // TODO: figure this out
  prediction_type::prediction_type_t pred_type;

  if (ld.rank)
    pred_type = prediction_type::action_scores;
  else if (ld.is_probabilities)
    pred_type = prediction_type::prob;
  else
    pred_type = prediction_type::multiclass;

  ld.read_example_this_loop = 0;
  ld.need_to_clear = false;
  learner<ldf>& l = init_learner(&ld, setup_base(all), predict_or_learn<true>, predict_or_learn<false>, 1, pred_type);
  if (ld.is_singleline)
    l.set_finish_example(finish_singleline_example);
  else
    l.set_finish_example(finish_multiline_example);
  l.set_finish(finish);
  l.set_end_examples(end_examples);
  l.set_end_pass(end_pass);
  all.cost_sensitive = make_base(l);
  return all.cost_sensitive;
}
