#include <float.h>
#include <math.h>
#include <stdio.h>

#include "csoaa.h"
#include "simple_label.h"
#include "cache.h"
#include "oaa.h"

using namespace std;
void feature_value(substring &s, v_array<substring>& name, float &v);
size_t hashstring (substring s, unsigned long h);

namespace CSOAA {

bool is_test_label(label* ld)
{
  if (ld->costs.index() == 0)
    return true;
  for (size_t i=0; i<ld->costs.index(); i++)
    if (FLT_MAX != ld->costs[i].x)
      return false;
  return true;
}
  
char* bufread_label(label* ld, char* c, io_buf& cache)
{
  uint32_t num = *(uint32_t *)c;
  c += sizeof(uint32_t);
  size_t total = sizeof(feature)*num;
  if (buf_read(cache, c, total) < total) 
    {
      cout << "error in demarshal of cost data" << endl;
      return c;
    }
  for (uint32_t i = 0; i<num; i++)
    {
      feature temp = *(feature *)c;
      c += sizeof(feature);
      push(ld->costs, temp);
    }
  
  return c;
}

size_t read_cached_label(void* v, io_buf& cache)
{
  label* ld = (label*) v;
  char *c;
  size_t total = sizeof(uint32_t);
  if (buf_read(cache, c, total) < total) 
    return 0;
  c = bufread_label(ld,c, cache);
  
  return total;
}

float weight(void* v)
{
  return 1.;
}

float initial(void* v)
{
  return 0.;
}

char* bufcache_label(label* ld, char* c)
{
  *(uint32_t *)c = ld->costs.index();
  c += sizeof(uint32_t);
  for (size_t i = 0; i< ld->costs.index(); i++)
    {
      *(feature *)c = ld->costs[i];
      c += sizeof(feature);
    }
  return c;
}

void cache_label(void* v, io_buf& cache)
{
  char *c;
  label* ld = (label*) v;
  buf_write(cache, c, sizeof(uint32_t)+sizeof(feature)*ld->costs.index());
  bufcache_label(ld,c);
}

void default_label(void* v)
{
}

void delete_label(void* v)
{
}

size_t increment=0;
v_array<substring> name;

void parse_label(void* v, v_array<substring>& words)
{
  label* ld = (label*)v;

  for (size_t i = 0; i < words.index(); i++)
    {
      feature f;
      feature_value(words[i], name, f.x);
      
      f.weight_index = 0;
      if (name.index() == 1 || name.index() == 2)
	{
	  f.weight_index = hashstring(name[0], 0);
	  if (f.weight_index < 1 || f.weight_index > global.k)
	    cerr << "invalid cost specification: " << f.weight_index << endl;
	}
      else 
	cerr << "malformed cost specification!" << endl;
      
      if (name.index() == 1)
	f.x = FLT_MAX;
      
      push(ld->costs, f);
    }

  if (words.index() == 0)
    {
      for (size_t i = 1; i <= global.k; i++)
	{
	  feature f = {f.x, i};
	  push(ld->costs, f);
	}
    }
}

void print_update(bool is_test, example *ec)
{
  if (global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)
    {
      char label_buf[32];
      if (is_test)
	strcpy(label_buf," unknown");
      else
	sprintf(label_buf," known");

      fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8i %8lu\n",
	      global.sd->sum_loss/global.sd->weighted_examples,
	      global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
	      (long int)global.sd->example_number,
	      global.sd->weighted_examples,
	      label_buf,
	      *(OAA::prediction_t*)&ec->final_prediction,
	      (long unsigned int)ec->num_features);
     
      global.sd->sum_loss_since_last_dump = 0.0;
      global.sd->old_weighted_examples = global.sd->weighted_examples;
      global.sd->dump_interval *= 2;
    }
}

void output_example(example* ec)
{
  label* ld = (label*)ec->ld;
  global.sd->weighted_examples += 1.;
  global.sd->total_features += ec->num_features;
  float loss = 0.;
  if (!is_test_label(ld))
    {//need to compute exact loss
      size_t pred = *(OAA::prediction_t*)&ec->final_prediction;

      float chosen_loss = FLT_MAX;
      float min = FLT_MAX;
      for (feature *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
        if (cl->weight_index == pred)
          chosen_loss = cl->x;
        if (cl->x < min)
          min = cl->x;
      }
      if (chosen_loss == FLT_MAX)
        cerr << "warning: csoaa predicted an invalid class" << endl;

      loss = chosen_loss - min;
    }

  global.sd->sum_loss += loss;
  global.sd->sum_loss_since_last_dump += loss;
  
  for (size_t i = 0; i<global.final_prediction_sink.index(); i++)
    {
      int f = global.final_prediction_sink[i];
      global.print(f, *(OAA::prediction_t*)&ec->final_prediction, 0, ec->tag);
    }
  
  global.sd->example_number++;

  print_update(is_test_label((label*)ec->ld), ec);
}

  void (*base_learner)(example*) = NULL;
  void (*base_finish)() = NULL;

  void learn(example* ec)
  {
    label* ld = (label*)ec->ld;
    float prediction = 1;
    float score = FLT_MAX;
    size_t current_increment = 0;

    for (feature *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        size_t i = cl->weight_index;
	
	label_data simple_temp;
	simple_temp.initial = 0.;

	if (cl->x == FLT_MAX)
	  {
	    simple_temp.label = FLT_MAX;
	    simple_temp.weight = 0.;
	  }
	else
	  {
	    simple_temp.label = cl->x;
	    simple_temp.weight = 1.;
	  }

	ec->ld = &simple_temp;

        size_t desired_increment = increment * (i-1);
        if (desired_increment != current_increment) {
	  OAA::update_indicies(ec, desired_increment - current_increment);
          current_increment = desired_increment;
        }
	ec->partial_prediction = 0.;

	base_learner(ec);
	if (ec->partial_prediction < score)
	  {
	    score = ec->partial_prediction;
	    prediction = i;
	  }
      }
    ec->ld = ld;
    *(OAA::prediction_t*)&(ec->final_prediction) = prediction;
    if (current_increment != 0)
      OAA::update_indicies(ec, -current_increment);
  }

