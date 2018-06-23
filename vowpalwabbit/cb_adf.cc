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

using namespace std;
using namespace LEARNER;
using namespace CB;
using namespace ACTION_SCORE;
using namespace GEN_CS;
using namespace CB_ALGS;

namespace CB_ADF
{
struct cb_adf
{
  vw* all;

  cb_to_cs_adf gen_cs;
  v_array<CB::label> cb_labels;
  COST_SENSITIVE::label cs_labels;
  v_array<COST_SENSITIVE::label> prepped_cs_labels;

  action_scores a_s;//temporary storage for mtr

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

  size_t i=0;
  for (example*& ec : examples)
  {
    if (ec->l.cb.costs.size() == 1 &&
        ec->l.cb.costs[0].cost != FLT_MAX &&
        ec->l.cb.costs[0].probability > 0)
      {
        ld = ec->l.cb;
        index = i;
      }
    ++i;
  }


  // handle -1 case.
  if (index == -1)
  {
    known_cost.probability = -1;
    return known_cost;
    //std::cerr << "None of the examples has known cost. Exiting." << endl;
    //throw exception();
  }

  bool shared = CB::ec_is_example_header(*examples[0]);

  known_cost = ld.costs[0];
  known_cost.action = index;
  if(shared)  // take care of shared example
    known_cost.action--;
  return known_cost;
}

void learn_IPS(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  gen_cs_example_ips(examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
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

template<bool predict>
void learn_MTR(cb_adf& mydata, multi_learner& base, multi_ex& examples)
{
  //uint32_t action = 0;
  if (predict) //first get the prediction to return
  {
    gen_cs_example_ips(examples, mydata.cs_labels);
    call_cs_ldf<false>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
    swap(examples[0]->pred.a_s, mydata.a_s);
  }
  //second train on _one_ action (which requires up to 3 examples).
  //We must go through the cost sensitive classifier layer to get
  //proper feature handling.
  gen_cs_example_mtr(mydata.gen_cs, examples, mydata.cs_labels);
  uint32_t nf = (uint32_t)examples[mydata.gen_cs.mtr_example]->num_features;
  float old_weight = examples[mydata.gen_cs.mtr_example]->weight;

	//adjust the importance weight to scale by a factor of 1/K (the last term)
  examples[mydata.gen_cs.mtr_example]->weight *= 1.f / examples[mydata.gen_cs.mtr_example]->l.cb.costs[0].probability * ((float)mydata.gen_cs.event_sum / (float)mydata.gen_cs.action_sum) * (1.f / mydata.gen_cs.num_actions);
  GEN_CS::call_cs_ldf<true>(base, mydata.gen_cs.mtr_ec_seq, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
  examples[mydata.gen_cs.mtr_example]->num_features = nf;
  examples[mydata.gen_cs.mtr_example]->weight = old_weight;
  swap(examples[0]->pred.a_s, mydata.a_s);
}

bool test_adf_sequence(multi_ex& ec_seq)
{
  uint32_t count = 0;
  for (size_t k=0; k<ec_seq.size(); k++)
  {
    example *ec = ec_seq[k];
    if (ec->l.cb.costs.size() > 1)
      THROW("cb_adf: badly formatted example, only one cost can be known.");

    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
      count += 1;

    if (CB::ec_is_example_header(*ec))
      if (k != 0)
        THROW("warning: example headers at position " << k << ": can only have in initial position!");
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
  data.gen_cs.known_cost = get_observed_cost(ec_seq);//need to set for test case
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
    default:
      THROW("Unknown cb_type specified for contextual bandit learning: " << data.gen_cs.cb_type);
    }

    /*      for (size_t i = 0; i < temp_scores.size(); i++)
    if (temp_scores[i] != data.ec_seq[0]->pred.a_s[i].score)
      cout << "problem! " << temp_scores[i] << " != " << data.ec_seq[0]->pred.a_s[i].score << " for " << data.ec_seq[0]->pred.a_s[i].action << endl;
      temp_scores.delete_v();*/
  }
  else
  {
    gen_cs_test_example(ec_seq, data.cs_labels);//create test labels.
    call_cs_ldf<false>(base, ec_seq, data.cb_labels, data.cs_labels, data.prepped_cs_labels, data.offset);
  }
}

void global_print_newline(vw& all)
{
  char temp[1];
  temp[0] = '\n';
  for (size_t i=0; i<all.final_prediction_sink.size(); i++)
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
  for (size_t i = 0; i < (*ec_seq).size(); i++)
    if (!CB::ec_is_example_header(*(*ec_seq)[i]))
      num_features += (*ec_seq)[i]->num_features;

  float loss = 0.;

  bool labeled_example = true;
  if (c.gen_cs.known_cost.probability > 0)
    loss = get_unbiased_cost(&(c.gen_cs.known_cost), c.gen_cs.pred_scores, action);
  else
    labeled_example = false;

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq->size(); i++)
    holdout_example &= (*ec_seq)[i]->test_only;

  all.sd->update(holdout_example, labeled_example, loss, ec.weight, num_features);
  return labeled_example;
}

void output_example(vw& all, cb_adf& c, example& ec, multi_ex* ec_seq)
{
  if (example_is_newline(ec) || (CB::ec_is_example_header(ec) && ec_seq->size() == 1)) return;

  bool labeled_example = update_statistics(all, c, ec, ec_seq);

  uint32_t action = ec.pred.a_s[0].action;
  for (int sink : all.final_prediction_sink)
    all.print(sink, (float)action, 0, ec.tag);

  if (all.raw_prediction > 0)
  {
    string outputString;
    stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) outputStringStream << ' ';
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

  if (example_is_newline_not_header(ec)) return;

  bool labeled_example = update_statistics(all, c, ec, ec_seq);

  for (int sink : all.final_prediction_sink)
    print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  {
    string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    {
      if (i > 0) outputStringStream << ' ';
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
  for(size_t i = 0; i < data.prepped_cs_labels.size(); i++)
    data.prepped_cs_labels[i].costs.delete_v();
  data.prepped_cs_labels.delete_v();
  data.cs_labels.costs.delete_v();

  data.a_s.delete_v();
  data.gen_cs.pred_scores.costs.delete_v();
}

void save_load(cb_adf& c, io_buf& model_file, bool read, bool text)
{
  if (c.all->model_file_ver < VERSION_FILE_WITH_CB_ADF_SAVE)
    return;
  stringstream msg;
  msg << "event_sum " << c.gen_cs.event_sum << "\n";
  bin_text_read_write_fixed(model_file, (char*)&c.gen_cs.event_sum, sizeof(c.gen_cs.event_sum),
                            "", read, msg, text);

  msg << "action_sum " << c.gen_cs.action_sum << "\n";
  bin_text_read_write_fixed(model_file, (char*)&c.gen_cs.action_sum, sizeof(c.gen_cs.action_sum),
                            "", read, msg, text);
}

}
using namespace CB_ADF;
base_learner* cb_adf_setup(arguments& arg)
{
  auto ld = scoped_calloc_or_throw<cb_adf>();
  std::string type_string;

  if (arg.new_options("Contextual Bandit with Action Dependent Features")
      .critical("cb_adf", "Do Contextual Bandit learning with multiline action dependent features.")
      .keep(ld->rank_all, "rank_all", "Return actions sorted by score order")
      (ld->no_predict, "no_predict", "Do not do a prediction when training")
      .keep("cb_type", type_string, (string)"ips", "contextual bandit method to use in {ips,dm,dr, mtr}")
			("cbify", ld->gen_cs.num_actions, 1U, "number of actions")
			.missing())
    return nullptr;

  ld->all = arg.all;

	//cb_to_cs_adf& c = ld.gen_cs;
	//c.num_actions = (uint32_t)(all.vm["cbify"].as<size_t>());

  // number of weight vectors needed
  size_t problem_multiplier = 1;//default for IPS
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
  else
    {
      arg.trace_message << "warning: cb_type must be in {'ips','dr','mtr','dm'}; resetting to ips." << std::endl;
      ld->gen_cs.cb_type = CB_TYPE_IPS;
    }

  arg.all->delete_prediction = ACTION_SCORE::delete_action_scores;

  // Push necessary flags.
  if ( (count(arg.args.begin(), arg.args.end(), "--csoaa_ldf") == 0 && count(arg.args.begin(), arg.args.end(), "--wap_ldf") == 0)
       || ld->rank_all || arg.vm.count("csoaa_rank") == 0)
  {
    if (count(arg.args.begin(), arg.args.end(), "--csoaa_ldf") == 0)
      arg.args.push_back("--csoaa_ldf");
    if (count(arg.args.begin(), arg.args.end(), "multiline") == 0)
      arg.args.push_back("multiline");
    if (count(arg.args.begin(), arg.args.end(), "--csoaa_rank") == 0)
      arg.args.push_back("--csoaa_rank");
  }
  if (count(arg.args.begin(), arg.args.end(), "--baseline") && check_baseline_enabled)
    arg.args.push_back("--check_enabled");

  auto base = as_multiline(setup_base(arg));
  arg.all->p->lp = CB::cb_label;
  arg.all->label_type = label_type::cb;

  cb_adf* bare = ld.get();
  learner<cb_adf,multi_ex>& l = init_learner(ld, base,
    CB_ADF::do_actual_learning<true>, CB_ADF::do_actual_learning<false>,
    problem_multiplier, prediction_type::action_scores);
  l.set_finish_example(CB_ADF::finish_multiline_example);

  bare->gen_cs.scorer = arg.all->scorer;

  l.set_finish(CB_ADF::finish);
  l.set_save_load(CB_ADF::save_load);
  return make_base(l);
}
