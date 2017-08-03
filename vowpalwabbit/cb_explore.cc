#include "reductions.h"
#include "cb_algs.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"

using namespace LEARNER;
using namespace ACTION_SCORE;
using namespace GEN_CS;
using namespace std;
using namespace CB_ALGS;
//All exploration algorithms return a vector of probabilities, to be used by GenericExplorer downstream

namespace CB_EXPLORE
{
  
  struct cb_explore
  {
    vw* all;
    cb_to_cs cbcs;
    v_array<uint32_t> preds;
    v_array<float> cover_probs;
    
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

  if (is_learn && ec.l.cb.costs[0].probability < 1)
    base.learn(ec);
  else
    base.predict(ec);

  probs.erase();
  if(data.tau > 0)
  { float prob = 1.f/(float)data.cbcs.num_actions;
    for(uint32_t i = 0; i < data.cbcs.num_actions; i++)
      probs.push_back({i,prob});
    data.tau--;
  }
  else
  { uint32_t chosen = ec.pred.multiclass-1;
    for(uint32_t i = 0; i < data.cbcs.num_actions; i++)
      probs.push_back({i,0.});
    probs[chosen].score = 1.0;
  }

  ec.pred.a_s = probs;
}

template <bool is_learn>
void predict_or_learn_greedy(cb_explore& data, base_learner& base, example& ec)
{ //Explore uniform random an epsilon fraction of the time.

  v_array<action_score> probs = ec.pred.a_s;
  probs.erase();

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  float prob = data.epsilon/(float)data.cbcs.num_actions;
  for(uint32_t i = 0; i < data.cbcs.num_actions; i++)
    probs.push_back({i,prob});
  uint32_t chosen = ec.pred.multiclass-1;
  probs[chosen].score += (1-data.epsilon);

  ec.pred.a_s = probs;
}

template <bool is_learn>
void predict_or_learn_bag(cb_explore& data, base_learner& base, example& ec)
{ //Randomize over predictions from a base set of predictors
  v_array<action_score> probs = ec.pred.a_s;
  probs.erase();
  
  for(uint32_t i = 0;i < data.cbcs.num_actions;i++)
    probs.push_back({i,0.});
  float prob = 1.f/(float)data.bag_size;
  for(size_t i = 0;i < data.bag_size;i++) {
    uint32_t count = BS::weight_gen(*data.all);
    if (is_learn && count > 0)
      base.learn(ec,i);
    else
      base.predict(ec, i);
    uint32_t chosen = ec.pred.multiclass-1;
    probs[chosen].score += prob;
    if (is_learn)
      for (uint32_t j = 1; j < count; j++)
	base.learn(ec,i);
  }
  
  ec.pred.a_s = probs;
}

void safety(v_array<action_score>& distribution, float min_prob, bool zeros)
{ //input: a probability distribution
  //output: a probability distribution with all events having probability > min_prob.  This includes events with probability 0 if zeros = true
  min_prob /= distribution.size();
  float touched_mass = 0.;
  float untouched_mass = 0.;
  for (uint32_t i = 0; i < distribution.size(); i++)
    if ((distribution[i].score > 0 || (distribution[i].score ==0 && zeros)) && distribution[i].score <= min_prob)
    { touched_mass += min_prob;
      distribution[i].score = min_prob;
    }
    else
      untouched_mass += distribution[i].score;

  if (touched_mass > 0.)
  { if (touched_mass > 0.999)
      THROW("Cannot safety this distribution");
    float ratio = (1.f - touched_mass) / untouched_mass;
    for (uint32_t i = 0; i < distribution.size(); i++)
      if (distribution[i].score > min_prob)
        distribution[i].score = distribution[i].score * ratio;
  }
}

void get_cover_probabilities(cb_explore& data, base_learner& base, example& ec, v_array<action_score>& probs)
{ float additive_probability = 1.f / (float)data.cover_size;
  data.preds.erase();

  for(uint32_t i = 0; i < data.cbcs.num_actions; i++)
    probs.push_back({i,0.});

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

  safety(probs, min_prob*num_actions, false);

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
    data.cs_label.costs.push_back({FLT_MAX,j+1,0.,0.});

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

  if (is_learn)
  { ec.l.cb = data.cb_label;
    base.learn(ec);

    //Now update oracles

    //1. Compute loss vector
    data.cs_label.costs.erase();
    float norm = min_prob * num_actions;
    ec.l.cb = data.cb_label;
    data.cbcs.known_cost = get_observed_cost(data.cb_label);
    gen_cs_example<false>(data.cbcs, ec, data.cb_label, data.cs_label);
    for(uint32_t i = 0; i < num_actions; i++)
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

void finish(cb_explore& data)
{ data.preds.delete_v();
  data.cover_probs.delete_v();
  cb_to_cs& c = data.cbcs;
  COST_SENSITIVE::cs_label.delete_label(&c.pred_scores);
  COST_SENSITIVE::cs_label.delete_label(&data.cs_label);
  COST_SENSITIVE::cs_label.delete_label(&data.second_cs_label);
}

void print_update_cb_explore(vw& all, bool is_test, example& ec, stringstream& pred_string)
{ if (all.sd->weighted_examples() >= all.sd->dump_interval && !all.quiet && !all.bfgs)
  { stringstream label_string;
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
    for(uint32_t i = 0; i < ec.pred.a_s.size(); i++)
      loss += get_unbiased_cost(c.known_cost, c.pred_scores, i)*ec.pred.a_s[i].score;
  
  all.sd->update(ec.test_only, get_observed_cost(ld) != nullptr, loss, 1.f, ec.num_features);

  char temp_str[20];
  stringstream ss, sso;
  float maxprob = 0.;
  uint32_t maxid;
  //cout<<ec.pred.scalars.size()<<endl;
  for(uint32_t i = 0; i < ec.pred.a_s.size(); i++)
  { sprintf(temp_str,"%f ", ec.pred.a_s[i].score);
    ss << temp_str;
    if(ec.pred.a_s[i].score > maxprob)
    { maxprob = ec.pred.a_s[i].score;
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
{ output_example(all, c, ec, ec.l.cb);
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
  ("cover",po::value<size_t>() ,"Online cover based exploration");
  add_options(all);

  po::variables_map& vm = all.vm;
  cb_explore& data = calloc_or_throw<cb_explore>();
  data.all = &all;
  data.cbcs.num_actions = (uint32_t)vm["cb_explore"].as<size_t>();
  uint32_t num_actions = data.cbcs.num_actions;

  if (count(all.args.begin(), all.args.end(),"--cb") == 0)
  { all.args.push_back("--cb");
    stringstream ss;
    ss << vm["cb_explore"].as<size_t>();
    all.args.push_back(ss.str());
  }

  char type_string[30];

  all.delete_prediction = delete_action_scores;
  data.cbcs.cb_type = CB_TYPE_DR;
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
    *all.file_options << " --cover " << data.cover_size;

    if (vm.count("epsilon"))
      data.epsilon = vm["epsilon"].as<float>();
    data.cover_probs = v_init<float>();
    data.cover_probs.resize(num_actions);
    data.preds = v_init<uint32_t>();
    data.preds.resize(data.cover_size);
    sprintf(type_string, "%f", data.epsilon);
    *all.file_options << " --epsilon " << type_string;
    l = &init_learner(&data, base, predict_or_learn_cover<true>, predict_or_learn_cover<false>, data.cover_size + 1, prediction_type::action_probs);
  }
  else if (vm.count("bag"))
  { data.bag_size = (uint32_t)vm["bag"].as<size_t>();
    *all.file_options << " --bag "<< data.bag_size;
    l = &init_learner(&data, base, predict_or_learn_bag<true>, predict_or_learn_bag<false>, data.bag_size, prediction_type::action_probs);
  }
  else if (vm.count("first") )
  { data.tau = (uint32_t)vm["first"].as<size_t>();
    *all.file_options << " --first "<< data.tau;
    l = &init_learner(&data, base, predict_or_learn_first<true>, predict_or_learn_first<false>, 1, prediction_type::action_probs);
  }
  else
  { data.epsilon = 0.05f;
    if (vm.count("epsilon"))
      data.epsilon = vm["epsilon"].as<float>();
    sprintf(type_string, "%f", data.epsilon);
    *all.file_options << " --epsilon "<<type_string;
    l = &init_learner(&data, base, predict_or_learn_greedy<true>, predict_or_learn_greedy<false>, 1, prediction_type::action_probs);
  }
  data.cbcs.scorer = all.scorer;
  l->set_finish(finish);
  l->set_finish_example(finish_example);
  return make_base(*l);
}

