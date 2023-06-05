// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#ifdef VW_FEAT_SEARCH_ENABLED
#  include "search.h"

namespace MulticlassTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void run(Search::search&, VW::multi_ex&);
extern Search::search_task task;
}  // namespace MulticlassTask
#endif