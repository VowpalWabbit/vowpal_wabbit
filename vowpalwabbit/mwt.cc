/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "vw.h"
#include "reductions.h"
#include "gd.h"
#include "cb_algs.h"
#include "io_buf.h"

using namespace std;
using namespace LEARNER;
using namespace CB_ALGS;

namespace MWT
{
struct policy_data
{ double cost;
  uint32_t action;
  bool seen;
};

struct mwt
{ bool namespaces[256]; // the set of namespaces to evaluate.
  v_array<policy_data > evals; // accrued losses of features.
  CB::cb_class* observation;
  v_array<uint64_t> policies;
  double total;
  uint32_t num_classes;
  bool learn;

  v_array<namespace_index> indices;// excluded namespaces
  features feature_space[256];
  vw* all;
};

inline bool observed_cost(CB::cb_class* cl)
{ //cost observed for this action if it has non zero probability and cost != FLT_MAX
  if (cl != nullptr)
    if (cl->cost != FLT_MAX && cl->probability > .0)
      return true;
  return false;
}

CB::cb_class* get_observed_cost(CB::label& ld)
{ for (auto& cl : ld.costs)
    if( observed_cost(&cl) )
      return &cl;
  return nullptr;
}

void value_policy(mwt& c, float val, uint64_t index)//estimate the value of a single feature.
{ if (val < 0 || floor(val) != val)
    cout << "error " << val << " is not a valid action " << endl;
  
  uint32_t value = (uint32_t) val;
  uint64_t new_index = (index & c.all->weights.mask()) >> c.all->weights.stride_shift();

  if (!c.evals[new_index].seen)
  { c.evals[new_index].seen = true;
    c.policies.push_back(new_index);
  }

  c.evals[new_index].action = value;
}

  template <bool learn, bool exclude, bool is_learn>
  void predict_or_learn(mwt& c, base_learner& base, example& ec)
  {
    c.observation = get_observed_cost(ec.l.cb);

    if (c.observation != nullptr)
      {
	c.total++;
	//For each nonzero feature in observed namespaces, check it's value.
	for (unsigned char ns : ec.indices)
	  if (c.namespaces[ns])
	    GD::foreach_feature<mwt, value_policy>(c.all, ec.feature_space[ns], c);
	for (uint64_t policy : c.policies)
	  {
	    c.evals[policy].cost += get_unbiased_cost(c.observation, c.evals[policy].action);
	    c.evals[policy].action = 0;
	  }
      }
    if (exclude || learn)
      {
	c.indices.erase();
	uint32_t stride_shift = c.all->weights.stride_shift();
	uint64_t weight_mask = c.all->weights.mask();
	for (unsigned char ns : ec.indices)
	  if (c.namespaces[ns])
	    {
	      c.indices.push_back(ns);
	      if (learn)
		{
		  c.feature_space[ns].erase();
		  for ( features::iterator& f : ec.feature_space[ns])
		    { uint64_t new_index=((f.index()& weight_mask) >> stride_shift)*c.num_classes +(uint64_t)f.value();
		      c.feature_space[ns].push_back(1, new_index << stride_shift);
		    }
		}
	      std::swap(c.feature_space[ns], ec.feature_space[ns]);
	    }
      }

    //modify the predictions to use a vector with a score for each evaluated feature.
    v_array<float> preds = ec.pred.scalars;
    
    if (learn)
      { if (is_learn)
	  base.learn(ec);
	else
	  base.predict(ec);
      }
    
    if (exclude || learn)
      while (c.indices.size() > 0)
	{ unsigned char ns = c.indices.pop();
	  swap(c.feature_space[ns], ec.feature_space[ns]);
	}
    
    //modify the predictions to use a vector with a score for each evaluated feature.
    preds.erase();
    if (learn)
      preds.push_back((float)ec.pred.multiclass);
    for(uint64_t index : c.policies)
      preds.push_back((float)c.evals[index].cost / (float)c.total);
    
    ec.pred.scalars = preds;
  }

