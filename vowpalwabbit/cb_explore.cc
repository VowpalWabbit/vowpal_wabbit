#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"
#include "cb_explore.h"
#include "gen_cs_example.h"
#include "learner.h"

using namespace LEARNER;
using namespace MultiWorldTesting;
using namespace MultiWorldTesting::SingleAction;

struct cb_explore;

struct vw_context
{
  cb_explore& data;
  base_learner& l;
  example& e;
  bool recorded;
};

void safety(v_array<float>& distribution, float min_prob);

//Generic policy class to be used by all exploration algorithms that use the IPolicy interface

class vw_policy : public IPolicy<vw_context>
{
public:
  vw_policy(size_t i) : m_index(i) { }

  u32 Choose_Action(vw_context& ctx)
  { ctx.l.predict(ctx.e, m_index);
    ctx.recorded = false;
    return (u32)(ctx.e.pred.multiclass);
  }

  virtual ~vw_policy()
  { }

private:
  size_t m_index;
};

//Online Cover algorithm

class vw_cover : public IScorer<vw_context>
{
public:
  vw_cover(float eps, size_t s, u32 num_a) :
    epsilon(eps), size(s), num_actions(num_a), counter(1)
  { probabilities = v_init<float>();
    probabilities.resize(num_actions + 1);
    predictions = v_init<uint32_t>();
    predictions.resize(s);
  }

  virtual ~vw_cover() { }

  v_array<float>& Get_Probabilities()
  {
    probabilities.erase();
    for (size_t i = 0; i < num_actions; i++)
      probabilities.push_back(0);
    return probabilities;
  };

  vector<float> Score_Actions(vw_context& ctx);

  float epsilon;
  size_t size;
  u32 num_actions;
  size_t counter;
  v_array<uint32_t> predictions;
  v_array<float> probabilities;
};

struct vw_recorder : public IRecorder<vw_context>
{ void Record(vw_context& context, u32 a, float p, string /*unique_key*/)
  { pred.action = a;
    pred.probability = p;
    context.recorded = true;
  }

  cb_explore_pred pred;

  virtual ~vw_recorder()
  { }
};

struct cb_explore
{
  cb_to_cs cbcs;
  v_array<float> preds;

  bool learn_only;

  CB::label cb_label;
  COST_SENSITIVE::label cs_label;
  COST_SENSITIVE::label second_cs_label;

  base_learner* cs;

  vw_policy* policy;
  TauFirstExplorer<vw_context>* tau_explorer;
  vw_recorder* recorder;
  MwtExplorer<vw_context>* mwt_explorer;
  EpsilonGreedyExplorer<vw_context>* greedy_explorer;

  BootstrapExplorer<vw_context>* bootstrap_explorer;
  vector<unique_ptr<IPolicy<vw_context>>> policies;

  vw_cover* cover;
  GenericExplorer<vw_context>* generic_explorer;
};

vector<float> vw_cover::Score_Actions(vw_context& ctx)
{ float additive_probability = 1.f / (float)size;
  for (size_t i = 0; i < size; i++)
    { //get predicted cost-sensitive predictions
      if (i == 0)
	ctx.data.cs->predict(ctx.e, i);
      else
	ctx.data.cs->predict(ctx.e, i + 1);
      uint32_t pred = ctx.e.pred.multiclass;
      probabilities[pred - 1] += additive_probability;
      predictions[i] = (uint32_t)pred;
    }
  uint32_t num_actions = ctx.data.cbcs.num_actions;
  float min_prob = epsilon * min(1.f / num_actions, 1.f / (float)sqrt(counter * num_actions));

  safety(probabilities, min_prob);

  vector<float> probs_vec;
  for (size_t i = 0; i < num_actions; i++)
    probs_vec.push_back(probabilities[i]);

  counter++;

  return probs_vec;
}

template <bool is_learn>
void predict_or_learn_first(cb_explore& data, base_learner& base, example& ec)
{ //Explore tau times, then act according to optimal.

  vw_context vwc = {data, base, ec};
  uint32_t action = 1;
  if(!is_learn || !data.learn_only)
    data.mwt_explorer->Choose_Action(*data.tau_explorer, StringUtils::to_string(ec.example_counter), vwc);

  if (is_learn)
    base.learn(ec);

  ec.pred.action_prob = data.recorder->pred;
}

template <bool is_learn>
void predict_or_learn_greedy(cb_explore& data, base_learner& base, example& ec)
{ //Explore uniform random an epsilon fraction of the time.

  vw_context vwc = {data, base, ec};
  uint32_t action = 1;
  if(!is_learn || !data.learn_only)
    data.mwt_explorer->Choose_Action(*data.greedy_explorer, StringUtils::to_string(ec.example_counter), vwc);

  if (is_learn)
    base.learn(ec);

  ec.pred.action_prob = data.recorder->pred;
}

