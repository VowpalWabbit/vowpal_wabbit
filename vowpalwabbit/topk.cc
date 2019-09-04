/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <sstream>

#include "topk.h"
#include "vw.h"

using namespace VW::config;

VW::topk::topk(uint32_t K) : m_K(K) {}

void VW::topk::predict(LEARNER::single_learner& base, multi_ex& ec_seq)
{
  for (auto ec : ec_seq)
  {
    base.predict(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void VW::topk::learn(LEARNER::single_learner& base, multi_ex& ec_seq)
{
  for (auto ec : ec_seq)
  {
    base.learn(*ec);
    update_priority_queue(ec->pred.scalar, ec->tag);
  }
}

void VW::topk::update_priority_queue(float pred, v_array<char>& tag)
{
  if (m_pr_queue.size() < m_K)
    m_pr_queue.push(std::make_pair(pred, tag));
  else if (m_pr_queue.top().first < pred)
  {
    m_pr_queue.pop();
    m_pr_queue.push(std::make_pair(pred, tag));
  }
}

std::vector<VW::topk::scored_example> VW::topk::drain_queue()
{
  std::vector<scored_example> result;
  result.reserve(m_pr_queue.size());
  while (!m_pr_queue.empty())
  {
    result.push_back(m_pr_queue.top());
    m_pr_queue.pop();
  }
  return result;
}

void print_result(int file_descriptor, std::vector<VW::topk::scored_example> const& items)
{
  if (file_descriptor >= 0)
  {
    std::stringstream ss;
    for (auto& item : items)
    {
      tmp_example = m_pr_queue.top();
      m_pr_queue.pop();
      ss << std::fixed << tmp_example.first << " ";
      print_tag(ss, tmp_example.second);
      ss << " \n";
    }
    ss << '\n';
    ssize_t len = ss.str().size();
#ifdef _WIN32
    ssize_t t = _write(file_descriptor, ss.str().c_str(), (unsigned int)len);
#else
    ssize_t t = write(file_descriptor, ss.str().c_str(), (unsigned int)len);
#endif
    if (t != len)
      std::cerr << "write error: " << strerror(errno) << std::endl;
  }
}

void output_example(vw& all, example& ec)
{
  label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX)
    all.sd->weighted_labels += ((double)ld.label) * ec.weight;

  print_update(all, ec);
}

template <bool is_learn>
void predict_or_learn(VW::topk& d, LEARNER::single_learner& base, multi_ex& ec_seq)
{
  if (is_learn)
    d.learn(base, ec_seq);
  else
    d.predict(base, ec_seq);
}

void finish_example(vw& all, VW::topk& d, multi_ex& ec_seq)
{
  for (auto ec : ec_seq) output_example(all, *ec);
  for (auto sink : all.final_prediction_sink) print_result(sink, d.drain_queue());
  VW::finish_example(all, ec_seq);
}

void finish(VW::topk& d) { d.~topk(); }

LEARNER::base_learner* topk_setup(options_i& options, vw& all)
{
  uint32_t K;
  option_group_definition new_options("Top K");
  new_options.add(make_option("top", K).keep().help("top k recommendation"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("top"))
    return nullptr;

  auto data = scoped_calloc_or_throw<VW::topk>(K);

  LEARNER::learner<VW::topk, multi_ex>& l =
      init_learner(data, as_singleline(setup_base(options, all)), predict_or_learn<true>, predict_or_learn<false>);
  l.set_finish_example(finish_example);
  l.set_finish(finish);

  return make_base(l);
}
