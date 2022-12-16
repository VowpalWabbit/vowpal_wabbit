// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"
#include "vw/text_parser/parse_example_text.h"

VW_DEPRECATED("substring_to_example moved to VW::parsers::text::substring_to_example")
void substring_to_example(VW::workspace* all, VW::example* ae, VW::string_view example)
{
  VW::parsers::text::substring_to_example(all, ae, example);
}

namespace VW
{
VW_DEPRECATED("read_line moved to VW::parsers::text::read_line")
void read_line(VW::workspace& all, example* ex, VW::string_view line) { VW::parsers::text::read_line(all, ex, line); }

VW_DEPRECATED("read_line moved to VW::parsers::text::read_line")
void read_line(VW::workspace& all, example* ex, const char* line) { VW::parsers::text::read_line(all, ex, line); }

VW_DEPRECATED("read_lines moved to VW::parsers::text::read_lines")
void read_lines(VW::workspace* all, const char* line, size_t len, VW::multi_ex& examples)
{
  VW::parsers::text::read_lines(all, line, len, examples);
}
}  // namespace VW

VW_DEPRECATED("read_features_string moved to VW::parsers::text::read_features_string")
int read_features_string(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  return VW::parsers::text::read_features_string(all, buf, examples);
}

VW_DEPRECATED("read_features moved to VW::parsers::text::read_features")
size_t read_features(io_buf& buf, char*& line, size_t& num_chars)
{
  return VW::parsers::text::read_features(buf, line, num_chars);
}
