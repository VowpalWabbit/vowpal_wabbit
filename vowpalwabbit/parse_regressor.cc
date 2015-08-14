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
#define sprintf_s snprintf
#endif
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <algorithm>

#include "rand48.h"
#include "global_data.h"
#include "vw_exception.h"

/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "6.1.3"
#define VERSION_FILE_WITH_CUBIC "6.1.3"
#define VERSION_FILE_WITH_RANK_IN_HEADER "7.8.0" // varsion since which rank was moved to vw::file_options
#define VERSION_FILE_WITH_INTERACTIONS "7.10.2" // first version that saves interacions among pairs and triples
#define VERSION_FILE_WITH_INTERACTIONS_IN_FO "7.10.3" // since this ver -q, --cubic and --interactions are stored in vw::file_options

void initialize_regressor(vw& all)
{
  // Regressor is already initialized.
  if (all.reg.weight_vector != nullptr) {
    return;
  }

  size_t length = ((size_t)1) << all.num_bits;
  all.reg.weight_mask = (length << all.reg.stride_shift) - 1;
  all.reg.weight_vector = calloc_or_die<weight>(length << all.reg.stride_shift);
  if (all.reg.weight_vector == nullptr)
    { THROW(" Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>"); }
  else
    if (all.initial_weight != 0.)
      for (size_t j = 0; j < length << all.reg.stride_shift; j+= ( ((size_t)1) << all.reg.stride_shift))
	all.reg.weight_vector[j] = all.initial_weight;      
    else
      if (all.random_positive_weights)
	for (size_t j = 0; j < length; j++)
	  all.reg.weight_vector[j << all.reg.stride_shift] = (float)(0.1 * frand48());
      else      
	if (all.random_weights)
	  for (size_t j = 0; j < length; j++)
	    all.reg.weight_vector[j << all.reg.stride_shift] = (float)(frand48() - 0.5);
}

const size_t buf_size = 512;

