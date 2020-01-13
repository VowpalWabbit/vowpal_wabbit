// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <fstream>
#include <iostream>

#include "crossplat_compat.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <cstdarg>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "crossplat_compat.h"
#include "rand48.h"
#include "global_data.h"
#include "vw_exception.h"
#include "vw_validate.h"
#include "vw_versions.h"
#include "options_serializer_boost_po.h"

template <class T>
class set_initial_wrapper
{
 public:
  static void func(weight& w, float& initial, uint64_t /* index */) { w = initial; }
};

template <class T>
class random_positive_wrapper
{
 public:
  static void func(weight& w, uint64_t index) { w = (float)(0.1 * merand48(index)); }
};

template <class T>
class random_weights_wrapper
{
 public:
  static void func(weight& w, uint64_t index) { w = (float)(merand48(index) - 0.5); }
};
// box-muller polar implementation
template <class T>
class polar_normal_weights_wrapper
{
 public:
  static void func(weight& w, uint64_t index)
  {
    static float x1 = 0.0;
    static float x2 = 0.0;
    static float temp = 0.0;
    do
    {
      x1 = 2.0f * merand48(index) - 1.0f;
      x2 = 2.0f * merand48(index) - 1.0f;
      temp = x1 * x1 + x2 * x2;
    } while ((temp >= 1.0) || (temp == 0.0));
    temp = sqrtf((-2.0f * logf(temp)) / temp);
    w = x1 * temp;
  }
};
// re-scaling to re-picking values outside the truncating boundary.
// note:- boundary is twice the standard deviation.
template <class T>
void truncate(vw& all, T& weights)
{
  static double sd = calculate_sd(all, weights);
  std::for_each(weights.begin(), weights.end(), [](float& v) {
    if (std::fabs(v) > sd * 2)
    {
      v = (float)std::remainder(static_cast<double>(v), sd * 2);
    }
  });
}

template <class T>
double calculate_sd(vw& /* all */, T& weights)
{
  static int my_size = 0;
  std::for_each(weights.begin(), weights.end(), [](float /* v */) { my_size += 1; });
  double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
  double mean = sum / my_size;
  std::vector<double> diff(my_size);
  std::transform(weights.begin(), weights.end(), diff.begin(), [mean](double x) { return x - mean; });
  double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  return std::sqrt(sq_sum / my_size);
}
template <class T>
void initialize_regressor(vw& all, T& weights)
{
  // Regressor is already initialized.

  if (weights.not_null())
    return;
  size_t length = ((size_t)1) << all.num_bits;
  try
  {
    uint32_t ss = weights.stride_shift();
    weights.~T();  // dealloc so that we can realloc, now with a known size
    new (&weights) T(length, ss);
  }
  catch (const VW::vw_exception&)
  {
    THROW(" Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>");
  }
  if (weights.mask() == 0)
  {
    THROW(" Failed to allocate weight array with " << all.num_bits << " bits: try decreasing -b <bits>");
  }
  else if (all.initial_weight != 0.)
    weights.template set_default<float, set_initial_wrapper<T> >(all.initial_weight);
  else if (all.random_positive_weights)
    weights.template set_default<random_positive_wrapper<T> >();
  else if (all.random_weights)
    weights.template set_default<random_weights_wrapper<T> >();
  else if (all.normal_weights)
  {
    weights.template set_default<polar_normal_weights_wrapper<T> >();
  }
  else if (all.tnormal_weights)
  {
    weights.template set_default<polar_normal_weights_wrapper<T> >();
    truncate(all, weights);
  }
}

void initialize_regressor(vw& all)
{
  if (all.weights.sparse)
    initialize_regressor(all, all.weights.sparse_weights);
  else
    initialize_regressor(all, all.weights.dense_weights);
}

constexpr size_t default_buf_size = 512;

bool resize_buf_if_needed(char*& __dest, size_t& __dest_size, const size_t __n)
{
  char* new_dest;
  if (__dest_size < __n)
  {
    if ((new_dest = (char*)realloc(__dest, __n)) == NULL)
      THROW("Can't realloc enough memory.")
    else
    {
      __dest = new_dest;
      __dest_size = __n;
      return true;
    }
  }
  return false;
}

inline void safe_memcpy(char*& __dest, size_t& __dest_size, const void* __src, size_t __n)
{
  resize_buf_if_needed(__dest, __dest_size, __n);
  memcpy(__dest, __src, __n);
}

// file_options will be written to when reading
void save_load_header(
    vw& all, io_buf& model_file, bool read, bool text, std::string& file_options, VW::config::options_i& options)
{
  char* buff2 = (char*)malloc(default_buf_size);
  size_t buf2_size = default_buf_size;

  try
  {
    if (model_file.files.size() > 0)
    {
      size_t bytes_read_write = 0;

      size_t v_length = (uint32_t)VW::version.to_string().length() + 1;
      std::stringstream msg;
      msg << "Version " << VW::version.to_string() << "\n";
      memcpy(buff2, VW::version.to_string().c_str(), std::min(v_length, buf2_size));
      if (read)
      {
        v_length = buf2_size;
        buff2[std::min(v_length, default_buf_size) - 1] = '\0';
      }
      bytes_read_write += bin_text_read_write(model_file, buff2, v_length, "", read, msg, text);
      all.model_file_ver = buff2;  // stored in all to check save_resume fix in gd
      VW::validate_version(all);

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH)
        model_file.verify_hash(true);

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_ID)
      {
        v_length = all.id.length() + 1;

        msg << "Id " << all.id << "\n";
        memcpy(buff2, all.id.c_str(), std::min(v_length, default_buf_size));
        if (read)
          v_length = default_buf_size;
        bytes_read_write += bin_text_read_write(model_file, buff2, v_length, "", read, msg, text);
        all.id = buff2;

        if (read && !options.was_supplied("id") && !all.id.empty())
        {
          file_options += " --id";
          file_options += " " + all.id;
        }
      }

      char model = 'm';

      bytes_read_write +=
          bin_text_read_write_fixed_validated(model_file, &model, 1, "file is not a model file", read, msg, text);

      msg << "Min label:" << all.sd->min_label << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(
          model_file, (char*)&all.sd->min_label, sizeof(all.sd->min_label), "", read, msg, text);

      msg << "Max label:" << all.sd->max_label << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(
          model_file, (char*)&all.sd->max_label, sizeof(all.sd->max_label), "", read, msg, text);

      msg << "bits:" << all.num_bits << "\n";
      uint32_t local_num_bits = all.num_bits;
      bytes_read_write += bin_text_read_write_fixed_validated(
          model_file, (char*)&local_num_bits, sizeof(local_num_bits), "", read, msg, text);

      if (read && !options.was_supplied("bit_precision"))
      {
        file_options += " --bit_precision";
        std::stringstream temp;
        temp << local_num_bits;
        file_options += " " + temp.str();
      }

      VW::validate_default_bits(all, local_num_bits);

      all.default_bits = false;
      all.num_bits = local_num_bits;

      VW::validate_num_bits(all);

      if (all.model_file_ver < VERSION_FILE_WITH_INTERACTIONS_IN_FO)
      {
        // -q, --cubic and --interactions are not saved in vw::file_options
        uint32_t pair_len = (uint32_t)all.pairs.size();

        msg << pair_len << " pairs: ";
        bytes_read_write +=
            bin_text_read_write_fixed_validated(model_file, (char*)&pair_len, sizeof(pair_len), "", read, msg, text);

        // TODO: validate pairs?
        for (size_t i = 0; i < pair_len; i++)
        {
          char pair[3] = {0, 0, 0};

          if (!read)
          {
            memcpy(pair, all.pairs[i].c_str(), 2);
            msg << all.pairs[i] << " ";
          }

          bytes_read_write += bin_text_read_write_fixed_validated(model_file, pair, 2, "", read, msg, text);
          if (read)
          {
            std::string temp(pair);
            if (count(all.pairs.begin(), all.pairs.end(), temp) == 0)
              all.pairs.push_back(temp);
          }
        }

        msg << "\n";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0, "", read, msg, text);

        uint32_t triple_len = (uint32_t)all.triples.size();

        msg << triple_len << " triples: ";
        bytes_read_write += bin_text_read_write_fixed_validated(
            model_file, (char*)&triple_len, sizeof(triple_len), "", read, msg, text);

        // TODO: validate triples?
        for (size_t i = 0; i < triple_len; i++)
        {
          char triple[4] = {0, 0, 0, 0};

          if (!read)
          {
            msg << all.triples[i] << " ";
            memcpy(triple, all.triples[i].c_str(), 3);
          }
          bytes_read_write += bin_text_read_write_fixed_validated(model_file, triple, 3, "", read, msg, text);
          if (read)
          {
            std::string temp(triple);
            if (count(all.triples.begin(), all.triples.end(), temp) == 0)
              all.triples.push_back(temp);
          }
        }

        msg << "\n";
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0, "", read, msg, text);

        if (all.model_file_ver >=
            VERSION_FILE_WITH_INTERACTIONS)  // && < VERSION_FILE_WITH_INTERACTIONS_IN_FO (previous if)
        {
          // the only version that saves interacions among pairs and triples
          uint32_t len = (uint32_t)all.interactions.size();

          msg << len << " interactions: ";
          bytes_read_write +=
              bin_text_read_write_fixed_validated(model_file, (char*)&len, sizeof(len), "", read, msg, text);

          for (size_t i = 0; i < len; i++)
          {
            uint32_t inter_len = 0;
            if (!read)
            {
              inter_len = (uint32_t)all.interactions[i].size();
              msg << "len: " << inter_len << " ";
            }
            bytes_read_write += bin_text_read_write_fixed_validated(
                model_file, (char*)&inter_len, sizeof(inter_len), "", read, msg, text);
            if (!read)
            {
              memcpy(buff2, all.interactions[i].c_str(), inter_len);

              msg << "interaction: ";
              msg.write(all.interactions[i].c_str(), inter_len);
            }

            bytes_read_write += bin_text_read_write_fixed_validated(model_file, buff2, inter_len, "", read, msg, text);

            if (read)
            {
              std::string temp(buff2, inter_len);
              all.interactions.push_back(temp);
            }
          }

          msg << "\n";
          bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0, "", read, msg, text);
        }
        else  // < VERSION_FILE_WITH_INTERACTIONS
        {
          // pairs and triples may be restored but not reflected in interactions
          all.interactions.insert(std::end(all.interactions), std::begin(all.pairs), std::end(all.pairs));
          all.interactions.insert(std::end(all.interactions), std::begin(all.triples), std::end(all.triples));
        }
      }

      if (all.model_file_ver <= VERSION_FILE_WITH_RANK_IN_HEADER)
      {
        // to fix compatibility that was broken in 7.9
        uint32_t rank = 0;
        msg << "rank:" << rank << "\n";
        bytes_read_write +=
            bin_text_read_write_fixed_validated(model_file, (char*)&rank, sizeof(rank), "", read, msg, text);
        if (rank != 0)
        {
          if (!options.was_supplied("rank"))
          {
            file_options += " --rank";
            std::stringstream temp;
            temp << rank;
            file_options += " " + temp.str();
          }
          else
            all.trace_message << "WARNING: this model file contains 'rank: " << rank
                              << "' value but it will be ignored as another value specified via the command line."
                              << std::endl;
        }
      }

      msg << "lda:" << all.lda << "\n";
      bytes_read_write +=
          bin_text_read_write_fixed_validated(model_file, (char*)&all.lda, sizeof(all.lda), "", read, msg, text);

      // TODO: validate ngram_len?
      uint32_t ngram_len = (uint32_t)all.ngram_strings.size();
      msg << ngram_len << " ngram:";
      bytes_read_write +=
          bin_text_read_write_fixed_validated(model_file, (char*)&ngram_len, sizeof(ngram_len), "", read, msg, text);
      for (size_t i = 0; i < ngram_len; i++)
      {
        // have '\0' at the end for sure
        char ngram[4] = {0, 0, 0, 0};
        if (!read)
        {
          msg << all.ngram_strings[i] << " ";
          memcpy(ngram, all.ngram_strings[i].c_str(), std::min(static_cast<size_t>(3), all.ngram_strings[i].size()));
        }
        bytes_read_write += bin_text_read_write_fixed_validated(model_file, ngram, 3, "", read, msg, text);
        if (read)
        {
          std::string temp(ngram);
          all.ngram_strings.push_back(temp);

          file_options += " --ngram";
          file_options += " " + temp;
        }
      }

      msg << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0, "", read, msg, text);

      // TODO: validate skips?
      uint32_t skip_len = (uint32_t)all.skip_strings.size();
      msg << skip_len << " skip:";
      bytes_read_write +=
          bin_text_read_write_fixed_validated(model_file, (char*)&skip_len, sizeof(skip_len), "", read, msg, text);

      for (size_t i = 0; i < skip_len; i++)
      {
        char skip[4] = {0, 0, 0, 0};
        if (!read)
        {
          msg << all.skip_strings[i] << " ";
          memcpy(skip, all.skip_strings[i].c_str(), std::min(static_cast<size_t>(3), all.skip_strings[i].size()));
        }

        bytes_read_write += bin_text_read_write_fixed_validated(model_file, skip, 3, "", read, msg, text);
        if (read)
        {
          std::string temp(skip);
          all.skip_strings.push_back(temp);

          file_options += " --skips";
          file_options += " " + temp;
        }
      }
      msg << "\n";
      bytes_read_write += bin_text_read_write_fixed_validated(model_file, nullptr, 0, "", read, msg, text);

      if (read)
      {
        uint32_t len;
        size_t ret = model_file.bin_read_fixed((char*)&len, sizeof(len), "");
        if (len > 104857600 /*sanity check: 100 Mb*/ || ret < sizeof(uint32_t))
          THROW("bad model format!");
        resize_buf_if_needed(buff2, buf2_size, len);
        bytes_read_write += model_file.bin_read_fixed(buff2, len, "") + ret;

        // Write out file options to caller.
        if (len > 0)
        {
          // There is a potential bug here if len is read out to be zero (e.g. corrupted file). If we naively
          // append buff2 into file_options it might contain old information and thus be invalid. Before, what
          // probably happened is boost::program_options did the right thing, but now we have to construct the
          // input to it where we do not know whether a particular option key can have multiple values or not.
          //
          // In some cases we end up with a std::string like: "--bit_precision 18 <something_not_an_int>", which will
          // cause a "bad program options value" exception, rather than the true "file is corrupted" issue. Only
          // pushing the contents of buff2 into file_options when it is valid will prevent this false error.
          file_options = file_options + " " + buff2;
        }
      }
      else
      {
        VW::config::options_serializer_boost_po serializer;
        for (auto const& option : options.get_all_options())
        {
          if (option->m_keep && options.was_supplied(option->m_name))
          {
            serializer.add(*option);
          }
        }

        auto serialized_keep_options = serializer.str();

        // We need to save our current PRG state
        if (all.save_resume && all.get_random_state()->get_current_state() != 0)
        {
          serialized_keep_options += " --random_seed";
          serialized_keep_options += " " + std::to_string(all.get_random_state()->get_current_state());
        }

        msg << "options:" << serialized_keep_options << "\n";

        uint32_t len = (uint32_t)serialized_keep_options.length();
        if (len > 0)
          safe_memcpy(buff2, buf2_size, serialized_keep_options.c_str(), len + 1);
        *(buff2 + len) = 0;
        bytes_read_write += bin_text_read_write(model_file, buff2, len + 1,  // len+1 to write a \0
            "", read, msg, text);
      }

      // Read/write checksum if required by version
      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_HASH)
      {
        uint32_t check_sum = (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH)
            ? model_file.hash()
            : (uint32_t)uniform_hash(model_file.space.begin(), bytes_read_write, 0);

        uint32_t check_sum_saved = check_sum;

        msg << "Checksum: " << check_sum << "\n";
        bin_text_read_write(model_file, (char*)&check_sum, sizeof(check_sum), "", read, msg, text);

        if (check_sum_saved != check_sum)
          THROW("Checksum is inconsistent, file is possibly corrupted.");
      }

      if (all.model_file_ver >= VERSION_FILE_WITH_HEADER_CHAINED_HASH)
      {
        model_file.verify_hash(false);
      }
    }
  }
  catch (...)
  {
    free(buff2);
    throw;
  }

  free(buff2);
}

