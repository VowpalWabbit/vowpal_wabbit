// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

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
