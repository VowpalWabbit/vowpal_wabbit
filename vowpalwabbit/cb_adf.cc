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

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2

struct cb_adf {
  v_array<example*> ec_seq;

  size_t cb_type;
  bool need_to_clear;
  vw* all;
  LEARNER::base_learner* scorer;
  CB::cb_class known_cost;
  v_array<CB::label> cb_labels;

  v_array<COST_SENSITIVE::label> cs_labels;
  COST_SENSITIVE::label pred_scores;

  base_learner* base;

  bool rank_all;
};

namespace CB_ADF {

  bool has_shared_example(v_array<example*> examples) {
    if (examples[0]->l.cb.costs.size() > 0 && examples[0]->l.cb.costs[0].probability == -1.f)
      return true;
    return false;
  }

  void gen_cs_example_ips(v_array<example*> examples, v_array<COST_SENSITIVE::label>& cs_labels)
  {
    if (cs_labels.size() < examples.size()) {
      cs_labels.resize(examples.size(), true);
      cs_labels.end = cs_labels.end_array;
    }
    for (size_t i = 0; i < examples.size(); i++)
      {
	CB::label ld = examples[i]->l.cb;

	COST_SENSITIVE::wclass wc;
	wc.class_index = 0;
	if ( ld.costs.size() == 1 && ld.costs[0].cost != FLT_MAX)  
	  wc.x = ld.costs[0].cost / ld.costs[0].probability;
	else 
	  wc.x = 0.f;
	cs_labels[i].costs.erase();
	cs_labels[i].costs.push_back(wc);
      }
    cs_labels[examples.size()-1].costs[0].x = FLT_MAX; //trigger end of multiline example.

    if (has_shared_example(examples))//take care of shared examples
      {
	cs_labels[0].costs[0].class_index = 0;
	cs_labels[0].costs[0].x = -FLT_MAX;
      }
  }
  
  template <bool is_learn>
  void gen_cs_example_dr(cb_adf& c, v_array<example*> examples, v_array<COST_SENSITIVE::label>& cs_labels)
  {
    //size_t mysize = examples.size();
    if (cs_labels.size() < examples.size()) {
      cs_labels.resize(examples.size(), true);
      cs_labels.end = cs_labels.end_array;		
    }

    c.pred_scores.costs.erase();
    bool shared = false;
    if(has_shared_example(examples))
      shared = true;

    int startK = 0;
    if(shared) startK = 1;

    for (size_t i = 0; i < examples.size(); i++)
      {
	examples[i]->l.cb.costs.erase();
	if(example_is_newline(*examples[i])) continue;
	//CB::label ld = examples[i]->l.cb;		

	COST_SENSITIVE::wclass wc;
	wc.class_index = 0;	
	
	if(c.known_cost.action + startK == i) {
	  int known_index = c.known_cost.action;
	  c.known_cost.action = 0;
	  //get cost prediction for this label
	  // num_actions should be 1 effectively.
	  // my get_cost_pred function will use 1 for 'index-1+base'			
	  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, &(c.known_cost), *(examples[i]), 0, 2);
	  c.known_cost.action = known_index;
	}
	else {
	  wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, nullptr, *(examples[i]), 0, 2);
	}

	if(shared)
	  wc.class_index = i-1;
	else
	  wc.class_index = i;
	c.pred_scores.costs.push_back(wc); // done
	wc.class_index = 0;

	//add correction if we observed cost for this action and regressor is wrong
	if (c.known_cost.probability != -1 && c.known_cost.action + startK == i)
	  {			
	    wc.x += (c.known_cost.cost - wc.x) / c.known_cost.probability;
	  }

	//cout<<"Action "<<c.known_cost.action<<" Cost "<<c.known_cost.cost<<" Probability "<<c.known_cost.probability<<endl;
	
