/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
/*
The offset tree algorithm for a fixed tree.  One difference with
respect to the original paper is that the rewards are in [-1,1] rather
than [0,1].

John Langford
*/

#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <pthread.h>
#include <time.h>
#include <boost/program_options.hpp>
#include "parse_regressor.h"
#include "parse_example.h"
#include "parse_args.h"
#include "parse_primitives.h"
#include "gd.h"
#include "hash.h"
#include "cache.h"

bool quiet = false;

struct offset_data {
  int label;
  float reward;
  float importance;
  float probability;
  v_array<char> tag;
};

void default_offset_label(void* v)
{
  offset_data* od = (offset_data*)v;
  od->label = -1;
  od->reward = 1.;
  od->importance = 0.;
  od->probability = 1.;
  od->tag.erase();
}

void delete_offset_label(void* v)
{
  offset_data* od = (offset_data*) v;
  if (od->tag.end_array != od->tag.begin)
    {
      free(od->tag.begin);
      od->tag.end_array = od->tag.begin;
    }
}

int max_label = 1;

void parse_offset_label(void* v, substring label_space, v_array<substring>& words)
{
  offset_data* od = (offset_data*) v;
  char* tab_location = safe_index(label_space.start,'\t',label_space.end);
  if (tab_location != label_space.end)
    label_space.start = tab_location+1;
  
  tokenize(' ',label_space, words);
  switch(words.index()) {
  case 0:
    break;
  case 1:
    od->label = int_of_substring(words[0]);
    break;
  case 2:
    od->label = int_of_substring(words[0]);
    od->reward = float_of_substring(words[1]);
    break;
  case 3:
    od->label = int_of_substring(words[0]);
    od->reward = float_of_substring(words[1]);
    od->importance = float_of_substring(words[2]);
    break;
  case 4:
    od->label = int_of_substring(words[0]);
    od->reward = float_of_substring(words[1]);
    od->importance = float_of_substring(words[2]);
    od->probability = float_of_substring(words[3]);
    break;
  case 5:
    cout << "Case 5" << endl;
    od->label = int_of_substring(words[0]);
    od->reward = float_of_substring(words[1]);
    od->importance = float_of_substring(words[2]);
    od->probability = float_of_substring(words[3]);

    push_many(od->tag, words[4].start, 
	      words[4].end - words[4].start);
    break;
  default:
    cerr << "malformed example!\n";
    cerr << "words.index() = " << words.index() << endl;
  }
  if (od->label > max_label)
    {
      cerr << "Error: observed label greater than max_label.  Bailing." << endl; 
      exit(1);
    }
}

void cache_offset_label(void* v, io_buf& cache)
{
  offset_data* od = (offset_data*) v;
  char *c;
  buf_write(cache, c, sizeof(od->label)+sizeof(od->reward)+sizeof(od->importance)+sizeof(od->probability)+int_size+od->tag.index());
  *(int*) c = od->label;
  c+= sizeof(od->label);
  *(float*)c = od->reward;
  c+= sizeof(od->reward);
  *(float*)c = od->importance;
  c+= sizeof(od->importance);
  *(float*)c = od->probability;
  c+= sizeof(od->probability);

  c = run_len_encode(c, od->tag.index());
  memcpy(c,od->tag.begin,od->tag.index());
  c += od->tag.index();
  cache.set(c);
}

size_t read_cached_offset_label(void* v, io_buf& cache)
{
  offset_data* od = (offset_data*) v;
  char *c;
  size_t total = sizeof(od->label)+sizeof(od->reward)+sizeof(od->importance)+sizeof(od->probability)+int_size;
  size_t tag_size = 0;
  if (buf_read(cache, c, total) < total) 
    return 0;

  od->label = *(int *)c;
  c += sizeof(od->label);
  od->reward = *(float *)c;
  c += sizeof(od->reward);
  od->importance = *(float *)c;
  c += sizeof(od->importance);
  od->probability = *(float *)c;
  c += sizeof(od->probability);
  c = run_len_decode(c, tag_size);

  cache.set(c);
  if (buf_read(cache, c, tag_size) < tag_size) 
    return 0;

  od->tag.erase();
  push_many(od->tag, c, tag_size); 
  return total+tag_size;
}

