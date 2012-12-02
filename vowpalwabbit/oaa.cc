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

using namespace std;

namespace OAA {

  char* bufread_label(mc_label* ld, char* c)
  {
    ld->label = *(size_t *)c;
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
    *(size_t *)c = ld->label;
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
    ld->label = -1;
    ld->weight = 1.;
  }

  void delete_label(void* v)
  {
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    mc_label* ld = (mc_label*)v;

    switch(words.index()) {
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
      cerr << "words.index() = " << words.index() << endl;
    }
  }

  //nonreentrant
  size_t k=0;
  size_t increment=0;
  size_t total_increment=0;

  void print_update(vw& all, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        mc_label* ld = (mc_label*) ec->ld;
        char label_buf[32];
        if (ld->label == INT_MAX)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf,"%8lu",(long unsigned int)ld->label);

        fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8lu %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long unsigned int)*(prediction_t*)&ec->final_prediction,
                (long unsigned int)ec->num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec)
  {
    mc_label* ld = (mc_label*)ec->ld;
    all.sd->weighted_examples += ld->weight;
    all.sd->total_features += ec->num_features;
    size_t loss = 1;
    if (ld->label == *(prediction_t*)&(ec->final_prediction))
      loss = 0;
    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  
    for (size_t* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, (float)(*(prediction_t*)&(ec->final_prediction)), 0, ec->tag);
  
    all.sd->example_number++;

    print_update(all, ec);
  }

  void (*base_learner)(void*,example*) = NULL;

  void learn_with_output(vw*all, example* ec, bool shouldOutput)
  {
    mc_label* mc_label_data = (mc_label*)ec->ld;
    size_t prediction = 1;
    float score = INT_MIN;
  
    if (mc_label_data->label > k && mc_label_data->label != (size_t)-1)
      cerr << "warning: label " << mc_label_data->label << " is greater than " << k << endl;
  
    string outputString;
    stringstream outputStringStream(outputString);

    for (size_t i = 1; i <= k; i++)
      {
        label_data simple_temp;
        simple_temp.initial = 0.;
        if (mc_label_data->label == i)
          simple_temp.label = 1;
        else
          simple_temp.label = -1;
        simple_temp.weight = mc_label_data->weight;
        ec->ld = &simple_temp;
        if (i != 1)
          update_example_indicies(all->audit, ec, increment);
        base_learner(all,ec);
        if (ec->partial_prediction > score)
          {
            score = ec->partial_prediction;
            prediction = i;
          }

        if (shouldOutput) {
          if (i > 1) outputStringStream << ' ';
          outputStringStream << i << ':' << ec->partial_prediction;
        }

        ec->partial_prediction = 0.;
      }
    ec->ld = mc_label_data;
    *(prediction_t*)&(ec->final_prediction) = prediction;
    update_example_indicies(all->audit, ec, -total_increment);

    if (shouldOutput) {
      outputStringStream << endl;
      all->print_text(all->raw_prediction, outputStringStream.str(), ec->tag);
    }
  }

  void learn(void*a, example* ec) {
    vw* all = (vw*)a;
    learn_with_output(all, ec, false);
  }

  void drive_oaa(void *in)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn_with_output(all, ec, all->raw_prediction > 0);
            output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
	  return;
        else 
          ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    //first parse for number of actions
    k = 0;
    if( vm_file.count("oaa") ) {
      k = vm_file["oaa"].as<size_t>();
      if( vm.count("oaa") && vm["oaa"].as<size_t>() != k )
        std::cerr << "warning: you specified a different number of actions through --oaa than the one loaded from predictor. Pursuing with loaded value of: " << k << endl;
    }
    else {
      k = vm["oaa"].as<size_t>();

      //append oaa with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --oaa " << k;
      all.options_from_file.append(ss.str());
    }

    *(all.p->lp) = mc_label_parser;
    all.driver = drive_oaa;
    base_learner = all.learn;
    all.base_learn = all.learn;
    all.learn = learn;

    all.base_learner_nb_w *= k;
    increment = (all.length()/all.base_learner_nb_w) * all.stride;
    total_increment = increment*(k-1);
  }
}
