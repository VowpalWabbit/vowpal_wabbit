#include "reductions.h"
#include "simple_label.h"

using namespace LEARNER;

namespace ALINK {
  const int autoconstant = 524267083;
  
  struct autolink {
    uint32_t d;
    uint32_t stride_shift;
  };

  template <bool is_learn>
  void predict_or_learn(autolink& b, learner& base, example& ec)
  {
    base.predict(ec);
    float base_pred = ec.final_prediction;

    // add features of label
    ec.indices.push_back(autolink_namespace);
    float sum_sq = 0;
    for (size_t i = 0; i < b.d; i++)
      if (base_pred != 0.)
	{
	  feature f = { base_pred, (uint32_t) (autoconstant + i < b.stride_shift) };
	  ec.atomics[autolink_namespace].push_back(f);
	  sum_sq += base_pred*base_pred;
	  base_pred *= ec.final_prediction;
	}
    ec.total_sum_feat_sq += sum_sq;

    // apply predict or learn
    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    ec.atomics[autolink_namespace].erase();
    ec.indices.pop();
    ec.total_sum_feat_sq -= sum_sq;
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    autolink* data = (autolink*)calloc_or_die(1,sizeof(autolink));
    data->d = (uint32_t)vm["autolink"].as<size_t>();
    data->stride_shift = all.reg.stride_shift;
    
    if (!vm_file.count("autolink")) 
      {
	std::stringstream ss;
	ss << " --autolink " << data->d << " ";
	all.options_from_file.append(ss.str());
      }

    learner* ret = new learner(data, all.l);
    ret->set_learn<autolink, predict_or_learn<true> >();
    ret->set_predict<autolink, predict_or_learn<false> >();
    return ret;
  }
}
