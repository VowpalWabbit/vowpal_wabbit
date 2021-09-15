// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "version.h"

#define EMPTY_VERSION_FILE "0.0.0"  // empty version, default
/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "7.6.0"
#define VERSION_FILE_WITH_CUBIC "6.1.3"
#define VERSION_FILE_WITH_RANK_IN_HEADER "7.8.0"  // version since which rank was moved to vw::file_options
#define VERSION_FILE_WITH_INTERACTIONS "7.10.2"   // first version that saves interactions among pairs and triples
#define VERSION_FILE_WITH_INTERACTIONS_IN_FO \
  "7.10.3"  // since this ver -q, --cubic and --interactions are stored in vw::file_options
#define VERSION_FILE_WITH_HEADER_HASH "8.0.1"  // first version with header hash used for validating model content
#define VERSION_FILE_WITH_HEADER_CHAINED_HASH \
  "8.0.2"  // first version with header's chained hash used for more reliably validating model content
#define VERSION_FILE_WITH_HEADER_ID "8.0.3"    // first version with user supplied header
#define VERSION_FILE_WITH_CB_ADF_SAVE "8.3.2"  // first version with user supplied header
#define VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG \
  "8.9.0"  // CCB optimization for models with only single slot used introduced in this version.
#define VERSION_FILE_WITH_CB_TO_CBADF "8.11.0"            // first version that maps --cb to use cb_adf
#define VERSION_FILE_WITH_REG_CB_SAVE_RESUME "8.11.0"     // version that ensures accuracy of loaded models in regcb
#define VERSION_FILE_WITH_SQUARE_CB_SAVE_RESUME "8.11.0"  // version that ensures accuracy of loaded models in squarecb
#define VERSION_FILE_WITH_FIRST_SAVE_RESUME "8.11.0"      // version that ensures accuracy of loaded models in first

namespace VW
{
namespace version_definitions
{
constexpr VW::version_struct VERSION_FILE_WITH_ACTIVE_SEEN_LABELS{9, 0, 0};
}
}  // namespace VW