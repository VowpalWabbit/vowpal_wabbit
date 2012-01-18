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

void initialize_regressor(regressor &r)
{
  size_t length = ((size_t)1) << global.num_bits;
  global.weight_mask = (global.stride * length) - 1;
  r.weight_vectors = (weight *)calloc(global.stride*length, sizeof(weight));
  if (r.weight_vectors == NULL)
    {
      cerr << global.program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
      exit (1);
    }
  
  if (global.per_feature_regularizer_input != "")
    {
      r.regularizers = (weight *)calloc(2*length, sizeof(weight));
      if (r.regularizers == NULL)
	{
	  cerr << global.program_name << ": Failed to allocate regularizers array: try decreasing -b <bits>" << endl;
	  exit (1);
	}
    }
  else
    r.regularizers = NULL;

  // random weight initialization for matrix factorization
  if (global.random_weights)
    {
      if (global.rank > 0)
	for (size_t j = 0; j < global.stride*length; j++)
	  r.weight_vectors[j] = (double) 0.1 * drand48(); 
      else
	for (size_t j = 0; j < length; j++)
	  r.weight_vectors[j] = drand48() - 0.5;
    }
  if (global.initial_weight != 0.)
    for (size_t j = 0; j < global.stride*length; j+=global.stride)
      r.weight_vectors[j] = global.initial_weight;
  if (global.lda)
    {
      size_t stride = global.stride;
      
      for (size_t j = 0; j < stride*length; j+=stride)
	{
	  for (size_t k = 0; k < global.lda; k++) {
	    if (global.random_weights) {
	      r.weight_vectors[j+k] = -log(drand48()) + 1.0;
	      r.weight_vectors[j+k] *= (float)global.lda_D / (float)global.lda
		/ global.length() * 200;
	    }
	  }
	  r.weight_vectors[j+global.lda] = global.initial_t;
	}
    }
  if(global.adaptive)
    for (size_t j = 1; j < global.stride*length; j+=global.stride)
      r.weight_vectors[j] = 1;
}

v_array<char> temp;

void read_vector(const char* file, regressor& r, bool& initialized, bool reg_vector)
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
  if (strcmp(temp.begin,version.c_str()) != 0)
    {
      cout << "source has possibly incompatible version!" << endl;
      exit(1);
    }
  
  source.read((char*)&global.sd->min_label, sizeof(global.sd->min_label));
  source.read((char*)&global.sd->max_label, sizeof(global.sd->max_label));
  
  size_t local_num_bits;
  source.read((char *)&local_num_bits, sizeof(local_num_bits));
  if (!initialized){
    if (global.default_bits != true && global.num_bits != local_num_bits)
      {
	cout << "Wrong number of bits for source!" << endl;
	exit (1);
      }
    global.default_bits = false;
    global.num_bits = local_num_bits;
  }
  else 
    if (local_num_bits != global.num_bits)
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
      global.rank = local_rank;
      global.lda = local_lda;
      //initialized = true;
    }
  else
    {
      cout << "can't combine regressors" << endl;
      exit(1);
    }

  if (global.rank > 0)
    {
      float temp = ceilf(logf((float)(global.rank*2+1)) / logf (2.f));
      global.stride = 1 << (int) temp;
      global.random_weights = true;
    }
  
  if (global.lda > 0)
    {
      // par->sort_features = true;
      float temp = ceilf(logf((float)(global.lda*2+1)) / logf (2.f));
      global.stride = 1 << (int) temp;
      global.random_weights = false;
    }

  if (!initialized)
    {
      global.pairs = local_pairs;
      initialize_regressor(r);
    }
  else
    if (local_pairs != global.pairs)
      {
	cout << "can't combine sources with different features!" << endl;
	for (size_t i = 0; i < local_pairs.size(); i++)
	  cout << local_pairs[i] << " " << local_pairs[i].size() << " ";
	cout << endl;
	for (size_t i = 0; i < global.pairs.size(); i++)
	  cout << global.pairs[i] << " " << global.pairs[i].size() << " ";
	cout << endl;
	exit (1);
      }
  size_t local_ngram;
  source.read((char*)&local_ngram, sizeof(local_ngram));
  size_t local_skips;
  source.read((char*)&local_skips, sizeof(local_skips));
  if (!initialized)
    {
      global.ngram = local_ngram;
      global.skips = local_skips;
      initialized = true;
    }
  else
    if (global.ngram != local_ngram || global.skips != local_skips)
      {
	cout << "can't combine sources with different ngram features!" << endl;
	exit(1);
      }

  size_t stride = global.stride;
  while (source.good())
    {
      uint32_t hash;
      source.read((char *)&hash, sizeof(hash));
      weight w = 0.;
      source.read((char *)&w, sizeof(float));
      
      if (source.good())
	{
	  if (global.rank != 0)
	    r.weight_vectors[hash] = w;
	  else 
	    if (global.lda == 0)
	      if (reg_vector) {
		r.regularizers[hash] = w;
		if (hash%2 == 1) // This is the prior mean; previous element was prior variance
		  r.weight_vectors[(hash/2*stride)] = w;
	      }
	      else
		r.weight_vectors[hash*stride] = w;
	    else
	      r.weight_vectors[hash] = w;
	}      
    }
  source.close();
}

