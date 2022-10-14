// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/version.h"

#include "vw/common/future_compat.h"

#include <fmt/format.h>

#include <cstdio>

namespace VW
{
version_struct::version_struct(const char* str)
{
#ifdef _WIN32
  sscanf_s(str, "%d.%d.%d", &major, &minor, &rev);
#else
  std::sscanf(str, "%d.%d.%d", &major, &minor, &rev);
#endif
}

std::string version_struct::to_string() const { return fmt::format("{}.{}.{}", major, minor, rev); }

version_struct version_struct::from_string(const char* str) { return version_struct{str}; }

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_DEPRECATED_USAGE
const std::string git_commit(COMMIT_VERSION);
VW_WARNING_STATE_POP

const std::string GIT_COMMIT(COMMIT_VERSION);
}  // namespace VW
