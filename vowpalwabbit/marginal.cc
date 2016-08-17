#include<unordered_map>
#include "reductions.h"

using namespace std;

namespace MARGINAL {

  typedef pair<float,float> marginal;

struct data
{ 
  float initial_numerator;
  float initial_denominator;
  bool id_features[256];
  feature temp[256];//temporary storage when reducing.
  audit_strings_ptr asp[256];
  unordered_map<uint64_t, marginal > marginals;
};

template <bool is_learn>
void predict_or_learn(data& sm, LEARNER::base_learner& base, example& ec)
{
  for (example::iterator i = ec.begin(); i!= ec.end(); ++i)
    {
      namespace_index n = i.index();
      if (sm.id_features[n])
	{
	  features& f = *i;
	  if (f.size() != 2)
	    {
	      cout << "warning: id feature namespace has " << f.size() << " features. Should have a constant then the id" << endl;
	      continue;
	    }
	  features::iterator i = f.begin();
	  float first_value = i.value();
	  float second_value = (++i).value();
	  uint64_t second_index = i.index();
	  if (first_value != 1. || second_value != 1.)
	    {
	      cout << "warning: bad id features, must have value 1." << endl;
	      continue;
	    }
	  if (sm.marginals.find(second_index) == sm.marginals.end())//need to initialize things.
	    sm.marginals.insert(make_pair(second_index,make_pair(sm.initial_numerator, sm.initial_denominator)));
	  f.begin().value() = sm.marginals[second_index].first / sm.marginals[second_index].second;
	  sm.temp[n]={second_value,second_index};
	  if (!f.space_names.empty())
	    sm.asp[n]=f.space_names[1];
	  f.truncate_to(1);
	}
    }
  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  for (example::iterator i = ec.begin(); i!= ec.end(); ++i)
    {
      namespace_index n = i.index();
      if (sm.id_features[n])
	{
	  features& f = *i;
	  if (f.size() != 1)
	    cout << "warning: id feature namespace has " << f.size() << " features. Should have a constant then the id" << endl;
	  else //do unsubstitution dance
	    {
	      f.begin().value() = 1.;
	      f.push_back(sm.temp[n].x, sm.temp[n].weight_index);
	      if (!f.space_names.empty())
		f.space_names.push_back(sm.asp[n]);
	      if (is_learn)
		{
		  uint64_t second_index = (++(f.begin())).index();
		  sm.marginals[second_index].first += ec.l.simple.label * ec.weight;
		  sm.marginals[second_index].second += ec.weight;
		}
	    }
	}
    }
}

  void finish(data& sm) { sm.marginals.~unordered_map(); }
}

using namespace MARGINAL;

LEARNER::base_learner* marginal_setup(vw& all)
{ if (missing_option<string, true>(all, "marginal", "substitute marginal label estimates for ids"))
    return nullptr;
  new_options(all)
    ("initial_denominator", po::value<float>()->default_value(1.f), "initial default value")
    ("initial_numerator", po::value<float>()->default_value(0.f), "initial default value");
  add_options(all);

  data& d = calloc_or_throw<data>();
  d.initial_numerator = all.vm["initial_numerator"].as<float>();
  d.initial_denominator = all.vm["initial_denominator"].as<float>();
  string s = (string)all.vm["marginal"].as<string>();

  for (size_t u = 0; u < 256; u++)
    if (s.find((char)u) != string::npos)
      d.id_features[u] = true;
  new(&d.marginals)unordered_map<uint64_t,marginal>();
  
  LEARNER::learner<data>& ret =
    init_learner(&d, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);
  ret.set_finish(finish);
  
  return make_base(ret);
}
