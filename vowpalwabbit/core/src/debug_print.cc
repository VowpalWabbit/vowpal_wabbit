// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/debug_print.h"

#include "vw/core/example.h"

std::string VW::debug::cb_label_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[l.cb={";
  const auto& costs = ec.l.cb.costs;
  for (const auto& cost : costs)
  {
    strstream << "{c=" << cost.cost << ",a=" << cost.action << ",p=" << cost.probability
              << ",pp=" << cost.partial_prediction << "}";
  }
  strstream << "}]";
  return strstream.str();
}

std::string VW::debug::simple_label_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[l=" << ec.l.simple.label << ",w=" << ec.weight << "]";
  return strstream.str();
}

std::string VW::debug::scalar_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "[p=" << ec.pred.scalar << ", pp=" << ec.partial_prediction << "]";
  return strstream.str();
}

std::string VW::debug::a_s_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.a_s[" << VW::to_string(ec.pred.a_s) << "]";
  return strstream.str();
}

std::string VW::debug::multiclass_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.multiclass = " << ec.pred.multiclass;
  return strstream.str();
}

std::string VW::debug::prob_dist_pred_to_string(const example& ec)
{
  std::stringstream strstream;
  strstream << "ec.pred.prob_dist[" << VW::to_string(ec.pred.pdf) << "]";
  return strstream.str();
}

std::string VW::debug::features_to_string(const example_predict& ec)
{
  std::stringstream strstream;
  strstream << "[off=" << ec.ft_offset << "]";
  for (auto& f : ec.feature_space)
  {
    auto ind_iter = f.indices.cbegin();
    auto val_iter = f.values.cbegin();
    for (; ind_iter != f.indices.cend(); ++ind_iter, ++val_iter)
    {
      strstream << "[h=" << *ind_iter << ","
                << "v=" << *val_iter << "]";
    }
  }
  return strstream.str();
}

std::string VW::debug::debug_depth_indent_string(const example& ec)
{
  return debug_depth_indent_string(ec.debug_current_reduction_depth);
}

std::string VW::debug::debug_depth_indent_string(const multi_ex& ec)
{
  return VW::debug::debug_depth_indent_string(*ec[0]);
}

std::string VW::debug::debug_depth_indent_string(const int32_t depth)
{
  constexpr const char* indent_str = "- ";
  constexpr const char* space_str = "  ";

  if (depth == 0) { return indent_str; }

  std::stringstream str_stream;
  for (int32_t i = 0; i < depth - 1; i++) { str_stream << space_str; }
  str_stream << indent_str;
  return str_stream.str();
}