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
using namespace CB;

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2

struct cb_adf {
  v_array<example*> ec_seq;
  
  bool need_to_clear;
  bool is_singleline;
  float csoaa_example_t;
  vw* all;
  
  base_learner* base;
  
  size_t cb_type;
  
  // An array of cb_label.
  v_array<CB::label> array;

  // suggest to add an array of cs_label.
  v_array<COST_SENSITIVE::label> cs_label_array;

  cb_class* known_cost;  
};

namespace CB_ADF {

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
  predict(data, base, data.ec_seq);  
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
	  learn(data, base, data.ec_seq);
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
    
  CB::print_update(all, CB::example_is_test(ec), ec, ec_seq);
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
  bool is_test_ec = CB::example_is_test(ec);
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
  if (missing_option<string, true>(all, "cb_adf", "Do Contextual Bandit learning with action dependent features.  Specify singleline or multiline."))
    return nullptr;

  cb_adf& ld = calloc_or_die<cb_adf>();

  ld.all = &all;
 
  string adf_arg;

  if( all.vm.count("cb_adf") )
    adf_arg = all.vm["cb_adf"].as<string>();

  if (adf_arg.compare("singleline") == 0 || adf_arg.compare("s") == 0) 
    ld.is_singleline = true;
  else
    ld.is_singleline = false;

  all.p->lp = CB::cb_label;

  if (count(all.args.begin(), all.args.end(),"--csoaa_ldf") == 0 && count(all.args.begin(), all.args.end(),"--wap_ldf") == 0)
    {
      all.args.push_back("--csoaa_ldf");
      all.args.push_back(adf_arg);
    }

  learner<cb_adf>& l = init_learner(&ld, setup_base(all), CB_ADF::predict_or_learn<true>, CB_ADF::predict_or_learn<false>);
  if (ld.is_singleline)
    l.set_finish_example(CB_ADF::finish_singleline_example);
  else
    l.set_finish_example(CB_ADF::finish_multiline_example);
  l.set_finish(CB_ADF::finish);
  l.set_end_examples(CB_ADF::end_examples); 
  return make_base(l);
}

inline bool observed_cost(cb_class* cl)
{
	//cost observed for this action if it has non zero probability and cost != FLT_MAX
	return (cl != nullptr && cl->cost != FLT_MAX && cl->probability > .0);
}

cb_class* get_observed_cost(CB::label ld)
{
	for (cb_class *cl = ld.costs.begin; cl != ld.costs.end; cl++)
	{
		if (observed_cost(cl))
			return cl;
	}
	return nullptr;
}

void predict(cb_adf& mydata, base_learner& base, v_array<example*> examples)
{

	// m2: still save, store, and restore
	// starting with 3 for loops
	// first of all, clear the container mydata.array.
	mydata.array.erase();

	// 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
	size_t index = 0;
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		mydata.array.push_back((**ec).l.cb);
		(**ec).l.cs = mydata.cs_label_array[index];  // To be checked with John.
		index++;
	}


	// 2nd: predict for each ex
	// // call base.predict for each vw exmaple in the sequence
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		base.predict(**ec);
	}

	// 3rd: restore cb_label for each example
	// (**ec).l.cb = mydata.array.element.
	size_t i = 0;
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		(**ec).l.cb = mydata.array[i];
		i++;
	}

}
void learn(cb_adf& mydata, base_learner& base, v_array<example*> examples)
{
	// find the line/entry with cost and prob.
	CB::label ld;
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		if ( (**ec).l.cb.costs.size() == 1 &&
			(**ec).l.cb.costs[0].cost != FLT_MAX &&
			(**ec).l.cb.costs[0].probability > 0)
		{
			ld = (**ec).l.cb;
		}
	}

	mydata.known_cost = get_observed_cost(ld);

	if (mydata.known_cost == nullptr)
	{
		cerr << "known cost is null." << endl;
	}

	// commented out. 
	// v_array<COST_SENSITIVE::label> array;
	// Use data structure mydata.cs_label_array to hold the cost - sensitive labels generated by gen_cs_example_ips.

	switch (mydata.cb_type)
	{
		case CB_TYPE_IPS:			
			gen_cs_example_ips(mydata, examples, mydata.cs_label_array);
			break;

		default:
			std::cerr << "Unknown cb_type specified for contextual bandit learning: " << mydata.cb_type << ". Exiting." << endl;
			throw exception();
	}

	if (mydata.cb_type != CB_TYPE_DM)
	{
		// 3 for-loops

		// first of all, clear the container mydata.array.
		mydata.array.erase();

		// 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
		size_t index = 0;
		for (example **ec = examples.begin; ec != examples.end; ec++)
		{
			mydata.array.push_back((**ec).l.cb);
			(**ec).l.cs = mydata.cs_label_array[index];  // To be checked with John.
			index++;
		}
		 

		// 2nd: learn for each ex
		// // call base.learn for each vw exmaple in the sequence
		for (example **ec = examples.begin; ec != examples.end; ec++)
		{
			base.learn(**ec);
		}

		// 3rd: restore cb_label for each example
		// (**ec).l.cb = mydata.array.element.
		size_t i = 0;
		for (example **ec = examples.begin; ec != examples.end; ec++)
		{
			(**ec).l.cb = mydata.array[i];
			i++;
		}
	}
}

