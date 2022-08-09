// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"
#include "vw/io/logger.h"

namespace VW
{
/**
 * Merge the differences of several workspaces into the given base workspace. This merges weights
 * and all training state. All given workspaces must be compatible with the base workspace, meaning they
 * should have the same reduction stack and same training based options.
 *
 * If the base workspace was not given, it is assumed that the given workspaces were trained from fresh.
 *
 * Note: This is an experimental API.
 *
 * @param base_workspace Optional common base model that all other models continued training from. If not supplied, then
 * all models are assumed to be trained from scratch.
 * @param workspaces_to_merge List of workspaces to merge.
 * @param logger Optional logger to be used for logging during function and is given to the resulting workspace
 * @return std::unique_ptr<VW::workspace> Pointer to the resulting workspace.
 */
std::unique_ptr<VW::workspace> merge_models(const VW::workspace* base_workspace,
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger = nullptr);
}  // namespace VW