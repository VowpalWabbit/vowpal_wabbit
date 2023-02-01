// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parse_regressor.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/io_buf.h"

#include <fstream>
#include <iostream>

#ifndef _WIN32
#  include <unistd.h>
#endif

#include "vw/common/random.h"
#include "vw/common/vw_exception.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/global_data.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/learner.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw_validate.h"
#include "vw/core/vw_versions.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <utility>

void initialize_weights_as_polar_normal(VW::weight* weights, uint64_t index)
{
  weights[0] = VW::details::merand48_boxmuller(index);
}

template <class T>
double calculate_sd(VW::workspace& /* all */, T& weights)
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

// re-scaling to re-picking values outside the truncating boundary.
// note:- boundary is twice the standard deviation.
template <class T>
void truncate(VW::workspace& all, T& weights)
{
  static double sd = calculate_sd(all, weights);
  std::for_each(weights.begin(), weights.end(),
      [](float& v)
      {
        if (std::fabs(v) > sd * 2) { v = static_cast<float>(std::remainder(static_cast<double>(v), sd * 2)); }
      });
}

template <class T>
void initialize_regressor(VW::workspace& all, T& weights)
{
  // Regressor is already initialized.
  if (weights.not_null()) { return; }

  size_t length = (static_cast<size_t>(1)) << all.num_bits;
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
  {
    auto initial_weight = all.initial_weight;
    auto initial_value_weight_initializer = [initial_weight](VW::weight* weights, uint64_t /*index*/)
    { weights[0] = initial_weight; };
    weights.set_default(initial_value_weight_initializer);
  }
  else if (all.random_positive_weights)
  {
    auto rand_state = *all.get_random_state();
    auto random_positive = [&rand_state](VW::weight* weights, uint64_t)
    { weights[0] = 0.1f * rand_state.get_and_update_random(); };
    weights.set_default(random_positive);
  }
  else if (all.random_weights)
  {
    auto rand_state = *all.get_random_state();
    auto random_neg_pos = [&rand_state](VW::weight* weights, uint64_t)
    { weights[0] = rand_state.get_and_update_random() - 0.5f; };
    weights.set_default(random_neg_pos);
  }
  else if (all.normal_weights) { weights.set_default(&initialize_weights_as_polar_normal); }
  else if (all.tnormal_weights)
  {
    weights.set_default(&initialize_weights_as_polar_normal);
    truncate(all, weights);
  }
}

void VW::details::initialize_regressor(VW::workspace& all)
{
  if (all.weights.sparse) { ::initialize_regressor(all, all.weights.sparse_weights); }
  else { ::initialize_regressor(all, all.weights.dense_weights); }
}

namespace
{
constexpr size_t DEFAULT_BUF_SIZE = 512;
}

