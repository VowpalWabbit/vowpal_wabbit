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

using namespace std;
using namespace LEARNER;

struct cb_adf {
  v_array<example*> ec_seq;
  
  bool need_to_clear;
  bool first_pass;
  bool is_singleline;
  float csoaa_example_t;
  vw* all;
  
  base_learner* base;
};

bool check_cb_adf_sequence(cb_adf& data, size_t start_K)
{
  bool isTest = CB::example_is_test(*data.ec_seq[start_K]);
  for (size_t k=start_K; k<data.ec_seq.size(); k++) {
    example *ec = data.ec_seq[k];
      
    // Each sub-example must have just one cost
    assert(ec->l.cs.costs.size()==1);
      
    if (CB::example_is_test(*ec) != isTest) {
      isTest = true;
      cerr << "warning: cb_adf example has mix of train/test data; assuming test" << endl;
    }
    if (CB::ec_is_example_header(*ec)) {
      cerr << "warning: example headers at position " << k << ": can only have in initial position!" << endl;
      throw exception();
    }
  }
  return isTest;
}

  bool ec_is_label_definition(example& ec) // label defs look like "0:___" or just "label:___"
  {
    if (ec.indices.size() != 1) return false;
    if (ec.indices[0] != 'l') return false;
    v_array<CB::cb_class> costs = ec.l.cb.costs;
    for (size_t j=0; j<costs.size(); j++)
      if ((costs[j].action != 0) || (costs[j].cost <= 0.)) return false;
    return true;    
  }

  bool ec_seq_is_label_definition(v_array<example*>ec_seq)
  {
    if (ec_seq.size() == 0) return false;
    bool is_lab = ec_is_label_definition(*ec_seq[0]);
    for (size_t i=1; i<ec_seq.size(); i++) {
      if (is_lab != ec_is_label_definition(*ec_seq[i])) {
        if (!((i == ec_seq.size()-1) && (example_is_newline(*ec_seq[i])))) {
          cerr << "error: mixed label definition and examples in ldf data!" << endl;
          throw exception();
        }
      }
    }
    return is_lab;
  }


template <bool is_learn>
void do_actual_learning(cb_adf& data, base_learner& base)
{
  //cdbg << "do_actual_learning size=" << data.ec_seq.size() << endl;
  if (data.ec_seq.size() <= 0) return;  // nothing to do

  /////////////////////// handle label definitions
  if (ec_seq_is_label_definition(data.ec_seq)) {
    // pass to cs_ldf
    return;
  }

  /////////////////////// add headers
  size_t K = data.ec_seq.size();
  size_t start_K = 0;
  if (CB::ec_is_example_header(*data.ec_seq[0])) {
    start_K = 1;
    for (size_t k=1; k<K; k++)
      LabelDict::add_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);
  }
  bool isTest = check_cb_adf_sequence(data, start_K);

  /////////////////////// do prediction
  size_t predicted_K = start_K;   
  //do prediction via CS_LDF oracle
  /*  for (size_t k=start_K; k<K; k++) {
    example *ec = data.ec_seq[k];
    make_single_prediction(data, base, *ec);
    if (ec->partial_prediction < min_score) {
      min_score = ec->partial_prediction;
      predicted_K = k;
    }
    }*/   

  /////////////////////// learn
  if (is_learn && !isTest) 
    {//do learning
    }

  // Mark the predicted subexample with its class_index, all other with 0
  for (size_t k=start_K; k<K; k++)
    data.ec_seq[k]->pred.multiclass = (k == predicted_K) ? data.ec_seq[k]->l.cs.costs[0].class_index : 0;

  /////////////////////// remove header
  if (start_K > 0)
    for (size_t k=1; k<K; k++)
      LabelDict::del_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);
}

