// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <cfloat>
#include <sstream>
#include <queue>
#include <utility>

#include "topk.h"
#include "learner.h"
#include "parse_args.h"
#include "vw.h"

using namespace VW::config;

namespace VW
{
class topk
{
  using container_t = std::multimap<float, v_array<char>>;

 public:
  using const_iterator_t = container_t::const_iterator;
  topk(uint32_t k_num);

  void predict(LEARNER::single_learner& base, multi_ex& ec_seq);
  void learn(LEARNER::single_learner& base, multi_ex& ec_seq);
  std::pair<const_iterator_t, const_iterator_t> get_container_view();
  void clear_container();

 private:
  void update_priority_queue(float pred, v_array<char>& tag);

  const uint32_t _k_num;
  container_t _pr_queue;
};
}  // namespace VW

VW::topk::topk(uint32_t k_num) : _k_num(k_num) {}

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
  if (_pr_queue.size() < _k_num)
  {
    _pr_queue.insert({pred, tag});
  }
  else if (_pr_queue.begin()->first < pred)
  {
    _pr_queue.erase(_pr_queue.begin());
    _pr_queue.insert({pred, tag});
  }
}

std::pair<VW::topk::const_iterator_t, VW::topk::const_iterator_t> VW::topk::get_container_view()
{
  return {_pr_queue.cbegin(), _pr_queue.cend()};
}

void VW::topk::clear_container() { _pr_queue.clear(); }

void print_result(int file_descriptor, std::pair<VW::topk::const_iterator_t, VW::topk::const_iterator_t> const& view)
{
  if (file_descriptor >= 0)
  {
    std::stringstream ss;
    for (auto it = view.first; it != view.second; it++)
    {
      ss << std::fixed << it->first << " ";
      print_tag_by_ref(ss, it->second);
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
  for (auto sink : all.final_prediction_sink) print_result(sink, d.get_container_view());
  d.clear_container();
  VW::finish_example(all, ec_seq);
}

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

  return make_base(l);
}
