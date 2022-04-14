// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/debug_print.h"

struct vw_dbg
{
  // clang-format off
  static constexpr bool default_log =         false;
  static constexpr bool learner =             false;
  static constexpr bool gd =                  false;
  static constexpr bool gd_predict =          false;
  static constexpr bool scorer =              false;
  static constexpr bool search =              false;
  static constexpr bool binary =              false;
  static constexpr bool cb_adf =              false;
  static constexpr bool csoaa =               false;
  static constexpr bool csoaa_ldf =           false;
  static constexpr bool cs_active =           false;
  static constexpr bool cats_tree =           false;
  static constexpr bool cats_pdf =            false;
  static constexpr bool cats =                false;
  static constexpr bool cb_explore_pdf =      false;
  static constexpr bool cb_explore =          false;
  static constexpr bool cbify =               false;
  static constexpr bool get_pmf =             false;
  static constexpr bool sample_pdf =          false;
  static constexpr bool cb_explore_get_pmf =  false;
  static constexpr bool cb_sample_pdf =       false;
  static constexpr bool ccb =                 false;
  static constexpr bool cb_sample =           false;
  // clang-format on

  static constexpr bool track_stack = default_log | learner | search | gd | gd_predict | binary | cb_adf | csoaa |
      cs_active | cats_tree | cats_pdf | cats | cb_explore_pdf | cb_explore | cbify | get_pmf | sample_pdf |
      cb_explore_get_pmf | cb_sample_pdf | ccb | scorer | cb_sample | csoaa_ldf;
};

#define VW_DEBUG_LOG vw_dbg::default_log

#define VW_LOG_SINK std::cout

// TODO: Should this be moved to the io library and made to use the logger?
//       we'll need to break the dependency on depth_indent_string() though
#define VW_DBG(e) \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK << ::VW::debug::debug_depth_indent_string(e)

#define VW_DBG_0 \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK
