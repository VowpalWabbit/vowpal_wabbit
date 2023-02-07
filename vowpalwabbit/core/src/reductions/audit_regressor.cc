// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/audit_regressor.h"

#include "vw/config/options.h"
#include "vw/core/interactions.h"
#include "vw/core/learner.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/table_formatter.h"
#include "vw/core/vw.h"

#include <fmt/format.h>

#include <string>

using namespace VW::config;
namespace
{
constexpr size_t NUM_COLS = 3;
constexpr std::array<VW::column_definition, NUM_COLS> AUDIT_REGRESSOR_COLUMNS = {
    VW::column_definition(8, VW::align_type::left, VW::wrap_type::wrap_space),    // example counter
    VW::column_definition(9, VW::align_type::right, VW::wrap_type::wrap_space),   // values audited
    VW::column_definition(12, VW::align_type::right, VW::wrap_type::wrap_space),  // total progress
};
const std::array<std::string, NUM_COLS> AUDIT_REGRESSOR_HEADER = {
    "example\ncounter", "values\naudited", "total\nprogress"};

class audit_regressor_data
{
public:
  audit_regressor_data(VW::workspace* all, std::unique_ptr<VW::io::writer>&& output) : all(all)
  {
    out_file.add_file(std::move(output));
  }

  VW::workspace* all;
  size_t increment = 0;
  size_t cur_class = 0;
  size_t total_class_cnt = 0;
  std::vector<std::string> ns_pre;
  VW::io_buf out_file;
  size_t loaded_regressor_values = 0;
  size_t values_audited = 0;
};

inline void audit_regressor_interaction(audit_regressor_data& dat, const VW::audit_strings* f)
{
  // same as audit_interaction in gd.cc
  if (f == nullptr)
  {
    dat.ns_pre.pop_back();
    return;
  }

  std::string ns_pre;
  if (!dat.ns_pre.empty()) { ns_pre += '*'; }

  if (!f->ns.empty() && ((f->ns) != " "))
  {
    ns_pre.append(f->ns);
    ns_pre += '^';
  }
  if (!f->name.empty())
  {
    ns_pre.append(f->name);
    dat.ns_pre.push_back(ns_pre);
  }
}

inline void audit_regressor_feature(audit_regressor_data& dat, const float, const uint64_t ft_idx)
{
  auto& weights = dat.all->weights;
  if (weights[ft_idx] != 0) { ++dat.values_audited; }
  else { return; }

  std::string ns_pre;
  for (const auto& s : dat.ns_pre) { ns_pre += s; }

  std::ostringstream tempstream;
  tempstream << ':' << ((ft_idx & weights.mask()) >> weights.stride_shift()) << ':' << weights[ft_idx];

  std::string temp = ns_pre + tempstream.str() + '\n';
  if (dat.total_class_cnt > 1)
  {  // add class prefix for multiclass problems
    temp = std::to_string(dat.cur_class) + ':' + temp;
  }

  dat.out_file.bin_write_fixed(temp.c_str(), static_cast<uint32_t>(temp.size()));

  weights[ft_idx] = 0.;  // mark value audited
}

void audit_regressor_lda(audit_regressor_data& rd, VW::LEARNER::learner& /* base */, VW::example& ec)
{
  VW::workspace& all = *rd.all;

  std::ostringstream tempstream;
  auto& weights = rd.all->weights;
  for (unsigned char* i = ec.indices.begin(); i != ec.indices.end(); i++)
  {
    auto& fs = ec.feature_space[*i];
    for (size_t j = 0; j < fs.size(); ++j)
    {
      tempstream << '\t' << fs.space_names[j].ns << '^' << fs.space_names[j].name << ':'
                 << ((fs.indices[j] >> weights.stride_shift()) & all.parse_mask);
      for (size_t k = 0; k < all.lda; k++)
      {
        VW::weight& w = weights[(fs.indices[j] + k)];
        tempstream << ':' << w;
        w = 0.;
      }
      tempstream << std::endl;
    }
  }

  rd.out_file.bin_write_fixed(tempstream.str().c_str(), static_cast<uint32_t>(tempstream.str().size()));
}

// This is a learner which does nothing with examples.
// void learn(audit_regressor_data&, VW::LEARNER::learner&, example&) {}

void audit_regressor(audit_regressor_data& rd, VW::LEARNER::learner& base, VW::example& ec)
{
  VW::workspace& all = *rd.all;

  if (all.lda > 0) { audit_regressor_lda(rd, base, ec); }
  else
  {
    rd.cur_class = 0;
    const uint64_t old_offset = ec.ft_offset;

    while (rd.cur_class < rd.total_class_cnt)
    {
      for (unsigned char* i = ec.indices.begin(); i != ec.indices.end(); ++i)
      {
        const auto& fs = ec.feature_space[static_cast<size_t>(*i)];
        if (!fs.space_names.empty())
        {
          for (size_t j = 0; j < fs.size(); ++j)
          {
            audit_regressor_interaction(rd, &fs.space_names[j]);
            audit_regressor_feature(rd, fs.values[j], static_cast<uint32_t>(fs.indices[j]) + ec.ft_offset);
            audit_regressor_interaction(rd, nullptr);
          }
        }
        else
        {
          for (size_t j = 0; j < fs.size(); ++j)
          {
            audit_regressor_feature(rd, fs.values[j], static_cast<uint32_t>(fs.indices[j]) + ec.ft_offset);
          }
        }
      }

      size_t num_interacted_features = 0;
      if (rd.all->weights.sparse)
      {
        VW::generate_interactions<audit_regressor_data, const uint64_t, audit_regressor_feature, true,
            audit_regressor_interaction, VW::sparse_parameters>(rd.all->interactions, rd.all->extent_interactions,
            rd.all->permutations, ec, rd, rd.all->weights.sparse_weights, num_interacted_features,
            rd.all->generate_interactions_object_cache_state);
      }
      else
      {
        VW::generate_interactions<audit_regressor_data, const uint64_t, audit_regressor_feature, true,
            audit_regressor_interaction, VW::dense_parameters>(rd.all->interactions, rd.all->extent_interactions,
            rd.all->permutations, ec, rd, rd.all->weights.dense_weights, num_interacted_features,
            rd.all->generate_interactions_object_cache_state);
      }

      ec.ft_offset += rd.increment;
      ++rd.cur_class;
    }

    ec.ft_offset = old_offset;  // make sure example is not changed.
  }
}

void print_row(std::ostream& output, size_t ex_processed, size_t vals_found, size_t progress)
{
  VW::format_row({std::to_string(ex_processed), std::to_string(vals_found), std::to_string(progress) + "%"},
      AUDIT_REGRESSOR_COLUMNS, 1, output);
  output << "\n";
}

void print_update_audit_regressor(VW::workspace& all, VW::shared_data& /* sd */, const audit_regressor_data& rd,
    const VW::example& ec, VW::io::logger& /* unused */)
{
  bool printed = false;
  if (static_cast<float>(ec.example_counter + std::size_t{1}) >= all.sd->dump_interval && !all.quiet)
  {
    print_row(*all.trace_message, ec.example_counter + 1, rd.values_audited,
        rd.values_audited * 100 / rd.loaded_regressor_values);
    all.sd->weighted_unlabeled_examples = static_cast<double>(ec.example_counter + 1);  // used in update_dump_interval
    all.sd->update_dump_interval();
    printed = true;
  }

  if (rd.values_audited == rd.loaded_regressor_values)
  {
    // all regressor values were audited
    if (!printed) { print_row(*all.trace_message, ec.example_counter + 1, rd.values_audited, 100); }
    VW::details::set_done(all);
  }
}

void finish(audit_regressor_data& rd)
{
  rd.out_file.flush();

  if (rd.values_audited < rd.loaded_regressor_values)
  {
    *rd.all->trace_message << fmt::format(
        "Note: for some reason audit couldn't find all regressor values in dataset ({} of {} found).\n",
        rd.values_audited, rd.loaded_regressor_values);
  }
}

template <class T>
size_t count_non_zero(const T& w)
{
  return std::count_if(w.cbegin(), w.cend(), [](float weight) { return weight != 0; });
}

void init_driver(audit_regressor_data& dat)
{
  // checks a few settings that might be applied after audit_regressor_setup() is called
  if ((dat.all->options->was_supplied("cache_file") || dat.all->options->was_supplied("cache")) &&
      !dat.all->options->was_supplied("kill_cache"))
  {
    THROW("audit_regressor is incompatible with a cache file. Use it in single pass mode only.")
  }

  dat.all->sd->dump_interval = 1.;  // regressor could initialize these if saved without --predict_only_model
  dat.all->sd->example_number = 0;

  dat.increment = dat.all->l->increment / dat.all->l->weights;
  dat.total_class_cnt = dat.all->l->weights;

  if (dat.all->options->was_supplied("csoaa"))
  {
    const size_t n = dat.all->options->get_typed_option<uint32_t>("csoaa").value();
    if (n != dat.total_class_cnt)
    {
      dat.total_class_cnt = n;
      dat.increment = dat.all->l->increment / n;
    }
  }

  // count non-null feature values in regressor
  dat.loaded_regressor_values = 0;
  if (dat.all->weights.sparse) { dat.loaded_regressor_values += count_non_zero(dat.all->weights.sparse_weights); }
  else { dat.loaded_regressor_values += count_non_zero(dat.all->weights.dense_weights); }

  if (dat.loaded_regressor_values == 0) { THROW("regressor has no non-zero weights. Nothing to audit.") }

  if (!dat.all->quiet)
  {
    *dat.all->trace_message << "Regressor contains " << dat.loaded_regressor_values << " values\n";
    VW::format_row(AUDIT_REGRESSOR_HEADER, AUDIT_REGRESSOR_COLUMNS, 1, *dat.all->trace_message);
    (*dat.all->trace_message) << "\n";
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::audit_regressor_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  std::string out_file;
  option_group_definition new_options("[Reduction] Audit Regressor");
  new_options.add(make_option("audit_regressor", out_file)
                      .keep()
                      .necessary()
                      .help("Stores feature names and their regressor values. Same dataset must be used for both "
                            "regressor training and this mode."));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (out_file.empty()) { THROW("audit_regressor argument (output filename) is missing.") }

  if (all.numpasses > 1) { THROW("audit_regressor can't be used with --passes > 1.") }

  all.audit = true;

  // TODO: work out how to handle the fact that this reduction produces no
  // predictions but also needs to inherit the type from the loaded base so that
  // the rest of the stack loads.
  auto dat = VW::make_unique<audit_regressor_data>(&all, VW::io::open_file_writer(out_file));
  auto ret = VW::LEARNER::make_reduction_learner(std::move(dat), require_singleline(stack_builder.setup_base_learner()),
      audit_regressor, audit_regressor, stack_builder.get_setupfn_name(audit_regressor_setup))
                 // learn does not predict or learn. nothing to be gained by calling predict() before learn()
                 .set_learn_returns_prediction(true)
                 .set_finish(::finish)
                 .set_print_update(::print_update_audit_regressor)
                 .set_init_driver(init_driver)
                 .build();

  return ret;
}
