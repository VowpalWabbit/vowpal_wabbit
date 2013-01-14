/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <iostream>
using namespace std;

#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "parse_regressor.h"
#include "loss_functions.h"
#include "global_data.h"
#include "io.h"
#include "rand48.h"

/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "6.1.3"
#define VERSION_FILE_WITH_CUBIC "6.1.3"

void initialize_regressor(vw& all)
{
  size_t length = ((size_t)1) << all.num_bits;
  all.weight_mask = (all.stride * length) - 1;
  all.reg.weight_vectors = (weight *)calloc(all.stride*length, sizeof(weight));
  if (all.reg.weight_vectors == NULL)
    {
      cerr << all.program_name << ": Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>" << endl;
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
	  all.reg.weight_vectors[j] = (float) (0.1 * frand48()); 
      else
	for (size_t j = 0; j < length; j++)
	  all.reg.weight_vectors[j*all.stride] = (float)(frand48() - 0.5);
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
	      all.reg.weight_vectors[j+k] = (float)(-log(frand48()) + 1.0f);
		  all.reg.weight_vectors[j+k] *= (float)(all.lda_D / all.lda / all.length() * 200);
	    }
	  }
	  all.reg.weight_vectors[j+all.lda] = all.initial_t;
	}
    }
  if(all.adaptive && all.initial_t > 0)
  {
    for (size_t j = 1; j < all.stride*length; j+=all.stride)
    {
      all.reg.weight_vectors[j] = all.initial_t;   //for adaptive update, we interpret initial_t as previously seeing initial_t fake datapoints, all with squared gradient=1
      //NOTE: this is not invariant to the scaling of the data (i.e. when combined with normalized). Since scaling the data scales the gradient, this should ideally be 
      //feature_range*initial_t, or something like that. We could potentially fix this by just adding this base quantity times the current range to the sum of gradients 
      //stored in memory at each update, and always start sum of gradients to 0, at the price of additional additions and multiplications during the update...
    }
  }
}

void free_regressor(regressor &r)
{
  if (r.weight_vectors != NULL)
    free(r.weight_vectors);
  if (r.regularizers != NULL)
    free(r.regularizers);
}

//if read_message is null, just read it in.  Otherwise do a comparison and barf on read_message.
size_t bin_read_fixed(io_buf& i, char* data, size_t len, const char* read_message)
{
  if (len > 0)
    {
      char* p;
      size_t ret = buf_read(i,p,len);
      if (*read_message == '\0')
	memcpy(data,p,len);
      else
	if (memcmp(data,p,len) != 0)
	  {
	    cout << read_message << endl;
	    exit(1);
	  }
      return ret;
    }
  return 0;
}

size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = bin_read_fixed(i,(char*)&obj_len,sizeof(obj_len),"");
  if (obj_len > len || ret < sizeof(uint32_t))
    {
      cerr << "bad model format!" <<endl;
      exit(1);
    }
  ret += bin_read_fixed(i,data,obj_len,read_message);

  return ret;
}

size_t bin_write_fixed(io_buf& o, const char* data, uint32_t len)
{
  if (len > 0)
    {
      char* p;
      buf_write (o, p, len);
      memcpy (p, data, len);
    }
  return len;
}

size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  bin_write_fixed(o,(char*)&len, sizeof(len));
  bin_write_fixed(o,data,len);
  return (len + sizeof(len));
}

