#include <float.h>
#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "vw.h"
#include "cb_explore.h"
#include "gen_cs_example.h"
#include "learner.h"
#include "mwt.h"
//#include "action_score.h"

using namespace LEARNER;
using namespace ACTION_SCORE;

//All exploration algorithms return a vector of probabilities, to be used by GenericExplorer downstream

namespace CB_EXPLORE{

  struct cb_explore;

  void safety(v_array<float>& distribution, float min_prob);


  struct cb_explore
  {
    cb_to_cs cbcs;
    v_array<uint32_t> preds;
    v_array<float> cover_probs;

    bool learn_only;

    CB::label cb_label;
    COST_SENSITIVE::label cs_label;
    COST_SENSITIVE::label second_cs_label;

    base_learner* cs;

    size_t tau;
    float epsilon;
    size_t bag_size;
    size_t cover_size;
    
    size_t counter;
  
  };


  template <bool is_learn>
  void predict_or_learn_first(cb_explore& data, base_learner& base, example& ec)
  { //Explore tau times, then act according to optimal.
    
    v_array<action_score> probs = ec.pred.a_s;
    probs.erase();

    if(!is_learn || !data.learn_only) {      
      if(data.tau) {
	float prob = 1.0/(float)data.cbcs.num_actions;
	for(int i = 0;i < data.cbcs.num_actions;i++) {
	  action_score a_s;
	  a_s.action = i;
	  a_s.score = prob;
	  probs.push_back(a_s);
	}
	data.tau--;
      }
      else {
	base.predict(ec);
	uint32_t chosen = ec.pred.multiclass-1;
	for(int i = 0;i < data.cbcs.num_actions;i++) {
	  action_score a_s;
	  a_s.action = i;
	  a_s.score = 0.;
	  probs.push_back(a_s);
	}
	probs[chosen].score = 1.0;
      }    
    }
    
    if (is_learn && ec.l.cb.costs[0].probability < 1) 
      base.learn(ec);

    ec.pred.a_s = probs;
  }

  template <bool is_learn>
  void predict_or_learn_greedy(cb_explore& data, base_learner& base, example& ec)
  { //Explore uniform random an epsilon fraction of the time.

    v_array<action_score> probs = ec.pred.a_s;
    probs.erase();

    if(!is_learn || !data.learn_only) {
      float prob = data.epsilon/(float)data.cbcs.num_actions;
      for(int i = 0;i < data.cbcs.num_actions;i++) {
	action_score a_s;
	a_s.action = i;
	a_s.score = prob;
	probs.push_back(a_s);
      }
      base.predict(ec);
      uint32_t chosen = ec.pred.multiclass-1;
      probs[chosen].score += (1-data.epsilon);
    }
        
    
    if (is_learn)
      base.learn(ec);
    
    ec.pred.a_s = probs;    
  }

  template <bool is_learn>
  void predict_or_learn_bag(cb_explore& data, base_learner& base, example& ec)
  { //Randomize over predictions from a base set of predictors

    v_array<action_score> probs = ec.pred.a_s;
    probs.erase();

    if(!is_learn || !data.learn_only) {
      for(int i = 0;i < data.cbcs.num_actions;i++) {
	action_score a_s;
	a_s.action = i;
	a_s.score = 0.;
	probs.push_back(a_s);
      }
      float prob = 1.0/(float)data.bag_size;
      for(int i = 0;i < data.bag_size;i++) {
	base.predict(ec, i);
	uint32_t chosen = ec.pred.multiclass-1;
	probs[chosen].score += prob;
      }
    }

    if (is_learn)
      for (size_t i = 0; i < data.bag_size; i++)
	{ uint32_t count = BS::weight_gen();
	  for (uint32_t j = 0; j < count; j++)
	    base.learn(ec,i);
	}
    
    ec.pred.a_s = probs;
  }

