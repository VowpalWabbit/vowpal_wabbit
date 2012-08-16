/*

Reduction for performing either On-Policy or Off-Policy value function based Reinforcement Learning. 
The algorithms implemented are:

    Sarsa(lambda) - On-Policy RL algorithm which uses VW for value function approximation. Updates 
    state-action values based upon the current estimates of the current state-action value and the 
    value for the next observed state-action pair. Full explanation of the algorithm can be found 
    in Reinforcement Learning: An Introduction (Sutton & Barto), and many other places.
    
    TreeBackup - Off-Policy RL algorithm which uses VW for value function approximation. Updates the 
    previous state-action value based upon the current estimate and the sum over state-action values 
    for all possible actions of the next state, multiplied by the probability of selecting that action. 
    Full explanation of this algorithm can be found in the paper Eligibility Traces for Off-Policy Evaluation 
    (Precup, Sutton, and Singh 2000). 

Both algorithms make use of eligibility traces, which are stored in an example structure that is updated 
incrementally. 

The input format is standard VW input, with the requirement that there be an ACTION namespace, whose first character 
is assumed to be 'A'. This is only a strict requirement for TreeBackup because the algorithm must be able to 
consider multiple actions at each step. 

The label should be the reward for the current time step, and the prediction returned is the current state-action value.
Finally, we use tags to mark the start and end of episodes. "'rl_start" for the beginning of an episode and "'rl_end" for the 
end of an episode. 

So a sequence could be as follows:

0.0 'rl_start |Action RIGHT |f X:0.0 Y:0.123
0.0 |Action UP |f X:0.25 Y:0.123
0.0 |Action UP |f X:0.25 Y:0.373
0.0 |Action RIGHT |f X:0.24 Y:0.623
0.0 |Action UP |f X:0.5 Y:0.623
1.0 'rl_end |Action DOWN

Note that there need not be any features passed on the last example of the episode because these features will not be used. 
The examples are temporally linked, so the general form is:

reward |Action action |featureNamespace statefeatures

0 'rl_start |Action a0 |f s0
r0 |Action a1 |f s1
r1 |Action a2 |f s2
....
rn 'rl_end |Action

This sequence forms the traditional experience trajectory of the RL agent:
(s0, a0) -- r0 --> (s1, a1) -- r1 --> (s2, a2) ...


The command line arguments are: 

--oprl lambda    Where lambda is the eligibility trace decay rate used by both algorithms. 
--gamma arg      Where arg provides the gamma value used in both algorithms which sets the horizon, and defaults to 1.0.

Currently the only way to switch between Sarsa and TreeBackup is in the code, because the main use for this seems to be 
off-policy learning. This switch and other values such as 'epsilon' for the exploration rate should ideally be set through 
command line arguments, but I was hesitant to add a whole mess of additional flags. 

*/


#include <float.h>
#include <math.h>
#include <stdio.h>
#include <boost/numeric/ublas/vector.hpp>

#include "rl.h"
#include "simple_label.h"
#include "cache.h"
#include "global_data.h"
#include "example.h"

#include "gd.h" // Making use of the build in training functions

using namespace std;

/*
    This was based on one of the existing reductions, so some of these functions may
    be unneccessary.
*/

namespace RL {

  // Constants that shouldn't be constant ideally
  const bool tree_backup = true;
  const float epsilon = 0.1;
  bool rl_loss = true; // Standard RL Loss (delta) or use VW's built in loss function

  // Run time arguments for RL algorithms
  float gamma = 1.0;
  float lambda = 0;

  // Previous example, needed to do temporal difference learning
  example* last_ec = NULL;

  // Eligibility traces
  example* traces = NULL;

  // Actions known to be available
  set<vector<feature_value> > actions;

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
    if(traces == NULL) 
      return;

