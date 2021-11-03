// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <cstdint>
#include "example.h"
#include "vw.h"
#include "vw_string_view.h"

void substring_to_example(vw* all, example* ae, VW::string_view example);

namespace VW
{
example& get_unused_example(VW::workspace* all);
void read_line(VW::workspace& all, example* ex, const char* line);  // read example from the line.
void read_lines(VW::workspace* all, const char* line, size_t len,
    v_array<example*>& examples);  // read examples from the new line separated strings.

std::unique_ptr<example_parser_i> make_text_parser(vw& all);
}  // namespace VW

int read_features_string(VW::workspace* all, io_buf& buf, v_array<example*>& examples);
size_t read_features(io_buf& buf, char*& line, size_t& num_chars);
