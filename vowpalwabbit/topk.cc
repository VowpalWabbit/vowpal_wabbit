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

typedef pair<float, v_array<char> > scored_example;

struct compare_scored_examples
{ bool operator()(scored_example const& a, scored_example const& b) const
  { return a.first > b.first; }
};

struct topk
{ uint32_t B; //rec number
  priority_queue<scored_example, vector<scored_example>, compare_scored_examples > pr_queue;
};

void print_result(int f, priority_queue<scored_example, vector<scored_example>, compare_scored_examples > &pr_queue)
{ if (f >= 0)
  { char temp[30];
    std::stringstream ss;
    scored_example tmp_example;
    while(!pr_queue.empty())
    { tmp_example = pr_queue.top();
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

void output_example(vw& all, topk& d, example& ec)
{ label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX)
    all.sd->weighted_labels += ld.label * ec.weight;

  if (example_is_newline(ec))
    for (int sink : all.final_prediction_sink)
      print_result(sink, d.pr_queue);

  print_update(all, ec);
}

template <bool is_learn>
void predict_or_learn(topk& d, LEARNER::base_learner& base, example& ec)
{ if (example_is_newline(ec)) return;//do not predict newline

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if(d.pr_queue.size() < d.B)
    d.pr_queue.push(make_pair(ec.pred.scalar, ec.tag));

  else if(d.pr_queue.top().first < ec.pred.scalar)
  { d.pr_queue.pop();
    d.pr_queue.push(make_pair(ec.pred.scalar, ec.tag));
  }
}

void finish_example(vw& all, topk& d, example& ec)
{ output_example(all, d, ec);
  VW::finish_example(all, &ec);
}


void finish(topk& d)
{ d.pr_queue = priority_queue<scored_example, vector<scored_example>, compare_scored_examples >();
}

LEARNER::base_learner* topk_setup(vw& all)
{ if (missing_option<size_t, false>(all, "top", "top k recommendation"))
    return nullptr;

  topk& data = calloc_or_throw<topk>();
  data.B = (uint32_t)all.vm["top"].as<size_t>();

  LEARNER::learner<topk>& l = init_learner(&data, setup_base(all), predict_or_learn<true>,
                              predict_or_learn<false>);
  l.set_finish_example(finish_example);
  l.set_finish(finish);

  return make_base(l);
}