const label_parser offset_label = {default_offset_label, parse_offset_label, 
				   cache_offset_label, read_cached_offset_label, delete_offset_label, sizeof(offset_data)};

// returns the (i)th bit of label, where i = bitNum.
size_t get_bit(size_t label, size_t bitNum)
{
  size_t retVal = (label >> bitNum) & 1;
  return retVal;
}

struct node {
  int label;
  int level;
};

void print_nothing(int,float,v_array<char>)
{}

int offset_tree_predict(int tree_height, int max, example* ec, regressor& reg, 
			size_t thread_num, gd_vars& vars)
{
  int new_label = 0;
  for (int i = tree_height-1; i >= 0; i--)
    {
      if ((new_label | (1 << i)) <= max || get_bit(max,i) != 0)
	{// a real choice exists
	  ((label_data*)ec->ld)->label = FLT_MAX;
	  
	  uint32_t offset = 0;
	  if (i != tree_height-1)
	    {
	      node temp = {new_label,i};
	      offset = uniform_hash(&temp,sizeof(temp),0);
	      ec->partial_prediction = 0;
	    }
	  if (reg.global->audit)
	    cout << "predicting at node " << new_label << "\theight\t" << i << endl;
	  float pred = offset_predict(reg, ec, thread_num,vars,offset);
	  if ( pred > 0.5 ) 
	    new_label = new_label | (1 << i);
	}
    }
  return new_label;
}

void print_stats(float& total, float& cumulative, size_t num_events, int label, int new_label)
{
  cerr << (cumulative+total) / num_events << '\t' << cumulative/num_events*2 
       << '\t' << num_events << '\t' << label << '\t' << new_label 
       << endl;
  total += cumulative;
  cumulative = 0.;
}

