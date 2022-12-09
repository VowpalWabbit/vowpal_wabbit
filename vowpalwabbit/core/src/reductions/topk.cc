// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/topk.h"

#include "vw/config/options.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/errno_handling.h"
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
public:
  using container_t = std::multimap<float, VW::v_array<char>>;
  using const_iterator_t = container_t::const_iterator;
  topk(uint32_t k_num);

  void predict(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq);
  void learn(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq);
  std::pair<const_iterator_t, const_iterator_t> get_container_view() const;

private:
  void clear_container();
  void update_priority_queue(float pred, const VW::v_array<char>& tag);

  const uint32_t _k_num;
  container_t _pr_queue;
};

topk::topk(uint32_t k_num) : _k_num(k_num) {}

void topk::predict(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  clear_container();
  for (auto* ec : ec_seq)
  {
    base.predict(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void topk::learn(VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  clear_container();
  for (auto* ec : ec_seq)
  {
    base.learn(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void topk::update_priority_queue(float pred, const VW::v_array<char>& tag)
{
  if (_pr_queue.size() < _k_num) { _pr_queue.insert({pred, tag}); }
  else if (_pr_queue.begin()->first < pred)
  {
    _pr_queue.erase(_pr_queue.begin());
    _pr_queue.insert({pred, tag});
  }
}

std::pair<topk::const_iterator_t, topk::const_iterator_t> topk::get_container_view() const
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
    if (t != len) { logger.err_error("write error: {}", VW::io::strerror_to_string(errno)); }
  }
}

void update_stats_topk(const VW::workspace& /* all */, shared_data& sd, const topk& /* data */,
    const VW::multi_ex& ec_seq, VW::io::logger& /* logger */)
{
  for (auto* ec : ec_seq)
  {
    const auto& ld = ec->l.simple;
    sd.update(ec->test_only, ld.label != FLT_MAX, ec->loss, ec->weight, ec->get_num_features());
    if (ld.label != FLT_MAX) { sd.weighted_labels += (static_cast<double>(ld.label)) * ec->weight; }
  }
}

void output_example_prediction_topk(
    VW::workspace& all, const topk& data, const VW::multi_ex& /* ec_seq */, VW::io::logger& logger)
{
  for (auto& sink : all.final_prediction_sink) { print_result(sink.get(), data.get_container_view(), logger); }
}

void print_update_topk(VW::workspace& all, shared_data& sd, const topk& /* data */, const VW::multi_ex& ec_seq,
    VW::io::logger& /* unused */)
{
  for (auto* ec : ec_seq) { VW::details::print_update_simple_label(all, sd, *ec, all.logger); }
}

template <bool is_learn>
void predict_or_learn(topk& d, VW::LEARNER::single_learner& base, VW::multi_ex& ec_seq)
{
  if (is_learn) { d.learn(base, ec_seq); }
  else { d.predict(base, ec_seq); }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::topk_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  uint32_t k{};
  option_group_definition new_options("[Reduction] Top K");
  new_options.add(make_option("top", k).keep().necessary().help("Top k recommendation"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto data = VW::make_unique<topk>(k);
  auto* l = VW::LEARNER::make_reduction_learner(std::move(data), as_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true>, predict_or_learn<false>, stack_builder.get_setupfn_name(topk_setup))
                .set_learn_returns_prediction(true)
                .set_input_prediction_type(VW::prediction_type_t::SCALAR)
                .set_output_prediction_type(VW::prediction_type_t::SCALAR)
                .set_output_example_prediction(output_example_prediction_topk)
                .set_print_update(print_update_topk)
                .set_update_stats(update_stats_topk)
                .build();
  return make_base(*l);
}