	//cout<<"Prediction = "<<wc.x<<" ";
	cs_labels[i].costs.erase();
	cs_labels[i].costs.push_back(wc);
      }
    COST_SENSITIVE::wclass wc;
    wc.class_index = 0;
    wc.x = FLT_MAX;
    cs_labels[examples.size()-1].costs.erase();
    cs_labels[examples.size()-1].costs.push_back(wc); //trigger end of multiline example.

    if (shared)//take care of shared examples
      {
	cs_labels[0].costs[0].class_index = 0;
	cs_labels[0].costs[0].x = -FLT_MAX;
      }

    //cout<<endl;

  }

  void get_observed_cost(cb_adf& mydata, v_array<example*>& examples)
  {
    CB::label ld;
    int index = -1;

    for (example **ec = examples.begin; ec != examples.end; ec++)
      {
	if ((**ec).l.cb.costs.size() == 1 &&
	    (**ec).l.cb.costs[0].cost != FLT_MAX &&
	    (**ec).l.cb.costs[0].probability > 0)
	  {
	    ld = (**ec).l.cb;
	    index = ec - examples.begin;
	  }
      }

  
    // handle -1 case.
    if (index == -1){
      mydata.known_cost.probability = -1;
      return;
      //std::cerr << "None of the examples has known cost. Exiting." << endl;
      //throw exception();
    }

    bool shared = false;
    if (has_shared_example(examples))
      shared = true;
  
    for (CB::cb_class *cl = ld.costs.begin; cl != ld.costs.end; cl++)
      {
	mydata.known_cost = ld.costs[0];
	mydata.known_cost.action = index;
      
	// take care of shared example
	if(shared)
	  mydata.known_cost.action--;	
      }
  }

  template<bool is_learn>
  void call_predict_or_learn(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
  {
    // m2: still save, store, and restore
    // starting with 3 for loops
    // first of all, clear the container mydata.array.
    mydata.cb_labels.erase();
  
    // 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
    size_t index = 0;
    for (example **ec = examples.begin; ec != examples.end; ec++)
      {
	mydata.cb_labels.push_back((**ec).l.cb);
	(**ec).l.cs = mydata.cs_labels[index++]; 
      }
  
    // 2nd: predict for each ex
    // // call base.predict for each vw exmaple in the sequence
    for (example **ec = examples.begin; ec != examples.end; ec++) {
      //cout<<"Number of features = "<<(**ec).num_features<<" label = "<<(**ec).l.cs.costs[0].x<<" "<<example_is_newline(**ec)<<endl;
      if (is_learn)
	base.learn(**ec);
      else
	base.predict(**ec);
    }

    //cout<<"Prediction size = "<<(**examples.begin).pred.multilabels.label_v.size()<<endl;
  
    // 3rd: restore cb_label for each example
    // (**ec).l.cb = mydata.array.element.
    size_t i = 0;
    for (example **ec = examples.begin; ec != examples.end; ec++)
      (**ec).l.cb = mydata.cb_labels[i++];
  }

  template<uint32_t reduction_type>
  void learn(cb_adf& mydata, base_learner& base, v_array<example*>& examples)
  {
	if(reduction_type == CB_TYPE_IPS)
      gen_cs_example_ips(examples, mydata.cs_labels);
    else if (reduction_type == CB_TYPE_DR)
      gen_cs_example_dr<true>(mydata, examples, mydata.cs_labels);
    else
      THROW("Unknown cb_type specified for contextual bandit learning: " << mydata.cb_type);
	
    call_predict_or_learn<true>(mydata,base,examples);
  }

  bool test_adf_sequence(cb_adf& data)
  {
    uint32_t count = 0;
    for (size_t k=0; k<data.ec_seq.size(); k++) {
      example *ec = data.ec_seq[k];
    
      if (ec->l.cb.costs.size() > 1)
	THROW("cb_adf: badly formatted example, only one cost can be known.");
    
      if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)      
	count += 1;
      
      if (CB::ec_is_example_header(*ec))
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
  {
    bool isTest = test_adf_sequence(data);
    get_observed_cost(data, data.ec_seq);
  
    if (isTest || !is_learn)
      {
	gen_cs_example_ips(data.ec_seq, data.cs_labels);//create test labels.
	call_predict_or_learn<false>(data, base, data.ec_seq);
      }
    else
      {
	if (data.cb_type == CB_TYPE_IPS)
	  {
	    learn<CB_TYPE_IPS>(data, base, data.ec_seq);
	  }
	else if (data.cb_type == CB_TYPE_DR)
	  {
	    learn<CB_TYPE_DR>(data, base, data.ec_seq);
	  }
	else
	  THROW("Unknown cb_type specified for contextual bandit learning: " << data.cb_type);
      }
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

  // how to 

  void output_example(vw& all, cb_adf& c, example& ec, v_array<example*>* ec_seq)
  {
    v_array<CB::cb_class> costs = ec.l.cb.costs;
    
    if (example_is_newline(ec)) return;
    if (CB::ec_is_example_header(ec)) return;

    all.sd->total_features += ec.num_features;

    float loss = 0.;

    if (!CB::example_is_test(ec)) {
      loss = get_unbiased_cost(&(c.known_cost), c.pred_scores, ec.pred.multiclass);
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
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
    
    CB::print_update(all, CB::example_is_test(ec), ec, ec_seq, false);
  }

  void output_rank_example(vw& all, cb_adf& c, example& head_ec, v_array<example*>* ec_seq)
  {
    label& ld = head_ec.l.cb;
    v_array<CB::cb_class> costs = ld.costs;
  
    if (example_is_newline(head_ec)) return;
  
    all.sd->total_features += head_ec.num_features;
  
    float loss = 0.;
    v_array<uint32_t> preds = head_ec.pred.multilabels.label_v;
    bool is_test = false;
    //cout<<"Preds size = "<<preds.size()<<endl;
    
    if (c.known_cost.probability > 0) {
      //c.pred_scores.costs.erase();
      loss = get_unbiased_cost(&(c.known_cost), c.pred_scores, preds[0]);
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
    }
    else
      is_test = true;
      
  
    //for (int i = 0; i < preds.size();i++)
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      MULTILABEL::print_multilabel(*sink, head_ec.pred.multilabels, head_ec.tag);
  
    if (all.raw_prediction > 0) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (size_t i = 0; i < costs.size(); i++) {
	if (i > 0) outputStringStream << ' ';
	outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), head_ec.tag);
    }
  
    CB::print_update(all, is_test, head_ec, ec_seq, true);
  }

  void output_example_seq(vw& all, cb_adf& data)
  {
    if (data.ec_seq.size() > 0) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;
      
      //bool hit_loss = false;

      if (data.rank_all)
	output_rank_example(all, data, **(data.ec_seq.begin), &(data.ec_seq));
      else
	{
	  for (example** ecc=data.ec_seq.begin; ecc!=data.ec_seq.end; ecc++)
	    output_example(all, data, **ecc, &(data.ec_seq));
	
	  if (all.raw_prediction > 0)
	    all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
	}
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
    data.cb_labels.delete_v();
    for(size_t i = 0; i < data.cs_labels.size(); i++)
      data.cs_labels[i].costs.delete_v();
    data.cs_labels.delete_v();
    data.pred_scores.costs.delete_v();
  }

  template <bool is_learn>
  void predict_or_learn(cb_adf& data, base_learner& base, example &ec) {
    vw* all = data.all;
    data.base = &base;
    bool is_test_ec = CB::example_is_test(ec);
    bool need_to_break = data.ec_seq.size() >= all->p->ring_size - 2;

    if ((example_is_newline(ec) && is_test_ec) || need_to_break) {
      data.ec_seq.push_back(&ec);
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
}

base_learner* cb_adf_setup(vw& all)
{
  if (missing_option(all, true, "cb_adf", "Do Contextual Bandit learning with multiline action dependent features."))
    return nullptr;
  new_options(all, "ADF Options")		
    ("rank_all", "Return actions sorted by score order")
    ("cb_type", po::value<string>(), "contextual bandit method to use in {ips,dm,dr}");
  add_options(all);		

  cb_adf& ld = calloc_or_die<cb_adf>();

  ld.all = &all;

  size_t problem_multiplier = 1;//default for IPS
  if (all.vm.count("cb_type"))
    {
      std::string type_string;

      type_string = all.vm["cb_type"].as<std::string>();
      *all.file_options << " --cb_type " << type_string;

      if (type_string.compare("dr") == 0) {
	ld.cb_type = CB_TYPE_DR;
	problem_multiplier = 2;
      }
      else if (type_string.compare("ips") == 0)
	{
	  ld.cb_type = CB_TYPE_IPS;
	  problem_multiplier = 1;
	}
      else {
	std::cerr << "warning: cb_type must be in {'ips','dr'}; resetting to ips." << std::endl;
	ld.cb_type = CB_TYPE_IPS;
      }
    }
  else {
    //by default use ips
    ld.cb_type = CB_TYPE_IPS;
    *all.file_options << " --cb_type ips";
  }

  if (all.vm.count("rank_all"))
    {
      ld.rank_all = true;
      all.multilabel_prediction = true;
      *all.file_options << " --rank_all";
    }

  // Push necessary flags.
  if ( (count(all.args.begin(), all.args.end(), "--csoaa_ldf") == 0 && count(all.args.begin(), all.args.end(), "--wap_ldf") == 0)
       || all.vm.count("rank_all"))
    {
      if (count(all.args.begin(), all.args.end(), "--csoaa_ldf") == 0)
	all.args.push_back("--csoaa_ldf");
      if (count(all.args.begin(), all.args.end(), "multiline") == 0)
	all.args.push_back("multiline");
      if (ld.rank_all && count(all.args.begin(), all.args.end(), "--csoaa_rank") == 0)
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
