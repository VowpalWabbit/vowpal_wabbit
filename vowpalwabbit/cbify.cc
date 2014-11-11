#include <float.h>
#include "reductions.h"
#include "multiclass.h"
#include "cost_sensitive.h"
#include "cb.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "../explore/static/MWTExplorer.h"

using namespace LEARNER;
using namespace MultiWorldTesting;

namespace CBIFY {

  struct cbify {
    size_t k;
    
    size_t tau;

    float epsilon;

    size_t counter;

    size_t bags;
    v_array<float> count;
    v_array<uint32_t> predictions;
    
    CB::label cb_label;
    COST_SENSITIVE::label cs_label;
    COST_SENSITIVE::label second_cs_label;

    learner* cs;
    vw* all;
  };
  
  uint32_t do_uniform(cbify& data)
  {  //Draw an action
    return (uint32_t)ceil(frand48() * data.k);
  }

  uint32_t choose_bag(cbify& data)
  {  //Draw an action
    return (uint32_t)floor(frand48() * data.bags);
  }

  float loss(uint32_t label, uint32_t final_prediction)
  {
    if (label != final_prediction)
      return 1.;
    else
      return 0.;
  }

  struct vw_context {
	  learner* l;
	  example* e;
	  cbify* data;
  };

  class vw_policy : public IPolicy<vw_context>
  {
  public:
	  vw_policy() : m_index(-1) { }
	  vw_policy(size_t i) : m_index((int)i) { }

	  u32 Choose_Action(vw_context& ctx)
	  {
		  if (m_index == -1)
		  {
			  ctx.l->predict(*ctx.e);
		  }
		  else
		  {
			  ctx.l->predict(*ctx.e, (size_t)m_index);
		  }
		  return (u32)(((CB::label*)ctx.e->ld)->prediction);
	  }
  private:
	  int m_index;
  };

  template <class Ctx>
  class vw_recorder : public IRecorder<Ctx>
  {
  public:
	  void Record(Ctx& context, u32 action, float probability, string unique_key)
	  {
		  m_action = action;
		  m_prob = probability;
	  }

	  u32 Get_Action() { return m_action; }
	  float Get_Prob() { return m_prob; }

  private:
	  u32 m_action;
	  float m_prob;
  };

  template <bool is_learn>
  void predict_or_learn_first(cbify& data, learner& base, example& ec)
  {//Explore tau times, then act according to optimal.
    MULTICLASS::multiclass* ld = (MULTICLASS::multiclass*)ec.ld;
    //Use CB to find current prediction for remaining rounds.

	vw_context vwc;
	vwc.l = &base;
	vwc.e = &ec;

	vw_recorder<vw_context> recorder;
	MwtExplorer<vw_context> mwt("vw", recorder);

	vw_policy policy;
	TauFirstExplorer<vw_context> explorer(policy, data.tau, data.k);

    if (data.tau && is_learn)
      {
	uint32_t action = mwt.Choose_Action(explorer, to_string(ec.example_counter), vwc);
	ec.loss = loss(ld->label, action);
	data.tau--;
	CB::cb_class l = {ec.loss, action, 1.f / data.k, 0};
	data.cb_label.costs.erase();
	data.cb_label.costs.push_back(l);
	ec.ld = &(data.cb_label);
	base.learn(ec);
	ld->prediction = action;
	ec.loss = l.cost;
      }
    else
      {
	data.cb_label.costs.erase();
	ec.ld = &(data.cb_label);
	ld->prediction = mwt.Choose_Action(explorer, to_string(ec.example_counter), vwc);
	ec.loss = loss(ld->label, ld->prediction);
      }
    ec.ld = ld;
  }

  template <bool is_learn>
  void predict_or_learn_greedy(cbify& data, learner& base, example& ec)
  {//Explore uniform random an epsilon fraction of the time.
    MULTICLASS::multiclass* ld = (MULTICLASS::multiclass*)ec.ld;
    ec.ld = &(data.cb_label);
    data.cb_label.costs.erase();
    
	vw_recorder<vw_context> recorder;
	MwtExplorer<vw_context> mwt("vw", recorder);

	vw_policy policy;
	EpsilonGreedyExplorer<vw_context> explorer(policy, data.epsilon, data.k);

	vw_context vwc;
	vwc.l = &base;
	vwc.e = &ec;
	mwt.Choose_Action(explorer, to_string(ec.example_counter), vwc);

	u32 action = recorder.Get_Action();
	float prob = recorder.Get_Prob();

	CB::cb_class l = { loss(ld->label, action), action, prob };
	data.cb_label.costs.push_back(l);
    
    if (is_learn)
      base.learn(ec);
    
    ld->prediction = action;
    ec.ld = ld;
    ec.loss = loss(ld->label, action);
  }