  void safety(v_array<action_score>& distribution, float min_prob)
  { float added_mass = 0.;
    for (uint32_t i = 0; i < distribution.size(); i++)
      if (distribution[i].score > 0 && distribution[i].score <= min_prob)
	{ added_mass += min_prob - distribution[i].score;
	  distribution[i].score = min_prob;
	}

    float ratio = 1.f / (1.f + added_mass);
    if (ratio < 0.999)
      { for (uint32_t i = 0; i < distribution.size(); i++)
	  if (distribution[i].score > min_prob)
	    distribution[i].score = distribution[i].score * ratio;
	safety(distribution, min_prob);
      }
  }

  void get_cover_probabilities(cb_explore& data, base_learner& base, example& ec, v_array<action_score>& probs)
  { 
    float additive_probability = 1.f / (float)data.cover_size;
    data.preds.erase();

    for(uint32_t i = 0;i < data.cbcs.num_actions;i++) {
      action_score a_s;
      a_s.action = i;
      a_s.score = 0.;
      probs.push_back(a_s);
    }

    for (size_t i = 0; i < data.cover_size; i++)
      { //get predicted cost-sensitive predictions
	if (i == 0)
	  data.cs->predict(ec, i);
	else
	  data.cs->predict(ec, i + 1);
	uint32_t pred = ec.pred.multiclass;
	probs[pred - 1].score += additive_probability;
	data.preds.push_back((uint32_t)pred);
      }
    uint32_t num_actions = data.cbcs.num_actions;
    float epsilon = data.epsilon;
    
    float min_prob = epsilon * min(1.f / num_actions, 1.f / (float)sqrt(data.counter * num_actions));
    
    safety(probs, min_prob);
    
    data.counter++;
  }

