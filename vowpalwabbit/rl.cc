#include <float.h>
#include <math.h>
#include <stdio.h>

#include "rl.h"
#include "simple_label.h"
#include "cache.h"
#include "global_data.h"
#include "example.h"

using namespace std;

namespace RL {

  const float pi = acos(-1);
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
  example* last_ec = NULL;
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

  void (*base_learner)(void*,example*) = NULL;

  void learn(void*a, example* ec)
  {
    vw* all = (vw*)a;
    reward_label* reward_label_data = (reward_label*)ec->ld;
    float prediction = 0.;
    float score = INT_MIN;
    bool doTrain = (reward_label_data->label != FLT_MAX) && (reward_label_data->weight > 0);
    size_t trace_length = ((size_t)1) << all->num_bits;

    // rl_start
    // Check for this tag to indicate the beginning of a new episode
    if(ec->tag.index() >= 8 && !strncmp((const char*) ec->tag.begin, "rl_start", 8)) {
      //      cerr << "New Episode..." << endl;
      if(last_ec != NULL) {
        // Free memory for last_ec
        dealloc_example(RL::delete_label, *last_ec);
	free(last_ec);
	last_ec = NULL;

        dealloc_example(RL::delete_label, *traces);
	free(traces);
	traces = NULL;
      }
	// Free memory for traces
    } 

    // Generate prediction
    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.label = reward_label_data->label; // later add in gamma*Q
    simple_temp.weight = 0.0; // Don't learn on this, use it to get Q_spap //reward_label_data->weight;
    ec->ld = &simple_temp;
    update_example_indicies(all->audit, ec, increment);
    base_learner(all,ec);
    float Q_spap = ec->partial_prediction;
    float delta = 0.0;

    if (doTrain && last_ec != NULL) {

      // Get previous state/reward
      reward_label* last_reward = (reward_label*)last_ec->ld;

      // Compute delta-target
      simple_temp.label = gamma * Q_spap + last_reward->label;
      simple_temp.weight = 0.0;//last_reward->weight;

      // Update/learn
      last_ec->ld = &simple_temp;
      update_example_indicies(all->audit, last_ec, increment);
      base_learner(all,last_ec);
      score = last_ec->partial_prediction;
      prediction = score;
      delta = simple_temp.label - prediction;
      last_ec->partial_prediction = 0.;
      last_ec->ld = last_reward;
      update_example_indicies(all->audit, last_ec, -total_increment);
    } else if(last_ec == NULL) {
    //       cerr << "New Episode" <<endl; 
    }
    

    ec->partial_prediction = 0.;
    ec->ld = reward_label_data;
    *(prediction_t*)&(ec->final_prediction) = Q_spap;
    update_example_indicies(all->audit, ec, -total_increment);
    if(doTrain) {
      if (last_ec != NULL) {
      
	//if (traces == NULL)
	//  traces = (float *)calloc(trace_length, sizeof(float));
	// Train on traces...
	size_t mask = all->weight_mask;
	float* weights = all->reg.weight_vectors;
	//cerr << "Delta: " << delta << endl;
      for (size_t* i = traces->indices.begin; i != traces->indices.end; i++) 
	{
      	  feature *f = traces->atomics[*i].begin;
	  for (; f != traces->atomics[*i].end; f++)
	    {
	      weights[f->weight_index & mask] += 0.0001 * delta * f->x;
	      //	      cerr << "(" << f->weight_index << "," << f->x  << ") ";
	    }
	}
      //            cerr << endl << endl;
	//	cerr << "Updating traces... " << (traces == NULL) << endl;
	// Decay traces and then update weights using them

	//	for (int i=0; i<(int)trace_length; i++) {
	//	  if (traces[i] != 0) {
	    //	    cout << traces[i] << " ";
	//	    traces[i] *= lambda*gamma;
	    //	    weights[i] += last_ec->eta_round * traces[i];
	    //   weights[i] += 0.025 * delta * traces[i];
	//	  }
	  //	  if(weights[i] != 0)
	  //	    cerr << weights[i] << " ";

      
	//		cout << endl;
	//		cout << "Delta: " << last_ec->eta_round/0.025 << " " << delta << endl;
	//	cerr << endl;
        // Add old features to traces

      //	for (int i=0; i<(int)trace_length; i++) {
      //	  if (traces[i] != 0) {
	    //	    cout << traces[i] << " ";
		    //	    weights[i] += last_ec->eta_round * traces[i];
      //	    weights[i] += 0.0001 * delta * traces[i];
      //	  }
	  //	  if(weights[i] != 0)
	  //	    cerr << weights[i] << " ";

      //	}
	
	// Lets try adding things to last_ec..
	// ec->indices are the name spaces really
	//	cerr << ec->indices.index() << " " << ec->indices[0] << " " << ec->atomics[98].begin->weight_index << " " << ec->atomics[98].begin->x << endl;
	//	cerr << last_ec->indices.index() << " " << last_ec->indices[0] << " " << last_ec->atomics[98].begin->weight_index << " " << last_ec->atomics[98].begin->x << endl;
	// for each name space in last_ec, find that namespace in ec (or add it)
      // decay traces...
      for(int i=0; i<traces->indices.index(); i++)  {
	for(int j=0; j<traces->atomics[traces->indices[i]].index(); j++) {
	  traces->atomics[traces->indices[i]][j].x *= lambda * gamma;
	}
      }

	for(int i=0; i<ec->indices.index(); i++) {
	  int j = 0;
	  bool found = false;
	  for(; j < traces->indices.index(); j++) {
	    if(ec->indices[i] == traces->indices[j]) {
	      found = true;
	      break;
	    }
	  }
	  if(found) {
	    //	    cerr << "last @ " << i << " <=> " << "current @ " << j << endl;
	    // Now though we need to check that they contain the same stuff...
	    size_t current_index = traces->indices[j];
	    size_t last_index = ec->indices[i];
	    for(int k=0; k<ec->atomics[last_index].index(); k++) {
	      j = 0;
	      found = false;
	      for(; j < traces->atomics[current_index].index(); j++) {
		if(ec->atomics[last_index][k].weight_index == traces->atomics[current_index][j].weight_index) {
		  found = true;
		  break;
		}
	      }
	      if(found) {
		traces->atomics[current_index][j].x += ec->atomics[last_index][k].x;
	      }
	      else {
		// not found, need to add it
		v_array<feature> *ecatomics = &traces->atomics[current_index];
		feature cpyfeat = { ec->atomics[last_index][k].x, ec->atomics[last_index][k].weight_index };
		push(*ecatomics, cpyfeat);
	      }
	    }
	  }
	  else {
	    //	    cerr << "Couldn't find " << i << " inserting.. " << ec->indices.index() << endl;
	    v_array<size_t> *ecindx = &traces->indices;
	    push(*ecindx, ec->indices[i]);
	    // okay but atomics for this index is empty... so add those too
	    v_array<feature> *ecatomics = &traces->atomics[ecindx->last()];
	    copy_array(*ecatomics, ec->atomics[ec->indices[i]]);
	    
	  }
	}
	//	for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
	//        {
	  
	  // then atomics are the features within those name spaces
	//	  feature *f = ec->atomics[*i].begin;
	//	  cerr << *i << " " << ec->indices.index() << endl;
	//          for (; f != ec->atomics[*i].end; f++)
	//	  {
	//	    cerr << "(" << f->weight_index << "," << f->x << ") ";
	//	  }
	//	  cerr << endl;
	//        }	
	//	cerr << endl;
	// Now we can clear the old example and copy over the new one
	dealloc_example(RL::delete_label, *last_ec);
	free(last_ec);
	last_ec = NULL;

      }
      // Copy over new example
      if(traces ==  NULL) {
	traces = alloc_example(sizeof(RL::reward_label));
	copy_example_data(traces, ec, sizeof(RL::reward_label));
      }
      if (last_ec == NULL) {
	last_ec = alloc_example(sizeof(RL::reward_label));
      }
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
	    /*	    if(last_ec != NULL) {
	      dealloc_example(RL::delete_label, *last_ec);
	      free(last_ec);
	      last_ec = NULL;
	      }*/
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