  template <bool is_learn>
  void predict_or_learn_bag(cbify& data, learner& base, example& ec)
  {//Randomize over predictions from a base set of predictors
    //Use CB to find current predictions.
    MULTICLASS::multiclass* ld = (MULTICLASS::multiclass*)ec.ld;
    ec.ld = &(data.cb_label);
    data.cb_label.costs.erase();

	vw_recorder<vw_context> recorder;
	MwtExplorer<vw_context> mwt("vw", recorder);

	vector<unique_ptr<IPolicy<vw_context>>> policies;
	for (size_t i = 0; i < data.bags; i++)
	{
		policies.push_back(unique_ptr<IPolicy<vw_context>>(new vw_policy(i)));
	}
	BaggingExplorer<vw_context> explorer(policies, data.bags, data.k);

	vw_context context;
	context.l = &base;
	context.e = &ec;
	uint32_t action = mwt.Choose_Action(explorer, to_string(ec.example_counter), context);

    assert(action != 0);
    if (is_learn)
      {
	assert(action == recorder.Get_Action());
	float probability = recorder.Get_Prob();

	CB::cb_class l = {loss(ld->label, action), 
			  action, probability};
	data.cb_label.costs.push_back(l);
	for (size_t i = 0; i < data.bags; i++)
	  {
	    uint32_t count = BS::weight_gen();
	    for (uint32_t j = 0; j < count; j++)
	      base.learn(ec,i);
	  }
      }
    ld->prediction = action;
    ec.ld = ld;
  }
  
