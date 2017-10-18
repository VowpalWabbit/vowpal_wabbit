/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <fstream>
#include <iostream>
using namespace std;

#include "crossplat_compat.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <algorithm>
#include <stdarg.h>
#include <numeric>
#include "rand48.h"
#include "global_data.h"
#include "vw_exception.h"
#include "vw_validate.h"
#include "vw_versions.h"

template <class T> class set_initial_wrapper
{
public:
  static void func(weight& w, float& initial, uint64_t index) { w = initial; }
};

template <class T> class random_positive_wrapper
{
public:
  static void func(weight& w, uint64_t index) { w = (float)(0.1 * merand48(index)); }
};

template <class T> class random_weights_wrapper
{
public:
  static void func(weight& w, uint64_t index) { w = (float)(merand48(index) - 0.5); }
};
// box-muller polar implementation
template <class T> class polar_normal_weights_wrapper
{
public:
  static void func(weight& w, uint64_t index)
    {
        static float x1 = 0.0;
        static float x2 = 0.0;
        static float temp  = 0.0;
         do {
                 x1 = 2.0 * merand48(index) - 1.0;
                 x2 = 2.0 * merand48(index) - 1.0;
                 temp = x1 * x1 + x2 * x2;
         } while ( (temp >= 1.0) || (temp == 0.0) );
         temp = sqrt( (-2.0 * log( temp ) ) / temp );
         w = x1 * temp;
    }
};
// re-scaling to re-picking values outside the truncating boundary.
// note:- boundary is twice the standard deviation.
template<class T> void truncate(vw& all,T& weights)
{
  static double sd = calculate_sd(all,weights);
  for_each(weights.begin(), weights.end(), [](float& v) {
	if( abs(v) > sd*2 ) {
           v = std::remainder(v,sd*2);
        }
  });
}

template<class T> double calculate_sd(vw& all,T& weights)
{
  static int my_size = 0;
  for_each(weights.begin(), weights.end(), [](float v) {my_size += 1;});
  double sum = accumulate(weights.begin(), weights.end(), 0.0);
  double mean = sum / my_size;
  vector<double> diff(my_size);
  transform(weights.begin(), weights.end(), diff.begin(), [mean](double x) { return x - mean; });
  double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  return sqrt(sq_sum / my_size);
}
template<class T> void initialize_regressor(vw& all, T& weights)
{ // Regressor is already initialized.

  if (weights.not_null())
    return;
  size_t length = ((size_t)1) << all.num_bits;
  try
    {
      uint32_t ss = weights.stride_shift();
      weights.~T();//dealloc so that we can realloc, now with a known size
      new(&weights) T(length, ss); }
  catch (VW::vw_exception anExc)
    { THROW(" Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>");
    }
  if (weights.mask() == 0)
    { THROW(" Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>"); }
  else if (all.initial_weight != 0.)
    weights.template set_default<float,set_initial_wrapper<T> >(all.initial_weight);
  else if (all.random_positive_weights)
    weights.template set_default<random_positive_wrapper<T> >();
  else if (all.random_weights)
    weights.template set_default<random_weights_wrapper<T> >();
  else if (all.normal_weights){
    weights.template set_default<polar_normal_weights_wrapper<T> >();
  }
  else if (all.tnormal_weights){
    weights.template set_default<polar_normal_weights_wrapper<T> >();
    truncate(all,weights);
  }
}

void initialize_regressor(vw& all)
{
  if (all.weights.sparse)
    initialize_regressor(all, all.weights.sparse_weights);
  else
    initialize_regressor(all, all.weights.dense_weights);
}

const size_t default_buf_size = 512;

bool resize_buf_if_needed(char *& __dest, size_t& __dest_size, const size_t __n)
{ if (__dest_size < __n)
  { if ( (__dest = (char*) realloc(__dest, __n)) == NULL)
      THROW("Can't realloc enough memory.");
    __dest_size = __n;
    return true;
  }
  return false;
}

int32_t safe_sprintf_s(char *& buf, size_t& buf_size, const char * fmt, ...)
{ va_list args;
  va_start(args,fmt);
  int32_t len = vsprintf_s(buf, buf_size, fmt, args);
  va_end(args);
  if (len < 0) THROW("Encoding error.");
  if (resize_buf_if_needed(buf, buf_size, len+1))
  { va_start(args,fmt);
    vsprintf_s(buf, buf_size, fmt, args);
    va_end(args);
  }


  return len;
}

inline void safe_memcpy(char *& __dest, size_t& __dest_size, const void *__src, size_t __n)
{ resize_buf_if_needed(__dest, __dest_size, __n);
  memcpy(__dest, __src, __n);
}

void save_load_header(vw& all, io_buf& model_file, bool read, bool text)
{ char* buff2 = (char*) malloc(default_buf_size);
  size_t buf2_size = default_buf_size;

  try
  { if (model_file.files.size() > 0)
    { size_t bytes_read_write = 0;

      uint32_t v_length = (uint32_t)version.to_string().length() + 1;
      stringstream msg;
      msg << "Version " << version.to_string() << "\n";
      memcpy(buff2, version.to_string().c_str(), min(v_length, buf2_size));
      if (read)
      { v_length = (uint32_t)buf2_size;
        if (v_length > 0) // all.model_file_ver = buff2; uses scanf which doesn't accept a maximum buffer length, but just expects valid zero terminated string
          buff2[min(v_length, default_buf_size) - 1] = '\0';
      }
      bytes_read_write += bin_text_read_write(model_file, buff2, v_length,
                                              "", read, msg, text);
      all.model_file_ver = buff2; //stored in all to check save_resume fix in gd
      VW::validate_version(all);

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH)
        model_file.verify_hash = true;

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_ID)
      { v_length = (uint32_t)all.id.length() + 1;

        msg << "Id " << all.id << "\n";
        memcpy(buff2, all.id.c_str(), min(v_length, default_buf_size));
        if (read)
          v_length = default_buf_size;
        bytes_read_write += bin_text_read_write(model_file, buff2, v_length,
                                                "", read, msg, text);
        all.id = buff2;

        if (read && find(all.args.begin(), all.args.end(), "--id") == all.args.end() && !all.id.empty())
        { all.args.push_back("--id");
          all.args.push_back(all.id);
        }
      }

      char model = 'm';

      bytes_read_write += bin_text_read_write_fixed_validated(model_file, &model, 1,
                          "file is not a model file", read,
                          msg, text);

      msg << "Min label:" << all.sd->min_label << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char*)&all.sd->min_label, sizeof(all.sd->min_label),
                          "", read, msg, text);


      msg << "Max label:" << all.sd->max_label << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char*)&all.sd->max_label, sizeof(all.sd->max_label),
                          "", read, msg, text);

      msg << "bits:" << all.num_bits << "\n";
      uint32_t local_num_bits = all.num_bits;
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&local_num_bits, sizeof(local_num_bits),
                          "", read, msg, text);

      if (read && find(all.args.begin(), all.args.end(), "--bit_precision") == all.args.end())
      { all.args.push_back("--bit_precision");
        all.args.push_back(boost::lexical_cast<std::string>(local_num_bits));
      }

      VW::validate_default_bits(all, local_num_bits);

      all.default_bits = false;
      all.num_bits = local_num_bits;

      VW::validate_num_bits(all);

      if (all.model_file_ver < VERSION_FILE_WITH_INTERACTIONS_IN_FO)
      { // -q, --cubic and --interactions are not saved in vw::file_options
        uint32_t pair_len = (uint32_t)all.pairs.size();

        msg << pair_len << " pairs: ";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&pair_len, sizeof(pair_len),
                            "", read, msg, text);

        // TODO: validate pairs?
        for (size_t i = 0; i < pair_len; i++)
        { char pair[3] = { 0, 0, 0 };

          if (!read)
          { memcpy(pair, all.pairs[i].c_str(), 2);
            msg << all.pairs[i] << " ";
          }

          bytes_read_write += bin_text_read_write_fixed_validated(model_file, pair, 2,
                              "", read, msg, text);
          if (read)
          { string temp(pair);
            if (count(all.pairs.begin(), all.pairs.end(), temp) == 0)
              all.pairs.push_back(temp);
          }
        }


        msg << "\n";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0,
                            "", read, msg, text);

        uint32_t triple_len = (uint32_t)all.triples.size();

        msg << triple_len << " triples: ";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&triple_len, sizeof(triple_len),
                            "", read, msg, text);

        // TODO: validate triples?
        for (size_t i = 0; i < triple_len; i++)
        { char triple[4] = { 0, 0, 0, 0 };

          if (!read)
          { msg << all.triples[i] << " ";
            memcpy(triple, all.triples[i].c_str(), 3);
          }
          bytes_read_write += bin_text_read_write_fixed_validated(model_file, triple, 3,
                              "", read, msg, text);
          if (read)
          { string temp(triple);
            if (count(all.triples.begin(), all.triples.end(), temp) == 0)
              all.triples.push_back(temp);
          }
        }

        msg << "\n";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0,
                            "", read, msg, text);

        if (all.model_file_ver >= VERSION_FILE_WITH_INTERACTIONS) // && < VERSION_FILE_WITH_INTERACTIONS_IN_FO (previous if)
        { // the only version that saves interacions among pairs and triples
          uint32_t len = (uint32_t)all.interactions.size();

          msg << len << " interactions: ";
          bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&len, sizeof(len),
                              "", read, msg, text);

          for (size_t i = 0; i < len; i++)
          { uint32_t inter_len = 0;
            if (!read)
            { inter_len = (uint32_t)all.interactions[i].size();
              msg << "len: " << inter_len << " ";
            }
            bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&inter_len, sizeof(inter_len),
                                "", read, msg, text);
            if (read)
            { v_string s = v_init<unsigned char>();
              s.resize(inter_len);
              s.end() += inter_len;
              all.interactions.push_back(s);
            }
            else
            { msg << "interaction: ";
              msg.write((char*)all.interactions[i].begin(), inter_len);
            }

            bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char*)all.interactions[i].begin(), inter_len,
                                "", read, msg, text);

          }

          msg << "\n";
          bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0,
                              "", read, msg, text);
        }
        else // < VERSION_FILE_WITH_INTERACTIONS
        { //pairs and triples may be restored but not reflected in interactions
          for (size_t i = 0; i < all.pairs.size(); i++)
            all.interactions.push_back(string2v_string(all.pairs[i]));
          for (size_t i = 0; i < all.triples.size(); i++)
            all.interactions.push_back(string2v_string(all.triples[i]));
        }
      }

      if (all.model_file_ver <= VERSION_FILE_WITH_RANK_IN_HEADER)
      { // to fix compatibility that was broken in 7.9
        uint32_t rank = 0;
        msg << "rank:" << rank << "\n";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char*)&rank, sizeof(rank),
                            "", read, msg, text);
        if (rank != 0)
        { if (std::find(all.args.begin(), all.args.end(), "--rank") == all.args.end())
          { all.args.push_back("--rank");
            stringstream temp;
            temp << rank;
            all.args.push_back(temp.str());
          }
          else
            all.trace_message << "WARNING: this model file contains 'rank: " << rank << "' value but it will be ignored as another value specified via the command line." << endl;
        }

      }

      msg << "lda:" << all.lda <<"\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char*)&all.lda, sizeof(all.lda),
                          "", read, msg, text);

      // TODO: validate ngram_len?
      uint32_t ngram_len = (uint32_t)all.ngram_strings.size();
      msg << ngram_len << " ngram:";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&ngram_len, sizeof(ngram_len),
                          "", read, msg, text);
      for (size_t i = 0; i < ngram_len; i++)
      { // have '\0' at the end for sure
        char ngram[4] = { 0, 0, 0, 0 };
        if (!read)
        { msg << all.ngram_strings[i] << " ";
          memcpy(ngram, all.ngram_strings[i].c_str(), min(3, all.ngram_strings[i].size()));
        }
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, ngram, 3,
                            "", read, msg, text);
        if (read)
        { string temp(ngram);
          all.ngram_strings.push_back(temp);

          all.args.push_back("--ngram");
          all.args.push_back(boost::lexical_cast<std::string>(temp));
        }
      }

      msg <<"\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0,
                          "", read, msg, text);

      // TODO: validate skips?
      uint32_t skip_len = (uint32_t)all.skip_strings.size();
      msg << skip_len << " skip:";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, (char *)&skip_len, sizeof(skip_len),
                          "", read, msg, text);

      for (size_t i = 0; i < skip_len; i++)
      { char skip[4] = { 0, 0, 0, 0 };
        if (!read)
        { msg << all.skip_strings[i] << " ";
          memcpy(skip, all.skip_strings[i].c_str(), min(3, all.skip_strings[i].size()));
        }

        bytes_read_write += bin_text_read_write_fixed_validated(model_file, skip, 3,
                            "", read, msg, text);
        if (read)
        { string temp(skip);
          all.skip_strings.push_back(temp);

          all.args.push_back("--skips");
          all.args.push_back(boost::lexical_cast<std::string>(temp));
        }
      }
      msg << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0,
                          "", read, msg, text);

      if (read)
      { uint32_t len;
        size_t ret = bin_read_fixed(model_file, (char*)&len, sizeof(len), "");
        if (len > 104857600 /*sanity check: 100 Mb*/ || ret < sizeof(uint32_t))
          THROW("bad model format!");
        resize_buf_if_needed(buff2, buf2_size, len);
        bytes_read_write += bin_read_fixed(model_file, buff2, len, "") + ret;
        all.file_options->str(buff2);
      }
      else
      { msg << "options:"<< all.file_options->str() << "\n";

        uint32_t len = (uint32_t)all.file_options->str().length();
        if (len > 0)
          safe_memcpy(buff2, buf2_size, all.file_options->str().c_str(), len + 1);
        *(buff2 + len) = 0;
        bytes_read_write += bin_text_read_write(model_file, buff2, len + 1, //len+1 to write a \0
                                                "", read, msg, text);
      }


      // Read/write checksum if required by version
      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_HASH)
      { uint32_t check_sum = (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH) ?
                             model_file.hash :
                             (uint32_t)uniform_hash(model_file.space.begin(), bytes_read_write, 0);

        uint32_t check_sum_saved = check_sum;

        msg << "Checksum: "<< check_sum << "\n";
        bin_text_read_write(model_file, (char*)&check_sum, sizeof(check_sum),
                            "", read, msg, text);

        if (check_sum_saved != check_sum)
          THROW("Checksum is inconsistent, file is possibly corrupted.");
      }

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH)
      { model_file.verify_hash = false;

        // reset the hash so that the io_buf can be re-used for loading
        // as it is done for Reload()
        model_file.hash = 0;
      }
    }
  }
  catch (...)
  { free(buff2);
    throw;
  }

  free(buff2);
}