    if(lambda == 0) { // If lambda is zero just delete the traces all together
      // Free memory for traces
      dealloc_example(RL::delete_label, *traces);
      free(traces);
      traces = NULL;
    } else {
      // Loop over the namespaces in the traces
      for(int i=0; i<(int)traces->indices.index(); i++)  {
	// For each feature, decay its value
	for(int j=0; j<(int)traces->atomics[traces->indices[i]].index(); j++) {
	  traces->atomics[traces->indices[i]][j].x *= lambda * gamma;
	}
      }
    }
  }

  // Update traces given a new example, instantiate if neccessary 
  // This is a little messy
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
  void train_on_traces(vw* all, example* ec, float delta) {
    if(ec == NULL) 
      return;

    if (delta != 0.) {
      // Copied from gd.cc (Can't use it directly without predict getting called)
      if (all->adaptive)
	if (all->power_t == 0.5 || !all->exact_adaptive_norm)
	  adaptive_inline_train(*all,ec,delta);
	else
	  general_adaptive_train(*all,ec,delta,all->power_t);
      else
	inline_train(*all, ec, delta);
      if (all->sd->contraction < 1e-10)  // updating weights now to avoid numerical instability
	sync_weights(*all);
    }
  }

  void (*base_learner)(void*,example*) = NULL;

  // Convert our vector of sortable feature-values back to an array of features
  vector<feature_value> varray_to_vector(v_array<feature> array) 
  {
    vector<feature_value> vect;
    for(feature* f = array.begin; f != array.end; f++) 
    {
      if(f->x != 0.) {
	feature_value copyF = {f->x,f->weight_index};
	vect.push_back(copyF);
      }
    }
    return vect;
  }

  // Needed this function from elsewhere
  feature copy_feature(feature src) {
    feature f = { src.x, src.weight_index };
    return f;
  }

  // Compute action values for the current example (state)
  vector<float> compute_action_values(vw* all, example* ec) 
  {
    vector<float> values;

    // Backup current action for the example (ec)
    v_array<feature> backup;
    copy_array(backup, ec->atomics[65], copy_feature);

    // Iterate over our known actions, generate a value for each one
    set<vector<feature_value> >::iterator iter = actions.begin();
    for( ; iter != actions.end(); iter++) {
      ec->atomics[65].erase();
      for(int i=0; i< (*iter).size(); i++) {
	feature f = {(*iter)[i].x,(*iter)[i].weight_index};
	push(ec->atomics[65], f);
      } 

      // Generate the prediction (state-action value)
      ec->partial_prediction = 0.;
      base_learner(all,ec);
      values.push_back(ec->partial_prediction);
      ec->done = false;
    }

    // Copy the example's actions back
    copy_array(ec->atomics[65], backup, copy_feature);
    return values;
  }

  void learn(void*a, example* ec)
  {
    vw* all = (vw*)a;
    reward_label* reward_label_data = (reward_label*)ec->ld;
    float prediction = 0.;
    float score = INT_MIN;

    // Train when a label is provided and the importance weight is greater than zero
    bool doTrain = (reward_label_data->label != FLT_MAX) && (reward_label_data->weight > 0);

    float Q_spap = 0.0;
    float pred_out = 0.0;

    // If there are actions provided, add them to the set of known actions
    if(ec->atomics[65].index() > 0)
    {
      // This is a set, so we don't get duplicates
      actions.insert(varray_to_vector(ec->atomics[65]));
    }

    // Check for rl_start tag to indicate the beginning of a new episode
    if(ec->tag.index() >= 8 && !strncmp((const char*) ec->tag.begin, "rl_start", 8)) {
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

    // Check for rl_end tag
    if(ec->tag.index() >= 6 && !strncmp((const char*) ec->tag.begin, "rl_end", 6)) {
      // If end of episode then Q_spap should always be zero, no extra computation needed
      Q_spap = 0.0;
    } else {
      // Otherwise, generate prediction for Q_spap
      ec->partial_prediction = 0.;
      ec->ld = &simple_temp;
      base_learner(all,ec);
      ec->done = false;

      // Leave this alone if doing Sarsa
      Q_spap = ec->partial_prediction;
      pred_out = Q_spap;

      // Tree Backup Algorithm?
      if(tree_backup) {
	Q_spap = 0.0;
	vector<float> values = compute_action_values(all, ec);
	vector<float> pi(values.size());
	int max_index = 0;

	// Assign an epsilon-greedy policy
	float rand_pi = (1.0/(float)values.size()) * epsilon;

	for(int i=0; i<values.size(); i++) {
	  if(values[i] > values[max_index])
	    max_index = i;
	  pi[i] = rand_pi;
	}
	pi[max_index] = (1.0 - epsilon) + rand_pi;
	
	for(int i=0; i<values.size(); i++) {
	  Q_spap += values[i] * pi[i];
	}
      }
    }

    // Prediction made, ready to update
    if (doTrain && last_ec != NULL) {

      // Get previous state/reward, only used so that it can be freed later
      reward_label* last_reward = (reward_label*)last_ec->ld;

      // Compute delta-target
      simple_temp.label = gamma * Q_spap + reward_label_data->label;
      simple_temp.weight = 0.0;
      
      // Update/learn on previous example
      last_ec->ld = &simple_temp;
      base_learner(all,last_ec);
      score = last_ec->partial_prediction;
      prediction = score;

      last_ec->partial_prediction = 0.;
      last_ec->ld = last_reward;

      // Handle Eligibility Traces
      // Decay X_t-1
      decay_traces();
      // Add x_t, then train
      update_traces(last_ec);

      if(rl_loss) {
	last_ec->eta_round = (gamma * Q_spap + reward_label_data->label - prediction) * all->eta;
      }

      // Train
      train_on_traces(all, traces, last_ec->eta_round);
    }     

    // Update prediction that gets returned: Q(s',a')
    ec->partial_prediction = 0.;
    ec->ld = reward_label_data; // Give current example its label back
    *(prediction_t*)&(ec->final_prediction) = pred_out;

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
    lambda = l;
    gamma = g;
    all.driver = drive_rl;
    base_learner = all.learn;
    all.learn = learn;
  }
}