void* offset_tree(void *in)
{
  go_params* params = (go_params*) in;
  int tree_height = (int)ceilf(log(max_label+1)/log(2.));

  size_t thread_num = params->thread_num;
  float constant_cumulative[max_label+1];
  for (int i = 0;i <= max_label; i++)
    constant_cumulative[i] = 0;

  example* ec = NULL;
  regressor reg = params->reg;
  float total = 0.;
  float cumulative = 0.;
  size_t num_events = 1;
  size_t next_dump = 1;

  label_data temp_label;

  while ( (ec = get_example(ec,thread_num)) )
    {
      offset_data* od = (offset_data*)ec->ld; 
      temp_label.undo = false;
      temp_label.weight = 1.;
      ec->ld = &temp_label;
      
      int new_label = offset_tree_predict(tree_height, max_label, ec, reg, 
					  thread_num, *(params->vars) );
      print_result(params->vars->predictions, new_label, od->tag);

      if (od -> label != -1)
	{
	  float value = od->reward * od->importance / od->probability;
	  constant_cumulative[od->label] += value;
	  if (new_label == od->label)
	    cumulative += value;
	  if (num_events == next_dump && !quiet)
	    {
	      print_stats(total,cumulative,num_events,od->label,new_label);
	      next_dump = next_dump*2;
	    }
	  num_events++;
	}
      if (od->label != -1 && fabs(od->reward) != 0. && params->vars->training)
	{//train
	  temp_label.weight = fabs(od->reward) * od->importance / od->probability;
	  new_label = od->label;
	  for (int i = 0; i < tree_height; i++)
	  {
	    new_label = new_label & ~(1 << i); // eat the bit to predict.
	    if ((new_label | (1 << i)) <= max_label || get_bit(max_label,i) != 0)
	      { // a real choice exists
		uint32_t offset = 0;
		if (i != tree_height-1)
		  {		      
		    node temp = {new_label,i};
		    offset = uniform_hash(&temp,sizeof(temp),0);
		  }
		
		size_t label_bit = get_bit(od->label,i);
		if (od->reward > 0)
		  ((label_data*)ec->ld)->label = label_bit;
		else
		  ((label_data*)ec->ld)->label = 1 - label_bit;
		
		if (reg.global->audit)
		  cerr << "training at node " << new_label << "\theight\t" << i << endl;
		train_offset_example(reg, ec, thread_num, *(params->vars), offset);
		ec->partial_prediction = 0;
		
		float prediction = offset_predict(reg,ec,thread_num,*(params->vars),offset);
		if ((prediction > 0.5 && label_bit != 1) 
		    || (prediction < 0.5 && label_bit != 0))
		  break;
	      }
	  }
	}
      ec->ld = (label_data*)od;
    }

  if (!quiet)
    {
      print_stats(total,cumulative, num_events, -1,-1);
      cerr << endl;
      for (int i = 0; i < max_label+1; i++)
	cerr << i << "\t";
      cerr << endl;
      
      for (int i = 0; i < max_label+1; i++)
	cerr << constant_cumulative[i]/num_events << "\t";
      cerr << endl;
    }
  return NULL;
}

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  size_t numpasses;
  float eta_decay;
  ofstream final_regressor;
  string final_regressor_name;
  regressor regressor1;
  example_source source;

  gd_vars vars;
  parser* p = new_parser(&source,&offset_label);

  int sum_sock = -1; // value will not be used.

  po::options_description desc("offset tree options");
  desc.add_options()
    ("max_label,m", po::value<int>(&max_label)->default_value(1), "Maximum label value");

  parse_args(argc, argv, desc, vars, eta_decay, 
	     numpasses, regressor1, p, 
	     final_regressor_name, sum_sock);
  if (!vars.quiet)
    cerr << "max_label =\t" << max_label << endl;

  quiet = vars.quiet;
  vars.quiet = true;
  vars.print = print_nothing;

    if (!quiet)
      {
        cerr << "average\tsince\texample\tcurrent\tcurrent" << endl;
        cerr << "reward\tlast\tcounter\tlabel\tpredict" << endl;
        cerr.precision(4);
      }

  size_t num_threads = regressor1.global->num_threads();

  for (; numpasses > 0; numpasses--) 
    {
      setup_parser(num_threads, p);
      pthread_t threads[num_threads];
      
      go_params* passers[num_threads];
      
      for (size_t i = 0; i < num_threads; i++) 
	{
	  passers[i] = (go_params*)calloc(1, sizeof(go_params));
	  passers[i]->init(&vars,regressor1, &final_regressor_name, i);

	  pthread_create(&threads[i], NULL, offset_tree, (void *) passers[i]);
      }

    for (size_t i = 0; i < num_threads; i++) 
      {
	pthread_join(threads[i], NULL);
	free(passers[i]);
      }
    destroy_parser(p);
    vars.eta *= eta_decay;
    reset_source(regressor1.global->num_bits, source);
  }
  if (final_regressor_name  != "")
    {
      final_regressor.open(final_regressor_name.c_str());
      dump_regressor(final_regressor, regressor1);
    }

  finalize_regressor(final_regressor, regressor1);
  finalize_source(source);
  source.global->pairs.~vector();
  free(source.global);
  float best_constant = vars.weighted_labels / vars.weighted_examples;
  float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
			 + (1.0 - best_constant)*best_constant*best_constant);

  if (!vars.quiet) 
    {
      cerr << endl << "finished run";
      cerr << endl << "number of examples = " << vars.example_number;
      cerr << endl << "weighted_examples = " << vars.weighted_examples;
      cerr << endl << "weighted_labels = " << vars.weighted_labels;
      cerr << endl << "average_loss = " << vars.sum_loss / vars.weighted_examples;
      cerr << endl << "best constant's loss = " << constant_loss;
      cerr << endl << "total feature number = " << vars.total_features;
      cerr << endl;
    }

  return 0;
}
