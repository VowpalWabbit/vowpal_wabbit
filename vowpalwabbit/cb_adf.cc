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
{ v_array<example*> ec_seq;
  bool need_to_clear;
  vw* all;

  cb_to_cs_adf gen_cs;
  v_array<CB::label> cb_labels;
  COST_SENSITIVE::label cs_labels;
  v_array<COST_SENSITIVE::label> prepped_cs_labels;

  action_scores a_s;//temporary storage for mtr

  uint64_t offset;
  bool predict;
  bool rank_all;
};

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

void learn_IPS(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ gen_cs_example_ips(examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

void learn_DR(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ gen_cs_example_dr<true>(mydata.gen_cs, examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

void learn_DM(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ gen_cs_example_dm(examples, mydata.cs_labels);
  call_cs_ldf<true>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
}

template<bool predict>
void learn_MTR(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
{ //uint32_t action = 0;
  if (predict) //first get the prediction to return
  { gen_cs_example_ips(examples, mydata.cs_labels);
    call_cs_ldf<false>(base, examples, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
    swap(examples[0]->pred.a_s, mydata.a_s);
  }
  //second train on _one_ action (which requires up to 3 examples).
  //We must go through the cost sensitive classifier layer to get
  //proper feature handling.
  gen_cs_example_mtr(mydata.gen_cs, examples, mydata.cs_labels);
  uint32_t nf = (uint32_t)examples[mydata.gen_cs.mtr_example]->num_features;
  float old_weight = examples[mydata.gen_cs.mtr_example]->weight;
  examples[mydata.gen_cs.mtr_example]->weight *= 1.f / examples[mydata.gen_cs.mtr_example]->l.cb.costs[0].probability * ((float)mydata.gen_cs.event_sum / (float)mydata.gen_cs.action_sum);
  GEN_CS::call_cs_ldf<true>(base, mydata.gen_cs.mtr_ec_seq, mydata.cb_labels, mydata.cs_labels, mydata.prepped_cs_labels, mydata.offset);
  examples[mydata.gen_cs.mtr_example]->num_features = nf;
  examples[mydata.gen_cs.mtr_example]->weight = old_weight;
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
{ data.gen_cs.known_cost = get_observed_cost(data.ec_seq);//need to set for test case
  if (is_learn && !test_adf_sequence(data))
  { /*	v_array<float> temp_scores;
    temp_scores = v_init<float>();
    do_actual_learning<false>(data,base);
    for (size_t i = 0; i < data.ec_seq[0]->pred.a_s.size(); i++)
    temp_scores.push_back(data.ec_seq[0]->pred.a_s[i].score);*/
    switch (data.gen_cs.cb_type)
    { case CB_TYPE_IPS:
        learn_IPS(data, base, data.ec_seq);
        break;
      case CB_TYPE_DR:
        learn_DR(data, base, data.ec_seq);
        break;
      case CB_TYPE_DM:
        learn_DM(data, base, data.ec_seq);
        break;
      case CB_TYPE_MTR:
        if (data.predict)
          learn_MTR<true>(data, base, data.ec_seq);
        else
          learn_MTR<false>(data, base, data.ec_seq);
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
  { gen_cs_test_example(data.ec_seq, data.cs_labels);//create test labels.
    call_cs_ldf<false>(base, data.ec_seq, data.cb_labels, data.cs_labels, data.prepped_cs_labels, data.offset);
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

  bool update_statistics(vw& all, cb_adf& c, example& ec, v_array<example*>* ec_seq)
  {
    size_t num_features = 0;
    
    uint32_t action = ec.pred.a_s[0].action;
    for (size_t i = 0; i < (*ec_seq).size(); i++)
      if (!CB::ec_is_example_header(*(*ec_seq)[i]))
	num_features += (*ec_seq)[i]->num_features;
    
    float loss = 0.;
    
    bool is_test = false;
    if (c.gen_cs.known_cost.probability > 0)
      loss = get_unbiased_cost(&(c.gen_cs.known_cost), c.gen_cs.pred_scores, action);
    else
      is_test = true;
    
    all.sd->update(ec.test_only, !is_test, loss, ec.weight, num_features);
    return is_test;
  }
  
void output_example(vw& all, cb_adf& c, example& ec, v_array<example*>* ec_seq)
{ if (example_is_newline_not_header(ec)) return;

  bool is_test = update_statistics(all, c, ec, ec_seq);

  uint32_t action = ec.pred.a_s[0].action;
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

void output_rank_example(vw& all, cb_adf& c, example& ec, v_array<example*>* ec_seq)
{ label& ld = ec.l.cb;
  v_array<CB::cb_class> costs = ld.costs;

  if (example_is_newline_not_header(ec)) return;

  bool is_test = update_statistics(all, c, ec, ec_seq);

  for (int sink : all.final_prediction_sink)
    print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, is_test, ec, ec_seq, true);
}

void output_example_seq(vw& all, cb_adf& data)
{ if (data.ec_seq.size() > 0)
  { 
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
  data.gen_cs.mtr_ec_seq.delete_v();
  data.cb_labels.delete_v();
  for(size_t i = 0; i < data.prepped_cs_labels.size(); i++)
    data.prepped_cs_labels[i].costs.delete_v();
  data.prepped_cs_labels.delete_v();
  data.cs_labels.costs.delete_v();

  data.a_s.delete_v();
  data.gen_cs.pred_scores.costs.delete_v();
}

template <bool is_learn>
void predict_or_learn(cb_adf& data, base_learner& base, example &ec)
{ vw* all = data.all;
  bool is_test_ec = CB::example_is_test(ec);
  bool need_to_break = VW::is_ring_example(*all, &ec) && (data.ec_seq.size() >= all->p->ring_size - 2);
  data.offset = ec.ft_offset;

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
    { ld.gen_cs.cb_type = CB_TYPE_DR;
      problem_multiplier = 2;
    }
    else if (type_string.compare("ips") == 0)
    { ld.gen_cs.cb_type = CB_TYPE_IPS;
      problem_multiplier = 1;
    }
    else if (type_string.compare("mtr") == 0)
    { ld.gen_cs.cb_type = CB_TYPE_MTR;
      problem_multiplier = 1;
    }
    else if (type_string.compare("dm") == 0)
    { ld.gen_cs.cb_type = CB_TYPE_DM;
      problem_multiplier = 1;
    }
    else
    { std::cerr << "warning: cb_type must be in {'ips','dr','mtr','dm'}; resetting to ips." << std::endl;
      ld.gen_cs.cb_type = CB_TYPE_IPS;
    }
  }
  else
  { //by default use ips
    ld.gen_cs.cb_type = CB_TYPE_IPS;
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
  all.label_type = label_type::cb;

  learner<cb_adf>& l = init_learner(&ld, base, CB_ADF::predict_or_learn<true>, CB_ADF::predict_or_learn<false>, problem_multiplier,
                                    prediction_type::action_scores);
  l.set_finish_example(CB_ADF::finish_multiline_example);

  ld.gen_cs.scorer = all.scorer;

  l.set_finish(CB_ADF::finish);
  l.set_end_examples(CB_ADF::end_examples);
  l.set_save_load(CB_ADF::save_load);
  return make_base(l);
}
