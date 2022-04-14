// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/topk.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <queue>
#include <sstream>
#include <utility>

using namespace VW::config;

namespace
{
class topk
{
  using container_t = std::multimap<float, v_array<char>>;

public:
  using const_iterator_t = container_t::const_iterator;
  topk(uint32_t k_num);

  void predict(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq);
  void learn(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq);
  std::pair<const_iterator_t, const_iterator_t> get_container_view();
  void clear_container();

private:
  void update_priority_queue(float pred, v_array<char>& tag);

  const uint32_t _k_num;
  container_t _pr_queue;
};

topk::topk(uint32_t k_num) : _k_num(k_num) {}

void topk::predict(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  for (auto ec : ec_seq)
  {
    base.predict(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void topk::learn(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  for (auto ec : ec_seq)
  {
    base.learn(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void topk::update_priority_queue(float pred, v_array<char>& tag)
{
  if (_pr_queue.size() < _k_num) { _pr_queue.insert({pred, tag}); }
  else if (_pr_queue.begin()->first < pred)
  {
    _pr_queue.erase(_pr_queue.begin());
    _pr_queue.insert({pred, tag});
  }
}

std::pair<topk::const_iterator_t, topk::const_iterator_t> topk::get_container_view()
{
  return {_pr_queue.cbegin(), _pr_queue.cend()};
}

void topk::clear_container() { _pr_queue.clear(); }

void print_result(VW::io::writer* file_descriptor,
    std::pair<topk::const_iterator_t, topk::const_iterator_t> const& view, VW::io::logger& logger)
{
  if (file_descriptor != nullptr)
  {
    std::stringstream ss;
    for (auto it = view.first; it != view.second; it++)
    {
      ss << std::fixed << it->first << " ";
      if (!it->second.empty()) { ss << " " << VW::string_view{it->second.begin(), it->second.size()}; }
      ss << " \n";
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    auto t = file_descriptor->write(ss.str().c_str(), len);
    if (t != len) { logger.err_error("write error: {}", VW::strerror_to_string(errno)); }
  }
}

void output_example(VW::workspace& all, const VW::example& ec)
{
  const label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.get_num_features());
  if (ld.label != FLT_MAX) { all.sd->weighted_labels += (static_cast<double>(ld.label)) * ec.weight; }

  print_update(all, ec);
}

template <bool is_learn>
void predict_or_learn(topk& d, VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  if (is_learn) { d.learn(base, ec_seq); }
  else
  {
    d.predict(base, ec_seq);
  }
}

void finish_example(VW::workspace& all, topk& d, VW::multi_ex& ec_seq)
{
  for (auto ec : ec_seq) { output_example(all, *ec); }
  for (auto& sink : all.final_prediction_sink) { print_result(sink.get(), d.get_container_view(), all.logger); }
  d.clear_container();
  VW::finish_example(all, ec_seq);
}

}  // namespace

VW::LEARNER::base_learner* VW::reductions::topk_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  uint32_t K;
  option_group_definition new_options("[Reduction] Top K");
  new_options.add(make_option("top", K).keep().necessary().help("Top k recommendation"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto data = VW::make_unique<topk>(K);
  auto* l = VW::LEARNER::make_reduction_learner(std::move(data), as_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(topk_setup))
                .set_learn_returns_prediction(true)
                .set_output_prediction_type(VW::prediction_type_t::scalar)
                .set_finish_example(::finish_example)
                .build();
  return make_base(*l);
}