void dump_regressor(vw& all, io_buf& buf, bool as_text)
{ save_load_header(all, buf, false, as_text);
  if (all.l != nullptr)
    all.l->save_load(buf, false, as_text);

  buf.flush(); // close_file() should do this for me ...
  buf.close_file();
}

void dump_regressor(vw& all, string reg_name, bool as_text)
{ if (reg_name == string(""))
    return;
  string start_name = reg_name+string(".writing");
  io_buf io_temp;

  io_temp.open_file(start_name.c_str(), all.stdin_off, io_buf::WRITE);

  dump_regressor(all, io_temp, as_text);

  remove(reg_name.c_str());
  rename(start_name.c_str(),reg_name.c_str());
}

void save_predictor(vw& all, string reg_name, size_t current_pass)
{ stringstream filename;
  filename << reg_name;
  if (all.save_per_pass)
    filename << "." << current_pass;
  dump_regressor(all, filename.str(), false);
}

void finalize_regressor(vw& all, string reg_name)
{ if (!all.early_terminate)
  { if (all.per_feature_regularizer_output.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_output, false);
    else
      dump_regressor(all, reg_name, false);
    if (all.per_feature_regularizer_text.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_text, true);
    else
    { dump_regressor(all, all.text_regressor_name, true);
      all.print_invert = true;
      dump_regressor(all, all.inv_hash_regressor_name, true);
      all.print_invert = false;
    }
  }
}

