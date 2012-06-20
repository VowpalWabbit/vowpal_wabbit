/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <fstream>
#include <iostream>
using namespace std;

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "parse_regressor.h"
#include "loss_functions.h"
#include "global_data.h"
#include "io.h"

/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "6.1.1"
#define VERSION_FILE_WITH_SEARN "6.1.2"

void initialize_regressor(vw& all)
{
  size_t length = ((size_t)1) << all.num_bits;
  all.weight_mask = (all.stride * length) - 1;
  all.reg.weight_vectors = (weight *)calloc(all.stride*length, sizeof(weight));
  if (all.reg.weight_vectors == NULL)
    {
      cerr << all.program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
      exit (1);
    }
  
  if (all.per_feature_regularizer_input != "")
    {
      all.reg.regularizers = (weight *)calloc(2*length, sizeof(weight));
      if (all.reg.regularizers == NULL)
	{
	  cerr << all.program_name << ": Failed to allocate regularizers array: try decreasing -b <bits>" << endl;
	  exit (1);
	}
    }
  else
    all.reg.regularizers = NULL;

  // random weight initialization for matrix factorization
  if (all.random_weights)
    {
      if (all.rank > 0)
	for (size_t j = 0; j < all.stride*length; j++)
	  all.reg.weight_vectors[j] = (double) 0.1 * drand48(); 
      else
	for (size_t j = 0; j < length; j++)
	  all.reg.weight_vectors[j] = drand48() - 0.5;
    }
  if (all.initial_weight != 0.)
    for (size_t j = 0; j < all.stride*length; j+=all.stride)
      all.reg.weight_vectors[j] = all.initial_weight;
  if (all.lda)
    {
      size_t stride = all.stride;
      
      for (size_t j = 0; j < stride*length; j+=stride)
	{
	  for (size_t k = 0; k < all.lda; k++) {
	    if (all.random_weights) {
	      all.reg.weight_vectors[j+k] = -log(drand48()) + 1.0;
	      all.reg.weight_vectors[j+k] *= (float)all.lda_D / (float)all.lda
		/ all.length() * 200;
	    }
	  }
	  all.reg.weight_vectors[j+all.lda] = all.initial_t;
	}
    }
  if(all.adaptive)
    for (size_t j = 1; j < all.stride*length; j+=all.stride)
      all.reg.weight_vectors[j] = 1;
}

//nonreentrant
v_array<char> temp;

