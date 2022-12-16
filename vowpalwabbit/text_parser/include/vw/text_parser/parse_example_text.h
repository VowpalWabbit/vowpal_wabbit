// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/string_view.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>

namespace VW
{
namespace parsers
{
namespace text
{
namespace details
{
void substring_to_example(VW::workspace* all, VW::example* ae, VW::string_view example);
size_t read_features(io_buf& buf, char*& line, size_t& num_chars);
}  // namespace details

void read_line(VW::workspace& all, example* ex, VW::string_view line);  // read example from the line.
void read_lines(VW::workspace* all, VW::string_view lines_view,
    VW::multi_ex& examples);  // read examples from the new line separated strings.

int read_features_string(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);
}  // namespace text
}  // namespace parsers
}  // namespace VW
