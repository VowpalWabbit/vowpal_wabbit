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

using namespace std;
using namespace LEARNER;
using namespace CB;
using namespace ACTION_SCORE;

// doubly robust
#define CB_TYPE_DR 0
// direct method
#define CB_TYPE_DM 1
// inverse propensity scoring
#define CB_TYPE_IPS 2
// multitask regression
#define CB_TYPE_MTR 3

struct cb_adf
{ v_array<example*> ec_seq;

  size_t cb_type;
  bool need_to_clear;
  vw* all;
  LEARNER::base_learner* scorer;
  CB::cb_class known_cost;

  // contextual bandit
  v_array<CB::label> cb_labels;

  // cost sensitive
  v_array<COST_SENSITIVE::label> cs_labels;

  // mtr
  uint32_t mtr_example;
  v_array<COST_SENSITIVE::label> mtr_cs_labels;
  v_array<example*> mtr_ec_seq;
  action_scores a_s;
  uint64_t action_sum;
  uint64_t event_sum;

  COST_SENSITIVE::label pred_scores;

  base_learner* base;

  bool rank_all;
  bool predict;
};

namespace CB_ADF
{
  bool example_is_newline_not_header(example& ec)
  { return (example_is_newline(ec) && !CB::ec_is_example_header(ec)); }

void gen_cs_example_ips(v_array<example*> examples, v_array<COST_SENSITIVE::label>& cs_labels)
{ if (cs_labels.size() < examples.size())
  { cs_labels.resize(examples.size());
    // TODO: introduce operation
    cs_labels.end() = cs_labels.end_array;
  }
  bool shared = CB::ec_is_example_header(*examples[0]);
  for (uint32_t i = 0; i < examples.size(); i++)
  { CB::label ld = examples[i]->l.cb;

    COST_SENSITIVE::wclass wc;
    if (shared && i > 0)
      wc.class_index = (uint32_t)i-1;
    else
      wc.class_index = (uint32_t)i;
    if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
      wc.x = ld.costs[0].cost / ld.costs[0].probability;
    else
      wc.x = 0.f;
    cs_labels[i].costs.erase();
    cs_labels[i].costs.push_back(wc);
  }
  cs_labels[examples.size() - 1].costs[0].x = FLT_MAX; //trigger end of multiline example.

  if (shared)//take care of shared examples
  { cs_labels[0].costs[0].class_index = 0;
    cs_labels[0].costs[0].x = -FLT_MAX;
  }
}

void gen_cs_example_dr(cb_adf& c, v_array<example*> examples, v_array<COST_SENSITIVE::label>& cs_labels)
{ //size_t mysize = examples.size();
  if (cs_labels.size() < examples.size())
  { cs_labels.resize(examples.size());
    cs_labels.end() = cs_labels.end_array;
  }

  c.pred_scores.costs.erase();

  bool shared = CB::ec_is_example_header(*examples[0]);
  int startK = 0;
  if (shared) startK = 1;

  for (size_t i = 0; i < examples.size(); i++)
  { if (example_is_newline_not_header(*examples[i])) continue;

    COST_SENSITIVE::wclass wc;
    wc.class_index = 0;

    if (c.known_cost.action + startK == i)
    { int known_index = c.known_cost.action;
      c.known_cost.action = 0;
      //get cost prediction for this label
      // num_actions should be 1 effectively.
      // my get_cost_pred function will use 1 for 'index-1+base'
      wc.x = CB_ALGS::get_cost_pred<true>(c.scorer, &(c.known_cost), *(examples[i]), 0, 2);
      c.known_cost.action = known_index;
    }
    else
      wc.x = CB_ALGS::get_cost_pred<true>(c.scorer, nullptr, *(examples[i]), 0, 2);

    if (shared)
      wc.class_index = (uint32_t)i - 1;
    else
      wc.class_index = (uint32_t)i;
    c.pred_scores.costs.push_back(wc); // done

    //add correction if we observed cost for this action and regressor is wrong
    if (c.known_cost.probability != -1 && c.known_cost.action + startK == i)
    { wc.x += (c.known_cost.cost - wc.x) / c.known_cost.probability;
    }
    cs_labels[i].costs.erase();
    cs_labels[i].costs.push_back(wc);
  }
  COST_SENSITIVE::wclass wc;
  wc.class_index = 0;
  wc.x = FLT_MAX;
  cs_labels[examples.size() - 1].costs.erase();
  cs_labels[examples.size() - 1].costs.push_back(wc); //trigger end of multiline example.

  if (shared)//take care of shared examples
  { cs_labels[0].costs[0].class_index = 0;
    cs_labels[0].costs[0].x = -FLT_MAX;
  }
}

