// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "search.h"

namespace MulticlassTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void finish(Search::search&);
void run(Search::search&, multi_ex&);
extern Search::search_task task;
}  // namespace MulticlassTask
