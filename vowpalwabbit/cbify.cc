#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/static/MWTExplorer.h"
#include "vw.h"

using namespace LEARNER;
using namespace MultiWorldTesting;

struct cbify;

struct vw_context {
  cbify& data;
  base_learner& l;
  example& e;
  bool recorded;
};

void safety(v_array<float>& distribution, float min_prob);

class vw_policy : public IPolicy<vw_context>
{
public:
  vw_policy(size_t i) : m_index(i) { }

  u32 Choose_Action(vw_context& ctx)
  {
    ctx.l.predict(ctx.e, m_index);
    ctx.recorded = false;
    return (u32)(ctx.e.pred.multiclass);
  }

  virtual ~vw_policy()
  { }

private:
  size_t m_index;
};

class vw_cover : public IScorer<vw_context>
{
public:
  vw_cover(float eps, size_t s, u32 num_a) :
    epsilon(eps), size(s), num_actions(num_a), counter(1)
  { 
    probabilities = v_init<float>();
    probabilities.resize(num_actions + 1);
    predictions = v_init<uint32_t>();
    predictions.resize(s);
  }

  virtual ~vw_cover()
  { }
  
  v_array<float>& Get_Scores()
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
{
  void Record(vw_context& context, u32 a, float p, string /*unique_key*/)
  {
    action = a;
    probability = p;
    context.recorded = true;
  }
  u32 action;
  float probability;

  virtual ~vw_recorder()
  { }
};

struct cbify {
  uint32_t k;
  
  CB::label cb_label;
  COST_SENSITIVE::label cs_label;
  COST_SENSITIVE::label second_cs_label;
  
  base_learner* cs;
  LEARNER::base_learner* reg;
  
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

float loss(uint32_t label, uint32_t final_prediction)
{
  if (label != final_prediction)
    return 1.;
  else
    return 0.;
}

vector<float> vw_cover::Score_Actions(vw_context& ctx)
{
  float additive_probability = 1.f / (float)size;
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
  float min_prob = epsilon * min(1.f / ctx.data.k, 1.f / (float)sqrt(counter * ctx.data.k));
  
  safety(probabilities, min_prob);
  
  vector<float> scores;
  for (size_t i = 0; i < ctx.data.k; i++)
    scores.push_back(probabilities[i]);
  
  counter++;
  
  return scores;
}

template <bool is_learn>
void predict_or_learn_first(cbify& data, base_learner& base, example& ec)
{//Explore tau times, then act according to optimal.
  MULTICLASS::label_t ld = ec.l.multi;
  
  data.cb_label.costs.erase();
  ec.l.cb = data.cb_label;
  //Use CB to find current prediction for remaining rounds.
  
  vw_context vwc = {data, base, ec};
  uint32_t action = data.mwt_explorer->Choose_Action(*data.tau_explorer, to_string((unsigned long long)ec.example_counter), vwc);
  
  if (vwc.recorded && is_learn)
    {
      CB::cb_class l = {loss(ld.label, action), action, data.recorder->probability };
      data.cb_label.costs.push_back(l);
      ec.l.cb = data.cb_label;
      base.learn(ec);
    }
  
  ec.pred.multiclass = action;
  ec.l.multi = ld;
}

template <bool is_learn>
void predict_or_learn_greedy(cbify& data, base_learner& base, example& ec)
{//Explore uniform random an epsilon fraction of the time.
  MULTICLASS::label_t ld = ec.l.multi;
  
  data.cb_label.costs.erase();
  ec.l.cb = data.cb_label;
  
  vw_context vwc = {data, base, ec};
  uint32_t action = data.mwt_explorer->Choose_Action(*data.greedy_explorer, to_string((unsigned long long)ec.example_counter), vwc);
  
  if (is_learn) {
    CB::cb_class l = { loss(ld.label, action), action, data.recorder->probability };
    data.cb_label.costs.push_back(l);
    ec.l.cb = data.cb_label;
    base.learn(ec);
  }  

  ec.pred.multiclass = action;
  ec.l.multi = ld;
}

template <bool is_learn>
void predict_or_learn_bag(cbify& data, base_learner& base, example& ec)
{//Randomize over predictions from a base set of predictors
  //Use CB to find current predictions.
  MULTICLASS::label_t ld = ec.l.multi;
  
  data.cb_label.costs.erase();
  ec.l.cb = data.cb_label;
  
  vw_context context = {data, base, ec};
  uint32_t action = data.mwt_explorer->Choose_Action(*data.bootstrap_explorer, to_string((unsigned long long)ec.example_counter), context);
  
  if (is_learn)
    {
      CB::cb_class l = {loss(ld.label, action), 
			action, data.recorder->probability};
      data.cb_label.costs.push_back(l);
      ec.l.cb = data.cb_label;
      for (size_t i = 0; i < data.policies.size(); i++)
	{
	  uint32_t count = BS::weight_gen();
	  for (uint32_t j = 0; j < count; j++)
	    base.learn(ec,i);
	}
    }
  ec.pred.multiclass = action;
  ec.l.multi = ld;
}
 
