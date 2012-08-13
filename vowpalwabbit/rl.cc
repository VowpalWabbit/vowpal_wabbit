#include <float.h>
#include <math.h>
#include <stdio.h>
//#include <boost/unordered_set.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "rl.h"
#include "simple_label.h"
#include "cache.h"
#include "global_data.h"
#include "example.h"

using namespace std;


namespace RL {

  // Constant for PI
  const float pi = acos(-1);

  // Run time arguments for RL algorithms
  float gamma = 1.0;
  float lambda = 0;

  char* bufread_label(reward_label* ld, char* c)
  {
    ld->label = *(float *)c;
    c += sizeof(ld->label);
    ld->weight = *(float *)c;
    c += sizeof(ld->weight);
    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    reward_label* ld = (reward_label*) v;
    char *c;
    size_t total = sizeof(ld->label)+sizeof(ld->weight);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c);

    return total;
  }

  float weight(void* v)
  {
    reward_label* ld = (reward_label*) v;
    return ld->weight;
  }

  float initial(void* v)
  {
    return 0.;
  }

  char* bufcache_label(reward_label* ld, char* c)
  {
    *(float *)c = ld->label;
    c += sizeof(ld->label);
    *(float *)c = ld->weight;
    c += sizeof(ld->weight);
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    reward_label* ld = (reward_label*) v;
    buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
    c = bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    reward_label* ld = (reward_label*) v;
    ld->label = 0.;
    ld->weight = 1.;
  }

  void delete_label(void* v)
  {
  }

  void parse_label(shared_data*, void* v, v_array<substring>& words)
  {
    reward_label* ld = (reward_label*)v;
    switch(words.index()) {
    case 0:
      ld->weight = 0.0;
      break;
    case 1:
      ld->label = float_of_substring(words[0]);
      ld->weight = 1.0;
      break;
    case 2:
      ld->label = float_of_substring(words[0]);
      ld->weight = float_of_substring(words[1]);
      break;
    default:
      cerr << "malformed example!\n";
      cerr << "words.index() = " << words.index() << endl;
    }
  }

  //nonreentrant
  size_t k=0;
  size_t increment=0;
  size_t total_increment=0;

  // Previous example, needed to do temporal difference learning
  example* last_ec = NULL;

  // Eligibility traces
  example* traces = NULL;

  void print_update(vw& all, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        reward_label* ld = (reward_label*) ec->ld;
        char label_buf[32];
	sprintf(label_buf,"%f",ld->label);

        fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %f %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                *(prediction_t*)&ec->final_prediction,
                (long unsigned int)ec->num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec)
  {
    reward_label* ld = (reward_label*)ec->ld;
    all.sd->weighted_examples += ld->weight;
    all.sd->total_features += ec->num_features;
    size_t loss = 1;
    if (ld->label == *(prediction_t*)&(ec->final_prediction))
      loss = 0;
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;

    for (size_t i = 0; i<all.final_prediction_sink.index(); i++)
      {
        int f = all.final_prediction_sink[i];
        all.print(f, *(prediction_t*)&(ec->final_prediction), 0, ec->tag);
      }
  
    all.sd->example_number++;

    print_update(all, ec);
  }

  // Decay eligibility traces
  void decay_traces() {
    // If traces haven't been instantiated, do nothing
    if(traces != NULL) {
      if(lambda == 0) { // If lambda is zero just delete the traces all together
	// Free memory for traces
        dealloc_example(RL::delete_label, *traces);
	free(traces);
	traces = NULL;
      } else {
	for(int i=0; i<(int)traces->indices.index(); i++)  {
	  for(int j=0; j<(int)traces->atomics[traces->indices[i]].index(); j++) {
	    traces->atomics[traces->indices[i]][j].x *= lambda * gamma;
	  }
	}
      }
    }
  }

  // Update traces given a new example, instantiate if neccessary 
  void update_traces(example* ec) {
    if(traces != NULL) {
      // For each namespace... search for it in the traces
      for(int i=0; i<(int)ec->indices.index(); i++) {
	int j = 0;
	bool found = false; // Found the namespace in the traces
	for(; j < (int)traces->indices.index(); j++) {
	  if(ec->indices[i] == traces->indices[j]) {
	    found = true;
	    break;
	  }
	}

	// If the namespace already exists in the trace...
	if(found) {
	  // Now though we need to check that they contain the same features...
	  size_t current_index = traces->indices[j];
	  size_t last_index = ec->indices[i];
	  // Searching the namespace for the features
	  for(int k=0; k<(int)ec->atomics[last_index].index(); k++) {
	    j = 0;
	    found = false; // Reusing the found flag
	    for(; j < (int)traces->atomics[current_index].index(); j++) {
	      if(ec->atomics[last_index][k].weight_index == traces->atomics[current_index][j].weight_index) {
		found = true;
		break;
	      }
	    }
	    if(found) {
	      traces->atomics[current_index][j].x += ec->atomics[last_index][k].x;
	    } else {
	      // not found, need to add the feature
	      v_array<feature> *ecatomics = &traces->atomics[current_index];
	      feature cpyfeat = { ec->atomics[last_index][k].x, ec->atomics[last_index][k].weight_index };
	      push(*ecatomics, cpyfeat);
	    }
	  }
	} else {
	  v_array<size_t> *ecindx = &traces->indices;
	  push(*ecindx, ec->indices[i]);
	  // okay but atomics for this index is empty... so add those too
	  v_array<feature> *ecatomics = &traces->atomics[ecindx->last()];
	  copy_array(*ecatomics, ec->atomics[ec->indices[i]]);
	}
      }
    } else { // Traces are NULL, so instantiate them!
	traces = alloc_example(sizeof(RL::reward_label));
	copy_example_data(traces, ec, sizeof(RL::reward_label));
    }
  }

  // Update weights using the traces and specified delta
  void train_on_traces(vw* all, float delta) {
    if(traces != NULL) {
      //      cerr << "Delta: " << delta << endl;
        // Train on traces...
	size_t mask = all->weight_mask;
	float* weights = all->reg.weight_vectors;
	for (size_t* i = traces->indices.begin; i != traces->indices.end; i++) {
      	  feature *f = traces->atomics[*i].begin;
	  for (; f != traces->atomics[*i].end; f++) {
	    weights[f->weight_index & mask] +=  all->eta * delta * f->x;
	    //	    cerr << f->x << ":" << *i << ":" << f->weight_index << " ";
	  }
	}
	//	cerr << endl;
    }
  }

  void (*base_learner)(void*,example*) = NULL;

  set<string> actions;

  substring chars2ss(char* str)
  {
    string fullstring(str);
    substring result;
    result.begin = (char*)fullstring.c_str();
    result.end = result.begin + fullstring.length();
    return result;
  }

  substring str2ss(string str) 
  {
    substring result;
    result.begin = (char*)str.c_str();
    result.end = result.begin + str.length();
    return result;
  }

  boost::numeric::ublas::vector<double> compute_action_values(vw* all, example* ec) 
  {
    size_t mask = all->weight_mask;
    float* weights = all->reg.weight_vectors;
    size_t p_mask  = all->parse_mask;
    parser* p = all->p;
    boost::numeric::ublas::vector<float> action_values(actions.size());
    
    for (size_t* i = ec->indices.begin; i != ec->indices.end && ec->audit_features[*i].begin != NULL; i++) {
      audit_data *ad = ec->audit_features[*i].begin;
      // Insert action...
      actions.insert(string(ad->space));

      set<string>::iterator iter;
      int counter = 0;
      for(iter = actions.begin(); iter != actions.end(); ++iter) 
      {
	ad = ec->audit_features[*i].begin;
	for (; ad != ec->audit_features[*i].end; ad++) {
	  substring ft = chars2ss(ad->feature);
	  substring current = str2ss(*iter);
	  size_t whash = (p->hasher(ft, p->hasher(current, 97562527))) & p_mask;
	  if(action_values.size() <= counter)
	    action_values.resize(counter+1);
	  action_values[counter] += weights[whash & mask] * ad->x;
	  //cerr << ad->space << ":" << ad->weight_index << " --> " << (*iter) << ":" << whash << endl;
	}
	counter++;
      }
      //      cerr << "...." << endl;
    
    }
    set<string>::iterator iter = actions.begin();
    for(int i=0; i<action_values.size(); i++) {
      cerr << *iter << ":" << action_values[i] << " ";
      ++iter;
    }
    cerr << endl;
    //    cerr << "--------------------"<<endl << endl;
    // END DEBUG CODE
    return action_values;
  }
  
  void learn(void*a, example* ec)
  {
    vw* all = (vw*)a;
    reward_label* reward_label_data = (reward_label*)ec->ld;
    float prediction = 0.;
    float score = INT_MIN;

    // Train when a label is provided and the importance weight is greater than zero
    bool doTrain = (reward_label_data->label != FLT_MAX) && (reward_label_data->weight > 0);
    boost::numeric::ublas::vector<float> values = compute_action_values(all, ec);

    // rl_start
    // Check for this tag to indicate the beginning of a new episode
    if(ec->tag.index() >= 8 && !strncmp((const char*) ec->tag.begin, "rl_start", 8)) {
      //      cerr << "New Episode..." << endl;
      if(last_ec != NULL) {
        // Free memory for last_ec
        dealloc_example(RL::delete_label, *last_ec);
	free(last_ec);
	last_ec = NULL;
      }
      if(traces != NULL) {
	// Free memory for traces
        dealloc_example(RL::delete_label, *traces);
	free(traces);
	traces = NULL;
      }
    } 

    // Generate prediction for Q(s',a')
    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.label = reward_label_data->label; 
    simple_temp.weight = 0.0; // Don't learn on this, use it to get Q_spap

    float Q_spap = 0.0;
    if(ec->tag.index() >= 6 && !strncmp((const char*) ec->tag.begin, "rl_end", 6)) {
      Q_spap = 0.0;
    } else {
      ec->ld = &simple_temp;
      update_example_indicies(all->audit, ec, increment);
      base_learner(all,ec);
      Q_spap = ec->partial_prediction;
    }

    // Training and we are ready to update
    if (doTrain && last_ec != NULL) {
      // Standard RL Loss (delta) or use VW's built in loss function
      bool rl_loss = true;

      // Get previous state/reward, only used so that it can be freed later
      reward_label* last_reward = (reward_label*)last_ec->ld;

      // Compute delta-target
      simple_temp.label = gamma * Q_spap + reward_label_data->label;
      
      // If we want to use VW's update system then set this to non-zero
      if(rl_loss) {
	simple_temp.weight = 0.0;
      } else {
	simple_temp.weight = reward_label_data->weight;
      }
      
      // Update/learn on previous example
      last_ec->ld = &simple_temp;
      update_example_indicies(all->audit, last_ec, increment);
      base_learner(all,last_ec);
      score = last_ec->partial_prediction;
      prediction = score;

      last_ec->partial_prediction = 0.;
      last_ec->ld = last_reward;
      update_example_indicies(all->audit, last_ec, -total_increment);

      // Handle Eligibility Traces
      // Decay X_t-1
      decay_traces();
      if(rl_loss) {
	// Add x_t, then train
	update_traces(last_ec);
	train_on_traces(all, gamma * Q_spap + reward_label_data->label - prediction);
      } else {
	// Train using traces (training for x_t already done by VW), then add x_t to the traces
	train_on_traces(all, (last_ec->eta_round / all->eta));
	update_traces(last_ec);
      }
    }     

    // Update prediction that gets returned: Q(s',a')
    ec->partial_prediction = 0.;
    ec->ld = reward_label_data; // Give current example its label back
    *(prediction_t*)&(ec->final_prediction) = Q_spap;
    update_example_indicies(all->audit, ec, -total_increment);

    // Only update the previous example if we are training (not just evaluating)
    if(doTrain) {
      // Free memory before copying over latest example
      if(last_ec != NULL) {
	dealloc_example(RL::delete_label, *last_ec);
	free(last_ec);
	last_ec = NULL;
      }

      // Copy over latest example, but only during training..
      last_ec = alloc_example(sizeof(RL::reward_label));
      copy_example_data(last_ec, ec, sizeof(RL::reward_label));
    }

  }

  void drive_rl(void *in)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn(all, ec);
	    output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
          {
	    // Clean up example and traces between sessions
	    if(last_ec != NULL) {
	      dealloc_example(RL::delete_label, *last_ec);
	      free(last_ec);
	      last_ec = NULL;
	    }

	    if(traces != NULL) {
	      dealloc_example(RL::delete_label, *traces);
	      free(traces);
	      traces = NULL;
	    }
	    all->finish(all);
            return;
          }
        else 
	  ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, double l, double g)
  {
    //    *(all.p->lp) = rl_label_parser;
    lambda = l;
    gamma = g;
    all.driver = drive_rl;
    base_learner = all.learn;
    all.learn = learn;
    increment = 0.0;//(all.length()) * all.stride;
    total_increment = increment*0.0;
  }
}
