// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"
#include "vw/io/logger.h"

namespace VW
{
/**
* Merge several workspace objects into a single workspace. This merges weights
* and all training state. All given workspaces must be compatible, meaning they
* should have the same reduction stack and same training based options.
*
* Note: This is an experimental API.
*
* @param workspaces_to_merge List of workspaces to merge.
* @param logger Optional logger to be used for logging during function and is given to the resulting workspace
* @return std::unique_ptr<VW::workspace> Pointer to the resulting workspace.
*/
std::unique_ptr<VW::workspace> merge_models(
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger = nullptr);
}