 void safety(v_array<float>& distribution, float min_prob)
 {
   float added_mass = 0.;
   for (uint32_t i = 0; i < distribution.size();i++)
     if (distribution[i] > 0 && distribution[i] <= min_prob)
       {
	 added_mass += min_prob - distribution[i];
	 distribution[i] = min_prob;
       }
   
   float ratio = 1.f / (1.f + added_mass);
   if (ratio < 0.999)
     {
       for (uint32_t i = 0; i < distribution.size(); i++)
	 if (distribution[i] > min_prob)
	   distribution[i] = distribution[i] * ratio; 
       safety(distribution, min_prob);
     }
 }
 
 void gen_cs_label(cbify& c, CB::cb_class& known_cost, example& ec, COST_SENSITIVE::label& cs_ld, uint32_t label)
 {
   COST_SENSITIVE::wclass wc;
   
   //get cost prediction for this label
   wc.x = CB_ALGS::get_cost_pred<false>(c.reg, &known_cost, ec, label, c.k);
   wc.class_index = label;
   wc.partial_prediction = 0.;
   wc.wap_value = 0.;
   
   //add correction if we observed cost for this action and regressor is wrong
   if( known_cost.action == label ) 
     wc.x += (known_cost.cost - wc.x) / known_cost.probability;
   
   cs_ld.costs.push_back( wc );
 }
 
 template <bool is_learn>
   void predict_or_learn_cover(cbify& data, base_learner& base, example& ec)
 {//Randomize over predictions from a base set of predictors
   //Use cost sensitive oracle to cover actions to form distribution.
   MULTICLASS::label_t ld = ec.l.multi;
   
   data.cs_label.costs.erase();
   for (uint32_t j = 0; j < data.k; j++)
     {
       COST_SENSITIVE::wclass wc;
       
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
   v_array<float>& scores = data.cover->Get_Scores();
   v_array<uint32_t>& predictions = data.cover->predictions;
   
   float additive_probability = 1.f / (float)cover_size;
   
   ec.l.cs = data.cs_label;
   
   float min_prob = epsilon * min(1.f / data.k, 1.f / (float)sqrt(counter * data.k));
   
   vw_context cp = {data, base, ec};
   uint32_t action = data.mwt_explorer->Choose_Action(*data.generic_explorer, to_string((unsigned long long)ec.example_counter), cp);
   
   if (is_learn)
     {
       data.cb_label.costs.erase();
       float probability = data.recorder->probability;
       CB::cb_class l = {loss(ld.label, action), 
			 action, probability};
       data.cb_label.costs.push_back(l);
       ec.l.cb = data.cb_label;
       base.learn(ec);
       
       //Now update oracles
       
       //1. Compute loss vector
       data.cs_label.costs.erase();
       float norm = min_prob * data.k;
       for (uint32_t j = 0; j < data.k; j++)
	 { //data.cs_label now contains an unbiased estimate of cost of each class.
	   gen_cs_label(data, l, ec, data.cs_label, j+1);
	   scores[j] = 0;
	 }
       
       ec.l.cs = data.second_cs_label;
       //2. Update functions
       for (size_t i = 0; i < cover_size; i++)
	 { //get predicted cost-sensitive predictions
	   for (uint32_t j = 0; j < data.k; j++)
	     {
	       float pseudo_cost = data.cs_label.costs[j].x - epsilon * min_prob / (max(scores[j], min_prob) / norm) + 1;
	       data.second_cs_label.costs[j].class_index = j+1;
	       data.second_cs_label.costs[j].x = pseudo_cost;
	     }
	   if (i != 0)
	     data.cs->learn(ec,i+1);
	   if (scores[predictions[i] - 1] < min_prob)
	     norm += max(0, additive_probability - (min_prob - scores[predictions[i] - 1]));
	   else
	     norm += additive_probability;
	   scores[predictions[i] - 1] += additive_probability;
	 }
     }
   
   ec.pred.multiclass = action;
   ec.l.multi = ld;
 }
  
template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; } 