void read_vector(vw& all, const char* file, bool& initialized, bool reg_vector)
{
  ifstream source(file);
  if (!source.is_open())
    {
      cout << "can't open " << file << endl << " ... exiting." << endl;
      exit(1);
    }

  
  size_t v_length;
  source.read((char*)&v_length, sizeof(v_length));
  temp.erase();
  if (temp.index() < v_length)
    reserve(temp, v_length);
  source.read(temp.begin,v_length);
  version_struct v_tmp(temp.begin);
  if (v_tmp < LAST_COMPATIBLE_VERSION)
    {
      cout << "source has possibly incompatible version! " << v_tmp.to_string() << endl;
      exit(1);
    }
  
  source.read((char*)&all.sd->min_label, sizeof(all.sd->min_label));
  source.read((char*)&all.sd->max_label, sizeof(all.sd->max_label));
  
  size_t local_num_bits;
  source.read((char *)&local_num_bits, sizeof(local_num_bits));
  if (!initialized){
    if (all.default_bits != true && all.num_bits != local_num_bits)
      {
	cout << "Wrong number of bits for source!" << endl;
	exit (1);
      }
    all.default_bits = false;
    all.num_bits = local_num_bits;
  }
  else 
    if (local_num_bits != all.num_bits)
      {
	cout << "can't combine sources with different feature number!" << endl;
	exit (1);
      }
  
  int len;
  source.read((char *)&len, sizeof(len));
  
  vector<string> local_pairs;
  for (; len > 0; len--)
    {
      char pair[2];
      source.read(pair, sizeof(char)*2);
      string temp(pair, 2);
      local_pairs.push_back(temp);
    }


  size_t local_rank;
  source.read((char*)&local_rank, sizeof(local_rank));
  size_t local_lda;
  source.read((char*)&local_lda, sizeof(local_lda));
  if (!initialized)
    {
      all.rank = local_rank;
      all.lda = local_lda;
      //initialized = true;
    }
  else
    {
      cout << "can't combine regressors" << endl;
      exit(1);
    }

  if (all.rank > 0)
    {
      float temp = ceilf(logf((float)(all.rank*2+1)) / logf (2.f));
      all.stride = 1 << (int) temp;
      all.random_weights = true;
    }
  
  if (all.lda > 0)
    {
      // par->sort_features = true;
      float temp = ceilf(logf((float)(all.lda*2+1)) / logf (2.f));
      all.stride = 1 << (int) temp;
      all.random_weights = false;
    }

  if (!initialized)
    {
      all.pairs = local_pairs;
      initialize_regressor(all);
    }
  else
    if (local_pairs != all.pairs)
      {
	cout << "can't combine sources with different features!" << endl;
	for (size_t i = 0; i < local_pairs.size(); i++)
	  cout << local_pairs[i] << " " << local_pairs[i].size() << " ";
	cout << endl;
	for (size_t i = 0; i < all.pairs.size(); i++)
	  cout << all.pairs[i] << " " << all.pairs[i].size() << " ";
	cout << endl;
	exit (1);
      }
  size_t local_ngram;
  source.read((char*)&local_ngram, sizeof(local_ngram));
  size_t local_skips;
  source.read((char*)&local_skips, sizeof(local_skips));
  if (!initialized)
    {
      all.ngram = local_ngram;
      all.skips = local_skips;
      //initialized = true;
    }
  else
    if (all.ngram != local_ngram || all.skips != local_skips)
      {
	cout << "can't combine sources with different ngram features!" << endl;
	exit(1);
      }

  //files beyond 6.1.2 contains extra SEARN parameters 
  if( v_tmp >= VERSION_FILE_WITH_SEARN )
  {
    bool local_searn;
    size_t local_searn_nb_actions;
    size_t base_learner_length;
    std::string local_searn_base_learner;
    size_t local_searn_trained_nb_policies;
    size_t local_searn_total_nb_policies;
    float local_searn_beta;
    size_t task_length;
    std::string local_searn_task;
    size_t local_searn_sequencetask_history;
    size_t local_searn_sequencetask_features;
    bool local_searn_sequencetask_bigrams;
    bool local_searn_sequencetask_bigram_features;

    source.read((char*)&local_searn, sizeof(local_searn));
    source.read((char*)&local_searn_nb_actions, sizeof(local_searn_nb_actions));
    source.read((char*)&base_learner_length, sizeof(base_learner_length));
    temp.erase();
    if (temp.index() < base_learner_length)
      reserve(temp, base_learner_length);
    source.read(temp.begin,base_learner_length);
    local_searn_base_learner.assign(temp.begin);
    source.read((char*)&local_searn_trained_nb_policies, sizeof(local_searn_trained_nb_policies));
    source.read((char*)&local_searn_total_nb_policies, sizeof(local_searn_total_nb_policies));
    source.read((char*)&local_searn_beta, sizeof(local_searn_beta));
    source.read((char*)&task_length, sizeof(task_length));
    temp.erase();
    if (temp.index() < task_length)
      reserve(temp, task_length);
    source.read(temp.begin,task_length);
    local_searn_task.assign(temp.begin);
    source.read((char*)&local_searn_sequencetask_history, sizeof(local_searn_sequencetask_history));
    source.read((char*)&local_searn_sequencetask_features, sizeof(local_searn_sequencetask_features));
    source.read((char*)&local_searn_sequencetask_bigrams, sizeof(local_searn_sequencetask_bigrams));
    source.read((char*)&local_searn_sequencetask_bigram_features, sizeof(local_searn_sequencetask_bigram_features));

    if (!initialized)
    {
      all.searn = local_searn;
      all.searn_nb_actions = local_searn_nb_actions;
      all.searn_base_learner.assign(local_searn_base_learner);
      all.searn_trained_nb_policies = local_searn_trained_nb_policies;
      all.searn_total_nb_policies = local_searn_total_nb_policies;
      all.searn_beta = local_searn_beta;
      all.searn_task.assign(local_searn_task);
      all.searn_sequencetask_history = local_searn_sequencetask_history;
      all.searn_sequencetask_features = local_searn_sequencetask_features;
      all.searn_sequencetask_bigrams = local_searn_sequencetask_bigrams;
      all.searn_sequencetask_bigram_features = local_searn_sequencetask_bigram_features;
      initialized = true;

      /*std::cerr << "searn: " << all.searn << endl;
      std::cerr << "searn_nb_actions: " << all.searn_nb_actions << endl;
      std::cerr << "searn_base_learner: " << all.searn_base_learner << endl;
      std::cerr << "searn_trained_nb_policies: " << all.searn_trained_nb_policies << endl;
      std::cerr << "searn_total_nb_policies: " << all.searn_total_nb_policies << endl;
      std::cerr << "searn_beta: " << all.searn_beta << endl;
      std::cerr << "searn_task: " << all.searn_task << endl;
      std::cerr << "searn_sequencetask_history: " << all.searn_sequencetask_history << endl;
      std::cerr << "searn_sequencetask_features: " << all.searn_sequencetask_features << endl;
      std::cerr << "searn_sequencetask_bigrams: " << all.searn_sequencetask_bigrams << endl;
      std::cerr << "searn_sequencetask_bigram_features: " << all.searn_sequencetask_bigram_features << endl;*/
    }
    else
    {
      if (all.searn != local_searn){
        cout << "can't combine sources for searn and not for searn!" << endl;
        exit(1);
      }

      if (all.searn_nb_actions != local_searn_nb_actions){
        cout << "can't combine sources with different number of actions!" << endl;
        exit(1);
      }

      if (all.searn_base_learner.compare(local_searn_base_learner) != 0){
        cout << "can't combine sources with different base learner!" << endl;
        exit(1);
      }

      if (all.searn_trained_nb_policies != local_searn_trained_nb_policies)
      {
        cout << "can't combine sources with different number of trained policies!" << endl;
        exit(1);
      }

      if (all.searn_total_nb_policies != local_searn_total_nb_policies)
      {
	cout << "can't combine sources with different total number of policies!" << endl;
	exit(1);
      }

      if (all.searn_beta != local_searn_beta){
        cout << "can't combine sources with different searn beta parameters!" << endl;
        exit(1);
      }

      if (all.searn_task.compare(local_searn_task) != 0){
        cout << "can't combine sources for different searn task!" << endl;
        exit(1);
      }

      if (all.searn_sequencetask_history != local_searn_sequencetask_history){
        cout << "can't combine sources using different history length!" << endl;
        exit(1);
      }

      if (all.searn_sequencetask_features != local_searn_sequencetask_features){
        cout << "can't combine sources using different feature history length!" << endl;
        exit(1);
      }

      if (all.searn_sequencetask_bigrams != local_searn_sequencetask_bigrams){
        cout << "can't combine sources using and not using bigrams!" << endl;
        exit(1);
      }

      if (all.searn_sequencetask_bigram_features != local_searn_sequencetask_bigram_features){
        cout << "can't combine sources using and not using bigram features!" << endl;
        exit(1);
      }
    }
  }

  size_t stride = all.stride;
  while (source.good())
    {
      uint32_t hash;
      source.read((char *)&hash, sizeof(hash));
      weight w = 0.;
      source.read((char *)&w, sizeof(float));
      
      if (source.good())
	{
	  if (all.rank != 0)
	    all.reg.weight_vectors[hash] = w;
	  else 
	    if (all.lda == 0)
	      if (reg_vector) {
		all.reg.regularizers[hash] = w;
		if (hash%2 == 1) // This is the prior mean; previous element was prior variance
		  all.reg.weight_vectors[(hash/2*stride)] = w;
	      }
	      else
		all.reg.weight_vectors[hash*stride] = w;
	    else
	      all.reg.weight_vectors[hash] = w;
	}      
    }
  source.close();
}

