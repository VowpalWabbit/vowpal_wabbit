// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/future_compat.h"

#include <cstddef>
#include <cstdint>

namespace VW
{
using weight = float;
constexpr size_t NUM_NAMESPACES = 256;
namespace details
{
constexpr int QUADRATIC_CONSTANT = 27942141;
constexpr int CUBIC_CONSTANT = 21791;
constexpr int CUBIC_CONSTANT2 = 37663;
constexpr int AFFIX_CONSTANT = 13903957;
constexpr uint64_t CONSTANT = 11650396;
constexpr float PROBABILITY_TOLERANCE = 1e-5f;

// FNV-like hash constant for 32bit
// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
constexpr uint32_t FNV_PRIME = 16777619;
constexpr unsigned char DEFAULT_NAMESPACE = 32;
constexpr unsigned char WILDCARD_NAMESPACE = 58;  // :
constexpr unsigned char WAP_LDF_NAMESPACE = 126;
constexpr unsigned char HISTORY_NAMESPACE = 127;
constexpr unsigned char CONSTANT_NAMESPACE = 128;
constexpr unsigned char NN_OUTPUT_NAMESPACE = 129;
constexpr unsigned char AUTOLINK_NAMESPACE = 130;
constexpr unsigned char NEIGHBOR_NAMESPACE =
    131;  // this is \x83 -- to do quadratic, say "-q a`printf "\x83"` on the command line
constexpr unsigned char AFFIX_NAMESPACE = 132;                     // this is \x84
constexpr unsigned char SPELLING_NAMESPACE = 133;                  // this is \x85
constexpr unsigned char CONDITIONING_NAMESPACE = 134;              // this is \x86
constexpr unsigned char DICTIONARY_NAMESPACE = 135;                // this is \x87
constexpr unsigned char NODE_ID_NAMESPACE = 136;                   // this is \x88
constexpr unsigned char BASELINE_ENABLED_MESSAGE_NAMESPACE = 137;  // this is \x89
constexpr unsigned char CCB_SLOT_NAMESPACE = 139;
constexpr unsigned char CCB_ID_NAMESPACE = 140;

constexpr const char* CCB_LABEL = "ccb";
constexpr const char* SLATES_LABEL = "slates";
constexpr const char* SHARED_TYPE = "shared";
constexpr const char* ACTION_TYPE = "action";
constexpr const char* SLOT_TYPE = "slot";
constexpr const char* CA_LABEL = "ca";
constexpr const char* PDF = "pdf";
constexpr const char* CHOSEN_ACTION = "chosen_action";
// GRAPH_FEEDBACK_TYPE is an experimental type for an experimental reduction
constexpr const char* GRAPH_FEEDBACK_TYPE = "graph";

static constexpr uint32_t SHARED_EX_INDEX = 0;
static constexpr uint32_t TOP_ACTION_INDEX = 0;
static constexpr const int DEFAULT_FLOAT_PRECISION = 6;
static constexpr const int DEFAULT_FLOAT_FORMATTING_DECIMAL_PRECISION = 2;
static constexpr const int AS_MANY_AS_NEEDED_FLOAT_FORMATTING_DECIMAL_PRECISION = -1;

}  // namespace details
}  // namespace VW

using weight VW_DEPRECATED("weight renamed to VW::weight") = VW::weight;

VW_DEPRECATED("NUM_NAMESPACES renamed to VW::NUM_NAMESPACES")
constexpr size_t NUM_NAMESPACES = 256;