void dump_regressor(vw& all, io_buf& buf, bool as_text)
{
  std::string unused;
  save_load_header(all, buf, false, as_text, unused, *all.options);
  if (all.l != nullptr)
    all.l->save_load(buf, false, as_text);

  buf.flush();  // close_file() should do this for me ...
  buf.close_file();
}

void dump_regressor(vw& all, std::string reg_name, bool as_text)
{
  if (reg_name == std::string(""))
    return;
  std::string start_name = reg_name + std::string(".writing");
  io_buf io_temp;

  io_temp.open_file(start_name.c_str(), all.stdin_off, io_buf::WRITE);

  dump_regressor(all, io_temp, as_text);

  remove(reg_name.c_str());

  if (0 != rename(start_name.c_str(), reg_name.c_str()))
    THROW("WARN: dump_regressor(vw& all, std::string reg_name, bool as_text): cannot rename: "
        << start_name.c_str() << " to " << reg_name.c_str());
}

void save_predictor(vw& all, std::string reg_name, size_t current_pass)
{
  std::stringstream filename;
  filename << reg_name;
  if (all.save_per_pass)
    filename << "." << current_pass;
  dump_regressor(all, filename.str(), false);
}

void finalize_regressor(vw& all, std::string reg_name)
{
  if (!all.early_terminate)
  {
    if (all.per_feature_regularizer_output.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_output, false);
    else
      dump_regressor(all, reg_name, false);
    if (all.per_feature_regularizer_text.length() > 0)
      dump_regressor(all, all.per_feature_regularizer_text, true);
    else
    {
      dump_regressor(all, all.text_regressor_name, true);
      all.print_invert = true;
      dump_regressor(all, all.inv_hash_regressor_name, true);
      all.print_invert = false;
    }
  }
}