template <bool is_learn>
void predict_or_learn_bag(cb_explore& data, base_learner& base, example& ec)
{ //Randomize over predictions from a base set of predictors

  vw_context context = {data, base, ec};
  uint32_t action = 1;
  if(!is_learn || !data.learn_only)
    data.mwt_explorer->Choose_Action(*data.bootstrap_explorer, StringUtils::to_string(ec.example_counter), context);

  if (is_learn)
    for (size_t i = 0; i < data.policies.size(); i++)
      { uint32_t count = BS::weight_gen();
	for (uint32_t j = 0; j < count; j++)
	  base.learn(ec,i);
      }

  ec.pred.action_prob = data.recorder->pred;
}

void safety(v_array<float>& distribution, float min_prob)
{ float added_mass = 0.;
  for (uint32_t i = 0; i < distribution.size(); i++)
    if (distribution[i] > 0 && distribution[i] <= min_prob)
      { added_mass += min_prob - distribution[i];
	distribution[i] = min_prob;
      }

  float ratio = 1.f / (1.f + added_mass);
  if (ratio < 0.999)
    { for (uint32_t i = 0; i < distribution.size(); i++)
	if (distribution[i] > min_prob)
	  distribution[i] = distribution[i] * ratio;
      safety(distribution, min_prob);
    }
}


template <bool is_learn>
void predict_or_learn_cover(cb_explore& data, base_learner& base, example& ec)
{ //Randomize over predictions from a base set of predictors
  //Use cost sensitive oracle to cover actions to form distribution.

  uint32_t num_actions = data.cbcs.num_actions;

  data.cs_label.costs.erase();
  for (uint32_t j = 0; j < num_actions; j++)
    { COST_SENSITIVE::wclass wc;

      //get cost prediction for this label
      wc.x = FLT_MAX;
      wc.class_index = j+1;
      wc.partial_prediction = 0.;
      wc.wap_value = 0.;
      data.cs_label.costs.push_back(wc);
    }

  float epsilon = data.cover->epsilon;
  size_t cover_size = data.cover->size;
  size_t counter = data.cover->counter;
  v_array<float>& probabilities = data.cover->Get_Probabilities();
  v_array<uint32_t>& predictions = data.cover->predictions;

  float additive_probability = 1.f / (float)cover_size;

  float min_prob = epsilon * min(1.f / num_actions, 1.f / (float)sqrt(counter * num_actions));

  data.cb_label.costs.erase();

  if(ec.l.cb.costs.size() > 0) {
    CB::cb_class cl;
    cl.cost = ec.l.cb.costs[0].cost;
    cl.action = ec.l.cb.costs[0].action;
    cl.probability = ec.l.cb.costs[0].probability;
    data.cb_label.costs.push_back(cl);
  }

  vw_context cp = {data, base, ec};
  if(!data.learn_only || !is_learn)
    data.mwt_explorer->Choose_Action(*data.generic_explorer, StringUtils::to_string(ec.example_counter), cp);

  if (is_learn) {
    ec.l.cb = data.cb_label;
    base.learn(ec);

    //Now update oracles

    //1. Compute loss vector
    data.cs_label.costs.erase();
    float norm = min_prob * num_actions;
    ec.l.cb = data.cb_label;
    data.cbcs.known_cost = get_observed_cost(data.cb_label);
    gen_cs_example<false>(data.cbcs, ec, data.cb_label, data.cs_label);
    for(uint32_t i = 0;i < num_actions;i++)
      probabilities[i] = 0;

    ec.l.cs = data.second_cs_label;
    //2. Update functions
    for (size_t i = 0; i < cover_size; i++)
      { //Create costs of each action based on online cover
	for (uint32_t j = 0; j < num_actions; j++)
	  { float pseudo_cost = data.cs_label.costs[j].x - epsilon * min_prob / (max(probabilities[j], min_prob) / norm) + 1;
	    data.second_cs_label.costs[j].class_index = j+1;
	    data.second_cs_label.costs[j].x = pseudo_cost;
	  }
	if (i != 0)
	  data.cs->learn(ec,i+1);
	if (probabilities[predictions[i] - 1] < min_prob)
	  norm += max(0, additive_probability - (min_prob - probabilities[predictions[i] - 1]));
	else
	  norm += additive_probability;
	probabilities[predictions[i] - 1] += additive_probability;
      }
  }

  ec.l.cb = data.cb_label;

  ec.pred.action_prob = data.recorder->pred;
}

template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

