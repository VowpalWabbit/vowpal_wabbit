// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "search.h"

namespace SequenceTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void run(Search::search&, multi_ex&);
extern Search::search_task task;
}  // namespace SequenceTask

namespace SequenceSpanTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void finish(Search::search&);
void run(Search::search&, multi_ex&);
void setup(Search::search&, multi_ex&);
void takedown(Search::search&, multi_ex&);
extern Search::search_task task;
}  // namespace SequenceSpanTask

namespace SequenceTaskCostToGo
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void run(Search::search&, multi_ex&);
extern Search::search_task task;
}  // namespace SequenceTaskCostToGo

namespace ArgmaxTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void run(Search::search&, multi_ex&);
void finish(Search::search&);
extern Search::search_task task;
}  // namespace ArgmaxTask

namespace SequenceTask_DemoLDF
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void finish(Search::search&);
void run(Search::search&, multi_ex&);
extern Search::search_task task;
}  // namespace SequenceTask_DemoLDF
