/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <numeric>
#include <vector>
#include <queue>

#include "reductions.h"

using namespace std;
using namespace LEARNER;

typedef pair<float, v_array<char> > scored_example;

struct compare_scored_examples
{
    bool operator()(scored_example const& a, scored_example const& b) const
    {
        return a.first > b.first;
    }
};

namespace TOPK {

  struct topk{
    uint32_t B; //rec number
    priority_queue<scored_example, vector<scored_example>, compare_scored_examples > pr_queue;
    vw* all;
  };

  void print_result(int f, priority_queue<scored_example, vector<scored_example>, compare_scored_examples > &pr_queue)
  {
    if (f >= 0)
    {
      char temp[30];
      std::stringstream ss;
      scored_example tmp_example;
      while(!pr_queue.empty())
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
        cerr << "write error" << endl;
    }    
  }

  void output_example(vw& all, topk& d, example& ec)
  {
    label_data* ld = (label_data*)ec.ld;
    
    all.sd->weighted_examples += ld->weight;
    all.sd->sum_loss += ec.loss;
    all.sd->sum_loss_since_last_dump += ec.loss;
    all.sd->total_features += ec.num_features;
    all.sd->example_number++;
 
    if (example_is_newline(ec))
      for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
        TOPK::print_result(*sink, d.pr_queue);
       
    print_update(all, ec);
  }

  template <bool is_learn>
  void predict_or_learn(topk& d, learner& base, example& ec)
  {
    if (example_is_newline(ec)) return;//do not predict newline

    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    if(d.pr_queue.size() < d.B)      
      d.pr_queue.push(make_pair(ec.final_prediction, ec.tag));

    else if(d.pr_queue.top().first < ec.final_prediction)
    {
      d.pr_queue.pop();
      d.pr_queue.push(make_pair(ec.final_prediction, ec.tag));
    }

  }

  void finish_example(vw& all, topk& d, example& ec)
  {
    TOPK::output_example(all, d, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    topk* data = (topk*)calloc_or_die(1, sizeof(topk));

    data->B = (uint32_t)vm["top"].as<size_t>();

    data->all = &all;

    learner* l = new learner(data, all.l);
    l->set_learn<topk, predict_or_learn<true> >();
    l->set_predict<topk, predict_or_learn<false> >();
    l->set_finish_example<topk,finish_example>();

    return l;
  }
}