// file_options will be written to when reading
void VW::details::save_load_header(VW::workspace& all, VW::io_buf& model_file, bool read, bool text,
    std::string& file_options, VW::config::options_i& options)
{
  if (model_file.num_files() > 0)
  {
    std::vector<char> buff2(DEFAULT_BUF_SIZE);
    size_t bytes_read_write = 0;

    size_t v_length = static_cast<uint32_t>(VW::VERSION.to_string().length()) + 1;
    std::stringstream msg;
    msg << "Version " << VW::VERSION.to_string() << "\n";
    memcpy(buff2.data(), VW::VERSION.to_string().c_str(), std::min(v_length, buff2.size()));
    if (read)
    {
      v_length = buff2.size();
      buff2[std::min(v_length, DEFAULT_BUF_SIZE) - 1] = '\0';
    }
    bytes_read_write += VW::details::bin_text_read_write(model_file, buff2.data(), v_length, read, msg, text);
    all.model_file_ver = VW::version_struct{buff2.data()};  // stored in all to check save_resume fix in gd
    VW::validate_version(all);

    if (all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_HEADER_CHAINED_HASH)
    {
      model_file.verify_hash(true);
    }

    if (all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_HEADER_ID)
    {
      v_length = all.id.length() + 1;

      msg << "Id " << all.id << "\n";
      memcpy(buff2.data(), all.id.c_str(), std::min(v_length, DEFAULT_BUF_SIZE));
      if (read) { v_length = DEFAULT_BUF_SIZE; }
      bytes_read_write += VW::details::bin_text_read_write(model_file, buff2.data(), v_length, read, msg, text);
      all.id = buff2.data();

      if (read && !options.was_supplied("id") && !all.id.empty())
      {
        file_options += " --id";
        file_options += " " + all.id;
      }
    }

    char model = 'm';
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, &model, 1, read, msg, text);
    if (model != 'm') { THROW("file is not a model file") }

    msg << "Min label:" << all.sd->min_label << "\n";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&all.sd->min_label), sizeof(all.sd->min_label), read, msg, text);

    msg << "Max label:" << all.sd->max_label << "\n";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&all.sd->max_label), sizeof(all.sd->max_label), read, msg, text);

    msg << "bits:" << all.num_bits << "\n";
    uint32_t local_num_bits = all.num_bits;
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&local_num_bits), sizeof(local_num_bits), read, msg, text);

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

    if (all.model_file_ver < VW::version_definitions::VERSION_FILE_WITH_INTERACTIONS_IN_FO)
    {
      if (!read) THROW("cannot write legacy format");

      // -q, --cubic and --interactions are not saved in vw::file_options
      uint32_t pair_len = 0;
      msg << pair_len << " pairs: ";
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
          model_file, reinterpret_cast<char*>(&pair_len), sizeof(pair_len), read, msg, text);

      // TODO: validate pairs?
      for (size_t i = 0; i < pair_len; i++)
      {
        char pair[3] = {0, 0, 0};

        // Only the read path is implemented since this is for old version read support.
        bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, pair, 2, read, msg, text);
        std::vector<VW::namespace_index> temp(pair, *(&pair + 1));
        if (std::count(all.interactions.begin(), all.interactions.end(), temp) == 0)
        {
          all.interactions.emplace_back(temp.begin(), temp.end());
        }
      }

      msg << "\n";
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, nullptr, 0, read, msg, text);

      uint32_t triple_len = 0;

      msg << triple_len << " triples: ";
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
          model_file, reinterpret_cast<char*>(&triple_len), sizeof(triple_len), read, msg, text);

      // TODO: validate triples?
      for (size_t i = 0; i < triple_len; i++)
      {
        char triple[4] = {0, 0, 0, 0};

        // Only the read path is implemented since this is for old version read support.
        bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, triple, 3, read, msg, text);

        std::vector<VW::namespace_index> temp(triple, *(&triple + 1));
        if (count(all.interactions.begin(), all.interactions.end(), temp) == 0)
        {
          all.interactions.emplace_back(temp.begin(), temp.end());
        }
      }

      msg << "\n";
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, nullptr, 0, read, msg, text);

      if (all.model_file_ver >=
          VW::version_definitions::VERSION_FILE_WITH_INTERACTIONS)  // && < VERSION_FILE_WITH_INTERACTIONS_IN_FO
                                                                    // (previous if)
      {
        if (!read) THROW("cannot write legacy format");

        // the only version that saves interacions among pairs and triples
        uint32_t len = 0;

        msg << len << " interactions: ";
        bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
            model_file, reinterpret_cast<char*>(&len), sizeof(len), read, msg, text);

        for (size_t i = 0; i < len; i++)
        {
          // Only the read path is implemented since this is for old version read support.
          uint32_t inter_len = 0;
          bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
              model_file, reinterpret_cast<char*>(&inter_len), sizeof(inter_len), read, msg, text);

          auto size =
              VW::details::bin_text_read_write_fixed_validated(model_file, buff2.data(), inter_len, read, msg, text);
          bytes_read_write += size;
          if (size != inter_len) { THROW("Failed to read interaction from model file."); }

          std::vector<VW::namespace_index> temp(buff2.data(), buff2.data() + size);
          if (count(all.interactions.begin(), all.interactions.end(), temp) == 0)
          {
            all.interactions.emplace_back(buff2.data(), buff2.data() + inter_len);
          }
        }

        msg << "\n";
        bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, nullptr, 0, read, msg, text);
      }
    }

    if (all.model_file_ver <= VW::version_definitions::VERSION_FILE_WITH_RANK_IN_HEADER)
    {
      // to fix compatibility that was broken in 7.9
      uint32_t rank = 0;
      msg << "rank:" << rank << "\n";
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
          model_file, reinterpret_cast<char*>(&rank), sizeof(rank), read, msg, text);
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
        {
          all.logger.err_warn(
              "This model file contains 'rank: {}' value but it will be ignored as another value specified via the "
              "command line.",
              rank);
        }
      }
    }

    msg << "lda:" << all.lda << "\n";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&all.lda), sizeof(all.lda), read, msg, text);

    // TODO: validate ngram_len?
    auto* g_transformer = all.skip_gram_transformer.get();
    uint32_t ngram_len =
        (g_transformer != nullptr) ? static_cast<uint32_t>(g_transformer->get_initial_ngram_definitions().size()) : 0;
    msg << ngram_len << " ngram:";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&ngram_len), sizeof(ngram_len), read, msg, text);

    std::vector<std::string> temp_vec;
    const auto& ngram_strings = g_transformer != nullptr ? g_transformer->get_initial_ngram_definitions() : temp_vec;
    for (size_t i = 0; i < ngram_len; i++)
    {
      // have '\0' at the end for sure
      char ngram[4] = {0, 0, 0, 0};
      if (!read)
      {
        msg << ngram_strings[i] << " ";
        memcpy(ngram, ngram_strings[i].c_str(), std::min(static_cast<size_t>(3), ngram_strings[i].size()));
      }
      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, ngram, 3, read, msg, text);
      if (read)
      {
        std::string temp(ngram);
        file_options += " --ngram";
        file_options += " " + temp;
      }
    }

    msg << "\n";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, nullptr, 0, read, msg, text);

    // TODO: validate skips?
    uint32_t skip_len =
        (g_transformer != nullptr) ? static_cast<uint32_t>(g_transformer->get_initial_skip_definitions().size()) : 0;
    msg << skip_len << " skip:";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(
        model_file, reinterpret_cast<char*>(&skip_len), sizeof(skip_len), read, msg, text);

    const auto& skip_strings = g_transformer != nullptr ? g_transformer->get_initial_skip_definitions() : temp_vec;
    for (size_t i = 0; i < skip_len; i++)
    {
      char skip[4] = {0, 0, 0, 0};
      if (!read)
      {
        msg << skip_strings[i] << " ";
        memcpy(skip, skip_strings[i].c_str(), std::min(static_cast<size_t>(3), skip_strings[i].size()));
      }

      bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, skip, 3, read, msg, text);
      if (read)
      {
        std::string temp(skip);
        file_options += " --skips";
        file_options += " " + temp;
      }
    }

    msg << "\n";
    bytes_read_write += VW::details::bin_text_read_write_fixed_validated(model_file, nullptr, 0, read, msg, text);

    if (read)
    {
      uint32_t len;
      size_t ret = model_file.bin_read_fixed(reinterpret_cast<char*>(&len), sizeof(len));
      if (len > 104857600 /*sanity check: 100 Mb*/ || ret < sizeof(uint32_t)) THROW("Bad model format.");
      if (buff2.size() < len) { buff2.resize(len); }
      bytes_read_write += model_file.bin_read_fixed(buff2.data(), len) + ret;

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
        file_options = file_options + " " + buff2.data();
      }
    }
    else
    {
      VW::config::cli_options_serializer serializer;

      std::map<std::string, std::set<char>> merged_values = {{"ignore", {}}, {"ignore_linear", {}}, {"keep", {}}};

      for (auto const& option : options.get_all_options())
      {
        if (option->m_keep && options.was_supplied(option->m_name))
        {
          if (merged_values.find(option->m_name) != merged_values.end())
          {
            // Merge and deduplicate the namespaces before serializing into the model file.
            const auto& typed = dynamic_cast<const VW::config::typed_option<std::vector<std::string>>&>(*option);
            for (const auto& v : typed.value()) { merged_values[option->m_name].insert(v.begin(), v.end()); }
            continue;
          }
          serializer.add(*option);
        }
      }

      auto serialized_keep_options = serializer.str();

      // Save deduplicated values for ignore, ignore_linear, or keep.
      for (const auto& kv : merged_values)
      {
        if (kv.second.empty()) { continue; }
        serialized_keep_options += " --" + kv.first + " " + std::string(kv.second.begin(), kv.second.end());
      }

      // We need to save our current PRG state
      if (all.get_random_state()->get_current_state() != 0)
      {
        serialized_keep_options += " --random_seed";
        serialized_keep_options += " " + std::to_string(all.get_random_state()->get_current_state());
      }

      msg << "options:" << serialized_keep_options << "\n";

      const auto len = serialized_keep_options.length();
      if (len > buff2.size()) { buff2.resize(len + 1); }
      memcpy(buff2.data(), serialized_keep_options.c_str(), len + 1);
      *(buff2.data() + len) = 0;
      bytes_read_write += VW::details::bin_text_read_write(model_file, buff2.data(), len + 1,  // len+1 to write a \0
          read, msg, text);
    }

    // Read/write checksum if required by version
    if (all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_HEADER_HASH)
    {
      uint32_t check_sum = (all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_HEADER_CHAINED_HASH)
          ? model_file.hash()
          : static_cast<uint32_t>(VW::uniform_hash(model_file.buffer_start(), bytes_read_write, 0));

      uint32_t check_sum_saved = check_sum;

      msg << "Checksum: " << check_sum << "\n";
      VW::details::bin_text_read_write(
          model_file, reinterpret_cast<char*>(&check_sum), sizeof(check_sum), read, msg, text);

      if (check_sum_saved != check_sum) { THROW("Checksum is inconsistent, file is possibly corrupted."); }
    }

    if (all.model_file_ver >= VW::version_definitions::VERSION_FILE_WITH_HEADER_CHAINED_HASH)
    {
      model_file.verify_hash(false);
    }
  }
}

