/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include "parse_regressor.h"
#include "loss_functions.h"
using namespace std;

void initialize_regressor(regressor &r)
{
  size_t length = ((size_t)1) << r.global->num_bits;
  r.global->thread_mask = (length >> r.global->thread_bits) - 1;
  size_t num_threads = r.global->num_threads();
  r.weight_vectors = (weight **)malloc(num_threads * sizeof(weight*));
  for (size_t i = 0; i < num_threads; i++)
    {
      r.weight_vectors[i] = (weight *)calloc(length/num_threads, sizeof(weight));
      if (r.weight_vectors[i] == NULL)
        {
          cerr << r.global->program_name << ": Failed to allocate weight array: try decreasing -b <bits>" << endl;
          exit (1);
        }
    }
}

/* 
   Read in regressors.  If multiple regressors are specified, do a weighted 
   average.  If none are specified, initialize according to global_seg & 
   numbits.
*/
void parse_regressor(vector<string> regressors, regressor &r)
{

  bool initialized = false;
  
  for (size_t i = 0; i < regressors.size(); i++)
    {
      ifstream regressor(regressors[i].c_str());
      size_t local_num_bits;
      regressor.read((char *)&local_num_bits, sizeof(local_num_bits));
      if (!initialized){
	r.global->num_bits = local_num_bits;
      }
      else 
	if (local_num_bits != r.global->num_bits)
	  {
	    cout << "can't combine regressors with different feature number!" << endl;
	    exit (1);
	  }

      size_t local_thread_bits;
      regressor.read((char*)&local_thread_bits, sizeof(local_thread_bits));
      if (!initialized){
	r.global->thread_bits = local_thread_bits;
      }
      else 
	if (local_thread_bits != r.global->thread_bits)
	  {
	    cout << "can't combine regressors trained with different numbers of threads!" << endl;
	    exit (1);
	  }

      int len;
      regressor.read((char *)&len, sizeof(len));

      vector<string> local_pairs;
      for (; len > 0; len--)
	{
	  char pair[2];
	  regressor.read(pair, sizeof(char)*2);
	  string temp(pair, 2);
	  local_pairs.push_back(temp);
	}
      if (!initialized)
	{
	  r.global->pairs = local_pairs;
	  initialize_regressor(r);
	  initialized = true;
	}
      else
	if (local_pairs != r.global->pairs)
	  {
	    cout << "can't combine regressors with different features!" << endl;
	    for (size_t i = 0; i < local_pairs.size(); i++)
	      cout << local_pairs[i] << " " << local_pairs[i].size() << " ";
	    cout << endl;
	    for (size_t i = 0; i < r.global->pairs.size(); i++)
	      cout << r.global->pairs[i] << " " << r.global->pairs[i].size() << " ";
	    cout << endl;
	    exit (1);
	  }
      while (regressor.good())
	{
	  uint32_t hash;
	  regressor.read((char *)&hash, sizeof(hash));
	  weight w = 0.;
	  regressor.read((char *)&w, sizeof(float));
	  
	  size_t num_threads = r.global->num_threads();
	  if (regressor.good()) 
	    r.weight_vectors[hash % num_threads][hash/num_threads] 
	      = r.weight_vectors[hash % num_threads][hash/num_threads] + w;
	}      
      regressor.close();
    }
  if (!initialized)
    initialize_regressor(r);

//  r.loss = getLossFunction(loss_function);
}

void free_regressor(regressor &r)
{
  for (size_t i = 0; i < r.global->num_threads(); i++)
    free(r.weight_vectors[i]);
  free(r.weight_vectors);
}

void dump_regressor(ofstream &o, regressor &r)
{
  if (o.is_open()) 
    {
      o.write((char *)&r.global->num_bits, sizeof(r.global->num_bits));
      o.write((char *)&r.global->thread_bits, sizeof(r.global->thread_bits));
      int len = r.global->pairs.size();
      o.write((char *)&len, sizeof(len));
      for (vector<string>::iterator i = r.global->pairs.begin(); i != r.global->pairs.end();i++) 
	o << (*i)[0] << (*i)[1];
      
      uint32_t length = 1 << r.global->num_bits;
      size_t num_threads = r.global->num_threads();
      for(uint32_t i = 0; i < length; i++)
	{
	  weight v = r.weight_vectors[i%num_threads][i/num_threads];
	  if (v != 0.)
	    {      
	      o.write((char *)&i, sizeof (i));
	      o.write((char *)&v, sizeof (v));
	    }
	}
    }

  o.close();
}

void finalize_regressor(ofstream &o, regressor &r)
{
  dump_regressor(o,r);
  free_regressor(r);
}