void global_print_newline(vw& all)
{
  char temp[1];
  temp[0] = '\n';
  for (size_t i=0; i<all.final_prediction_sink.size(); i++) {
    int f = all.final_prediction_sink[i];
    ssize_t t;
    t = io_buf::write_file_or_socket(f, temp, 1);
    if (t != 1)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void output_example(vw& all, example& ec, bool& hit_loss, v_array<example*>* ec_seq)
{
  v_array<CB::cb_class> costs = ec.l.cb.costs;
    
  if (example_is_newline(ec)) return;
  if (CB::ec_is_example_header(ec)) return;
  if (ec_is_label_definition(ec)) return;

  all.sd->total_features += ec.num_features;

  float loss = 0.;

  if (!CB::example_is_test(ec)) {
    for (size_t j=0; j<costs.size(); j++) {
      if (hit_loss) break;
      if (ec.pred.multiclass == costs[j].action) {
        loss = costs[j].cost;
        hit_loss = true;
      }
    }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
    assert(loss >= 0);
  }
  
  for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
    all.print(*sink, (float)ec.pred.multiclass, 0, ec.tag);

  if (all.raw_prediction > 0) {
    string outputString;
    stringstream outputStringStream(outputString);
    for (size_t i = 0; i < costs.size(); i++) {
      if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    //outputStringStream << endl;
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }
    
  COST_SENSITIVE::print_update(all, COST_SENSITIVE::example_is_test(ec), ec, ec_seq);
}

void output_example_seq(vw& all, cb_adf& data)
{
  if ((data.ec_seq.size() > 0) && !ec_seq_is_label_definition(data.ec_seq)) {
    all.sd->weighted_examples += 1;
    all.sd->example_number++;

    bool hit_loss = false;
    for (example** ecc=data.ec_seq.begin; ecc!=data.ec_seq.end; ecc++)
      output_example(all, **ecc, hit_loss, &(data.ec_seq));

    if (!data.is_singleline && (all.raw_prediction > 0))
      all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
  }
}

void clear_seq_and_finish_examples(vw& all, cb_adf& data)
{
  if (data.ec_seq.size() > 0) 
    for (example** ecc=data.ec_seq.begin; ecc!=data.ec_seq.end; ecc++)
      if ((*ecc)->in_use)
        VW::finish_example(all, *ecc);
  data.ec_seq.erase();
}

void end_pass(cb_adf& data)
{
  data.first_pass = false;
}

void finish_singleline_example(vw& all, cb_adf&, example& ec)
{
  if (! ec_is_label_definition(ec)) {
    all.sd->weighted_examples += 1;
    all.sd->example_number++;
  }
  bool hit_loss = false;
  output_example(all, ec, hit_loss, nullptr);
  VW::finish_example(all, &ec);
}

void finish_multiline_example(vw& all, cb_adf& data, example& ec)
{
  if (data.need_to_clear) {
    if (data.ec_seq.size() > 0) {
      output_example_seq(all, data);
      global_print_newline(all);
    }        
    clear_seq_and_finish_examples(all, data);
    data.need_to_clear = false;
    if (ec.in_use) VW::finish_example(all, &ec);
  }
}

void end_examples(cb_adf& data)
{
  if (data.need_to_clear)
    data.ec_seq.erase();
}


void finish(cb_adf& data)
{
  data.ec_seq.delete_v();
}

template <bool is_learn>
void predict_or_learn(cb_adf& data, base_learner& base, example &ec) {
  vw* all = data.all;
  data.base = &base;
  bool is_test_ec = COST_SENSITIVE::example_is_test(ec);
  bool need_to_break = data.ec_seq.size() >= all->p->ring_size - 2;

  if (ec_is_label_definition(ec)) {
    if (data.ec_seq.size() > 0) {
      cerr << "error: label definition encountered in data block" << endl;
      throw exception();
    }
    data.ec_seq.push_back(&ec);
    do_actual_learning<is_learn>(data, base);
    data.need_to_clear = true;
  } else if ((example_is_newline(ec) && is_test_ec) || need_to_break) {
    if (need_to_break && data.first_pass)
      cerr << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << endl;
    do_actual_learning<is_learn>(data, base);
    data.need_to_clear = true;
  } else {
    if (data.need_to_clear) {  // should only happen if we're NOT driving
      data.ec_seq.erase();
      data.need_to_clear = false;
    }
    data.ec_seq.push_back(&ec);
  }
}

base_learner* cb_adf_setup(vw& all)
{
  if (missing_option<string, true>(all, "cb_adf", "Do Contextual Bandit learning with action dependent features.  Specify singleline or multiline."))
    return nullptr;

  cb_adf& ld = calloc_or_die<cb_adf>();

  ld.all = &all;
  ld.first_pass = true;
 
  string adf_arg;

  if( all.vm.count("cb_adf") )
    adf_arg = all.vm["cb_adf"].as<string>();

  if (adf_arg.compare("singleline") == 0 || adf_arg.compare("s") == 0) 
    ld.is_singleline = true;
  else
    ld.is_singleline = false;

  all.p->lp = CB::cb_label;

  learner<cb_adf>& l = init_learner(&ld, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);
  if (ld.is_singleline)
    l.set_finish_example(finish_singleline_example);
  else
    l.set_finish_example(finish_multiline_example);
  l.set_finish(finish);
  l.set_end_examples(end_examples); 
  l.set_end_pass(end_pass);
  return make_base(l);
}
