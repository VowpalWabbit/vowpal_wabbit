// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "action_score.h"

#include "global_data.h"
#include "vw/io/logger.h"
#include "io_buf.h"
#include "text_utils.h"
#include "v_array.h"
#include "vw/common/string_view.h"

namespace ACTION_SCORE
{
void print_action_score(
    VW::io::writer* f, const VW::v_array<action_score>& a_s, const VW::v_array<char>& tag, VW::io::logger& logger)
{
  if (f == nullptr) { return; }

  std::stringstream ss;
  ss << VW::to_string(a_s);
  if (!tag.empty()) { ss << " " << VW::string_view(tag.begin(), tag.size()); }
  ss << '\n';
  const auto ss_str = ss.str();
  ssize_t len = ss_str.size();
  ssize_t t = f->write(ss_str.c_str(), static_cast<unsigned int>(len));
  if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
}

std::ostream& operator<<(std::ostream& os, const action_score& a_s)
{
  os << "(" << a_s.action << "," << a_s.score << ")";
  return os;
}
}  // namespace ACTION_SCORE

namespace VW
{
std::string to_string(const ACTION_SCORE::action_scores& action_scores_or_probs, int decimal_precision)
{
  std::ostringstream ss;
  std::string delim;
  for (const auto& item : action_scores_or_probs)
  {
    ss << delim << fmt::format("{}:{}", item.action, VW::fmt_float(item.score, decimal_precision));
    delim = ",";
  }
  return ss.str();
}

}  // namespace VW