void finish(cb_explore& data)
{
  data.preds.delete_v();
  cb_to_cs& c = data.cbcs;
  COST_SENSITIVE::cs_label.delete_label(&c.pred_scores);
  CB::cb_label.delete_label(&data.cb_label);
  COST_SENSITIVE::cs_label.delete_label(&data.cs_label);
  COST_SENSITIVE::cs_label.delete_label(&data.second_cs_label);
  delete_it(data.policy);
  delete_it(data.tau_explorer);
  delete_it(data.greedy_explorer);
  delete_it(data.bootstrap_explorer);
  delete_it(data.generic_explorer);
  delete_it(data.mwt_explorer);
  delete_it(data.recorder);
  if (data.cover != nullptr)
    { data.cover->predictions.delete_v();
      data.cover->probabilities.delete_v();
    }
  delete_it(data.cover);
  if (data.policies.size() > 0)
    data.policies.~vector();
}

base_learner* cb_explore_setup(vw& all)
{ //parse and set arguments
  if (missing_option<size_t, true>(all, "cb_explore", "Online explore-exploit for a <k> action contextual bandit problem"))
    return nullptr;
  new_options(all, "CB_EXPLORE options")
    ("first", po::value<size_t>(), "tau-first exploration")
    ("epsilon",po::value<float>() ,"epsilon-greedy exploration")
    ("bag",po::value<size_t>() ,"bagging-based exploration")
    ("cover",po::value<size_t>() ,"bagging-based exploration")
    ("learn_only","for not calling predict when learn is true");
  add_options(all);

  po::variables_map& vm = all.vm;
  cb_explore& data = calloc_or_throw<cb_explore>();
  data.cbcs.num_actions = (uint32_t)vm["cb_explore"].as<size_t>();
  uint32_t num_actions = data.cbcs.num_actions;

  data.preds = v_init<float>();

  if (count(all.args.begin(), all.args.end(),"--cb") == 0)
    { all.args.push_back("--cb");
      stringstream ss;
      ss << vm["cb_explore"].as<size_t>();
      all.args.push_back(ss.str());
    }

  if(vm.count("learn_only"))
    data.learn_only = true;
  else
    data.learn_only = false;

  data.cbcs.cb_type = CB_TYPE_DR;
  //ALEKH: Others TBD later
  // if (count(all.args.begin(), all.args.end(), "--cb_type") == 0)
  //   data.cbcs->cb_type = CB_TYPE_DR;
  // else
  //   data.cbcs->cb_type = (size_t)vm["cb_type"].as<size_t>();

  base_learner* base = setup_base(all);

  learner<cb_explore>* l;
  data.recorder = new vw_recorder();
  data.mwt_explorer = new MwtExplorer<vw_context>("vw", *data.recorder);
  if (vm.count("cover"))
    { size_t cover = (uint32_t)vm["cover"].as<size_t>();
      data.cs = all.cost_sensitive;
      data.second_cs_label.costs.resize(num_actions);
      data.second_cs_label.costs.end() = data.second_cs_label.costs.begin()+num_actions;
      float epsilon = 0.05f;
      if (vm.count("epsilon"))
	epsilon = vm["epsilon"].as<float>();
      data.cover = new vw_cover(epsilon, cover, (u32)num_actions);
      data.generic_explorer = new GenericExplorer<vw_context>(*data.cover, (u32)num_actions);
      l = &init_multiclass_learner(&data, base, predict_or_learn_cover<true>,
				   predict_or_learn_cover<false>, all.p, cover + 1);
    }
  else if (vm.count("bag"))
    { size_t bags = (uint32_t)vm["bag"].as<size_t>();
      for (size_t i = 0; i < bags; i++)
	data.policies.push_back(unique_ptr<IPolicy<vw_context>>(new vw_policy(i)));
      data.bootstrap_explorer = new BootstrapExplorer<vw_context>(data.policies, (u32)num_actions);
      l = &init_multiclass_learner(&data, base, predict_or_learn_bag<true>,
				   predict_or_learn_bag<false>, all.p, bags);
    }
  else if (vm.count("first") )
    { uint32_t tau = (uint32_t)vm["first"].as<size_t>();
      data.policy = new vw_policy(0);
      data.tau_explorer = new TauFirstExplorer<vw_context>(*data.policy, (u32)tau, (u32)num_actions);
      l = &init_multiclass_learner(&data, base, predict_or_learn_first<true>,
				   predict_or_learn_first<false>, all.p, 1);
    }
  else
    { float epsilon = 0.05f;
      if (vm.count("epsilon"))
	epsilon = vm["epsilon"].as<float>();
      data.policy = new vw_policy(0);
      data.greedy_explorer = new EpsilonGreedyExplorer<vw_context>(*data.policy, epsilon, (u32)num_actions);
      l = &init_multiclass_learner(&data, base, predict_or_learn_greedy<true>,
				   predict_or_learn_greedy<false>, all.p, 1);
    }
  data.cbcs.scorer = all.scorer;
  l->set_finish(finish);

  return make_base(*l);
}