void save_load_header(vw& all, io_buf& model_file, bool read, bool text)
{
    char buff[buf_size];
    char buff2[buf_size];
    uint32_t text_len;

    if (model_file.files.size() > 0)
    {
        uint32_t v_length = (uint32_t)version.to_string().length() + 1;
        text_len = sprintf_s(buff, buf_size, "Version %s\n", version.to_string().c_str());
        memcpy(buff2, version.to_string().c_str(), min(v_length, buf_size));
        if (read)
        {
            v_length = buf_size;
        }
        bin_text_read_write(model_file, buff2, v_length,
            "", read,
            buff, text_len, text);
        all.model_file_ver = buff2; //stord in all to check save_resume fix in gd

        if (all.model_file_ver < LAST_COMPATIBLE_VERSION || all.model_file_ver > PACKAGE_VERSION)
        {
            THROW("Model has possibly incompatible version! " << all.model_file_ver.to_string());
        }

        char model = 'm';
        bin_text_read_write_fixed(model_file, &model, 1,
            "file is not a model file", read,
            "", 0, text);

        text_len = sprintf_s(buff, buf_size, "Min label:%f\n", all.sd->min_label);
        bin_text_read_write_fixed(model_file, (char*)&all.sd->min_label, sizeof(all.sd->min_label),
            "", read,
            buff, text_len, text);

        if (read && find(all.args.begin(), all.args.end(), "--min_prediction") == all.args.end())
        {
            all.args.push_back("--min_prediction");
            all.args.push_back(boost::lexical_cast<std::string>(all.sd->min_label));
        }

        text_len = sprintf_s(buff, buf_size, "Max label:%f\n", all.sd->max_label);
        bin_text_read_write_fixed(model_file, (char*)&all.sd->max_label, sizeof(all.sd->max_label),
            "", read,
            buff, text_len, text);

        if (read && find(all.args.begin(), all.args.end(), "--max_prediction") == all.args.end())
        {
            all.args.push_back("--max_prediction");
            all.args.push_back(boost::lexical_cast<std::string>(all.sd->max_label));
        }

        text_len = sprintf_s(buff, buf_size, "bits:%d\n", (int)all.num_bits);
        uint32_t local_num_bits = all.num_bits;
        bin_text_read_write_fixed(model_file, (char *)&local_num_bits, sizeof(local_num_bits),
            "", read,
            buff, text_len, text);

        if (read && find(all.args.begin(), all.args.end(), "--bit_precision") == all.args.end())
        {
            all.args.push_back("--bit_precision");
            all.args.push_back(boost::lexical_cast<std::string>(local_num_bits));
        }

        if (all.default_bits != true && all.num_bits != local_num_bits)
        {
            THROW("-b bits mismatch: command-line " << all.num_bits << " != " << local_num_bits << " stored in model");
        }

        all.default_bits = false;
        all.num_bits = local_num_bits;

        if (all.model_file_ver < VERSION_FILE_WITH_INTERACTIONS_IN_FO)
        {  // -q, --cubic and --interactions are saved in vw::file_options
            uint32_t pair_len = (uint32_t)all.pairs.size();
            text_len = sprintf_s(buff, buf_size, "%d pairs: ", (int)pair_len);
            bin_text_read_write_fixed(model_file, (char *)&pair_len, sizeof(pair_len),
                "", read,
                buff, text_len, text);

            for (size_t i = 0; i < pair_len; i++)
            {
                char pair[3] = { 0, 0, 0 };
                if (!read)
                {
                    memcpy(pair, all.pairs[i].c_str(), 2);
                    text_len = sprintf_s(buff, buf_size, "%s ", all.pairs[i].c_str());
                }

                bin_text_read_write_fixed(model_file, pair, 2,
                    "", read,
                    buff, text_len, text);
                if (read)
                {
                    string temp(pair);
                    if (count(all.pairs.begin(), all.pairs.end(), temp) == 0)
                        all.pairs.push_back(temp);
                }
            }

            bin_text_read_write_fixed(model_file, buff, 0,
                "", read,
                "\n", 1, text);

            uint32_t triple_len = (uint32_t)all.triples.size();
            text_len = sprintf_s(buff, buf_size, "%d triples: ", (int)triple_len);
            bin_text_read_write_fixed(model_file, (char *)&triple_len, sizeof(triple_len),
                "", read,
                buff, text_len, text);

            for (size_t i = 0; i < triple_len; i++)
            {
                char triple[4] = { 0, 0, 0, 0 };
                if (!read)
                {
                    text_len = sprintf_s(buff, buf_size, "%s ", all.triples[i].c_str());
                    memcpy(triple, all.triples[i].c_str(), 3);
                }
                bin_text_read_write_fixed(model_file, triple, 3,
                    "", read,
                    buff, text_len, text);
                if (read)
                {
                    string temp(triple);
                    if (count(all.triples.begin(), all.triples.end(), temp) == 0)
                        all.triples.push_back(temp);
                }
            }
            bin_text_read_write_fixed(model_file, buff, 0,
                "", read,
                "\n", 1, text);

            if (all.model_file_ver >= VERSION_FILE_WITH_INTERACTIONS) // && < VERSION_FILE_WITH_INTERACTIONS_IN_FO (previous if)
            { // the only version that saves interacions among pairs and triples
                uint32_t len = (uint32_t)all.interactions.size();
                text_len = sprintf_s(buff, buf_size, "%d interactions: ", (int)len);
                bin_text_read_write_fixed(model_file, (char *)&len, sizeof(len),
                    "", read,
                    buff, text_len, text);

                for (size_t i = 0; i < len; i++)
                {
                    uint32_t inter_len = 0;
                    if (!read)
                    {
                        inter_len = (uint32_t)all.interactions[i].size();
                        text_len = sprintf_s(buff, buf_size, "len: %d ", inter_len);
                    }
                    bin_text_read_write_fixed(model_file, (char *)&inter_len, sizeof(inter_len),
                        "", read,
                        buff, text_len, text);
                    if (read)
                    {
                        v_string s = v_init<unsigned char>();
                        s.resize(inter_len);
                        s.end += inter_len;
                        all.interactions.push_back(s);
                    }
                    else
                        text_len = sprintf_s(buff, buf_size, "interaction: %.*s ", inter_len, all.interactions[i].begin);

                    bin_text_read_write_fixed(model_file, (char*)all.interactions[i].begin, inter_len,
                        "", read,
                        buff, text_len, text);

                }

                bin_text_read_write_fixed(model_file, buff, 0,
                    "", read,
                    "\n", 1, text);
            }
            else // < VERSION_FILE_WITH_INTERACTIONS
            {
                //pairs and triples may be restored but not reflected in interactions
                for (size_t i = 0; i < all.pairs.size(); i++)
                    all.interactions.push_back(string2v_string(all.pairs[i]));
                for (size_t i = 0; i < all.triples.size(); i++)
                    all.interactions.push_back(string2v_string(all.triples[i]));
            }
        }

        if (all.model_file_ver <= VERSION_FILE_WITH_RANK_IN_HEADER)
        { // to fix compatibility that was broken in 7.9
            uint32_t rank = 0;
            text_len = sprintf_s(buff, buf_size, "rank:%d\n", (int)rank);
            bin_text_read_write_fixed(model_file, (char*)&rank, sizeof(rank),
                "", read,
                buff, text_len, text);
            if (rank != 0)
            {
                if (std::find(all.args.begin(), all.args.end(), "--rank") == all.args.end())
                {
                    all.args.push_back("--rank");
                    sprintf_s(buff, buf_size, "%d", (int)rank);
                    all.args.push_back(buff);
                }
                else
                    cerr << "WARNING: this model file contains 'rank: " << rank << "' value but it will be ignored as another value specified via the command line." << endl;
            }

        }

        text_len = sprintf_s(buff, buf_size, "lda:%d\n", (int)all.lda);
        bin_text_read_write_fixed(model_file, (char*)&all.lda, sizeof(all.lda),
            "", read,
            buff, text_len, text);

        if (read && all.lda > 0)
        {
            all.args.push_back("--lda");
            all.args.push_back(boost::lexical_cast<std::string>(all.lda));
        }

        uint32_t ngram_len = (uint32_t)all.ngram_strings.size();
        text_len = sprintf_s(buff, buf_size, "%d ngram: ", (int)ngram_len);
        bin_text_read_write_fixed(model_file, (char *)&ngram_len, sizeof(ngram_len),
            "", read,
            buff, text_len, text);
        for (size_t i = 0; i < ngram_len; i++)
        {
            // have '\0' at the end for sure
            char ngram[4] = { 0, 0, 0, 0 };
            if (!read) {
                text_len = sprintf_s(buff, buf_size, "%s ", all.ngram_strings[i].c_str());
                memcpy(ngram, all.ngram_strings[i].c_str(), min(3, all.ngram_strings[i].size()));
            }
            bin_text_read_write_fixed(model_file, ngram, 3,
                "", read,
                buff, text_len, text);
            if (read)
            {
                string temp(ngram);
                all.ngram_strings.push_back(temp);

                all.args.push_back("--ngram");
                all.args.push_back(boost::lexical_cast<std::string>(temp));
            }
        }

        bin_text_read_write_fixed(model_file, buff, 0,
            "", read,
            "\n", 1, text);

        uint32_t skip_len = (uint32_t)all.skip_strings.size();
        text_len = sprintf_s(buff, buf_size, "%d skip: ", (int)skip_len);
        bin_text_read_write_fixed(model_file, (char *)&skip_len, sizeof(skip_len),
            "", read,
            buff, text_len, text);

        for (size_t i = 0; i < skip_len; i++)
        {
            char skip[4] = { 0, 0, 0, 0 };
            if (!read) {
                text_len = sprintf_s(buff, buf_size, "%s ", all.skip_strings[i].c_str());
                memcpy(skip, all.skip_strings[i].c_str(), min(3, all.skip_strings[i].size()));
            }
            bin_text_read_write_fixed(model_file, skip, 3,
                "", read,
                buff, text_len, text);
            if (read)
            {
                string temp(skip);
                all.skip_strings.push_back(temp);

                all.args.push_back("--skips");
                all.args.push_back(boost::lexical_cast<std::string>(temp));
            }
        }
        bin_text_read_write_fixed(model_file, buff, 0,
            "", read,
            "\n", 1, text);

        text_len = sprintf_s(buff, buf_size, "options:%s\n", all.file_options->str().c_str());
        uint32_t len = (uint32_t)all.file_options->str().length() + 1;
        memcpy(buff2, all.file_options->str().c_str(), min(len, buf_size));
        
        if (read)
        {
            len = buf_size;
        }
        
        bin_text_read_write(model_file, buff2, len,
            "", read,
            buff, text_len, text);
        
        if (read)
        {
            all.file_options->str(buff2);
        }
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
  stringstream filename;
  filename << reg_name;
  if (all.save_per_pass)
    filename << "." << current_pass;
  dump_regressor(all, filename.str(), false);
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

void parse_regressor_args(vw& all, io_buf& io_temp)
{
  po::variables_map& vm = all.vm;
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
}

void parse_mask_regressor_args(vw& all)
{
  po::variables_map& vm = all.vm;
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
      all.file_options->str("");
    }
  }
}

namespace VW
{
	void save_predictor(vw& all, string reg_name)
	{
		dump_regressor(all, reg_name, false);
	}
}

