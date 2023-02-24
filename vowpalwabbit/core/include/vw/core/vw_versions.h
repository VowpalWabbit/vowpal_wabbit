// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/version.h"

namespace VW
{
namespace version_definitions
{
/// empty version, default
constexpr VW::version_struct EMPTY_VERSION_FILE{0, 0, 0};

/// The last version where files are backward compatible
constexpr VW::version_struct LAST_COMPATIBLE_VERSION{7, 6, 0};
constexpr VW::version_struct VERSION_FILE_WITH_CUBIC{6, 1, 3};

/// version since which rank was moved to vw::file_options
constexpr VW::version_struct VERSION_FILE_WITH_RANK_IN_HEADER{7, 8, 0};

/// first version that saves interactions among pairs and triples
constexpr VW::version_struct VERSION_FILE_WITH_INTERACTIONS{7, 10, 2};

/// since this ver -q, --cubic and --interactions are stored in vw::file_options
constexpr VW::version_struct VERSION_FILE_WITH_INTERACTIONS_IN_FO{7, 10, 3};

/// first version with header hash used for validating model content
constexpr VW::version_struct VERSION_FILE_WITH_HEADER_HASH{8, 0, 1};

/// first version with header's chained hash used for more reliably validating model content
constexpr VW::version_struct VERSION_FILE_WITH_HEADER_CHAINED_HASH{8, 0, 2};

/// first version with user supplied header
constexpr VW::version_struct VERSION_FILE_WITH_HEADER_ID{8, 0, 3};

/// first version with user supplied header
constexpr VW::version_struct VERSION_FILE_WITH_CB_ADF_SAVE{8, 3, 2};

/// CCB optimization for models with only single slot used introduced in this version,
constexpr VW::version_struct VERSION_FILE_WITH_CCB_MULTI_SLOTS_SEEN_FLAG{8, 9, 0};

/// first version that maps --cb to use cb_adf
constexpr VW::version_struct VERSION_FILE_WITH_CB_TO_CBADF{8, 11, 0};

/// version that ensures accuracy of loaded models in regcb
constexpr VW::version_struct VERSION_FILE_WITH_REG_CB_SAVE_RESUME{8, 11, 0};

/// version that ensures accuracy of loaded models in squarecb
constexpr VW::version_struct VERSION_FILE_WITH_SQUARE_CB_SAVE_RESUME{8, 11, 0};

/// version that ensures accuracy of loaded models in first
constexpr VW::version_struct VERSION_FILE_WITH_FIRST_SAVE_RESUME{8, 11, 0};

constexpr VW::version_struct VERSION_SAVE_RESUME_FIX{7, 10, 1};

constexpr VW::version_struct VERSION_PASS_UINT64{8, 3, 3};

/// Added serialized seen min and max labels in the --active reduction
constexpr VW::version_struct VERSION_FILE_WITH_ACTIVE_SEEN_LABELS{9, 0, 0};

/// Moved option values from command line to model data
constexpr VW::version_struct VERSION_FILE_WITH_L1_AND_L2_STATE_IN_MODEL_DATA{9, 0, 0};

/// Moved option values from command line to model data
constexpr VW::version_struct VERSION_FILE_WITH_FLAT_EXAMPLE_TAG_FIX{9, 6, 0};

/// PLT had an incorrect save_load impl which relied on the adaptive value prior to this version.
constexpr VW::version_struct VERSION_FILE_WITH_PLT_SAVE_LOAD_FIX{9, 7, 0};

}  // namespace version_definitions
}  // namespace VW