  template <bool is_learn>
  void predict_or_learn_cover(cb_explore& data, base_learner& base, example& ec)
  { //Randomize over predictions from a base set of predictors
    //Use cost sensitive oracle to cover actions to form distribution.

    uint32_t num_actions = data.cbcs.num_actions;

    v_array<action_score> probs = ec.pred.a_s;
    probs.erase();
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

    float epsilon = data.epsilon;
    size_t cover_size = data.cover_size;
    size_t counter = data.counter;
    v_array<float>& probabilities = data.cover_probs;
    v_array<uint32_t>& predictions = data.preds;

    float additive_probability = 1.f / (float)cover_size;

    float min_prob = epsilon * min(1.f / num_actions, 1.f / (float)sqrt(counter * num_actions));

    data.cb_label = ec.l.cb;

    ec.l.cs = data.cs_label;
    get_cover_probabilities(data, base, ec, probs);
	
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
	      //cout<<pseudo_cost<<" ";
	    }
	  //cout<<epsilon<<" "<<endl;
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
    ec.pred.a_s = probs;
  }

  template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

  void finish(cb_explore& data)
  { data.preds.delete_v();
    data.cover_probs.delete_v();
    cb_to_cs& c = data.cbcs;
    COST_SENSITIVE::cs_label.delete_label(&c.pred_scores);
    COST_SENSITIVE::cs_label.delete_label(&data.cs_label);
    COST_SENSITIVE::cs_label.delete_label(&data.second_cs_label);
  }

  void print_update_cb_explore(vw& all, bool is_test, example& ec, stringstream& pred_string)
  { if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
	stringstream label_string;
	if (is_test)
	  label_string << " unknown";
	else
	  label_string << ec.l.cb.costs[0].action;
	all.sd->print_update(all.holdout_set_off, all.current_pass, label_string.str(), pred_string.str(), ec.num_features, all.progress_add, all.progress_arg);
      }
  }

  void output_example(vw& all, cb_explore& data, example& ec, CB::label& ld)
  { float loss = 0.;

    cb_to_cs& c = data.cbcs;
  
    if ((c.known_cost = get_observed_cost(ld)) != nullptr)
      for(uint32_t i = 0;i < ec.pred.a_s.size();i++)
	loss += get_unbiased_cost(c.known_cost, c.pred_scores, i)*ec.pred.a_s[i].score;
  
    all.sd->update(ec.test_only, loss, 1.f, ec.num_features);
    
    char temp_str[20];
    stringstream ss, sso;
    float maxprob = 0.;
    uint32_t maxid;
    //cout<<ec.pred.scalars.size()<<endl;
    for(uint32_t i = 0;i < ec.pred.a_s.size();i++) {
      sprintf(temp_str,"%f ", ec.pred.a_s[i].score);
      ss << temp_str;
      if(ec.pred.a_s[i].score > maxprob) {
	maxprob = ec.pred.a_s[i].score;
	maxid = i+1;
      }
    }

    sprintf(temp_str, "%d:%f", maxid, maxprob);
    sso << temp_str;
    //cout<<sso.str()<<endl;
    
    for (int sink : all.final_prediction_sink)
      all.print_text(sink, ss.str(), ec.tag);

    print_update_cb_explore(all, is_test_label(ld), ec, sso);
  }

  void finish_example(vw& all, cb_explore& c, example& ec)
  {   
    output_example(all, c, ec, ec.l.cb);
    VW::finish_example(all, &ec);
  }
}
using namespace CB_EXPLORE;


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

  if (count(all.args.begin(), all.args.end(),"--cb") == 0)
    { all.args.push_back("--cb");
      stringstream ss;
      ss << vm["cb_explore"].as<size_t>();
      all.args.push_back(ss.str());
    }

  char type_string[30];
  if(vm.count("learn_only"))
    data.learn_only = true;
  else
    data.learn_only = false;

  data.cbcs.cb_type = CB_TYPE_DR;
  all.delete_prediction = delete_action_scores;
  //ALEKH: Others TBD later
  // if (count(all.args.begin(), all.args.end(), "--cb_type") == 0)
  //   data.cbcs->cb_type = CB_TYPE_DR;
  // else
  //   data.cbcs->cb_type = (size_t)vm["cb_type"].as<size_t>();

  base_learner* base = setup_base(all);

  learner<cb_explore>* l;
  if (vm.count("cover"))
    { data.cover_size = (uint32_t)vm["cover"].as<size_t>();
      data.cs = all.cost_sensitive;
      data.second_cs_label.costs.resize(num_actions);
      data.second_cs_label.costs.end() = data.second_cs_label.costs.begin()+num_actions;
      data.epsilon = 0.05f;
	  sprintf(type_string, "%lu", data.cover_size);
	  *all.file_options << " --cover " << type_string;

      if (vm.count("epsilon"))
	data.epsilon = vm["epsilon"].as<float>();
      data.cover_probs = v_init<float>();
      data.cover_probs.resize(num_actions);
      data.preds = v_init<uint32_t>();
      data.preds.resize(data.cover_size);
	  sprintf(type_string, "%f", data.epsilon);
	  *all.file_options << " --epsilon " << type_string;
      l = &init_learner(&data, base, predict_or_learn_cover<true>, predict_or_learn_cover<false>, data.cover_size + 1);
    }
  else if (vm.count("bag"))
    { data.bag_size = (uint32_t)vm["bag"].as<size_t>();
      sprintf(type_string, "%lu", data.bag_size);
      *all.file_options << " --bag "<<type_string;
      l = &init_learner(&data, base, predict_or_learn_bag<true>, predict_or_learn_bag<false>, data.bag_size);
    }
  else if (vm.count("first") )
    { data.tau = (uint32_t)vm["first"].as<size_t>();
      sprintf(type_string, "%lu", data.tau);
      *all.file_options << " --first "<<type_string;
      l = &init_learner(&data, base, predict_or_learn_first<true>, predict_or_learn_first<false>, 1);
    }
  else
    { data.epsilon = 0.05f;
      if (vm.count("epsilon"))
	data.epsilon = vm["epsilon"].as<float>();
      sprintf(type_string, "%f", data.epsilon);
      *all.file_options << " --epsilon "<<type_string;
      l = &init_learner(&data, base, predict_or_learn_greedy<true>, predict_or_learn_greedy<false>, 1);
    }
  data.cbcs.scorer = all.scorer;
  l->set_finish(finish);
  l->set_finish_example(finish_example);
  return make_base(*l);
}

