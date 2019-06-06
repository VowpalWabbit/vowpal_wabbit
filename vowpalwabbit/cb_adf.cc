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
#include "cb_algs.h"
#include "vw_exception.h"
#include "gen_cs_example.h"
#include "vw_versions.h"
#include "explore.h"

using namespace std;
using namespace LEARNER;
using namespace CB;
using namespace ACTION_SCORE;
using namespace GEN_CS;
using namespace CB_ALGS;
using namespace VW::config;
using namespace exploration;

namespace CB_ADF
{
struct cb_adf
{
  vw* all;

  cb_to_cs_adf gen_cs;
  v_array<CB::label> cb_labels;
  COST_SENSITIVE::label cs_labels;
  v_array<COST_SENSITIVE::label> prepped_cs_labels;

  action_scores a_s;              // temporary storage for mtr and sm
  action_scores prob_s;           // temporary storage for sm; stores softmax values
  v_array<uint32_t> backup_nf;    // temporary storage for sm; backup for numFeatures in examples
  v_array<float> backup_weights;  // temporary storage for sm; backup for weights in examples

  uint64_t offset;
  bool no_predict;
  bool rank_all;
};

CB::cb_class get_observed_cost(multi_ex& examples)
{
  CB::label ld;
  ld.costs = v_init<cb_class>();
  int index = -1;
  CB::cb_class known_cost;

  size_t i = 0;
  for (example*& ec : examples)
  {
    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX && ec->l.cb.costs[0].probability > 0)
    {
      ld = ec->l.cb;
      index = (int)i;
    }
    ++i;
  }

  // handle -1 case.
  if (index == -1)
  {
    known_cost.probability = -1;
    return known_cost;
    // std::cerr << "None of the examples has known cost. Exiting." << endl;
    // throw exception();
  }

  known_cost = ld.costs[0];
  known_cost.action = index;
  return known_cost;
}

void learn_IPS(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  gen_cs_example_ips(examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

void learn_SM(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  gen_cs_test_example(examples, mydata.cs_labels);  // create test labels.
  call_cs_ldf<false>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);

  // Can probably do this more efficiently than 6 loops over the examples...
  //[1: initialize temporary storage;
  // 2: find chosen action;
  // 3: create cs_labels (gen_cs_example_sm);
  // 4: get probability of chosen action;
  // 5: backup example wts;
  // 6: restore example wts]
  mydata.a_s.clear();
  mydata.prob_s.clear();
  // TODO: Check that predicted scores are always stored with the first example
  for (uint32_t i = 0; i < examples[0]->pred.a_s.size(); i++)
  {
    mydata.a_s.push_back({examples[0]->pred.a_s[i].action, examples[0]->pred.a_s[i].score});
    mydata.prob_s.push_back({examples[0]->pred.a_s[i].action, 0.0});
  }

  float sign_offset = 1.0;  // To account for negative rewards/costs
  uint32_t chosen_action = 0;
  float example_weight = 1.0;

  for (uint32_t i = 0; i < examples.size(); i++)
  {
    CB::label ld = examples[i]->l.cb;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    {
      chosen_action = i;
      example_weight = ld.costs[0].cost / safe_probability(ld.costs[0].probability);

      // Importance weights of examples cannot be negative.
      // So we use a trick: set |w| as weight, and use sign(w) as an offset in the regression target.
      if (ld.costs[0].cost < 0.0)
      {
        sign_offset = -1.0;
        example_weight = -example_weight;
      }
      break;
    }
  }

  gen_cs_example_sm(examples, chosen_action, sign_offset, mydata.a_s, mydata.cs_labels);

  // Lambda is -1 in the call to generate_softmax because in vw, lower score is better; for softmax higher score is
  // better.
  generate_softmax(
      -1.0, begin_scores(mydata.a_s), end_scores(mydata.a_s), begin_scores(mydata.prob_s), end_scores(mydata.prob_s));

  // TODO: Check Marco's example that causes VW to report prob > 1.

  for (uint32_t i = 0; i < mydata.prob_s.size(); i++)  // Scale example_wt by prob of chosen action
  {
    if (mydata.prob_s[i].action == chosen_action)
    {
      example_weight *= mydata.prob_s[i].score;
      break;
    }
  }

  mydata.backup_weights.clear();
  mydata.backup_nf.clear();
  for (uint32_t i = 0; i < mydata.prob_s.size(); i++)
  {
    uint32_t current_action = mydata.prob_s[i].action;
    mydata.backup_weights.push_back(examples[current_action]->weight);
    mydata.backup_nf.push_back((uint32_t)examples[current_action]->num_features);

    if (current_action == chosen_action)
      examples[current_action]->weight = example_weight * (1.0f - mydata.prob_s[i].score);
    else
      examples[current_action]->weight = example_weight * mydata.prob_s[i].score;

    if (examples[current_action]->weight <= 1e-15)
      examples[current_action]->weight = 0;
  }

  // Do actual training
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);

  // Restore example weights and numFeatures
  for (uint32_t i = 0; i < mydata.prob_s.size(); i++)
  {
    uint32_t current_action = mydata.prob_s[i].action;
    examples[current_action]->weight = mydata.backup_weights[i];
    examples[current_action]->num_features = mydata.backup_nf[i];
  }
}