void VW::details::dump_regressor(VW::workspace& all, VW::io_buf& buf, bool as_text)
{
  if (buf.num_output_files() == 0) { THROW("Cannot dump regressor with an io buffer that has no output files."); }
  std::string unused;
  if (all.l != nullptr) { all.l->pre_save_load(all); }
  VW::details::save_load_header(all, buf, false, as_text, unused, *all.options);
  if (all.l != nullptr) { all.l->save_load(buf, false, as_text); }

  buf.flush();  // close_file() should do this for me ...
  buf.close_file();
}

void VW::details::dump_regressor(VW::workspace& all, const std::string& reg_name, bool as_text)
{
  if (reg_name == std::string("")) { return; }
  std::string start_name = reg_name + std::string(".writing");
  VW::io_buf io_temp;
  io_temp.add_file(VW::io::open_file_writer(start_name));

  dump_regressor(all, io_temp, as_text);

  remove(reg_name.c_str());

  if (0 != rename(start_name.c_str(), reg_name.c_str()))
    THROW("WARN: dump_regressor(VW::workspace& all, std::string reg_name, bool as_text): cannot rename: "
        << start_name.c_str() << " to " << reg_name.c_str());
}

void VW::details::save_predictor(VW::workspace& all, const std::string& reg_name, size_t current_pass)
{
  std::stringstream filename;
  filename << reg_name;
  if (all.save_per_pass) { filename << "." << current_pass; }
  dump_regressor(all, filename.str(), false);
}

