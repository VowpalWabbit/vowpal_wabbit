// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/debug_print.h"

class vw_dbg
{
public:
  // clang-format off
  static constexpr bool DEFAULT_LOG =         false;
  static constexpr bool LEARNER =             false;
  static constexpr bool GD =                  false;
  static constexpr bool GD_PREDICT =          false;
  static constexpr bool SCORER =              false;
  static constexpr bool SEARCH =              false;
  static constexpr bool BINARY =              false;
  static constexpr bool CB_ADF =              false;
  static constexpr bool CSOAA =               false;
  static constexpr bool CSOAA_LDF =           false;
  static constexpr bool CS_ACTIVE =           false;
  static constexpr bool CATS_TREE =           false;
  static constexpr bool CATS_PDF =            false;
  static constexpr bool CATS =                false;
  static constexpr bool CB_EXPLORE_PDF =      false;
  static constexpr bool CB_EXPLORE =          false;
  static constexpr bool CBIFY =               false;
  static constexpr bool GET_PMF =             false;
  static constexpr bool SAMPLE_PDF =          false;
  static constexpr bool CB_EXPLORE_GET_PMF =  false;
  static constexpr bool CB_SAMPLE_PDF =       false;
  static constexpr bool CCB =                 false;
  static constexpr bool CB_SAMPLE =           false;
  // clang-format on

  static constexpr bool TRACK_STACK = DEFAULT_LOG | LEARNER | SEARCH | GD | GD_PREDICT | BINARY | CB_ADF | CSOAA |
      CS_ACTIVE | CATS_TREE | CATS_PDF | CATS | CB_EXPLORE_PDF | CB_EXPLORE | CBIFY | GET_PMF | SAMPLE_PDF |
      CB_EXPLORE_GET_PMF | CB_SAMPLE_PDF | CCB | SCORER | CB_SAMPLE | CSOAA_LDF;
};

#define VW_DEBUG_LOG vw_dbg::DEFAULT_LOG

#define VW_LOG_SINK std::cout

// TODO: Should this be moved to the io library and made to use the logger?
//       we'll need to break the dependency on depth_indent_string() though
#define VW_DBG(e) \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK << ::VW::debug::debug_depth_indent_string(e)

#define VW_DBG_0 \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK
