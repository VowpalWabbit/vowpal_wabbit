// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/merge.h"

#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/core/vw_math.h"
#include "vw/io/io_adapter.h"

#include <algorithm>
#include <limits>

namespace
{
std::string get_keep_command_line(const VW::workspace& workspace)
{
  VW::config::cli_options_serializer serializer;
  for (auto const& option : workspace.options->get_all_options())
  {
    if (workspace.options->was_supplied(option->m_name) && option->m_keep) { serializer.add(*option); }
  }

  return serializer.str();
}

void validate_compatibility(const std::vector<const VW::workspace*>& workspaces, VW::io::logger* logger)
{
  if (workspaces.size() < 2) { THROW("Must specify at least two model files to merge."); }

  const auto& ref_model = *workspaces[0];
  for (auto model_it = workspaces.begin() + 1; model_it != workspaces.end(); model_it++)
  {
    const auto* incompatible = VW::are_features_compatible(ref_model, **model_it);
    if (incompatible != nullptr)
    {
      THROW("Model is incompatible with the destination model. Reason: " << incompatible);
    }
  }

  bool at_least_one_has_no_preserve = false;
  for (const auto* model : workspaces)
  {
    if ((!model->preserve_performance_counters) && (model->sd->weighted_labeled_examples == 0.f))
    {
      at_least_one_has_no_preserve = true;
      break;
    }
  }

  if (at_least_one_has_no_preserve && (logger != nullptr))
  {
    logger->warn(
        "At least one model given to merge_models has no recorded examples. This could result in inaccurate merging. "
        "To fix this issue, pass the --preserve_performance_counters flag to the model if loading from disk or if "
        "training in memory then pass some examples prior to merging.");
  }

  std::vector<std::string> destination_enabled_learners;
  ref_model.l->get_enabled_learners(destination_enabled_learners);
  for (const auto* model : workspaces)
  {
    std::vector<std::string> source_enabled_learners;
    model->l->get_enabled_learners(source_enabled_learners);

    if (source_enabled_learners != destination_enabled_learners)
    {
      auto message = fmt::format("Enabled learners are not identical between models.\n One: {}\n Other:{} ",
          fmt::join(source_enabled_learners, ", "), fmt::join(destination_enabled_learners, ", "));
      THROW(message);
    }
  }

  auto destination_command_line = get_keep_command_line(ref_model);
  for (const auto* model : workspaces)
  {
    auto src_command_line = get_keep_command_line(*model);
    if (destination_command_line != src_command_line)
    {
      THROW("Command lines are not identical between models. One: '" << destination_command_line << "', Other: '"
                                                                     << src_command_line << "'");
    }
  }
}

std::unique_ptr<VW::workspace> copy_workspace(const VW::workspace* ws, VW::io::logger* logger = nullptr)
{
  assert(ws != nullptr);
  auto command_line = VW::split_command_line(get_keep_command_line(*ws));
  if (logger == nullptr) { command_line.emplace_back("--quiet"); }
  else { command_line.emplace_back("--driver_output_off"); }
  command_line.emplace_back("--preserve_performance_counters");

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf temp_buffer;
  temp_buffer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*const_cast<VW::workspace*>(ws), temp_buffer);
  return VW::initialize(VW::make_unique<VW::config::options_cli>(command_line),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()), nullptr, nullptr, logger);
}

std::vector<float> calc_per_model_weighting(const std::vector<float>& example_counts)
{
  const auto sum = std::accumulate(example_counts.begin(), example_counts.end(), 0.f);
  std::vector<float> per_model_weighting(example_counts.size(), 0.f);
  for (size_t i = 0; i < example_counts.size(); i++) { per_model_weighting[i] = example_counts[i] / sum; }
  return per_model_weighting;
}

}  // namespace

namespace
{
// These are a bit risky, but it feels like a much nicer user interface for a
// user to be able to pass a ref to a writer to a function rather than require a
// unique pointer especially since we do not take ownership.
class reader_ref_adapter : public VW::io::reader
{
public:
  reader_ref_adapter(VW::io::reader& ref) : VW::io::reader(false), _inner_ref(ref) {}
  ssize_t read(char* buffer, size_t num_bytes) override { return _inner_ref.read(buffer, num_bytes); }

private:
  VW::io::reader& _inner_ref;
};

class writer_ref_adapter : public VW::io::writer
{
public:
  writer_ref_adapter(VW::io::writer& ref) : _inner_ref(ref) {}
  ssize_t write(const char* buffer, size_t num_bytes) override { return _inner_ref.write(buffer, num_bytes); }
  void flush() override { _inner_ref.flush(); }

private:
  VW::io::writer& _inner_ref;
};

}  // namespace

