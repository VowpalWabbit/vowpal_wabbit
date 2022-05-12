// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "vw/common/string_view.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>

void substring_to_example(VW::workspace* all, VW::example* ae, VW::string_view example);

namespace VW
{
example& get_unused_example(VW::workspace* all);
void read_line(VW::workspace& all, example* ex, const char* line);  // read example from the line.
void read_lines(VW::workspace* all, const char* line, size_t len,
    VW::multi_ex& examples);  // read examples from the new line separated strings.

}  // namespace VW

int read_features_string(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);
size_t read_features(io_buf& buf, char*& line, size_t& num_chars);
