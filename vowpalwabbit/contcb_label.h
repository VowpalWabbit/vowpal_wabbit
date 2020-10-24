// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "label_parser.h"

namespace VW
{
namespace continuous_cb
{
struct label
{
  float action;
  float cost;
};

extern label_parser contcb_label;

}  // namespace continuous_cb
}  // namespace VW
