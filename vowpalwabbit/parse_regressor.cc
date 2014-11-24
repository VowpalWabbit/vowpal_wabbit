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
#include <algorithm>

#include "parse_regressor.h"
#include "loss_functions.h"
#include "io_buf.h"
#include "memory.h"
#include "rand48.h"
#include "global_data.h"

/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "6.1.3"
#define VERSION_FILE_WITH_CUBIC "6.1.3"

void initialize_regressor(vw& all)
{
  // Regressor is already initialized.
  if (all.reg.weight_vector != NULL) {
    return;
  }

  size_t length = ((size_t)1) << all.num_bits;
  all.reg.weight_mask = (length << all.reg.stride_shift) - 1;
  all.reg.weight_vector = (weight *)calloc_or_die(length << all.reg.stride_shift, sizeof(weight));
  if (all.reg.weight_vector == NULL)
    {
      cerr << all.program_name << ": Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>" << endl;
      throw exception();
    }
  if (all.random_weights)
    {
      for (size_t j = 0; j < length; j++)
	all.reg.weight_vector[j << all.reg.stride_shift] = (float)(frand48() - 0.5);
    }
  if (all.random_positive_weights)
    {
      for (size_t j = 0; j < length; j++)
	all.reg.weight_vector[j << all.reg.stride_shift] = (float)(0.1 * frand48());
    }
  if (all.initial_weight != 0.)
    for (size_t j = 0; j < length << all.reg.stride_shift; j+= ( ((size_t)1) << all.reg.stride_shift))
      all.reg.weight_vector[j] = all.initial_weight;
}

const size_t buf_size = 512;

void save_load_header(vw& all, io_buf& model_file, bool read, bool text)
{

  char buff[buf_size];
  char buff2[buf_size];
  uint32_t text_len;

  if (model_file.files.size() > 0)
    {
      uint32_t v_length = (uint32_t)version.to_string().length()+1;
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
	  throw exception();
	}
      
      char model = 'm';
      bin_text_read_write_fixed(model_file,&model,1,
				"file is not a model file", read, 
				"", 0, text);
      
      text_len = sprintf(buff, "Min label:%f\n", all.sd->min_label);
      bin_text_read_write_fixed(model_file,(char*)&all.sd->min_label, sizeof(all.sd->min_label), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "Max label:%f\n", all.sd->max_label);
      bin_text_read_write_fixed(model_file,(char*)&all.sd->max_label, sizeof(all.sd->max_label), 
				"", read, 
				buff, text_len, text);
      
      text_len = sprintf(buff, "bits:%d\n", (int)all.num_bits);
      uint32_t local_num_bits = all.num_bits;
      bin_text_read_write_fixed(model_file,(char *)&local_num_bits, sizeof(local_num_bits), 
				"", read, 
				buff, text_len, text);
      if (all.default_bits != true && all.num_bits != local_num_bits)
	{
	  cout << "vw: -b bits mismatch: command-line " << all.num_bits << " != " << local_num_bits << " stored in model" << endl;
	  throw exception();
	}
      all.default_bits = false;
      all.num_bits = local_num_bits;
      
      uint32_t pair_len = (uint32_t)all.pairs.size();
      text_len = sprintf(buff, "%d pairs: ", (int)pair_len);
      bin_text_read_write_fixed(model_file,(char *)&pair_len, sizeof(pair_len), 
				"", read, 
				buff, text_len, text);
      for (size_t i = 0; i < pair_len; i++)
	{
	  char pair[2];
	  if (!read)
	    {
	      memcpy(pair,all.pairs[i].c_str(),2);
	      text_len = sprintf(buff, "%s ", all.pairs[i].c_str());
	    }
	  bin_text_read_write_fixed(model_file, pair,2, 
				    "", read,
				    buff, text_len, text);
	  if (read)
	    {
	      string temp(pair, 2);
	      if (count(all.pairs.begin(), all.pairs.end(), temp) == 0)
		all.pairs.push_back(temp);
	    }
	}
      bin_text_read_write_fixed(model_file,buff,0,
				"", read,
				"\n",1,text);
      
      uint32_t triple_len = (uint32_t)all.triples.size();
      text_len = sprintf(buff, "%d triples: ", (int)triple_len);
      bin_text_read_write_fixed(model_file,(char *)&triple_len, sizeof(triple_len), 
				"", read, 
			    buff,text_len, text);
      for (size_t i = 0; i < triple_len; i++)
	{
	  char triple[3];
	  if (!read)
	    {
	      text_len = sprintf(buff, "%s ", all.triples[i].c_str());
	      memcpy(triple, all.triples[i].c_str(), 3);
	    }
	  bin_text_read_write_fixed(model_file,triple,3, 
				    "", read,
				    buff,text_len,text);
	  if (read)
	    {
	      string temp(triple,3);
	      if (count(all.triples.begin(), all.triples.end(), temp) == 0)
		all.triples.push_back(temp);
	    }
	}
      bin_text_read_write_fixed(model_file,buff,0,
				"", read, 
				"\n",1, text);
      
      text_len = sprintf(buff, "rank:%d\n", (int)all.rank);
      bin_text_read_write_fixed(model_file,(char*)&all.rank, sizeof(all.rank), 
				"", read, 
				buff,text_len, text);
      
      text_len = sprintf(buff, "lda:%d\n", (int)all.lda);
      bin_text_read_write_fixed(model_file,(char*)&all.lda, sizeof(all.lda), 
				"", read, 
				buff, text_len,text);
      
      uint32_t ngram_len = (uint32_t)all.ngram_strings.size();
      text_len = sprintf(buff, "%d ngram: ", (int)ngram_len);
      bin_text_read_write_fixed(model_file,(char *)&ngram_len, sizeof(ngram_len), 
				"", read, 
				buff,text_len, text);
      for (size_t i = 0; i < ngram_len; i++)
	{
	  char ngram[3] = {0,0,0};
	  if (!read) {
	    text_len = sprintf(buff, "%s ", all.ngram_strings[i].c_str());
	    memcpy(ngram, all.ngram_strings[i].c_str(), min(3, all.ngram_strings[i].size()));
	  }
	  bin_text_read_write_fixed(model_file,ngram,3, 
				    "", read,
				    buff,text_len,text);
	  if (read)
	    {
	      string temp(ngram,3);
	      all.ngram_strings.push_back(temp);
	    }
	}
      if(read)
	compile_gram(all.ngram_strings, all.ngram, (char*)"grams", all.quiet);
      
      bin_text_read_write_fixed(model_file,buff,0,
				"", read, 
				"\n",1, text);
      
      uint32_t skip_len = (uint32_t)all.skip_strings.size();
      text_len = sprintf(buff, "%d skip: ", (int)skip_len);
      bin_text_read_write_fixed(model_file,(char *)&skip_len, sizeof(skip_len), 
				"", read, 
				buff,text_len, text);
      for (size_t i = 0; i < skip_len; i++)
	{
	  char skip[3] = {0,0,0};
	  if (!read) {
	    text_len = sprintf(buff, "%s ", all.skip_strings[i].c_str());
	    memcpy(skip, all.skip_strings[i].c_str(), min(3, all.skip_strings[i].size()));
	  }
	  bin_text_read_write_fixed(model_file,skip,3, 
				    "", read,
				    buff,text_len,text);
	  if (read)
	    {
	      string temp(skip,3);
	      all.skip_strings.push_back(temp);
	    }
	}
      if(read)
	compile_gram(all.skip_strings, all.skips, (char*)"skips", all.quiet);
      bin_text_read_write_fixed(model_file,buff,0,
				"", read, 
				"\n",1, text);
      
      text_len = sprintf(buff, "options:%s\n", all.file_options.c_str());
      uint32_t len = (uint32_t)all.file_options.length()+1;
      memcpy(buff2, all.file_options.c_str(),len);
      if (read)
	len = buf_size;
      bin_text_read_write(model_file,buff2, len, 
			  "", read,
			  buff, text_len, text);
      if (read)
	all.file_options.assign(buff2);
    }

}

void dump_regressor(vw& all, string reg_name, bool as_text)
{
  if (reg_name == string(""))
    return;
  string start_name = reg_name+string(".writing");
  io_buf io_temp;

  io_temp.open_file(start_name.c_str(), all.stdin_off, io_buf::WRITE);
  
  save_load_header(all, io_temp, false, as_text);
  all.l->save_load(io_temp, false, as_text);

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
  dump_regressor(all, string(filename), false);
  delete[] filename;
}

void finalize_regressor(vw& all, string reg_name)
{
  if (!all.early_terminate){
    if (all.per_feature_regularizer_output.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_output, false);
    else
      dump_regressor(all, reg_name, false);
    if (all.per_feature_regularizer_text.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_text, true);
    else{
      dump_regressor(all, all.text_regressor_name, true);
      all.print_invert = true;
      dump_regressor(all, all.inv_hash_regressor_name, true);
      all.print_invert = false;
    }
  }
}

void parse_regressor_args(vw& all, po::variables_map& vm, io_buf& io_temp)
{
  vector<string> regs;
  if (vm.count("initial_regressor") || vm.count("i"))
    regs = vm["initial_regressor"].as< vector<string> >();

  if (vm.count("input_feature_regularizer"))
    regs.push_back(vm["input_feature_regularizer"].as<string>());

  if (regs.size() > 0) {
    io_temp.open_file(regs[0].c_str(), all.stdin_off, io_buf::READ);
    if (!all.quiet) {
        //cerr << "initial_regressor = " << regs[0] << endl;
      if (regs.size() > 1) {
        cerr << "warning: ignoring remaining " << (regs.size() - 1) << " initial regressors" << endl;
      }
    }
  }

  save_load_header(all, io_temp, true, false);
}

void parse_mask_regressor_args(vw& all, po::variables_map& vm){

  if (vm.count("feature_mask")) {
    size_t length = ((size_t)1) << all.num_bits;  
    string mask_filename = vm["feature_mask"].as<string>();
    if (vm.count("initial_regressor")){ 
      vector<string> init_filename = vm["initial_regressor"].as< vector<string> >();
      if(mask_filename == init_filename[0]){//-i and -mask are from same file, just generate mask
           
        return;
      }
    }

    //all other cases, including from different file, or -i does not exist, need to read in the mask file
    io_buf io_temp_mask;
    io_temp_mask.open_file(mask_filename.c_str(), false, io_buf::READ);
    save_load_header(all, io_temp_mask, true, false);
    all.l->save_load(io_temp_mask, true, false);
    io_temp_mask.close_file();

    // Deal with the over-written header from initial regressor
    if (vm.count("initial_regressor")) {
      vector<string> init_filename = vm["initial_regressor"].as< vector<string> >();

      // Load original header again.
      io_buf io_temp;
      io_temp.open_file(init_filename[0].c_str(), false, io_buf::READ);
      save_load_header(all, io_temp, true, false);
      io_temp.close_file();

      // Re-zero the weights, in case weights of initial regressor use different indices
      for (size_t j = 0; j < length; j++){
        all.reg.weight_vector[j << all.reg.stride_shift] = 0.;
      }
    } else {
      // If no initial regressor, just clear out the options loaded from the header.
      all.file_options.assign("");
    }
  }
}


