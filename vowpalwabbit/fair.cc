#include<unordered_map>
#include "reductions.h"

namespace FAIR
{
  using namespace std;
  struct fair
  {
    unsigned char fair_space;
    float lambda;
    uint32_t k;
    
    unordered_map<uint64_t, uint64_t> attribute_counts;
    uint64_t event_count;

    uint64_t first_attribute;
    
    COST_SENSITIVE::label cs_label;
  };
  
  template <bool is_learn>
  void predict_or_learn(fair& data, LEARNER::base_learner& base, example& ec)
  {
    data.cs_label.costs.erase();
    MULTICLASS::label_t mc_label = ec.l.multi;
    
    if (!is_learn)
      {
	ec.l.cs = data.cs_label;
	base.predict(ec);
      }
    else
      {
	for (features::iterator& f : ec.feature_space[data.fair_space]) //update stats
	  {
	    uint64_t key = f.index() + ec.ft_offset;
	    if(data.attribute_counts.find(key) == data.attribute_counts.end())
	      data.attribute_counts.insert(make_pair(key,0));
	    data.attribute_counts[key]++;
	    if (data.first_attribute == 0)
	      data.first_attribute = key;
	  }
	data.event_count++;
	
	//construct cost sensitive label
	for (uint32_t i = 1; i <= data.k; i++)
	  { COST_SENSITIVE::wclass wc = {0., i, 0., 0.};
	    if (i == mc_label.label)
	      wc.x = 0.;
	    else
	      wc.x = 1.;

	    if (i == 2)
	      if (ec.feature_space[data.fair_space].size() > 0)
		{
		  uint64_t key = ec.feature_space[data.fair_space].indicies[0] + ec.ft_offset;
		  float attribute_fraction = (float) data.attribute_counts[key] / (float) data.event_count; 

		  if (key == data.first_attribute)
		    wc.x += data.lambda / attribute_fraction;
		  else
		    wc.x -= data.lambda / attribute_fraction;
		}
		
	    data.cs_label.costs.push_back(wc);
	  }
	
	ec.l.cs = data.cs_label;
	base.learn(ec);
      }
    ec.l.multi = mc_label;
  }
}

using namespace FAIR;
  
LEARNER::base_learner* fair_setup(vw& all)
{ if (missing_option<uint32_t, true>(all, "fair", "make classification fair with respect to some attributes for <k> classes"))
    return nullptr;
  new_options(all)
    ("lambda", po::value<float>()->default_value(0.f), "lagrangian parameter")
    ("space", po::value<char>(), "protected attribute space");

  fair& data = calloc_or_throw<fair>();
  
  data.k = (uint32_t)all.vm["fair"].as<uint32_t>();
  if (all.vm.count("space"))
    data.fair_space = (char)all.vm["space"].as<char>();
  if (all.vm.count("lambda"))
    data.lambda = (float)all.vm["lambda"].as<float>();

  LEARNER::learner<fair>& ret =
    init_learner(&data, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);

  return make_base(ret);
}
