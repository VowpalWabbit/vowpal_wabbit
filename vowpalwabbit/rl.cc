#include <float.h>
#include <math.h>
#include <stdio.h>

#include "rl.h"
#include "simple_label.h"
#include "cache.h"

using namespace std;

namespace RL {

const float pi = acos(-1);
  float gamma = 1.0;

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
    bool doTrain = reward_label_data->label != FLT_MAX;

    // rl_start
    // Check for this tag to indicate the beginning of a new episode
    if(ec->tag.index() >= 8 && !strncmp((const char*) ec->tag.begin, "rl_start", 8)) {
      last_ec = NULL;
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

    if (doTrain && last_ec != NULL) {
      // Get previous state/reward
      reward_label* last_reward = (reward_label*)last_ec->ld;

      // Compute delta-target
      simple_temp.label = gamma * Q_spap + last_reward->label;
      simple_temp.weight = last_reward->weight;

      // Update/learn
      last_ec->ld = &simple_temp;
      update_example_indicies(all->audit, last_ec, increment);
      base_learner(all,last_ec);
      score = last_ec->partial_prediction;
      prediction = score;

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
      // Clear out old example (actually we don't need to do this EVERY time I think..
      if (last_ec != NULL) {
	dealloc_example(RL::delete_label, *last_ec);
	free(last_ec);
	last_ec = NULL;
	}
      // Copy over new example
      if (last_ec == NULL)
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
	    if(last_ec != NULL) {
	      dealloc_example(RL::delete_label, *last_ec);
	      free(last_ec);
	      last_ec = NULL;
	    }
	    all->finish(all);
            return;
          }
        else 
	  ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, double s)
  {
    //    *(all.p->lp) = rl_label_parser;
    gamma = s;
    all.driver = drive_rl;
    base_learner = all.learn;
    all.learn = learn;
    increment = (all.length()) * all.stride;
    total_increment = increment*0.0;
  }
}