void parse_regressor_args(po::variables_map& vm, regressor& r, string& final_regressor_name, bool quiet)
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
    read_vector(regs[i].c_str(), r, initialized, false);
  
  if (global.per_feature_regularizer_input != "")
    read_vector(global.per_feature_regularizer_input.c_str(), r, initialized, true);
      
  if (!initialized)
    {
      if(vm.count("noop") || vm.count("sendto"))
	{
	  r.weight_vectors = NULL;
	  r.regularizers = NULL;
	}
      else
	initialize_regressor(r);
    }
}

void free_regressor(regressor &r)
{
  if (r.weight_vectors != NULL)
    free(r.weight_vectors);
  if (r.regularizers != NULL)
    free(r.regularizers);
}

void save_predictor(string reg_name, size_t current_pass)
{
   if(global.save_per_pass) {
    char* filename = new char[reg_name.length()+4];
    sprintf(filename,"%s.%lu",reg_name.c_str(),(long unsigned)current_pass);
    dump_regressor(string(filename), global.reg);
    delete filename;
  }
}  

void dump_regressor(string reg_name, regressor &r, bool as_text, bool reg_vector)
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
  size_t v_length = version.length()+1;
  if (!as_text) {
    io_temp.write_file(f,(char*)&v_length, sizeof(v_length));
    io_temp.write_file(f,version.c_str(),v_length);
  
    io_temp.write_file(f,(char*)&global.sd->min_label, sizeof(global.sd->min_label));
    io_temp.write_file(f,(char*)&global.sd->max_label, sizeof(global.sd->max_label));
  
    io_temp.write_file(f,(char *)&global.num_bits, sizeof(global.num_bits));
    int len = global.pairs.size();
    io_temp.write_file(f,(char *)&len, sizeof(len));
    for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) 
      io_temp.write_file(f,i->c_str(),2);

    io_temp.write_file(f,(char*)&global.rank, sizeof(global.rank));
    io_temp.write_file(f,(char*)&global.lda, sizeof(global.lda));

    io_temp.write_file(f,(char*)&global.ngram, sizeof(global.ngram));
    io_temp.write_file(f,(char*)&global.skips, sizeof(global.skips));
  }
  else {
    char buff[512];
    int len;
    len = sprintf(buff, "Version %s\n", version.c_str());
    io_temp.write_file(f, buff, len);
    len = sprintf(buff, "Min label:%f max label:%f\n", global.sd->min_label, global.sd->max_label);
    io_temp.write_file(f, buff, len);
    len = sprintf(buff, "bits:%d\n", (int)global.num_bits);
    io_temp.write_file(f, buff, len);
    for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) {
      len = sprintf(buff, "%s ", i->c_str());
      io_temp.write_file(f, buff, len);
    }
    if (global.pairs.size() > 0)
      {
	len = sprintf(buff, "\n");
	io_temp.write_file(f, buff, len);
      }
    len = sprintf(buff, "ngram:%d skips:%d\nindex:weight pairs:\n", (int)global.ngram, (int)global.skips);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "rank:%d\n", (int)global.rank);
    io_temp.write_file(f, buff, len);

    len = sprintf(buff, "lda:%d\n", (int)global.lda);
    io_temp.write_file(f, buff, len);
  }
  
  uint32_t length = 1 << global.num_bits;
  size_t stride = global.stride;
  if (reg_vector)
    length *= 2;
  for(uint32_t i = 0; i < length; i++)
    {
      if ((global.lda == 0) && (global.rank == 0))
	{
	  weight v;
	  if (reg_vector)
	    v = r.regularizers[i];
	  else
	    v = r.weight_vectors[stride*i];
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
	  size_t K = global.lda;

	  if (global.rank != 0)
	    K = global.rank*2+1;
	  
          for (size_t k = 0; k < K; k++)
            {
              weight v = r.weight_vectors[stride*i+k];
              uint32_t ndx = stride*i+k;
              if (!as_text) {
                io_temp.write_file(f,(char *)&ndx, sizeof (ndx));
                io_temp.write_file(f,(char *)&v, sizeof (v));
              } else {
                char buff[512];
		int len;

		if (global.rank != 0)
		  len = sprintf(buff, "%f ", v);
		else
		  len = sprintf(buff, "%f ", v + global.lda_rho);

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

void finalize_regressor(string reg_name, regressor &r)
{
  dump_regressor(reg_name, r, false);
  dump_regressor(global.text_regressor_name, r, true);
  dump_regressor(global.per_feature_regularizer_output, r, false, true);
  dump_regressor(global.per_feature_regularizer_text, r, true, true);
  free_regressor(r);
}