void VW::details::finalize_regressor(VW::workspace& all, const std::string& reg_name)
{
  if (!all.early_terminate)
  {
    if (all.per_feature_regularizer_output.length() > 0)
    {
      dump_regressor(all, all.per_feature_regularizer_output, false);
    }
    else { dump_regressor(all, reg_name, false); }
    if (all.per_feature_regularizer_text.length() > 0) { dump_regressor(all, all.per_feature_regularizer_text, true); }
    else
    {
      dump_regressor(all, all.text_regressor_name, true);
      all.print_invert = true;
      dump_regressor(all, all.inv_hash_regressor_name, true);
      all.print_invert = false;
    }
  }
}

void VW::details::read_regressor_file(
    VW::workspace& all, const std::vector<std::string>& all_intial, VW::io_buf& io_temp)
{
  if (all_intial.size() > 0)
  {
    io_temp.add_file(VW::io::open_file_reader(all_intial[0]));

    if (!all.quiet)
    {
      // *(all.trace_message) << "initial_regressor = " << regs[0] << std::endl;
      if (all_intial.size() > 1)
      {
        all.logger.err_warn("Ignoring remaining {} initial regressors", (all_intial.size() - 1));
      }
    }
  }
}

void VW::details::parse_mask_regressor_args(
    VW::workspace& all, const std::string& feature_mask, std::vector<std::string> initial_regressors)
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
    VW::io_buf io_temp_mask;
    io_temp_mask.add_file(VW::io::open_file_reader(feature_mask));

    save_load_header(all, io_temp_mask, true, false, file_options, *all.options);
    all.l->save_load(io_temp_mask, true, false);
    io_temp_mask.close_file();

    // Deal with the over-written header from initial regressor
    if (initial_regressors.size() > 0)
    {
      // Load original header again.
      VW::io_buf io_temp;
      io_temp.add_file(VW::io::open_file_reader(initial_regressors[0]));

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
