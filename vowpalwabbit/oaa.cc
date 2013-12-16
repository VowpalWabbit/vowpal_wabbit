/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "oaa.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"

using namespace std;

namespace OAA {

  struct oaa{
    uint32_t k;
    vw* all;
  };

    char* bufread_label(mc_label* ld, char* c)
  {
    ld->label = *(uint32_t *)c;
    c += sizeof(ld->label);
    ld->weight = *(float *)c;
    c += sizeof(ld->weight);
    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    mc_label* ld = (mc_label*) v;
    char *c;
    size_t total = sizeof(ld->label)+sizeof(ld->weight);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c);

    return total;
  }

  float weight(void* v)
  {
    mc_label* ld = (mc_label*) v;
    return (ld->weight > 0) ? ld->weight : 0.f;
  }

  float initial(void* v)
  {
    return 0.;
  }

  char* bufcache_label(mc_label* ld, char* c)
  {
    *(uint32_t *)c = ld->label;
    c += sizeof(ld->label);
    *(float *)c = ld->weight;
    c += sizeof(ld->weight);
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    mc_label* ld = (mc_label*) v;
    buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
    c = bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    mc_label* ld = (mc_label*) v;
    ld->label = (uint32_t)-1;
    ld->weight = 1.;
  }

  void delete_label(void* v)
  {
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    mc_label* ld = (mc_label*)v;

    switch(words.size()) {
    case 0:
      break;
    case 1:
      ld->label = int_of_substring(words[0]);
      ld->weight = 1.0;
      break;
    case 2:
      ld->label = int_of_substring(words[0]);
      ld->weight = float_of_substring(words[1]);
      break;
    default:
      cerr << "malformed example!\n";
      cerr << "words.size() = " << words.size() << endl;
    }
    if (ld->label == 0)
      {
	cout << "label 0 is not allowed for multiclass.  Valid labels are {1,k}" << endl;
	throw exception();
      }
  }

  void print_update(vw& all, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        mc_label* ld = (mc_label*) ec->ld;
        char label_buf[32];
        if (ld->label == INT_MAX)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf,"%8ld",(long int)ld->label);

        if(!all.holdout_set_off && all.current_pass >= 1)
        {
          if(all.sd->holdout_sum_loss == 0. && all.sd->weighted_holdout_examples == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss/all.sd->weighted_holdout_examples);

          if(all.sd->holdout_sum_loss_since_last_dump == 0. && all.sd->weighted_holdout_examples_since_last_dump == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss_since_last_dump/all.sd->weighted_holdout_examples_since_last_dump);

            fprintf(stderr, "%8ld %8.1f   %s %8ld %8lu h\n",
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
	      (long int)ec->final_prediction,
	      (long unsigned int)ec->num_features);

          all.sd->weighted_holdout_examples_since_last_dump = 0;
          all.sd->holdout_sum_loss_since_last_dump = 0.0;
        }
        else
          fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8ld %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long int)ec->final_prediction,
                (long unsigned int)ec->num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec)
  {
    mc_label* ld = (mc_label*)ec->ld;

    size_t loss = 1;
    if (ld->label == (uint32_t)ec->final_prediction)
      loss = 0;

    if(ec->test_only)
    {
      all.sd->weighted_holdout_examples += ec->global_weight;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += ec->global_weight;
      all.sd->weighted_holdout_examples_since_last_pass += ec->global_weight;
      all.sd->holdout_sum_loss += loss;
      all.sd->holdout_sum_loss_since_last_dump += loss;
      all.sd->holdout_sum_loss_since_last_pass += loss;//since last pass
    }
    else
    {
      all.sd->weighted_examples += ld->weight;
      all.sd->total_features += ec->num_features;
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      all.sd->example_number++;
    }
 
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, ec->final_prediction, 0, ec->tag);

    OAA::print_update(all, ec);
  }

  void finish_example(vw& all, void*, example* ec)
  {
    output_example(all, ec);
    VW::finish_example(all, ec);
  }

  void learn(void* d, learner& base, example* ec)
  {
    oaa* o=(oaa*)d;

    vw* all = o->all;

    bool shouldOutput = all->raw_prediction > 0;

    mc_label* mc_label_data = (mc_label*)ec->ld;
    float prediction = 1;
    float score = INT_MIN;
  
    if (mc_label_data->label == 0 || (mc_label_data->label > o->k && mc_label_data->label != (uint32_t)-1))
      cout << "label " << mc_label_data->label << " is not in {1,"<< o->k << "} This won't work right." << endl;
    
    string outputString;
    stringstream outputStringStream(outputString);

    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.weight = mc_label_data->weight;
    ec->ld = &simple_temp;

    for (size_t i = 1; i <= o->k; i++)
      {
        if (mc_label_data->label == i)
          simple_temp.label = 1;
        else
          simple_temp.label = -1;
        base.learn(ec, i-1);
        if (ec->partial_prediction > score)
          {
            score = ec->partial_prediction;
            prediction = (float)i;
          }
	
        if (shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec->partial_prediction;
        }
      }	
    ec->ld = mc_label_data;
    ec->final_prediction = prediction;

    if (shouldOutput) 
      all->print_text(all->raw_prediction, outputStringStream.str(), ec->tag);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    oaa* data = (oaa*)calloc(1, sizeof(oaa));
    //first parse for number of actions
    if( vm_file.count("oaa") ) {
      data->k = (uint32_t)vm_file["oaa"].as<size_t>();
      if( vm.count("oaa") && (uint32_t)vm["oaa"].as<size_t>() != data->k )
        std::cerr << "warning: you specified a different number of actions through --oaa than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
    }
    else {
      data->k = (uint32_t)vm["oaa"].as<size_t>();

      //append oaa with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --oaa " << data->k;
      all.options_from_file.append(ss.str());
    }

    data->all = &all;
    *(all.p->lp) = mc_label_parser;

    learner* l = new learner(data, learn, all.l, data->k);
    l->set_finish_example(finish_example);

    return l;
  }
}