  CB::cb_class get_observed_cost(v_array<example*>& examples)
{ CB::label ld;
  ld.costs = v_init<cb_class>();
  int index = -1;
  CB::cb_class known_cost;

  for (example*& ec : examples)
  { if (ec->l.cb.costs.size() == 1 &&
        ec->l.cb.costs[0].cost != FLT_MAX &&
        ec->l.cb.costs[0].probability > 0)
    { ld = ec->l.cb;
      index = (int)(&ec - examples.begin());
    }
  }


  // handle -1 case.
  if (index == -1)
  { known_cost.probability = -1;
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

template<bool is_learn>
void call_predict_or_learn(cb_adf& mydata, base_learner& base, v_array<example*>& examples,
                           v_array<CB::label>& cb_labels, v_array<COST_SENSITIVE::label>& cs_labels)
{ // first of all, clear the container mydata.array.
  cb_labels.erase();

  // 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
  size_t index = 0;
  for (example* ec : examples)
  { cb_labels.push_back(ec->l.cb);
    ec->l.cs = cs_labels[index++];
  }

  // 2nd: predict for each ex
  // // call base.predict for each vw exmaple in the sequence
  for (example* ec : examples)
  { if (is_learn)
      base.learn(*ec);
    else
      base.predict(*ec);
  }

  // 3rd: restore cb_label for each example
  // (**ec).l.cb = array.element.
  size_t i = 0;
  for (example* ec : examples)
    ec->l.cb = cb_labels[i++];

  // if (!mydata.rank_all)
  //   { uint32_t action = 0;
  //     for (size_t i = 0; i < examples.size(); i++)
  // 	if (!CB::ec_is_example_header(*examples[i]) && !example_is_newline_not_header(*examples[i]))
  // 	  if (examples[i]->pred.multiclass != 0)
  // 	    action = examples[i]->pred.multiclass;
  //     examples[0]->pred.multiclass = action;
  // }
}

void learn_IPS(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ gen_cs_example_ips(examples, mydata.cs_labels);
  call_predict_or_learn<true>(mydata, base, examples, mydata.cb_labels, mydata.cs_labels);
}

void learn_DR(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{
  gen_cs_example_dr(mydata, examples, mydata.cs_labels);
  call_predict_or_learn<true>(mydata, base, examples, mydata.cb_labels, mydata.cs_labels);
}

void gen_cs_example_MTR(cb_adf& data, v_array<example*>& ec_seq, v_array<example*>& mtr_ec_seq, v_array<COST_SENSITIVE::label>& mtr_cs_labels)
{ mtr_ec_seq.erase();
  bool shared = CB::ec_is_example_header(*(ec_seq[0]));
  data.action_sum += ec_seq.size()-2; //-1 for shared -1 for end example
  if (!shared)
    data.action_sum += 1;
  data.event_sum++;
  uint32_t keep_count = 0;

  for (size_t i = 0; i < ec_seq.size(); i++)
  { CB::label ld = ec_seq[i]->l.cb;

    COST_SENSITIVE::wclass wc = {0, 0};

    bool keep_example = false;
    if (shared && i == 0)
    { wc.x = -FLT_MAX;
      keep_example = true;
    }
    else if (ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)
    { wc.x = ld.costs[0].cost;
      data.mtr_example = (uint32_t)i;
      keep_example = true;
    }

    else if (i == ec_seq.size() - 1)
    { wc.x = FLT_MAX; //trigger end of multiline example.
      keep_example = true;
    }

    if (keep_example)
    { mtr_ec_seq.push_back(ec_seq[i]);
      mtr_cs_labels[keep_count].costs.erase();
      mtr_cs_labels[keep_count++].costs.push_back(wc);
    }
  }
}

template<bool predict>
void learn_MTR(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ //uint32_t action = 0;
  if (predict) //first get the prediction to return
  { gen_cs_example_ips(examples, mydata.cs_labels);
    call_predict_or_learn<false>(mydata, base, examples, mydata.cb_labels, mydata.cs_labels);
    //    if (!mydata.rank_all) //preserve prediction
    //      action = examples[0]->pred.multiclass;
    //    else
      swap(examples[0]->pred.a_s, mydata.a_s);
  }
  //second train on _one_ action (which requires up to 3 examples).
  //We must go through the cost sensitive classifier layer to get
  //proper feature handling.
  gen_cs_example_MTR(mydata, examples, mydata.mtr_ec_seq, mydata.mtr_cs_labels);
  uint32_t nf = (uint32_t)examples[mydata.mtr_example]->num_features;
  float old_weight = examples[mydata.mtr_example]->weight;
  examples[mydata.mtr_example]->weight *= 1.f / examples[mydata.mtr_example]->l.cb.costs[0].probability * ((float)mydata.event_sum / (float)mydata.action_sum);
  call_predict_or_learn<true>(mydata, base, mydata.mtr_ec_seq, mydata.cb_labels, mydata.mtr_cs_labels);
  examples[mydata.mtr_example]->num_features = nf;
  examples[mydata.mtr_example]->weight = old_weight;
  //  if (!mydata.rank_all) //restore prediction
  //    examples[0]->pred.multiclass = action;
  //  else
    swap(examples[0]->pred.a_s, mydata.a_s);
}

bool test_adf_sequence(cb_adf& data)
{ uint32_t count = 0;
  for (size_t k=0; k<data.ec_seq.size(); k++)
  { example *ec = data.ec_seq[k];

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
void do_actual_learning(cb_adf& data, base_learner& base)
{ bool isTest = test_adf_sequence(data);
  data.known_cost = get_observed_cost(data.ec_seq);

  if (isTest || !is_learn)
  { gen_cs_example_ips(data.ec_seq, data.cs_labels);//create test labels.
    call_predict_or_learn<false>(data, base, data.ec_seq, data.cb_labels, data.cs_labels);
  }
  else
  { switch (data.cb_type)
    { case CB_TYPE_IPS:
        learn_IPS(data, base, data.ec_seq);
        break;
      case CB_TYPE_DR:
        learn_DR(data, base, data.ec_seq);
        break;
      case CB_TYPE_MTR:
        if (data.predict)
          learn_MTR<true>(data, base, data.ec_seq);
        else
          learn_MTR<false>(data, base, data.ec_seq);
        break;
      default:
        THROW("Unknown cb_type specified for contextual bandit learning: " << data.cb_type);
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

// how to

void output_example(vw& all, cb_adf& c, example& ec, v_array<example*>* ec_seq)
{ if (example_is_newline_not_header(ec)) return;

  size_t num_features = 0;

  float loss = 0.;

  uint32_t action = ec.pred.a_s[0].action;
  for (size_t i = 0; i < (*ec_seq).size(); i++)
    if (!CB::ec_is_example_header(*(*ec_seq)[i]))
      num_features += (*ec_seq)[i]->num_features;

  all.sd->total_features += num_features;

  bool is_test = false;
  if (c.known_cost.probability > 0)
  { loss = get_unbiased_cost(&(c.known_cost), c.pred_scores, action);
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }
  else
    is_test = true;

  for (int sink : all.final_prediction_sink)
    all.print(sink, (float)action, 0, ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, is_test, ec, ec_seq, true);
}

void output_rank_example(vw& all, cb_adf& c, example& head_ec, v_array<example*>* ec_seq)
{ label& ld = head_ec.l.cb;
  v_array<CB::cb_class> costs = ld.costs;

  if (example_is_newline_not_header(head_ec)) return;

  size_t num_features = 0;
  for (size_t i = 0; i < (*ec_seq).size(); i++)
    if (!CB::ec_is_example_header(*(*ec_seq)[i])) {
      num_features += (*ec_seq)[i]->num_features;
      //cout<<(*ec_seq)[i]->num_features<<" ";
    }
  //cout<<endl;

  all.sd->total_features += num_features;

  float loss = 0.;
  action_scores& preds = head_ec.pred.a_s;
  bool is_test = false;

  if (c.known_cost.probability > 0)
  { loss = get_unbiased_cost(&(c.known_cost), c.pred_scores, preds[0].action);
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  }
  else
    is_test = true;

  for (int sink : all.final_prediction_sink)
    print_action_score(sink, head_ec.pred.a_s, head_ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), head_ec.tag);
  }

  CB::print_update(all, is_test, head_ec, ec_seq, true);
}

void output_example_seq(vw& all, cb_adf& data)
{ if (data.ec_seq.size() > 0)
  { all.sd->weighted_examples += 1;
    all.sd->example_number++;

    //bool hit_loss = false;

    if (data.rank_all)
      output_rank_example(all, data, **(data.ec_seq.begin()), &(data.ec_seq));
    else
    { output_example(all, data, **(data.ec_seq.begin()), &(data.ec_seq));

      if (all.raw_prediction > 0)
        all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
    }
  }
}

void clear_seq_and_finish_examples(vw& all, cb_adf& data)
{ if (data.ec_seq.size() > 0)
    for (example* ecc : data.ec_seq)
      if (ecc->in_use)
        VW::finish_example(all, ecc);
  data.ec_seq.erase();
}

void finish_multiline_example(vw& all, cb_adf& data, example& ec)
{ if (data.need_to_clear)
  { if (data.ec_seq.size() > 0)
    { output_example_seq(all, data);
      global_print_newline(all);
    }
    clear_seq_and_finish_examples(all, data);
    data.need_to_clear = false;
  }
}

void end_examples(cb_adf& data)
{ if (data.need_to_clear)
    data.ec_seq.erase();
}

void finish(cb_adf& data)
{ data.ec_seq.delete_v();
  data.mtr_ec_seq.delete_v();
  data.cb_labels.delete_v();
  for(size_t i = 0; i < data.cs_labels.size(); i++)
    data.cs_labels[i].costs.delete_v();
  data.cs_labels.delete_v();

  for(size_t i = 0; i < data.mtr_cs_labels.size(); i++)
    data.mtr_cs_labels[i].costs.delete_v();
  data.mtr_cs_labels.delete_v();
  data.a_s.delete_v();
  data.pred_scores.costs.delete_v();
}

template <bool is_learn>
void predict_or_learn(cb_adf& data, base_learner& base, example &ec)
{ vw* all = data.all;
  data.base = &base;
  bool is_test_ec = CB::example_is_test(ec);
  bool need_to_break = VW::is_ring_example(*all, &ec) && (data.ec_seq.size() >= all->p->ring_size - 2);

  if ((example_is_newline_not_header(ec) && is_test_ec) || need_to_break)
  { data.ec_seq.push_back(&ec);
    do_actual_learning<is_learn>(data, base);
    // using flag to clear, because ec_seq is used in finish_example
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
}

base_learner* cb_adf_setup(vw& all)
{ if (missing_option(all, true, "cb_adf", "Do Contextual Bandit learning with multiline action dependent features."))
    return nullptr;
  new_options(all, "ADF Options")
  ("rank_all", "Return actions sorted by score order")
  ("no_predict", "Do not do a prediction when training")
  ("cb_type", po::value<string>(), "contextual bandit method to use in {ips,dm,dr}");
  add_options(all);

  cb_adf& ld = calloc_or_throw<cb_adf>();

  ld.all = &all;

  // number of weight vectors needed
  size_t problem_multiplier = 1;//default for IPS
  if (all.vm.count("cb_type"))
  { std::string type_string;

    type_string = all.vm["cb_type"].as<std::string>();
    *all.file_options << " --cb_type " << type_string;

    if (type_string.compare("dr") == 0)
    { ld.cb_type = CB_TYPE_DR;
      problem_multiplier = 2;
    }
    else if (type_string.compare("ips") == 0)
    { ld.cb_type = CB_TYPE_IPS;
      problem_multiplier = 1;
    }
    else if (type_string.compare("mtr") == 0)
    { ld.cb_type = CB_TYPE_MTR;
      ld.mtr_cs_labels.resize(3);//shared, task, and end_sequence examples
      ld.mtr_cs_labels.end() = ld.mtr_cs_labels.end_array;
      problem_multiplier = 1;
    }
    else
    { std::cerr << "warning: cb_type must be in {'ips','dr'}; resetting to ips." << std::endl;
      ld.cb_type = CB_TYPE_IPS;
    }
  }
  else
  { //by default use ips
    ld.cb_type = CB_TYPE_IPS;
    *all.file_options << " --cb_type ips";
  }

  if (all.vm.count("rank_all"))
  { ld.rank_all = true;
    *all.file_options << " --rank_all";
  }
  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  if (all.vm.count("no_predict"))
    ld.predict = false;
  else
    ld.predict = true;

  // Push necessary flags.
  if ( (count(all.args.begin(), all.args.end(), "--csoaa_ldf") == 0 && count(all.args.begin(), all.args.end(), "--wap_ldf") == 0)
       || all.vm.count("rank_all") || all.vm.count("csoaa_rank") == 0)
  { if (count(all.args.begin(), all.args.end(), "--csoaa_ldf") == 0)
      all.args.push_back("--csoaa_ldf");
    if (count(all.args.begin(), all.args.end(), "multiline") == 0)
      all.args.push_back("multiline");
    if (count(all.args.begin(), all.args.end(), "--csoaa_rank") == 0)
      all.args.push_back("--csoaa_rank");
  }

  base_learner* base = setup_base(all);
  all.p->lp = CB::cb_label;

  learner<cb_adf>& l = init_learner(&ld, base, CB_ADF::predict_or_learn<true>, CB_ADF::predict_or_learn<false>, problem_multiplier);
  l.set_finish_example(CB_ADF::finish_multiline_example);

  l.increment = base->increment;
  ld.scorer = all.scorer;

  l.set_finish(CB_ADF::finish);
  l.set_end_examples(CB_ADF::end_examples);
  return make_base(l);
}