  void print_scalars(int f, v_array<float>& scalars, v_array<char>& tag)
  { if (f >= 0)
      { std::stringstream ss;
	
	for (size_t i = 0; i < scalars.size(); i++)
	  { if (i > 0)
	      ss << ' ';
	    ss << scalars[i];
	  }
	for (size_t i = 0; i < tag.size(); i++)
	  { if (i == 0)
	      ss << ' ';
	    ss << tag[i];
	  }
	ss << '\n';
	ssize_t len = ss.str().size();
	ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
	if (t != len)
	  cerr << "write error: " << strerror(errno) << endl;
      }
  }

void finish_example(vw& all, mwt& c, example& ec)
{ float loss = 0.;
  if (c.learn)
  { if (c.observation != nullptr)
      loss = get_unbiased_cost(c.observation, (uint32_t)ec.pred.scalars[0]);
  }
  all.sd->update(ec.test_only, loss, 1.f, ec.num_features);

  for (int sink : all.final_prediction_sink)
    print_scalars(sink, ec.pred.scalars, ec.tag);

  if (c.learn)
  { v_array<float> temp = ec.pred.scalars;
    ec.pred.multiclass = (uint32_t)temp[0];
    CB::print_update(all, c.observation != nullptr, ec, nullptr, false);
    ec.pred.scalars = temp;
  }
  VW::finish_example(all, &ec);
}

void finish(mwt& c)
{ c.evals.delete_v();
  c.policies.delete_v();
  for (size_t i = 0; i < 256; i++)
    c.feature_space[i].delete_v();
  c.indices.delete_v();
}

void save_load(mwt& c, io_buf& model_file, bool read, bool text)
{ if (model_file.files.size() == 0)
    return;

  stringstream msg;

  // total
  msg << "total: " << c.total;
  bin_text_read_write_fixed_validated(model_file, (char*)&c.total, sizeof(c.total), "", read, msg, text);

  // policies
  size_t policies_size = c.policies.size();
  bin_text_read_write_fixed_validated(model_file, (char*)&policies_size, sizeof(policies_size), "", read, msg, text);

  if (read)
  { c.policies.resize(policies_size);
    c.policies.end() = c.policies.begin() + policies_size;
  }
  else
  { msg << "policies: ";
    for (feature_index& policy : c.policies)
      msg << policy << " ";
  }

  bin_text_read_write_fixed_validated(model_file, (char*)c.policies.begin(), policies_size * sizeof(feature_index),
                                      "", read, msg, text);

  // c.evals is already initialized nicely to the same size as the regressor.
  for (feature_index& policy : c.policies)
  { policy_data& pd = c.evals[policy];
    if (read)
      msg << "evals: "<< policy << ":" << pd.action << ":" << pd.cost << " ";
    bin_text_read_write_fixed_validated(model_file, (char*)&c.evals[policy], sizeof(policy_data),
                                        "", read, msg, text);
  }
}
}
using namespace MWT;

base_learner* mwt_setup(vw& all)
{ if (missing_option<string, true>(all, "multiworld_test", "Evaluate features as a policies"))
    return nullptr;
  new_options(all, "MWT options")
  ("learn", po::value<uint32_t>(), "Do Contextual Bandit learning on <n> classes.")
  ("exclude_eval", "Discard mwt policy features before learning");
  add_options(all);

  mwt& c = calloc_or_throw<mwt>();

  string s = all.vm["multiworld_test"].as<string>();
  for (size_t i = 0; i < s.size(); i++)
    c.namespaces[(unsigned char)s[i]] = true;
  c.all = &all;

  calloc_reserve(c.evals, all.length());
  c.evals.end() = c.evals.begin() + all.length();

  all.delete_prediction = delete_scalars;
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  if (all.vm.count("learn"))
  { c.num_classes = all.vm["learn"].as<uint32_t>();
    c.learn = true;

    if (count(all.args.begin(), all.args.end(),"--cb") == 0)
    { all.args.push_back("--cb");
      stringstream ss;
      ss << c.num_classes;
      all.args.push_back(ss.str());
    }
  }

  learner<mwt>* l;
  if (c.learn)
    if (all.vm.count("exclude_eval"))
      l = &init_learner(&c, setup_base(all), predict_or_learn<true, true, true>, predict_or_learn<true, true, false>, 1, prediction_type::scalars);
    else
      l = &init_learner(&c, setup_base(all), predict_or_learn<true, false, true>, predict_or_learn<true, false, false>, 1, prediction_type::scalars);
  else
    l = &init_learner(&c, setup_base(all), predict_or_learn<false, false, true>, predict_or_learn<false, false, false>, 1, prediction_type::scalars);

  l->set_save_load(save_load);
  l->set_finish_example(finish_example);
  l->set_finish(finish);
  return make_base(*l);
}
