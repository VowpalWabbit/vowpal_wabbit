#pragma once

/* Define the last version where files are backward compatible. */
#define LAST_COMPATIBLE_VERSION "6.1.3"
#define VERSION_FILE_WITH_CUBIC "6.1.3"
#define VERSION_FILE_WITH_RANK_IN_HEADER "7.8.0" // varsion since which rank was moved to vw::file_options
#define VERSION_FILE_WITH_INTERACTIONS "7.10.2" // first version that saves interacions among pairs and triples
#define VERSION_FILE_WITH_INTERACTIONS_IN_FO "7.10.3" // since this ver -q, --cubic and --interactions are stored in vw::file_options
#define VERSION_FILE_WITH_HEADER_HASH "8.0.1" // first version with header hash used for validating model content