  void finish(cbify& data)
  { CB::cb_label.delete_label(&data.cb_label);
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
      {
	data.cover->predictions.delete_v();
	data.cover->probabilities.delete_v();
      }
    delete_it(data.cover);
    if (data.policies.size() > 0)
      data.policies.~vector();
  }

  base_learner* cbify_setup(vw& all)
  {//parse and set arguments
    if (missing_option<size_t, true>(all, "cbify", "Convert multiclass on <k> classes into a contextual bandit problem"))
      return nullptr;
    new_options(all, "CBIFY options")
      ("first", po::value<size_t>(), "tau-first exploration")
      ("epsilon",po::value<float>() ,"epsilon-greedy exploration")
      ("bag",po::value<size_t>() ,"bagging-based exploration")
      ("cover",po::value<size_t>() ,"bagging-based exploration");
    add_options(all);

    po::variables_map& vm = all.vm;
    cbify& data = calloc_or_die<cbify>();
    data.k = (uint32_t)vm["cbify"].as<size_t>();

    if (count(all.args.begin(), all.args.end(),"--cb") == 0)
      {
	all.args.push_back("--cb");
	stringstream ss;
	ss << vm["cbify"].as<size_t>();
	all.args.push_back(ss.str());
      }
    base_learner* base = setup_base(all);
    
    learner<cbify>* l;
    data.recorder = new vw_recorder();
    data.mwt_explorer = new MwtExplorer<vw_context>("vw", *data.recorder);
    if (vm.count("cover"))
      {
	size_t cover = (uint32_t)vm["cover"].as<size_t>();
	data.cs = all.cost_sensitive;
	data.second_cs_label.costs.resize(data.k);
	data.second_cs_label.costs.end = data.second_cs_label.costs.begin+data.k;
	float epsilon = 0.05f;
	if (vm.count("epsilon")) 
	  epsilon = vm["epsilon"].as<float>();
	data.cover = new vw_cover(epsilon, cover, (u32)data.k);
	data.generic_explorer = new GenericExplorer<vw_context>(*data.cover, (u32)data.k);
	l = &init_multiclass_learner(&data, base, predict_or_learn_cover<true>, 
					     predict_or_learn_cover<false>, all.p, cover + 1);
      }
    else if (vm.count("bag"))
      {
	size_t bags = (uint32_t)vm["bag"].as<size_t>();
	for (size_t i = 0; i < bags; i++)
	  data.policies.push_back(unique_ptr<IPolicy<vw_context>>(new vw_policy(i)));
	data.bootstrap_explorer = new BootstrapExplorer<vw_context>(data.policies, (u32)data.k);
	l = &init_multiclass_learner(&data, base, predict_or_learn_bag<true>, 
					     predict_or_learn_bag<false>, all.p, bags);
      }
    else if (vm.count("first") )
      {
	uint32_t tau = (uint32_t)vm["first"].as<size_t>();
	data.policy = new vw_policy(0);
	data.tau_explorer = new TauFirstExplorer<vw_context>(*data.policy, (u32)tau, (u32)data.k);
	l = &init_multiclass_learner(&data, base, predict_or_learn_first<true>, 
					     predict_or_learn_first<false>, all.p, 1);
      }
    else
      {
	float epsilon = 0.05f;
	if (vm.count("epsilon"))
	  epsilon = vm["epsilon"].as<float>();
	data.policy = new vw_policy(0);
	data.greedy_explorer = new EpsilonGreedyExplorer<vw_context>(*data.policy, epsilon, (u32)data.k);
	l = &init_multiclass_learner(&data, base, predict_or_learn_greedy<true>, 
				     predict_or_learn_greedy<false>, all.p, 1);
      }
    data.reg = all.scorer;
    l->set_finish(finish);
    
    return make_base(*l);
  }
