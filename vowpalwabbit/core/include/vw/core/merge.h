// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"
#include "vw/io/logger.h"

namespace VW
{
std::unique_ptr<VW::workspace> merge_models(const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger = nullptr);
}