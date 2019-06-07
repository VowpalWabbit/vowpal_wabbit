/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <sstream>
#include <queue>

#include "reductions.h"
#include "vw.h"

using namespace std;
using namespace VW::config;

using scored_example = pair<float, v_array<char>>;

struct compare_scored_examples
{
  bool operator()(scored_example const& a, scored_example const& b) const { return a.first > b.first; }
};

struct topk
{
  uint32_t K;  // rec number
  priority_queue<scored_example, vector<scored_example>, compare_scored_examples> pr_queue;
  vw* all;
};

void print_result(int f, priority_queue<scored_example, vector<scored_example>, compare_scored_examples>& pr_queue)
{
  if (f >= 0)
  {
    char temp[30];
    std::stringstream ss;
    scored_example tmp_example;
    while (!pr_queue.empty())
    {
      tmp_example = pr_queue.top();
      pr_queue.pop();
      sprintf(temp, "%f", tmp_example.first);
      ss << temp;
      ss << ' ';
      print_tag(ss, tmp_example.second);
      ss << ' ';
      ss << '\n';
    }
    ss << '\n';
    ssize_t len = ss.str().size();
#ifdef _WIN32
    ssize_t t = _write(f, ss.str().c_str(), (unsigned int)len);
#else
    ssize_t t = write(f, ss.str().c_str(), (unsigned int)len);
#endif
    if (t != len)
      cerr << "write error: " << strerror(errno) << endl;
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
void predict_or_learn(topk& d, LEARNER::single_learner& base, multi_ex& ec_seq)
{
  for (auto example : ec_seq)
  {
    auto ec = *example;

    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    if (d.pr_queue.size() < d.K)
      d.pr_queue.push(make_pair(ec.pred.scalar, ec.tag));
    else if (d.pr_queue.top().first < ec.pred.scalar)
    {
      d.pr_queue.pop();
      d.pr_queue.push(make_pair(ec.pred.scalar, ec.tag));
    }

    output_example(*d.all, ec);
  }
}

void finish_example(vw& all, topk& d, multi_ex& ec_seq)
{
  for (int sink : all.final_prediction_sink) print_result(sink, d.pr_queue);

  VW::clear_seq_and_finish_examples(all, ec_seq);
}

void finish(topk& d) { d.pr_queue = priority_queue<scored_example, vector<scored_example>, compare_scored_examples>(); }

LEARNER::base_learner* topk_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<topk>();

  option_group_definition new_options("Top K");
  new_options.add(make_option("top", data->K).keep().help("top k recommendation"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("top"))
    return nullptr;

  data->all = &all;

  LEARNER::learner<topk, multi_ex>& l =
      init_learner(data, as_singleline(setup_base(options, all)), predict_or_learn<true>, predict_or_learn<false>);
  l.set_finish_example(finish_example);
  l.set_finish(finish);

  return make_base(l);
}