void read_regressor_file(vw& all, std::vector<std::string> all_intial, io_buf& io_temp)
{
  if (all_intial.size() > 0)
  {
    io_temp.open_file(all_intial[0].c_str(), all.stdin_off, io_buf::READ);
    if (!all.quiet)
    {
      // all.trace_message << "initial_regressor = " << regs[0] << std::endl;
      if (all_intial.size() > 1)
      {
        all.trace_message << "warning: ignoring remaining " << (all_intial.size() - 1) << " initial regressors"
                          << std::endl;
      }
    }
  }
}

void parse_mask_regressor_args(vw& all, std::string feature_mask, std::vector<std::string> initial_regressors)
{
  // TODO does this extra check need to be used? I think it is duplicated but there may be some logic I am missing.
  std::string file_options;
  if (!feature_mask.empty())
  {
    if (initial_regressors.size() > 0)
    {
      if (feature_mask == initial_regressors[0])  //-i and -mask are from same file, just generate mask
      {
        return;
      }
    }

    // all other cases, including from different file, or -i does not exist, need to read in the mask file
    io_buf io_temp_mask;
    io_temp_mask.open_file(feature_mask.c_str(), false, io_buf::READ);
    save_load_header(all, io_temp_mask, true, false, file_options, *all.options);
    all.l->save_load(io_temp_mask, true, false);
    io_temp_mask.close_file();

    // Deal with the over-written header from initial regressor
    if (initial_regressors.size() > 0)
    {
      // Load original header again.
      io_buf io_temp;
      io_temp.open_file(initial_regressors[0].c_str(), false, io_buf::READ);
      save_load_header(all, io_temp, true, false, file_options, *all.options);
      io_temp.close_file();

      // Re-zero the weights, in case weights of initial regressor use different indices
      all.weights.set_zero(0);
    }
    else
    {
      // If no initial regressor, just clear out the options loaded from the header.
      // TODO clear file options
      // all.opts_n_args.file_options.str("");
    }
  }
}

namespace VW
{
void save_predictor(vw& all, std::string reg_name) { dump_regressor(all, reg_name, false); }

void save_predictor(vw& all, io_buf& buf) { dump_regressor(all, buf, false); }
}  // namespace VW
