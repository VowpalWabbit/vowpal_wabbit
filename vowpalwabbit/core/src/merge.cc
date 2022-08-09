// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/merge.h"

#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/core/vw_math.h"
#include "vw/io/io_adapter.h"

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

void validate_compatibility(const VW::workspace* base_workspace,
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger)
{
  if (workspaces_to_merge.size() < 2) { THROW("Must specify at least two model files to merge."); }

  const auto& ref_model = base_workspace != nullptr ? *base_workspace : *workspaces_to_merge[0];
  for (const auto& model : workspaces_to_merge)
  {
    const auto* incompatible = VW::are_features_compatible(ref_model, *model);
    if (incompatible != nullptr)
    { THROW("Model is incompatible with the destination model. Reason: " << incompatible); }
  }

  bool at_least_one_has_no_preserve = false;
  for (const auto* model : workspaces_to_merge)
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

  std::vector<std::string> destination_enabled_reductions;
  ref_model.l->get_enabled_reductions(destination_enabled_reductions);
  for (const auto& model : workspaces_to_merge)
  {
    std::vector<std::string> source_enabled_reductions;
    model->l->get_enabled_reductions(source_enabled_reductions);

    if (source_enabled_reductions != destination_enabled_reductions)
    {
      THROW("Enabled reductions are not identical between models.\n One: " << source_enabled_reductions << "\n Other: "
                                                                           << destination_enabled_reductions);
    }
  }

  auto destination_command_line = get_keep_command_line(ref_model);
  for (const auto& model : workspaces_to_merge)
  {
    auto src_command_line = get_keep_command_line(*model);
    if (destination_command_line != src_command_line)
    {
      THROW("Command lines are not identical between models. One: '" << destination_command_line << "', Other: '"
                                                                     << src_command_line << "'");
    }
  }
}

void merge_shared_data(
    const shared_data& base, shared_data& destination, const std::vector<const shared_data*>& sources)
{
  for (const auto* source : sources)
  {
    destination.sum_loss += (source->sum_loss - base.sum_loss);
    destination.weighted_labeled_examples += (source->weighted_labeled_examples - base.weighted_labeled_examples);
    destination.weighted_labels += (source->weighted_labels - base.weighted_labels);
    destination.weighted_unlabeled_examples += (source->weighted_unlabeled_examples - base.weighted_unlabeled_examples);
    destination.example_number += (source->example_number - base.example_number);
    destination.total_features += (source->total_features - base.total_features);
  }

  destination.sum_loss += base.sum_loss;
  destination.weighted_labeled_examples += base.weighted_labeled_examples;
  destination.weighted_labels += base.weighted_labels;
  destination.weighted_unlabeled_examples += base.weighted_unlabeled_examples;
  destination.example_number += base.example_number;
  destination.total_features += base.total_features;
}
}  // namespace

std::unique_ptr<VW::workspace> get_beginning_destination_workspace(
    const VW::workspace* optional_base_workspace, const std::vector<std::string>& command_line, VW::io::logger* logger)
{
  if (optional_base_workspace == nullptr)
  {
    return VW::initialize_experimental(
        VW::make_unique<VW::config::options_cli>(command_line), nullptr, nullptr, nullptr, logger);
  }

  // If we were given a base workspace, we need to make a copy of it.
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf temp_buffer;
  temp_buffer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*const_cast<VW::workspace*>(optional_base_workspace), temp_buffer);
  return VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(command_line),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()), nullptr, nullptr, logger);
}

std::vector<float> calc_per_model_weighting(const std::vector<float>& example_counts)
{
  const auto sum = std::accumulate(example_counts.begin(), example_counts.end(), 0.f);
  std::vector<float> per_model_weighting(example_counts.size(), 0.f);
  for (size_t i = 0; i < example_counts.size(); i++) { per_model_weighting[i] = example_counts[i] / sum; }
  return per_model_weighting;
}

namespace VW
{
// Experimental.
std::unique_ptr<VW::workspace> merge_models(const VW::workspace* base_workspace,
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger)
{
  validate_compatibility(base_workspace, workspaces_to_merge, logger);

  auto dest_command_line = VW::split_command_line(get_keep_command_line(*workspaces_to_merge[0]));
  if (logger == nullptr) { dest_command_line.emplace_back("--quiet"); }
  else
  {
    dest_command_line.emplace_back("--driver_output_off");
  }
  dest_command_line.emplace_back("--preserve_performance_counters");

  auto base_workspace_concrete = get_beginning_destination_workspace(base_workspace, dest_command_line, nullptr);
  auto dest_workspace = get_beginning_destination_workspace(base_workspace, dest_command_line, logger);

  auto destination_workspace = VW::initialize_experimental(
      VW::make_unique<VW::config::options_cli>(dest_command_line), nullptr, nullptr, nullptr, logger);

  float base_example_count = base_workspace_concrete->sd->weighted_labeled_examples;
  if (VW::math::are_same(base_example_count, 0.f) && base_workspace != nullptr && logger != nullptr)
  {
    logger->warn(
        "Base model has no examples, ensure it was loaded in using --preserve_performance_counters. If this is an "
        "empty model, then the base model doesnt need to be passed.");
  }

  std::vector<float> example_counts;
  example_counts.reserve(workspaces_to_merge.size());
  for (const auto& model : workspaces_to_merge) { example_counts.push_back(model->sd->weighted_labeled_examples); }
  const auto per_model_weighting = calc_per_model_weighting(example_counts);

  auto* target_learner = destination_workspace->l;
  while (target_learner != nullptr)
  {
    if (target_learner->has_merge())
    {
      std::vector<const LEARNER::base_learner*> source_learners;
      for (const auto& model : workspaces_to_merge)
      {
        auto* source_data = model->l->get_learner_by_name_prefix(target_learner->get_name());
        source_learners.push_back(source_data);
      }

      auto* base_workspace_learner = base_workspace_concrete->l->get_learner_by_name_prefix(target_learner->get_name());

      target_learner->merge(per_model_weighting, *base_workspace_concrete, workspaces_to_merge, base_workspace_learner,
          source_learners, *destination_workspace, target_learner);
    }
    // If this is a base reduction and has no merge then emit an error because a base with no merge is almost certainly
    // not going to work.
    else if (!target_learner->has_merge() && target_learner->get_learn_base() == nullptr)
    {
      THROW("Base learner '" << target_learner->get_name()
                             << "' does not have a merge function defined. Since it is a base learner, merging will "
                                "not work as expected.");
    }
    else if (!target_learner->has_merge() && target_learner->learner_defines_own_save_load())
    {
      if (logger != nullptr)
      {
        logger->warn(
            "Reduction '{}' supports save/load but does not have a merge function defined. Merging will still run but "
            "this reduction will not be merged and may result in incorrect results.",
            target_learner->get_name());
      }
    }
    target_learner = target_learner->get_learn_base();
  }

  std::vector<const shared_data*> shared_datas;
  shared_datas.reserve(workspaces_to_merge.size());
  for (const auto& model : workspaces_to_merge) { shared_datas.push_back(model->sd); }

  // Merge shared data too
  merge_shared_data(*base_workspace_concrete->sd, *destination_workspace->sd, shared_datas);

  return destination_workspace;
}

}  // namespace VW