namespace VW
{
void model_delta::serialize(VW::io::writer& output) const
{
  io_buf buffer;
  buffer.add_file(VW::make_unique<writer_ref_adapter>(output));
  VW::save_predictor(*_ws, buffer);
}

std::unique_ptr<model_delta> model_delta::deserialize(VW::io::reader& input)
{
  auto command_line = std::vector<std::string>{"--preserve_performance_counters", "--quiet"};
  return VW::make_unique<model_delta>(VW::initialize(
      VW::make_unique<VW::config::options_cli>(command_line), VW::make_unique<reader_ref_adapter>(input)));
}

VW::model_delta merge_deltas(const std::vector<const VW::model_delta*>& deltas_to_merge, VW::io::logger* logger)
{
  // Get workspace pointers from deltas
  std::vector<const VW::workspace*> workspaces_to_merge;
  workspaces_to_merge.reserve(deltas_to_merge.size());
  for (const auto delta_ptr : deltas_to_merge) { workspaces_to_merge.push_back(delta_ptr->unsafe_get_workspace_ptr()); }
  validate_compatibility(workspaces_to_merge, logger);

  // Get VW command line and create output workspace
  auto command_line = VW::split_command_line(get_keep_command_line(*workspaces_to_merge[0]));
  if (logger == nullptr) { command_line.emplace_back("--quiet"); }
  else { command_line.emplace_back("--driver_output_off"); }
  command_line.emplace_back("--preserve_performance_counters");
  auto dest_workspace =
      VW::initialize(VW::make_unique<VW::config::options_cli>(command_line), nullptr, nullptr, nullptr, logger);

  // Get example counts and compute weighting of models
  std::vector<float> example_counts;
  example_counts.reserve(workspaces_to_merge.size());
  for (const auto* delta : workspaces_to_merge) { example_counts.push_back(delta->sd->weighted_labeled_examples); }
  const auto per_model_weighting = calc_per_model_weighting(example_counts);

  // Iterate through learners and merge each one
  auto* target_learner = dest_workspace->l.get();
  while (target_learner != nullptr)
  {
    if (target_learner->has_merge())
    {
      std::vector<const VW::LEARNER::learner*> learners_to_merge;
      for (const auto* delta : workspaces_to_merge)
      {
        auto* source_data = delta->l->get_learner_by_name_prefix(target_learner->get_name());
        learners_to_merge.push_back(source_data);
      }

      target_learner->merge(
          per_model_weighting, workspaces_to_merge, learners_to_merge, *dest_workspace, *target_learner);
    }
    // If this is a bottom learner and has no merge then emit an error because a bottom with no merge is almost
    // certainly not going to work.
    else if (!target_learner->has_merge() && target_learner->get_base_learner() == nullptr)
    {
      THROW(
          "Bottom learner '" << target_learner->get_name()
                             << "' does not have a merge function defined. Since it is a bottom learner, merging will "
                                "not work as expected.");
    }
    else if (!target_learner->has_merge() && target_learner->learner_defines_own_save_load())
    {
      if (logger != nullptr)
      {
        logger->warn(
            "Learner '{}' supports save/load but does not have a merge function defined. Merging will still run but "
            "this learner will not be merged and may result in incorrect results.",
            target_learner->get_name());
      }
    }
    target_learner = target_learner->get_base_learner();
  }

  // Merge shared data
  for (const auto* delta : workspaces_to_merge)
  {
    dest_workspace->sd->sum_loss += delta->sd->sum_loss;
    dest_workspace->sd->weighted_labeled_examples += delta->sd->weighted_labeled_examples;
    dest_workspace->sd->weighted_labels += delta->sd->weighted_labels;
    dest_workspace->sd->weighted_unlabeled_examples += delta->sd->weighted_unlabeled_examples;
    dest_workspace->sd->example_number += delta->sd->example_number;
    dest_workspace->sd->total_features += delta->sd->total_features;
    dest_workspace->sd->t += delta->sd->t;
    dest_workspace->sd->max_label = std::max(dest_workspace->sd->max_label, delta->sd->max_label);
    dest_workspace->sd->min_label = std::min(dest_workspace->sd->min_label, delta->sd->min_label);
  }

  return VW::model_delta(std::move(dest_workspace));
}

std::unique_ptr<VW::workspace> merge_models(const VW::workspace* base_workspace,
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger)
{
  std::vector<VW::model_delta> deltas;
  deltas.reserve(workspaces_to_merge.size());

  if (base_workspace != nullptr)
  {
    for (const auto* ws : workspaces_to_merge) { deltas.emplace_back(*ws - *base_workspace); }
  }
  else
  {
    for (const auto* ws : workspaces_to_merge)
    {
      // No base workspace to subtract, but we must make a copy of workspace to give delta ownership of it
      deltas.emplace_back(copy_workspace(ws, logger));
    }
  }

  std::vector<const VW::model_delta*> delta_ptrs;
  delta_ptrs.reserve(deltas.size());
  for (const auto& d : deltas) { delta_ptrs.push_back(&d); }
  VW::model_delta merged = merge_deltas(delta_ptrs, logger);

  if (base_workspace != nullptr) { return *base_workspace + merged; }
  return std::unique_ptr<VW::workspace>(merged.unsafe_release_workspace_ptr());
}
}  // namespace VW

