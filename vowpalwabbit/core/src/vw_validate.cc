// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/vw_validate.h"

#include "vw/core/global_data.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw_versions.h"
#include "vw/io/logger.h"

namespace VW
{
void validate_version(VW::workspace& all)
{
  if (all.model_file_ver < VW::version_definitions::LAST_COMPATIBLE_VERSION)
    THROW("Model has possibly incompatible version! " << all.model_file_ver.to_string());
  if (all.model_file_ver > VW::VERSION)
  {
    all.logger.err_warn("Model version is more recent than VW version. This may not work.");
  }
}

void validate_min_max_label(VW::workspace& all)
{
  if (all.sd->max_label < all.sd->min_label) THROW("Max label cannot be less than min label.");
}

void validate_default_bits(VW::workspace& all, uint32_t local_num_bits)
{
  if (all.default_bits != true && all.num_bits != local_num_bits)
    THROW("-b bits mismatch: command-line " << all.num_bits << " != " << local_num_bits << " stored in model");
}

void validate_num_bits(VW::workspace& all)
{
  if (all.num_bits > sizeof(size_t) * 8 - 3)
    THROW("Only " << sizeof(size_t) * 8 - 3 << " or fewer bits allowed.  If this is a serious limit, speak up.");
}
}  // namespace VW
