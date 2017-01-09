#include<unordered_map>
#include "reductions.h"

using namespace std;

namespace MARGINAL
{

typedef pair<double,double> marginal;

struct data
{ float initial_numerator;
  float initial_denominator;
  float decay;
  bool id_features[256];
  features temp[256];//temporary storage when reducing.
  unordered_map<uint64_t, marginal > marginals;
  vw* all;
};

template <bool is_learn>
void predict_or_learn(data& sm, LEARNER::base_learner& base, example& ec)
{
  uint64_t mask = sm.all->weights.mask();

  for (example::iterator i = ec.begin(); i!= ec.end(); ++i)
    { namespace_index n = i.index();
      if (sm.id_features[n])
	{ std::swap(sm.temp[n],*i);
	  features& f = *i;
	  f.erase();
	  for (features::iterator j = sm.temp[n].begin(); j != sm.temp[n].end(); ++j)
	    { float first_value = j.value();
	      uint64_t first_index = j.index() & mask;
	      if (++j == sm.temp[n].end())
		{ cout << "warning: id feature namespace has " << sm.temp[n].size() << " features. Should be a multiple of 2" << endl;
		  break;
		}
	      float second_value = j.value();
	      uint64_t second_index = j.index() & mask;
	      if (first_value != 1. || second_value != 1.)
		{ cout << "warning: bad id features, must have value 1." << endl;
		  continue;
		}
	      uint64_t key = second_index + ec.ft_offset;
	      if (sm.marginals.find(key) == sm.marginals.end())//need to initialize things.
		sm.marginals.insert(make_pair(key,make_pair(sm.initial_numerator, sm.initial_denominator)));
	      f.push_back((float)(sm.marginals[key].first / sm.marginals[key].second), first_index);
	      if (!sm.temp[n].space_names.empty())
		f.space_names.push_back(sm.temp[n].space_names[2*(f.size()-1)]);
	    }
	}
    }

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  for (example::iterator i = ec.begin(); i!= ec.end(); ++i)
  { namespace_index n = i.index();
    if (sm.id_features[n])
    { if (is_learn)
        for (features::iterator j = sm.temp[n].begin(); j != sm.temp[n].end(); ++j)
        { if (++j == sm.temp[n].end())
            break;
          uint64_t second_index = j.index() & mask;
          uint64_t key = second_index + ec.ft_offset;
          marginal& m = sm.marginals[key];
          m.first = m.first * (1. - sm.decay) + ec.l.simple.label * ec.weight;
          m.second = m.second * (1. - sm.decay) + ec.weight;
        }
      std::swap(sm.temp[n],*i);
    }
  }
}

void finish(data& sm)
{ sm.marginals.~unordered_map();
  for (size_t i =0; i < 256; i++)
    sm.temp[i].delete_v();
}
  
  void save_load(data& sm, io_buf& io, bool read, bool text)
  {
    uint64_t stride_shift = sm.all->weights.stride_shift();

    if (io.files.size() == 0) 
      return;
    stringstream msg;
    uint64_t total_size;
    if (!read)
      { total_size = (uint64_t)sm.marginals.size();
	msg << "marginals size = " << total_size << "\n";
      }
    bin_text_read_write_fixed_validated(io, (char*)&total_size, sizeof(total_size), "", read, msg, text);
    
    auto iter = sm.marginals.begin();
    for (size_t i = 0; i < total_size; ++i)
      { uint64_t index;
	if (!read)
	  { index = iter->first >> stride_shift;
	    msg << index << ":";
	  }
	bin_text_read_write_fixed(io, (char*)&index, sizeof(index), "", read, msg, text);
	double numerator;
	if (!read)
	  { numerator = iter->second.first;
	    msg << numerator << ":";
	  }
	bin_text_read_write_fixed(io, (char*)&numerator, sizeof(numerator), "", read, msg, text);
	double denominator;
	if (!read)
	  { denominator = iter->second.second;
	    msg << denominator << "\n";
	  }
	bin_text_read_write_fixed(io, (char*)&denominator, sizeof(denominator), "", read, msg, text);
	if (read)
	  sm.marginals.insert(make_pair(index << stride_shift,make_pair(numerator,denominator)));
	else
	  ++iter;
      }
  }
}

using namespace MARGINAL;

LEARNER::base_learner* marginal_setup(vw& all)
{ if (missing_option<string, true>(all, "marginal", "substitute marginal label estimates for ids"))
    return nullptr;
  new_options(all)
  ("initial_denominator", po::value<float>()->default_value(1.f), "initial denominator")
  ("initial_numerator", po::value<float>()->default_value(0.5f), "initial numerator")
  ("decay", po::value<float>()->default_value(0.f), "decay multiplier per event (1e-3 for example)");
  add_options(all);

  data& d = calloc_or_throw<data>();
  d.initial_numerator = all.vm["initial_numerator"].as<float>();
  d.initial_denominator = all.vm["initial_denominator"].as<float>();
  d.decay = all.vm["decay"].as<float>();
  d.all = &all;
  string s = (string)all.vm["marginal"].as<string>();

  for (size_t u = 0; u < 256; u++)
    if (s.find((char)u) != string::npos)
      d.id_features[u] = true;
  new(&d.marginals)unordered_map<uint64_t,marginal>();

  LEARNER::learner<data>& ret =
    init_learner(&d, setup_base(all), predict_or_learn<true>, predict_or_learn<false>);
  ret.set_finish(finish);
  ret.set_save_load(save_load);

  return make_base(ret);
}