void predict(cb_adf& mydata, base_learner& base, v_array<example*> examples)
{

	// m2: still save, store, and restore
	// starting with 3 for loops
	// first of all, clear the container mydata.array.
	mydata.array.erase();

	// 1st: save cb_label (into mydata) and store cs_label for each example, which will be passed into base.learn.
	size_t index = 0;
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		mydata.array.push_back((**ec).l.cb);
		(**ec).l.cs = mydata.cs_label_array[index];  // To be checked with John.
		index++;
	}


	// 2nd: predict for each ex
	// // call base.predict for each vw exmaple in the sequence
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		base.predict(**ec);
	}

	// 3rd: restore cb_label for each example
	// (**ec).l.cb = mydata.array.element.
	size_t i = 0;
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		(**ec).l.cb = mydata.array[i];
		i++;
	}

}

void gen_cs_example_ips(cb_adf& c, v_array<example*> examples, v_array<COST_SENSITIVE::label> array)
{
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		// Get CB::label for each example/line.
		CB::label ld = (**ec).l.cb;

		if ( ld.costs.size() == 1 )  // 2nd line
		{
			COST_SENSITIVE::wclass wc;
			wc.x = ld.costs[0].cost / ld.costs[0].probability;

			COST_SENSITIVE::label cs_ld;
			cs_ld.costs.push_back(wc);

			array.push_back(cs_ld);
		}
		else if (ld.costs.size() == 0)
		{
			COST_SENSITIVE::wclass wc;
			wc.x = 0.;

			if (true)  // compare if (**ec).tag == "shared"
			{
				COST_SENSITIVE::label cs_ld;
				cs_ld.costs.push_back(wc);
				array.push_back(cs_ld);
			}
			else
			{
				// In this case, push in an instance of wclass with maximum cost as merely a placeholder.
				wc.x = FLT_MAX;
				COST_SENSITIVE::label cs_ld;
				cs_ld.costs.push_back(wc);
				array.push_back(cs_ld);
			}
		}
	}
    
}

template <bool is_learn>
void gen_cs_label(cb_adf& c, example& ec, v_array<COST_SENSITIVE::label> array)
{
	COST_SENSITIVE::wclass wc;
	wc.wap_value = 0.;

	//get cost prediction for this label
	// num_actions should be 1 effectively.
	// my get_cost_pred function will use 1 for 'index-1+base'
	wc.x = CB_ALGS::get_cost_pred<is_learn>(c.scorer, c.known_cost, ec, 1, 1);

	//add correction if we observed cost for this action and regressor is wrong
	if (c.known_cost != nullptr && c.known_cost->action == label)
	{
		c.nb_ex_regressors++;
		c.avg_loss_regressors += (1.0f / c.nb_ex_regressors)*
			((c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x)
			- c.avg_loss_regressors);
		c.last_pred_reg = wc.x;
		c.last_correct_cost = c.known_cost->cost;

		wc.x += (c.known_cost->cost - wc.x) / c.known_cost->probability;
	}

	// do two-step pushes as for learn.
	COST_SENSITIVE::label cs_ld;
	cs_ld.costs.push_back(wc);
	array.push_back(cs_ld);
}

void gen_cs_example_dr(cb_adf& c, v_array<example*> examples, v_array<COST_SENSITIVE::label> array)
{
	for (example **ec = examples.begin; ec != examples.end; ec++)
	{
		// Get CB::label for each example/line.
		CB::label ld = (**ec).l.cb;

		if (ld.costs.size() == 1)  // 2nd line
		{
			gen_cs_label<true>(c, **ec, array);
		}
		else if (ld.costs.size() == 0)
		{
			gen_cs_label<false>(c, **ec, array);
		}
	}
}
