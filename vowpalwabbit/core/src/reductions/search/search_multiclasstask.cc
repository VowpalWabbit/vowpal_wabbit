// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/search/search_multiclasstask.h"

#include "vw/core/crossplat_compat.h"

namespace MulticlassTask
{
Search::search_task task = {"multiclasstask", run, initialize, nullptr, nullptr, nullptr};
}

namespace MulticlassTask
{
class task_data
{
public:
  size_t max_label;
  size_t num_level;
  VW::v_array<uint32_t> y_allowed;
};

void initialize(Search::search& sch, size_t& num_actions, VW::config::options_i& /*vm*/)
{
  task_data* my_task_data = new task_data();
  sch.set_options(0);
  size_t num_learner_ceil = 1;
  while (num_learner_ceil < num_actions) { num_learner_ceil *= 2; }
  // TODO: temporary fix to ensure enough learners, look into case where learner is created outside bounds
  sch.set_feature_width(num_learner_ceil);
  my_task_data->max_label = num_actions;
  my_task_data->num_level = static_cast<size_t>(ceil(log(num_actions) / log(2)));
  my_task_data->y_allowed.push_back(1);
  my_task_data->y_allowed.push_back(2);
  sch.set_task_data(my_task_data);
}

void run(Search::search& sch, VW::multi_ex& ec)
{
  task_data* my_task_data = sch.get_task_data<task_data>();
  size_t gold_label = ec[0]->l.multi.label;
  size_t label = 0;
  size_t learner_id = 0;

  for (size_t i = 0; i < my_task_data->num_level; i++)
  {
    size_t mask = VW::details::UINT64_ONE << (my_task_data->num_level - i - 1);
    size_t y_allowed_size = (label + mask + 1 <= my_task_data->max_label) ? 2 : 1;
    action oracle = (((gold_label - 1) & mask) > 0) + 1;
    size_t prediction = sch.predict(*ec[0], 0, &oracle, 1, nullptr, nullptr, my_task_data->y_allowed.begin(),
        y_allowed_size, nullptr, learner_id);  // TODO: do we really need y_allowed?
    learner_id = (learner_id << 1) + prediction;
    if (prediction == 2) { label += mask; }
  }
  label += 1;
  sch.loss(!(label == gold_label));
  if (sch.output().good()) { sch.output() << label << ' '; }
}
}  // namespace MulticlassTask
