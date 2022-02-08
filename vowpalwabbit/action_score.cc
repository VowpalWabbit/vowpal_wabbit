// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "action_score.h"

#include "v_array.h"
#include "io_buf.h"
#include "global_data.h"

#include "io/logger.h"
#include "vw_string_view.h"

namespace ACTION_SCORE
{
void print_action_score(std::ostream& output, const v_array<action_score>& a_s, const v_array<char>& tag)
{
  for (size_t i = 0; i < a_s.size(); i++)
  {
    if (i > 0) output << ',';
    output << a_s[i].action << ':' << a_s[i].score;
  }
  if (!tag.empty()) { output << " " << tag; }
  output << '\n';
}

std::ostream& operator<<(std::ostream& os, const action_score& a_s)
{
  os << "(" << a_s.action << "," << a_s.score << ")";
  return os;
}
}  // namespace ACTION_SCORE