void finish()
{
  if (name.begin != NULL)
    free(name.begin);
  base_finish();
}

void drive_csoaa()
{
  example* ec = NULL;
  while ( true )
    {
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  learn(ec);
          output_example(ec);
	  free_example(ec);
	}
      else if (parser_done())
	{
	  finish();
	  return;
	}
      else 
	;
    }
}

  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)())
{
  *(global.lp) = cs_label_parser;
  global.k = s;
  global.driver = drive_csoaa;
  base_learner = base_l;
  base_finish = base_f;
  increment = (global.length()/global.k) * global.stride;
}

}

namespace CSOAA_LDF {

  v_array<example*> ec_seq = v_array<example*>();
  size_t read_example_this_loop = 0;

  void (*base_learner)(example*) = NULL;
  void (*base_finish)() = NULL;

  void do_actual_learning()
  {
    if (ec_seq.index() <= 0) return;  // nothing to do

    int K = ec_seq.index();
    float min_cost = FLT_MAX;
    v_array<float> predictions = v_array<float>();
    float min_score = FLT_MAX;
    size_t prediction = 0;
    float prediction_cost = 0.;
    bool isTest = example_is_test(*ec_seq.begin);
    for (int k=0; k<K; k++) {
      example *ec = ec_seq.begin[k];
      label   *ld = (label*)ec->ld;

      label_data simple_label;
      simple_label.initial = 0.;
      simple_label.label = FLT_MAX;
      simple_label.weight = 0.;

      if (ld->weight < min_cost) 
        min_cost = ld->weight;
      if (example_is_test(ec) != isTest) {
        isTest = true;
        cerr << "warning: got mix of train/test data; assuming test" << endl;
      }

      ec->ld = &simple_label;
      base_learner(ec); // make a prediction
      push(predictions, ec->partial_prediction);
      if (ec->partial_prediction < min_score) {
        min_score = ec->partial_prediction;
        prediction = ld->label;
        prediction_cost = ld->weight;
      }

      ec->ld = ld;
    }
    prediction_cost -= min_cost;
    // do actual learning
    for (int k=0; k<K; k++) {
      example *ec = ec_seq.begin[k];
      label   *ld = (label*)ec->ld;

      // learn
      label_data simple_label;
      simple_label.initial = 0.;
      simple_label.label = ld->weight;
      simple_label.weight = 1.;
      ec->ld = &simple_label;
      ec->partial_prediction = 0.;
      base_learner(ec);

      // fill in test predictions
      *(OAA::prediction_t*)&(ec->final_prediction) = (prediction == ld->label) ? 1 : 0;
      ec->partial_prediction = predictions.begin[k];
      
      // restore label
      ec->ld = ld;
    }
    predictions.erase();
    free(predictions.begin);
  }

  void output_example(example* ec)
  {
    label* ld = (label*)ec->ld;
    global.sd->weighted_examples += 1.;
    global.sd->total_features += ec->num_features;
    float loss = 0.;
    if (!example_is_test(ec) && (ec->final_prediction == 1))
      loss = ld->weight;
    global.sd->sum_loss += loss;
    global.sd->sum_loss_since_last_dump += loss;

    for (size_t i = 0; i<global.final_prediction_sink.index(); i++) {
      int f = global.final_prediction_sink[i];
      global.print(f, *(OAA::prediction_t*)&ec->final_prediction, 0, ec->tag);
    }
  
    global.sd->example_number++;

    CSOAA::print_update(example_is_test(ec), ec);
  }

  void clear_seq(bool output)
  {
    if (ec_seq.index() > 0) 
      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++) {
        if (output)
          output_example(*ecc);
        free_example(*ecc);
      }
    ec_seq.erase();
  }

  void learn(example *ec) {
    if (ec_seq.index() >= global.ring_size - 2) { // give some wiggle room
      cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << endl;
      do_actual_learning();
      clear_seq(true);
    }

    if (example_is_newline(ec)) {
      do_actual_learning();
      clear_seq(true);
      global_print_newline();
    } else {
      push(ec_seq, ec);
    }
  }

  void finish()
  {
    clear_seq(true);
    if (ec_seq.begin != NULL)
      free(ec_seq.begin);
    base_finish();
  }

  void drive_csoaa_ldf()
  {
    example* ec = NULL;
    read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example()) != NULL) { // semiblocking operation
        learn(ec);
      } else if (parser_done()) {
        do_actual_learning();
        finish();
        return;
      }
    }
  }

  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)())
  {
    *(global.lp) = OAA::mc_label_parser;
    global.driver = drive_csoaa_ldf;
    base_learner = base_l;
    base_finish = base_f;
  }

  void global_print_newline()
  {
    char temp[1];
    temp[0] = '\n';
    for (size_t i=0; i<global.final_prediction_sink.index(); i++) {
      int f = global.final_prediction_sink[i];
      ssize_t t = write(f, temp, 1);
      if (t != 1)
        std::cerr << "write error" << std::endl;
    }
  }

}