std::unique_ptr<VW::workspace> VW::operator+(const VW::workspace& base, const VW::model_delta& md)
{
  const VW::workspace* delta = md.unsafe_get_workspace_ptr();
  validate_compatibility(std::vector<const VW::workspace*>{&base, delta}, nullptr);
  auto dest_command_line = VW::split_command_line(get_keep_command_line(base));
  dest_command_line.emplace_back("--quiet");
  dest_command_line.emplace_back("--preserve_performance_counters");

  auto destination_workspace =
      VW::initialize(VW::make_unique<VW::config::options_cli>(dest_command_line), nullptr, nullptr, nullptr, nullptr);

  auto* target_learner = destination_workspace->l.get();
  while (target_learner != nullptr)
  {
    if (target_learner->has_add())
    {
      auto learner_name = target_learner->get_name();
      const VW::LEARNER::learner* learner = base.l->get_learner_by_name_prefix(learner_name);
      const VW::LEARNER::learner* delta_learner = delta->l->get_learner_by_name_prefix(learner_name);

      target_learner->add(base, *delta, learner, delta_learner, *destination_workspace, target_learner);
    }
    // If this is a bottom learner and has no merge then emit an error because a bottom with no merge is almost
    // certainly not going to work.
    else if (!target_learner->has_merge() && target_learner->get_base_learner() == nullptr)
    {
      THROW(
          "Bottom learner '" << target_learner->get_name()
                             << "' does not have a merge function defined. Since it is a bottom learner, merging will "
                                "not work as expected.");
    }
    // Skip the save-load case for now

    target_learner = target_learner->get_base_learner();
  }

  // Add shared data
  auto& output_sd = *destination_workspace->sd;
  output_sd.sum_loss = base.sd->sum_loss + delta->sd->sum_loss;
  output_sd.weighted_labeled_examples = base.sd->weighted_labeled_examples + delta->sd->weighted_labeled_examples;
  output_sd.weighted_labels = base.sd->weighted_labels + delta->sd->weighted_labels;
  output_sd.weighted_unlabeled_examples = base.sd->weighted_unlabeled_examples + delta->sd->weighted_unlabeled_examples;
  output_sd.example_number = base.sd->example_number + delta->sd->example_number;
  output_sd.total_features = base.sd->total_features + delta->sd->total_features;
  output_sd.t = base.sd->t + delta->sd->t;
  output_sd.max_label = std::max(base.sd->max_label, delta->sd->max_label);
  output_sd.min_label = std::min(base.sd->min_label, delta->sd->min_label);

  return destination_workspace;
}

VW::model_delta VW::operator-(const VW::workspace& ws1, const VW::workspace& ws2)
{
  validate_compatibility(std::vector<const VW::workspace*>{&ws1, &ws2}, nullptr);
  auto dest_command_line = VW::split_command_line(get_keep_command_line(ws1));
  dest_command_line.emplace_back("--quiet");
  dest_command_line.emplace_back("--preserve_performance_counters");

  auto destination_workspace =
      VW::initialize(VW::make_unique<VW::config::options_cli>(dest_command_line), nullptr, nullptr, nullptr, nullptr);

  auto* target_learner = destination_workspace->l.get();
  while (target_learner != nullptr)
  {
    if (target_learner->has_subtract())
    {
      auto learner_name = target_learner->get_name();
      const VW::LEARNER::learner* ws1_learner = ws1.l->get_learner_by_name_prefix(learner_name);
      const VW::LEARNER::learner* ws2_learner = ws2.l->get_learner_by_name_prefix(learner_name);

      target_learner->subtract(ws1, ws2, ws1_learner, ws2_learner, *destination_workspace, target_learner);
    }
    // If this is a bottom learner and has no merge then emit an error because a bottom with no merge is almost
    // certainly not going to work.
    else if (!target_learner->has_merge() && target_learner->get_base_learner() == nullptr)
    {
      THROW(
          "Bottom learner '" << target_learner->get_name()
                             << "' does not have a merge function defined. Since it is a bottom learner, merging will "
                                "not work as expected.");
    }
    // Skip the save-load case for now

    target_learner = target_learner->get_base_learner();
  }

  // Subtract shared data
  auto& output_sd = *destination_workspace->sd;
  output_sd.sum_loss = ws1.sd->sum_loss - ws2.sd->sum_loss;
  output_sd.weighted_labeled_examples = ws1.sd->weighted_labeled_examples - ws2.sd->weighted_labeled_examples;
  output_sd.weighted_labels = ws1.sd->weighted_labels - ws2.sd->weighted_labels;
  output_sd.weighted_unlabeled_examples = ws1.sd->weighted_unlabeled_examples - ws2.sd->weighted_unlabeled_examples;
  output_sd.example_number = ws1.sd->example_number - ws2.sd->example_number;
  output_sd.total_features = ws1.sd->total_features - ws2.sd->total_features;
  output_sd.t = ws1.sd->t - ws2.sd->t;
  output_sd.max_label = std::max(ws1.sd->max_label, ws2.sd->max_label);
  output_sd.min_label = std::min(ws1.sd->min_label, ws2.sd->min_label);

  return VW::model_delta(std::move(destination_workspace));
}
