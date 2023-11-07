// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"
#include "vw/text_parser/parse_example_text.h"

VW_DEPRECATED("substring_to_example moved to VW::parsers::text::details::substring_to_example")
inline void substring_to_example(VW::workspace* all, VW::example* ae, VW::string_view example)
{
  VW::parsers::text::details::substring_to_example(all, ae, example);
}

namespace VW
{
VW_DEPRECATED("read_line moved to VW::parsers::text::read_line")
inline void read_line(VW::workspace& all, example* ex, VW::string_view line)
{
  VW::parsers::text::read_line(all, ex, line);
}

VW_DEPRECATED("read_line moved to VW::parsers::text::read_line and should be used with a string_view")
inline void read_line(VW::workspace& all, example* ex, const char* line)
{
  VW::parsers::text::read_line(all, ex, VW::string_view(line));
}

VW_DEPRECATED("read_lines moved to VW::parsers::text::read_lines and should be used with a string_view")
inline void read_lines(VW::workspace* all, const char* line, size_t len, VW::multi_ex& examples)
{
  VW::parsers::text::read_lines(all, VW::string_view(line, len), examples);
}

VW_DEPRECATED("read_lines moved to VW::parsers::text::read_lines")
inline void read_lines(VW::workspace* all, VW::string_view lines, VW::multi_ex& examples)
{
  VW::parsers::text::read_lines(all, lines, examples);
}

}  // namespace VW

VW_DEPRECATED("read_features_string moved to VW::parsers::text::read_features_string")
inline int read_features_string(VW::workspace* all, VW::io_buf& buf, VW::multi_ex& examples)
{
  return VW::parsers::text::read_features_string(all, buf, examples);
}

VW_DEPRECATED("read_features moved to VW::parsers::text::details::read_features")
inline size_t read_features(VW::io_buf& buf, char*& line, size_t& num_chars)
{
  return VW::parsers::text::details::read_features(buf, line, num_chars);
}