void parse_regressor_args(vw& all, io_buf& io_temp)
{ po::variables_map& vm = all.vm;
  vector<string> regs;
  if (vm.count("initial_regressor") || vm.count("i"))
    regs = vm["initial_regressor"].as< vector<string> >();

  if (vm.count("input_feature_regularizer"))
    regs.push_back(vm["input_feature_regularizer"].as<string>());

  if (regs.size() > 0)
  { io_temp.open_file(regs[0].c_str(), all.stdin_off, io_buf::READ);
    if (!all.quiet)
    { //all.trace_message << "initial_regressor = " << regs[0] << endl;
      if (regs.size() > 1)
      { all.trace_message << "warning: ignoring remaining " << (regs.size() - 1) << " initial regressors" << endl;
      }
    }
  }
}

void parse_mask_regressor_args(vw& all)
{ po::variables_map& vm = all.vm;
  if (vm.count("feature_mask"))
  { string mask_filename = vm["feature_mask"].as<string>();
    if (vm.count("initial_regressor"))
    { vector<string> init_filename = vm["initial_regressor"].as< vector<string> >();
      if(mask_filename == init_filename[0])   //-i and -mask are from same file, just generate mask
      { return;
      }
    }

    //all other cases, including from different file, or -i does not exist, need to read in the mask file
    io_buf io_temp_mask;
    io_temp_mask.open_file(mask_filename.c_str(), false, io_buf::READ);
    save_load_header(all, io_temp_mask, true, false);
    all.l->save_load(io_temp_mask, true, false);
    io_temp_mask.close_file();

    // Deal with the over-written header from initial regressor
    if (vm.count("initial_regressor"))
    { vector<string> init_filename = vm["initial_regressor"].as< vector<string> >();

      // Load original header again.
      io_buf io_temp;
      io_temp.open_file(init_filename[0].c_str(), false, io_buf::READ);
      save_load_header(all, io_temp, true, false);
      io_temp.close_file();

      // Re-zero the weights, in case weights of initial regressor use different indices
      all.weights.set_zero(0);
    }
    else
    { // If no initial regressor, just clear out the options loaded from the header.
      all.file_options->str("");
    }
  }
}

namespace VW
{
void save_predictor(vw& all, string reg_name)
{ dump_regressor(all, reg_name, false);
}

void save_predictor(vw& all, io_buf& buf)
{ dump_regressor(all, buf, false);
}
}