size_t bin_text_write(io_buf& io, char* data, size_t len, 
		      const char* text_data, size_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else 
    if (len > 0)
      return bin_write (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
size_t bin_text_read_write(io_buf& io, char* data, size_t len, 
			 const char* read_message, bool read, 
			 const char* text_data, size_t text_len, bool text)
{
  if (read)
    return bin_read(io, data, len, read_message);
  else
    return bin_text_write(io,data,len, text_data, text_len, text);
}

size_t bin_text_write_fixed(io_buf& io, char* data, size_t len, 
		      const char* text_data, size_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else 
    return bin_write_fixed (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
size_t bin_text_read_write_fixed(io_buf& io, char* data, size_t len, 
			       const char* read_message, bool read, 
			       const char* text_data, size_t text_len, bool text)
{
  if (read)
    return bin_read_fixed(io, data, len, read_message);
  else
    return bin_text_write_fixed(io, data, len, text_data, text_len, text);
}

const size_t buf_size = 512;

void save_load(void* in, io_buf& model_file, bool reg_vector, bool read, bool text)
{
  vw* all = (vw*)in;

  char buff[buf_size];
  char buff2[buf_size];
  size_t text_len;

  if (model_file.files.size() > 0)
    {
      uint32_t v_length = version.to_string().length()+1;
      text_len = sprintf(buff, "Version %s\n", version.to_string().c_str());
      memcpy(buff2,version.to_string().c_str(),v_length);
      if (read)
	v_length = buf_size;
      bin_text_read_write(model_file, buff2, v_length, 
			  "", read, 
			  buff, text_len, text);
      version_struct v_tmp(buff2);
      if (v_tmp < LAST_COMPATIBLE_VERSION)
	{
	  cout << "Model has possibly incompatible version! " << v_tmp.to_string() << endl;
	  exit(1);
	}
      
      char model = 'm';
      bin_text_read_write_fixed(model_file,&model,1,
				"file is not a model file", read, 
				"", 0, text);
      
      text_len = sprintf(buff, "Min label:%f\n", all->sd->min_label);
      bin_text_read_write_fixed(model_file,(char*)&all->sd->min_label, sizeof(all->sd->min_label), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "Max label:%f\n", all->sd->max_label);
      bin_text_read_write_fixed(model_file,(char*)&all->sd->max_label, sizeof(all->sd->max_label), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "bits:%d\n", (int)all->num_bits);
      uint32_t local_num_bits = all->num_bits;
      bin_text_read_write_fixed(model_file,(char *)&local_num_bits, sizeof(local_num_bits), 
				"", read, 
				buff, text_len, text);
      if (all->default_bits != true && all->num_bits != local_num_bits)
	{
	  cout << "Wrong number of bits for source!" << endl;
	  exit (1);
	}
      all->default_bits = false;
      all->num_bits = local_num_bits;
      
      uint32_t pair_len = all->pairs.size();
      text_len = sprintf(buff, "%d pairs: ", (int)pair_len);
      bin_text_read_write_fixed(model_file,(char *)&pair_len, sizeof(pair_len), 
				"", read, 
				buff, text_len, text);
      for (size_t i = 0; i < pair_len; i++)
	{
	  char pair[2];
	  if (!read)
	    {
	      memcpy(pair,all->pairs[i].c_str(),2);
	      text_len = sprintf(buff, "%s ", all->pairs[i].c_str());
	    }
	  bin_text_read_write_fixed(model_file, pair,2, 
				    "", read,
				    buff, text_len, text);
	  if (read)
	    {
	      string temp(pair, 2);
	      all->pairs.push_back(temp);
	    }
	}
      bin_text_read_write_fixed(model_file,buff,0,
				"", read,
				"\n",1,text);
      
      uint32_t triple_len = all->triples.size();
      text_len = sprintf(buff, "%d triples: ", (int)triple_len);
      bin_text_read_write_fixed(model_file,(char *)&triple_len, sizeof(triple_len), 
				"", read, 
			    buff,text_len, text);
      for (size_t i = 0; i < triple_len; i++)
	{
	  text_len = sprintf(buff, "%s ", all->triples[i].c_str());
	  char triple[3];
	  if (!read)
	    memcpy(triple, all->triples[i].c_str(), 3);
	  bin_text_read_write_fixed(model_file,triple,3, 
				    "", read,
				    buff,text_len,text);
	  if (read)
	    {
	      string temp(triple,3);
	      all->triples.push_back(temp);
	    }
	}
      bin_text_read_write_fixed(model_file,buff,0,
				"", read, 
				"\n",1, text);
      
      text_len = sprintf(buff, "rank:%d\n", (int)all->rank);
      bin_text_read_write_fixed(model_file,(char*)&all->rank, sizeof(all->rank), 
				"", read, 
				buff,text_len, text);
      
      text_len = sprintf(buff, "lda:%d\n", (int)all->lda);
      bin_text_read_write_fixed(model_file,(char*)&all->lda, sizeof(all->lda), 
				"", read, 
				buff, text_len,text);
      
      text_len = sprintf(buff, "ngram:%d\n", (int)all->ngram); 
      bin_text_read_write_fixed(model_file,(char*)&all->ngram, sizeof(all->ngram), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "skips:%d\n", (int)all->skips);
      bin_text_read_write_fixed(model_file,(char*)&all->skips, sizeof(all->skips), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "options:%s\n", all->options_from_file.c_str());
      uint32_t len = all->options_from_file.length()+1;
      memcpy(buff2, all->options_from_file.c_str(),len);
      if (read)
	len = buf_size;
      bin_text_read_write(model_file,buff2, len, 
			  "", read,
			  buff, text_len, text);
      if (read)
	all->options_from_file.assign(buff2);
    }

  //optimizer specific stuff
  if (read)
    initialize_regressor(*all);

  int c = 0;
  if (model_file.files.size() > 0)
    {
      uint32_t length = 1 << all->num_bits;
      uint32_t stride = all->stride;
      if (reg_vector)
	length *= 2;
      uint32_t i = 0;
      size_t brw = 1;
      do 
	{
	  brw = 1;
	  weight* v;
	  if ((all->lda == 0) && (all->rank == 0))
	    if (read)
	      {
		c++;
		brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
		if (brw > 0)
		  {
		    assert (i< length);		
		    if (reg_vector)
		      v = &(all->reg.regularizers[i]);
		    else
		      v = &(all->reg.weight_vectors[stride*i]);
		    if (brw > 0)
		      brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
		  }
	      }
	    else // write binary or text
	      {
		if (reg_vector)
		  v = &(all->reg.regularizers[i]);
		else
		  v = &(all->reg.weight_vectors[stride*i]);
		if (*v != 0.)
		  {
		    c++;
		    int text_len = sprintf(buff, "%d", i);
		    brw = bin_text_read_write_fixed(model_file,(char *)&i, sizeof (i),
						  "", read,
						    buff, text_len, text);
		    
		    
		    text_len = sprintf(buff, ":%f\n", *v);
		    brw+= bin_text_read_write_fixed(model_file,(char *)v, sizeof (*v),
						    "", read,
						    buff, text_len, text);
		    if (read && reg_vector && i%2 == 1) // This is the prior mean
		      all->reg.weight_vectors[(i/2*stride)] = *v;
		  }
	      }
	  else
	    {
	      size_t K = all->lda;
	      
	      if (all->rank != 0)
		K = all->rank*2+1;
	      
	      for (uint32_t k = 0; k < K; k++)
		{
		  uint32_t ndx = stride*i+k;
		  
		  bin_text_read_write_fixed(model_file,(char *)&ndx, sizeof (ndx),
					    "", read,
					    "", 0, text);
		  
		  weight* v = &(all->reg.weight_vectors[ndx]);
		  if (all->rank != 0)
		    text_len = sprintf(buff, "%f ", *v);
		  else
		    text_len = sprintf(buff, "%f ", *v + all->lda_rho);
		  
		  bin_text_read_write_fixed(model_file,(char *)v, sizeof (*v),
					    "", read,
					    buff, text_len, text);
		  
		}
	      if (text)
		bin_text_read_write_fixed(model_file,buff,0,
					  "", read,
					  "\n",1,text);
	    }
	  if (!read)
	    i++;
	}  
      while ((!read && i < length) || (read && brw >0));
    }
}

void dump_regressor(vw& all, string reg_name, bool as_text, bool reg_vector)
{
  if (reg_name == string(""))
    return;
  string start_name = reg_name+string(".writing");
  io_buf io_temp;

  io_temp.open_file(start_name.c_str(), all.stdin_off, io_buf::WRITE);
  
  save_load(&all, io_temp, reg_vector, false, as_text);

  io_temp.flush(); // close_file() should do this for me ...
  io_temp.close_file();
  remove(reg_name.c_str());
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
  

  io_buf io_temp;

  if (regs.size() > 0)
    io_temp.open_file(regs[0].c_str(), all.stdin_off, io_buf::READ);

  save_load(&all, io_temp, false, true, false);
  io_temp.close_file();

  if (all.per_feature_regularizer_input != "")
    {
      io_temp.open_file(all.per_feature_regularizer_input.c_str(), all.stdin_off, io_buf::READ);
      save_load(&all, io_temp, true, true, false);
      io_temp.close_file();
    }
 
  if(vm.count("noop") || vm.count("sendto"))
    {
      all.reg.weight_vectors = NULL;
      all.reg.regularizers = NULL;
    }
}