void learn_DR(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  gen_cs_example_dr<true>(mydata.gen_cs, examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

void learn_DM(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  gen_cs_example_dm(examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

template <bool predict>
void learn_MTR(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  // uint32_t action = 0;
  if (predict)  // first get the prediction to return
  {
    gen_cs_example_ips(examples, mydata.cs_labels);
    call_cs_ldf<false>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
    swap(examples[0]->pred.a_s, mydata.a_s);
  }
  // second train on _one_ action (which requires up to 3 examples).
  // We must go through the cost sensitive classifier layer to get
  // proper feature handling.
  gen_cs_example_mtr(mydata.gen_cs, examples, mydata.cs_labels);
  uint32_t nf = (uint32_t)examples[mydata.gen_cs.mtr_example]->num_features;
  float old_weight = examples[mydata.gen_cs.mtr_example]->weight;
  examples[mydata.gen_cs.mtr_example]->weight *= 1.f / examples[mydata.gen_cs.mtr_example]->l.cb.costs[0].probability *
      ((float)mydata.gen_cs.event_sum / (float)mydata.gen_cs.action_sum);

  // TODO!!! mydata.cb_labels are not getting properly restored (empty costs are dropped)
  GEN_CS::call_cs_ldf<true>(
      base, mydata.gen_cs.mtr_ec_seq, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
  examples[mydata.gen_cs.mtr_example]->num_features = nf;
  examples[mydata.gen_cs.mtr_example]->weight = old_weight;
  swap(examples[0]->pred.a_s, mydata.a_s);
}

bool test_adf_sequence(multi_ex& ec_seq)
{
  uint32_t count = 0;
  for (size_t k = 0; k < ec_seq.size(); k++)
  {
    example* ec = ec_seq[k];
    if (ec->l.cb.costs.size() > 1)
      THROW("cb_adf: badly formatted example, only one cost can be known.");

    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
      count += 1;
  }
  if (count == 0)
    return true;
  else if (count == 1)
    return false;
  else
    THROW("cb_adf: badly formatted example, only one line can have a cost");
}

template <bool is_learn>
void do_actual_learning(cb_adf& data, multi_learner& base, multi_ex& ec_seq)
{
  data.offset = ec_seq[0]->ft_offset;
  data.gen_cs.known_cost = get_observed_cost(ec_seq);  // need to set for test case
  if (is_learn && !test_adf_sequence(ec_seq))
  {
    /*	v_array<float> temp_scores;
    temp_scores = v_init<float>();
    do_actual_learning<false>(data,base);
    for (size_t i = 0; i < data.ec_seq[0]->pred.a_s.size(); i++)
    temp_scores.push_back(data.ec_seq[0]->pred.a_s[i].score);*/
    switch (data.gen_cs.cb_type)
    {
      case CB_TYPE_IPS:
        learn_IPS(data, base, ec_seq);
        break;
      case CB_TYPE_DR:
        learn_DR(data, base, ec_seq);
        break;
      case CB_TYPE_DM:
        learn_DM(data, base, ec_seq);
        break;
      case CB_TYPE_MTR:
        if (data.no_predict)
          learn_MTR<false>(data, base, ec_seq);
        else
          learn_MTR<true>(data, base, ec_seq);
        break;
      case CB_TYPE_SM:
        learn_SM(data, base, ec_seq);
        break;
      default:
        THROW("Unknown cb_type specified for contextual bandit learning: " << data.gen_cs.cb_type);
    }

    /*      for (size_t i = 0; i < temp_scores.size(); i++)
    if (temp_scores[i] != data.ec_seq[0]->pred.a_s[i].score)
      cout << "problem! " << temp_scores[i] << " != " << data.ec_seq[0]->pred.a_s[i].score << " for " <<
    data.ec_seq[0]->pred.a_s[i].action << endl; temp_scores.delete_v();*/
  }
  else
  {
    gen_cs_test_example(ec_seq, data.cs_labels);  // create test labels.
    call_cs_ldf<false>(base, ec_seq, data.cb_labels, data.cs_labels, data.prepped_cs_labels, data.offset);
  }
}

void global_print_newline(vw& all)
{
  char temp[1];
  temp[0] = '\n';
  for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
  {
    int f = all.final_prediction_sink[i];
    ssize_t t;
    t = io_buf::write_file_or_socket(f, temp, 1);
    if (t != 1)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

// how to

bool update_statistics(vw& all, cb_adf& c, example& ec, multi_ex* ec_seq)
{
  size_t num_features = 0;

  uint32_t action = ec.pred.a_s[0].action;
  for (const auto& example : *ec_seq) num_features += example->num_features;

  float loss = 0.;

  bool labeled_example = true;
  if (c.gen_cs.known_cost.probability > 0)
    loss = get_unbiased_cost(&(c.gen_cs.known_cost), c.gen_cs.pred_scores, action);
  else
    labeled_example = false;

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq->size(); i++) holdout_example &= (*ec_seq)[i]->test_only;

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);
  return labeled_example;
}

void output_example(vw& all, cb_adf& c, example& ec, multi_ex* ec_seq)
{
  if (example_is_newline_not_header(ec))
    return;

  bool labeled_example = update_statistics(all, c, ec, ec_seq);

  uint32_t action = ec.pred.a_s[0].action;
  for (int sink : all.final_prediction_sink) all.print(sink, (float)action, 0, ec.tag);

  if (all.raw_prediction > 0)
  {
    string outputString;
    stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, !labeled_example, ec, ec_seq, true);
}

void output_rank_example(vw& all, cb_adf& c, example& ec, multi_ex* ec_seq)
{
  label& ld = ec.l.cb;
  v_array<CB::cb_class> costs = ld.costs;

  if (example_is_newline_not_header(ec))
    return;

  bool labeled_example = update_statistics(all, c, ec, ec_seq);

  for (int sink : all.final_prediction_sink) print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  {
    string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0)
        outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, !labeled_example, ec, ec_seq, true);
}

void output_example_seq(vw& all, cb_adf& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    if (data.rank_all)
      output_rank_example(all, data, **(ec_seq.begin()), &(ec_seq));
    else
    {
      output_example(all, data, **(ec_seq.begin()), &(ec_seq));

      if (all.raw_prediction > 0)
        all.print_text(all.raw_prediction, "", ec_seq[0]->tag);
    }
  }
}

void finish_multiline_example(vw& all, cb_adf& data, multi_ex& ec_seq)
{
  if (ec_seq.size() > 0)
  {
    output_example_seq(all, data, ec_seq);
    global_print_newline(all);
  }
  VW::clear_seq_and_finish_examples(all, ec_seq);
}

void finish(cb_adf& data)
{
  data.gen_cs.mtr_ec_seq.~multi_ex();
  data.cb_labels.delete_v();
  for (size_t i = 0; i < data.prepped_cs_labels.size(); i++) data.prepped_cs_labels[i].costs.delete_v();
  data.prepped_cs_labels.delete_v();
  data.cs_labels.costs.delete_v();
  data.backup_weights.delete_v();
  data.backup_nf.delete_v();
  data.prob_s.delete_v();

  data.a_s.delete_v();
  data.gen_cs.pred_scores.costs.delete_v();
}

void save_load(cb_adf& c, io_buf& model_file, bool read, bool text)
{
  if (c.all->model_file_ver < VERSION_FILE_WITH_CB_ADF_SAVE)
    return;
  stringstream msg;
  msg << "event_sum " << c.gen_cs.event_sum << "\n";
  bin_text_read_write_fixed(model_file, (char*)&c.gen_cs.event_sum, sizeof(c.gen_cs.event_sum), "", read, msg, text);

  msg << "action_sum " << c.gen_cs.action_sum << "\n";
  bin_text_read_write_fixed(model_file, (char*)&c.gen_cs.action_sum, sizeof(c.gen_cs.action_sum), "", read, msg, text);
}

}  // namespace CB_ADF
using namespace CB_ADF;
base_learner* cb_adf_setup(options_i& options, vw& all)
{
  auto ld = scoped_calloc_or_throw<cb_adf>();
  bool cb_adf_option = false;
  std::string type_string = "mtr";

  option_group_definition new_options("Contextual Bandit with Action Dependent Features");
  new_options
      .add(make_option("cb_adf", cb_adf_option)
               .keep()
               .help("Do Contextual Bandit learning with multiline action dependent features."))
      .add(make_option("rank_all", ld->rank_all).keep().help("Return actions sorted by score order"))
      .add(make_option("no_predict", ld->no_predict).help("Do not do a prediction when training"))
      .add(make_option("cb_type", type_string)
               .keep()
               .help("contextual bandit method to use in {ips, dm, dr, mtr, sm}. Default: mtr"));
  options.add_and_parse(new_options);

  if (!cb_adf_option)
    return nullptr;

  // Ensure serialization of this option in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  ld->all = &all;

  // number of weight vectors needed
  size_t problem_multiplier = 1;  // default for IPS
  bool check_baseline_enabled = false;

  if (type_string.compare("dr") == 0)
  {
    ld->gen_cs.cb_type = CB_TYPE_DR;
    problem_multiplier = 2;
    // only use baseline when manually enabled for loss estimation
    check_baseline_enabled = true;
  }
  else if (type_string.compare("ips") == 0)
    ld->gen_cs.cb_type = CB_TYPE_IPS;
  else if (type_string.compare("mtr") == 0)
    ld->gen_cs.cb_type = CB_TYPE_MTR;
  else if (type_string.compare("dm") == 0)
    ld->gen_cs.cb_type = CB_TYPE_DM;
  else if (type_string.compare("sm") == 0)
    ld->gen_cs.cb_type = CB_TYPE_SM;
  else
  {
    all.trace_message << "warning: cb_type must be in {'ips','dr','mtr','dm','sm'}; resetting to mtr." << std::endl;
    ld->gen_cs.cb_type = CB_TYPE_MTR;
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Push necessary flags.
  if ((!options.was_supplied("csoaa_ldf") && !options.was_supplied("wap_ldf")) || ld->rank_all ||
      !options.was_supplied("csoaa_rank"))
  {
    if (!options.was_supplied("csoaa_ldf"))
    {
      options.insert("csoaa_ldf", "multiline");
    }

    if (!options.was_supplied("csoaa_rank"))
    {
      options.insert("csoaa_rank", "");
    }
  }

  if (options.was_supplied("baseline") && check_baseline_enabled)
  {
    options.insert("check_enabled", "");
  }

  auto base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  cb_adf* bare = ld.get();
  learner<cb_adf, multi_ex>& l = init_learner(ld, base, CB_ADF::do_actual_learning<true>,
      CB_ADF::do_actual_learning<false>, problem_multiplier, prediction_type::action_scores);
  l.set_finish_example(CB_ADF::finish_multiline_example);

  bare->gen_cs.scorer = all.scorer;

  l.set_finish(CB_ADF::finish);
  l.set_save_load(CB_ADF::save_load);
  return make_base(l);
}