void parse_regressor_args(vw& all, po::variables_map& vm, string& final_regressor_name, bool quiet)
{
  if (vm.count("final_regressor")) {
    final_regressor_name = vm["final_regressor"].as<string>();
    if (!quiet)
      cerr << "final_regressor = " << vm["final_regressor"].as<string>() << endl;
  }
  else
    final_regressor_name = "";

  vector<string> regs;
  if (vm.count("initial_regressor") || vm.count("i"))
    regs = vm["initial_regressor"].as< vector<string> >();
  
  /* 
     Read in regressors.  If multiple regressors are specified, do a weighted 
     average.  If none are specified, initialize according to global_seg & 
     numbits.
  */
  bool initialized = false;

  for (size_t i = 0; i < regs.size(); i++)
    read_vector(all, regs[i].c_str(), initialized, false);
  
  if (all.per_feature_regularizer_input != "")
    read_vector(all, all.per_feature_regularizer_input.c_str(), initialized, true);
      
  if (!initialized)
    {
      if(vm.count("noop") || vm.count("sendto"))
	{
	  all.reg.weight_vectors = NULL;
	  all.reg.regularizers = NULL;
	}
      else
	initialize_regressor(all);
    }
}

void free_regressor(regressor &r)
{
  if (r.weight_vectors != NULL)
    free(r.weight_vectors);
  if (r.regularizers != NULL)
    free(r.regularizers);
}