  uint32_t choose_action(v_array<float>& distribution)
  {
    float value = frand48();
    for (uint32_t i = 0; i < distribution.size();i++)
      {
	if (value <= distribution[i])
	  return i+1;	    
	else
	  value -= distribution[i];
      }
    //some rounding problem presumably.
    return 1;
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

  void gen_cs_label(vw& all, CB::cb_class& known_cost, example& ec, COST_SENSITIVE::label& cs_ld, uint32_t label)
  {
    COST_SENSITIVE::wclass wc;
    
    //get cost prediction for this label
    wc.x = CB_ALGS::get_cost_pred<false>(all, &known_cost, ec, label, all.sd->k);
    wc.class_index = label;
    wc.partial_prediction = 0.;
    wc.wap_value = 0.;
    
    //add correction if we observed cost for this action and regressor is wrong
    if( known_cost.action == label ) 
      wc.x += (known_cost.cost - wc.x) / known_cost.probability;
    
    cs_ld.costs.push_back( wc );
  }

  class vw_scorer : public IScorer<vw_context>
  {
  public:
	  vector<float> Score_Actions(vw_context& ctx)
	  {
		  float additive_probability = 1.f / (float)ctx.data->bags;
		  for (size_t i = 0; i < ctx.data->bags; i++)
		  { //get predicted cost-sensitive predictions
			  if (i == 0)
				  ctx.data->cs->predict(*ctx.e, i);
			  else
				  ctx.data->cs->predict(*ctx.e, i + 1);
			  ctx.data->count[ctx.data->cs_label.prediction - 1] += additive_probability;
			  ctx.data->predictions[i] = (uint32_t)ctx.data->cs_label.prediction;
		  }
		  float min_prob = ctx.data->epsilon * min(1.f / ctx.data->k, 1.f / (float)sqrt(ctx.data->counter * ctx.data->k));

		  safety(ctx.data->count, min_prob);

		  vector<float> scores;
		  for (size_t i = 0; i < ctx.data->k; i++)
		  {
			  scores.push_back(ctx.data->count[i]);
		  }
		  return scores;
	  }
  };

  template <bool is_learn>
  void predict_or_learn_cover(cbify& data, learner& base, example& ec)
  {//Randomize over predictions from a base set of predictors
    //Use cost sensitive oracle to cover actions to form distribution.
    MULTICLASS::multiclass* ld = (MULTICLASS::multiclass*)ec.ld;
    data.counter++;

    data.count.erase();
    data.cs_label.costs.erase();
    for (uint32_t j = 0; j < data.k; j++)
      {
	data.count.push_back(0);

	COST_SENSITIVE::wclass wc;
	
	//get cost prediction for this label
	wc.x = FLT_MAX;
	wc.class_index = j+1;
	wc.partial_prediction = 0.;
	wc.wap_value = 0.;
	data.cs_label.costs.push_back(wc);
      }

    float additive_probability = 1.f / (float)data.bags;

    ec.ld = &data.cs_label;

    float min_prob = data.epsilon * min (1.f / data.k, 1.f / (float)sqrt(data.counter * data.k));
    
	vw_recorder<vw_context> recorder;
	MwtExplorer<vw_context> mwt("vw", recorder);

	vw_scorer scorer;
	GenericExplorer<vw_context> explorer(scorer, data.k);

	vw_context cp;
	cp.data = &data;
	cp.e = &ec;
	uint32_t action = mwt.Choose_Action(explorer, to_string(ec.example_counter), cp);
	
    if (is_learn)
      {
	data.cb_label.costs.erase();
	float probability = recorder.Get_Prob();
	CB::cb_class l = {loss(ld->label, action), 
			  action, probability};
	data.cb_label.costs.push_back(l);
	ec.ld = &(data.cb_label);
	base.learn(ec);

	//Now update oracles
	
	//1. Compute loss vector
	data.cs_label.costs.erase();
	float norm = min_prob * data.k;
	for (uint32_t j = 0; j < data.k; j++)
	  { //data.cs_label now contains an unbiased estimate of cost of each class.
	    gen_cs_label(*data.all, l, ec, data.cs_label, j+1);
	    data.count[j] = 0;
	  }
	
	ec.ld = &data.second_cs_label;
	//2. Update functions
	for (size_t i = 0; i < data.bags; i++)
	  { //get predicted cost-sensitive predictions
	    for (uint32_t j = 0; j < data.k; j++)
	      {
		float pseudo_cost = data.cs_label.costs[j].x - data.epsilon * min_prob / (max(data.count[j], min_prob) / norm) + 1;
		data.second_cs_label.costs[j].class_index = j+1;
		data.second_cs_label.costs[j].x = pseudo_cost;
	      }
	    if (i != 0)
	      data.cs->learn(ec,i+1);
	    if (data.count[data.predictions[i]-1] < min_prob)
	      norm += max(0, additive_probability - (min_prob - data.count[data.predictions[i]-1]));
	    else
	      norm += additive_probability;
	    data.count[data.predictions[i]-1] += additive_probability;
	  }
      }

    ld->prediction = action;
    ec.ld = ld;
  }
  
  void init_driver(cbify&) {}

  void finish_example(vw& all, cbify&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  void finish(cbify& data)
  {
    CB::cb_label.delete_label(&data.cb_label);
  }

  learner* setup(vw& all, po::variables_map& vm)
  {//parse and set arguments
    cbify* data = (cbify*)calloc_or_die(1, sizeof(cbify));

    data->epsilon = 0.05f;
    data->counter = 0;
    data->tau = 1000;
    data->all = &all;
    po::options_description cb_opts("CBIFY options");
    cb_opts.add_options()
      ("first", po::value<size_t>(), "tau-first exploration")
      ("epsilon",po::value<float>() ,"epsilon-greedy exploration")
      ("bag",po::value<size_t>() ,"bagging-based exploration")
      ("cover",po::value<size_t>() ,"bagging-based exploration");
    
    vm = add_options(all, cb_opts);
    
    data->k = (uint32_t)vm["cbify"].as<size_t>();
    
    //appends nb_actions to options_from_file so it is saved to regressor later
    std::stringstream ss;
    ss << " --cbify " << data->k;
    all.file_options.append(ss.str());

    all.p->lp = MULTICLASS::mc_label;
    learner* l;
    if (vm.count("cover"))
      {
	data->bags = (uint32_t)vm["cover"].as<size_t>();
	data->cs = all.cost_sensitive;
	data->count.resize(data->k+1);
	data->predictions.resize(data->bags);
	data->second_cs_label.costs.resize(data->k);
	data->second_cs_label.costs.end = data->second_cs_label.costs.begin+data->k;
	if ( vm.count("epsilon") ) 
	  data->epsilon = vm["epsilon"].as<float>();
	l = new learner(data, all.l, data->bags + 1);
	l->set_learn<cbify, predict_or_learn_cover<true> >();
	l->set_predict<cbify, predict_or_learn_cover<false> >();
      }
    else if (vm.count("bag"))
      {
	data->bags = (uint32_t)vm["bag"].as<size_t>();
	data->count.resize(data->k+1);
	l = new learner(data, all.l, data->bags);
	l->set_learn<cbify, predict_or_learn_bag<true> >();
	l->set_predict<cbify, predict_or_learn_bag<false> >();
      }
    else if (vm.count("first") )
      {
	data->tau = (uint32_t)vm["first"].as<size_t>();
	l = new learner(data, all.l, 1);
	l->set_learn<cbify, predict_or_learn_first<true> >();
	l->set_predict<cbify, predict_or_learn_first<false> >();
      }
    else
      {
	if ( vm.count("epsilon") ) 
	  data->epsilon = vm["epsilon"].as<float>();
	l = new learner(data, all.l, 1);
	l->set_learn<cbify, predict_or_learn_greedy<true> >();
	l->set_predict<cbify, predict_or_learn_greedy<false> >();
      }

    l->set_finish_example<cbify,finish_example>();
    l->set_finish<cbify,finish>();
    l->set_init_driver<cbify,init_driver>();
    
    return l;
  }
}