void dump_regressor(vw& all, string reg_name, bool as_text, bool reg_vector)
{
  if (reg_name == string(""))
    return;
  string start_name = reg_name+string(".writing");
  io_buf io_temp;

  int f = io_temp.open_file(start_name.c_str(),io_buf::WRITE);
  
  if (f<0)
    {
      cout << "can't open: " << start_name << " for writing, exiting" << endl;
      exit(1);
    }
  size_t v_length = version.to_string().length()+1;
  if (!as_text) {
    io_temp.write_file(f,(char*)&v_length, sizeof(v_length));
    io_temp.write_file(f,version.to_string().c_str(),v_length);
  
    io_temp.write_file(f,(char*)&all.sd->min_label, sizeof(all.sd->min_label));
    io_temp.write_file(f,(char*)&all.sd->max_label, sizeof(all.sd->max_label));
  
    io_temp.write_file(f,(char *)&all.num_bits, sizeof(all.num_bits));
    int len = all.pairs.size();
    io_temp.write_file(f,(char *)&len, sizeof(len));
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) 
      io_temp.write_file(f,i->c_str(),2);

    io_temp.write_file(f,(char*)&all.rank, sizeof(all.rank));
    io_temp.write_file(f,(char*)&all.lda, sizeof(all.lda));

    io_temp.write_file(f,(char*)&all.ngram, sizeof(all.ngram));
    io_temp.write_file(f,(char*)&all.skips, sizeof(all.skips));

    io_temp.write_file(f,(char*)&all.searn, sizeof(all.searn));
    io_temp.write_file(f,(char*)&all.searn_nb_actions, sizeof(all.searn_nb_actions));

    size_t base_learner_length = all.searn_base_learner.length()+1;
    io_temp.write_file(f,(char*)&base_learner_length, sizeof(base_learner_length));
    io_temp.write_file(f,all.searn_base_learner.c_str(), base_learner_length);

    io_temp.write_file(f,(char*)&all.searn_trained_nb_policies, sizeof(all.searn_trained_nb_policies));
    io_temp.write_file(f,(char*)&all.searn_total_nb_policies, sizeof(all.searn_total_nb_policies));
    io_temp.write_file(f,(char*)&all.searn_beta, sizeof(all.searn_beta));

    size_t task_length = all.searn_task.length()+1;
    io_temp.write_file(f,(char*)&task_length, sizeof(task_length));
    io_temp.write_file(f,all.searn_task.c_str(), task_length);
    io_temp.write_file(f,(char*)&all.searn_sequencetask_history, sizeof(all.searn_sequencetask_history));
    io_temp.write_file(f,(char*)&all.searn_sequencetask_features, sizeof(all.searn_sequencetask_features));
    io_temp.write_file(f,(char*)&all.searn_sequencetask_bigrams, sizeof(all.searn_sequencetask_bigrams));
    io_temp.write_file(f,(char*)&all.searn_sequencetask_bigram_features, sizeof(all.searn_sequencetask_bigram_features));
  }
  else {
    char buff[512];
    int len;
    len = sprintf(buff, "Version %s\n", version.to_string().c_str());
    io_temp.write_file(f, buff, len);
    len = sprintf(buff, "Min label:%f max label:%f\n", all.sd->min_label, all.sd->max_label);
    io_temp.write_file(f, buff, len);
    len = sprintf(buff, "bits:%d\n", (int)all.num_bits);
    io_temp.write_file(f, buff, len);
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) {
      len = sprintf(buff, "%s ", i->c_str());
      io_temp.write_file(f, buff, len);
    }
    if (all.pairs.size() > 0)
      {
	len = sprintf(buff, "\n");
	io_temp.write_file(f, buff, len);
      }
    len = sprintf(buff, "ngram:%d skips:%d\nindex:weight pairs:\n", (int)all.ngram, (int)all.skips);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "rank:%d\n", (int)all.rank);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "lda:%d\n", (int)all.lda);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn:%s\n", all.searn ? "true" : "false");
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_nb_actions:%d\n", (int)all.searn_nb_actions);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_base_learner:%s\n", all.searn_base_learner.c_str());
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_trained_nb_policies:%d\n", (int)all.searn_trained_nb_policies);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_total_nb_policies:%d\n", (int)all.searn_total_nb_policies);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_beta:%f\n", all.searn_beta);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_task:%s\n", all.searn_task.c_str());
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_sequencetask_history:%d\n", (int)all.searn_sequencetask_history);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_sequencetask_features:%d\n", (int)all.searn_sequencetask_features);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_sequencetask_bigrams:%s\n", all.searn_sequencetask_bigrams ? "true" : "false");
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "searn_sequencetask_bigram_features:%s\n", all.searn_sequencetask_bigram_features ? "true" : "false");
    io_temp.write_file(f, buff, len);
  }
  
  uint32_t length = 1 << all.num_bits;
  size_t stride = all.stride;
  if (reg_vector)
    length *= 2;
  for(uint32_t i = 0; i < length; i++)
    {
      if ((all.lda == 0) && (all.rank == 0))
	{
	  weight v;
	  if (reg_vector)
	    v = all.reg.regularizers[i];
	  else
	    v = all.reg.weight_vectors[stride*i];
	  if (v != 0.)
	    {
              if (!as_text) {
                io_temp.write_file(f,(char *)&i, sizeof (i));
                io_temp.write_file(f,(char *)&v, sizeof (v));
              } else {
                char buff[512];
                int len = sprintf(buff, "%d:%f\n", i, v);
                io_temp.write_file(f, buff, len);
              }
	    }
	}
      else
	{
	  size_t K = all.lda;

	  if (all.rank != 0)
	    K = all.rank*2+1;
	  
          for (size_t k = 0; k < K; k++)
            {
              weight v = all.reg.weight_vectors[stride*i+k];
              uint32_t ndx = stride*i+k;
              if (!as_text) {
                io_temp.write_file(f,(char *)&ndx, sizeof (ndx));
                io_temp.write_file(f,(char *)&v, sizeof (v));
              } else {
                char buff[512];
		int len;

		if (all.rank != 0)
		  len = sprintf(buff, "%f ", v);
		else
		  len = sprintf(buff, "%f ", v + all.lda_rho);

                io_temp.write_file(f, buff, len);
              }
            }
          if (as_text)
            io_temp.write_file(f, "\n", 1);
	}
    }

  io_temp.close_file();
  rename(start_name.c_str(),reg_name.c_str());
}

void save_predictor(vw& all, string reg_name, size_t current_pass)
{
  char* filename = new char[reg_name.length()+4];
  if (all.save_per_pass)
    sprintf(filename,"%s.%lu",reg_name.c_str(),(long unsigned)current_pass);
  else
    sprintf(filename,"%s",reg_name.c_str());
  dump_regressor(all, string(filename), false, false);
  delete[] filename;
}

void finalize_regressor(vw& all, string reg_name)
{
  dump_regressor(all, reg_name, false, false);
  dump_regressor(all, all.text_regressor_name, true, false);
  dump_regressor(all, all.per_feature_regularizer_output, false, true);
  dump_regressor(all, all.per_feature_regularizer_text, true, true);
  free_regressor(all.reg);
}
