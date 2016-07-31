#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"

using namespace LEARNER;
using namespace ACTION_SCORE;
using namespace std;
using namespace CB_ALGS;
//All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities are the probability with which each action should be replaced to the top of the list. 

//tau first
#define EXPLORE_FIRST 0
//epsilon greedy
#define EPS_GREEDY 1
// bagging explorer
#define BAG_EXPLORE 2
//softmax
#define SOFTMAX 3
//cover
#define COVER 4

namespace CB_EXPLORE_ADF{

  struct cb_explore_adf
  {

    v_array<example*> ec_seq;
    v_array<action_score> action_probs;

    size_t explore_type;

    size_t tau;
    float epsilon;
    size_t bag_size;
    size_t cover_size;
    float lambda;

    bool need_to_clear;
    vw* all;
    LEARNER::base_learner* cs_ldf_learner;
    CB::cb_class known_cost;
  };

  template<class T> void swap(T& ele1, T& ele2)
  {
    T temp = ele2;
    ele2 = ele1;
    ele1 = temp;
  }

  void multiline_learn(base_learner& base, v_array<example*>& examples, uint32_t id = 0)
  { for (example* ec : examples) base.learn(*ec, id);}
  void multiline_predict(base_learner& base, v_array<example*>& examples, uint32_t id = 0)
  { for (example* ec : examples) base.predict(*ec, id);}

  bool test_adf_sequence(cb_explore_adf& data)
  {
    uint32_t count = 0;
    for (size_t k = 0; k < data.ec_seq.size(); k++)
    {
      example *ec = data.ec_seq[k];

      if (ec->l.cb.costs.size() > 1)
        THROW("cb_adf: badly formatted example, only one cost can be known.");

      if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
        count += 1;

      if (CB::ec_is_example_header(*ec))
        if (k != 0)
          THROW("warning: example headers at position " << k << ": can only have in initial position!");
    }
    if (count == 0)
      return true;
    else if (count == 1)
      return false;
    else
      THROW("cb_adf: badly formatted example, only one line can have a cost");
  }
  
  template <bool is_learn>
  void predict_or_learn_first(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  { //Explore tau times, then act according to optimal.
    if (is_learn && data.known_cost.probability < 1 && !test_adf_sequence(data))
      multiline_learn(base, examples);
    else
      multiline_predict(base, examples);

    v_array<action_score>& preds = examples[0]->pred.a_s;
    uint32_t num_actions = (uint32_t)preds.size();

    if (data.tau) {
      float prob = 1.f / (float)num_actions;
      for (size_t i = 0; i < num_actions; i++) 
	preds[i].score = prob;
      data.tau--;
    }
    else {
      for (size_t i = 1; i < num_actions; i++) 
	preds[i].score = 0.;
      preds[0].score = 1.0;
    }
    CB_EXPLORE::safety(preds, data.epsilon, true);
  }
  
  template <bool is_learn>
  void predict_or_learn_greedy(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  { //Explore uniform random an epsilon fraction of the time.
    if (is_learn && !test_adf_sequence(data))
      multiline_learn(base, examples);
    else
      multiline_predict(base, examples);

    v_array<action_score>& preds = examples[0]->pred.a_s;
    uint32_t num_actions = (uint32_t)preds.size();

    float prob = data.epsilon/(float)num_actions;
    for (size_t i = 0; i < num_actions; i++)
      preds[i].score = prob;
    preds[0].score += 1.f - data.epsilon;
  }

  template <bool is_learn>
  void predict_or_learn_bag(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  { //Randomize over predictions from a base set of predictors
    v_array<action_score>& preds = examples[0]->pred.a_s;
    uint32_t num_actions = (uint32_t)(examples.size() - 1);
    if (CB::ec_is_example_header(*examples[0]))
      num_actions--;

    data.action_probs.resize(num_actions);
    for (uint32_t i = 0; i < num_actions; i++)
      data.action_probs[i] = {i,0.};
    float prob = 1.f / (float)data.bag_size;
    bool test_sequence = test_adf_sequence(data);
    for (uint32_t i = 0; i < data.bag_size; i++) 
      {
	uint32_t count = BS::weight_gen();
	if (is_learn && count > 0 && !test_sequence)
	  multiline_learn(base, examples, i);
	else
	  multiline_predict(base, examples, i);
	assert(preds.size() == num_actions);
	data.action_probs[preds[0].action].score += prob;
	if (is_learn && !test_sequence)
	  for (uint32_t j = 1; j < count; j++)
	    multiline_learn(base, examples, i);
      }
    
    CB_EXPLORE::safety(data.action_probs, data.epsilon, true);
    
    for (size_t i = 0; i < num_actions; i++) 
      preds[i].score = data.action_probs[preds[i].action].score;
  }
  
  /*
  void get_cover_probabilities(cb_explore_adf& data, base_learner& base, v_array<example*>& examples, v_array<action_score>& preds, uint32_t num_actions)
  { 
    float additive_probability = 1.f / (float)data.cover_size;
    data.base_predictions.erase();
    
    for(uint32_t i = 0;i < num_actions;i++)
      preds.push_back({i,0.});
    
    for (size_t i = 0; i < data.cover_size; i++)
      { //get predicted cost-sensitive predictions
	if (i == 0)
	  data.cs_ldf_learner->predict(ec, i);
	else
	  data.cs_ldf_learner->predict(ec, i + 1);
	uint32_t pred = ec.pred.multiclass;
	preds[pred - 1].score += additive_probability;
	data.preds.push_back((uint32_t)pred);
      }
    
    safety(probs, data.epsilon / num_actions, false);
    }*/
  /*    
  template <bool is_learn>
  void predict_or_learn_cover(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  { //Randomize over predictions from a base set of predictors
    //Use cost sensitive oracle to cover actions to form distribution.
    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    v_array<action_score>& preds = ec.pred.a_s;
    uint32_t num_actions = preds.size();
    
    float additive_probability = 1.f / (float)data.cover_size;
    float min_prob = data.epsilon * min(1.f / num_actions);
    v_array<action_score>& probs = data.action_probs;
    for(uint32_t i = 0;i < num_actions;i++)
      probs[i] = {i,0.};

    //1. Compute loss vector
    data.cs_label.costs.erase();
    float norm = min_prob * num_actions;
    data.cbcs.known_cost = get_observed_cost(data.cb_label);
    gen_cs_example<false>(data.cbcs, ec, data.cb_label, data.cs_label);
    
    ec.l.cs = data.second_cs_label;
    //2. Update functions
    for (size_t i = 0; i < data.cover_size; i++)
      { //Create costs of each action based on online cover
	for (uint32_t j = 0; j < num_actions; j++)
	  { float pseudo_cost = data.cs_label.costs[j].x - data.epsilon * min_prob / (max(action_scores[j].score, min_prob) / norm) + 1;
	    data.second_cs_label.costs[j].class_index = j+1;
	    data.second_cs_label.costs[j].x = pseudo_cost;
	  }
	if (i != 0)
	  if (is_learn)
	    data.cs->learn(ec,i+1);
	  else
	    data.cs->predict(ec,i+1);
	if (actions_scores[predictions[i] - 1].score < min_prob)
	  norm += max(0, additive_probability - (min_prob - action_scores[predictions[i] - 1].score));
	else
	  norm += additive_probability;
	action_scores[predictions[i] - 1].score += additive_probability;
      }
    
    ec.l.cb = data.cb_label;

    CB_EXPLORE::safety(data.action_probs, data.epsilon, true);
    for (size_t i = 0; i < num_actions; i++) 
      preds[i].score = data.action_probs[preds[i].action].score;
      }*/
  
  template <bool is_learn>
  void predict_or_learn_softmax(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  {
    if (is_learn && !test_adf_sequence(data)) 
      multiline_learn(base, examples);
    else
      multiline_predict(base, examples);
    
    v_array<action_score>& preds = examples[0]->pred.a_s;
    uint32_t num_actions = (uint32_t)preds.size();
    float norm = 0.;
    float max_score = preds[0].score;

    for (size_t i = 0; i < num_actions; i++) {
      float prob = exp(data.lambda*(preds[i].score - max_score));
      preds[i].score = prob;
      norm += prob;
    }
    for (size_t i = 0; i < num_actions; i++)
      preds[i].score /= norm;
    CB_EXPLORE::safety(preds, data.epsilon, true);
  }
  
  void end_examples(cb_explore_adf& data)
  {
    if (data.need_to_clear)
      data.ec_seq.erase();
  }

  template<class T> inline void delete_it(T* p) { if (p != nullptr) delete p; }

  void finish(cb_explore_adf& data)
  {
    data.ec_seq.delete_v();
    data.action_probs.delete_v();
  }


  //Semantics: Currently we compute the IPS loss no matter what flags
  //are specified. We print the first action and probability, based on
  //ordering by scores in the final output.

  void output_example(vw& all, cb_explore_adf& c, example& ec, v_array<example*>* ec_seq)
  {
    if (CB_ALGS::example_is_newline_not_header(ec)) return;

    size_t num_features = 0;

    float loss = 0.;
    ACTION_SCORE::action_scores preds = (*ec_seq)[0]->pred.a_s;

    for (size_t i = 0; i < (*ec_seq).size(); i++)
      if (!CB::ec_is_example_header(*(*ec_seq)[i])) {
        num_features += (*ec_seq)[i]->num_features;
        //cout<<(*ec_seq)[i]->num_features<<" ";
      }
    //cout<<endl;

    all.sd->total_features += num_features;

    bool is_test = false;
    if (c.known_cost.probability > 0) {
      for (uint32_t i = 0; i < preds.size(); i++) {
        float l = get_unbiased_cost(&c.known_cost, preds[i].action);
        //cout<<l<<":"<<l*preds[i].score<<" ";
        loss += l*preds[i].score;
      }
      //cout<<endl;
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
    }
    else
      is_test = true;

    for (int sink : all.final_prediction_sink)
      print_action_score(sink, ec.pred.a_s, ec.tag);

    if (all.raw_prediction > 0)
    {
      string outputString;
      stringstream outputStringStream(outputString);
      v_array<CB::cb_class> costs = ec.l.cb.costs;

      for (size_t i = 0; i < costs.size(); i++)
      {
        if (i > 0) outputStringStream << ' ';
        outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }

    CB::print_update(all, is_test, ec, ec_seq, true);
  }

  void output_example_seq(vw& all, cb_explore_adf& data)
  {
    if (data.ec_seq.size() > 0)
    {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;
      output_example(all, data, **(data.ec_seq.begin()), &(data.ec_seq));
      if (all.raw_prediction > 0)
        all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
    }
  }


  void clear_seq_and_finish_examples(vw& all, cb_explore_adf& data)
  {
    if (data.ec_seq.size() > 0)
      for (example* ecc : data.ec_seq)
        if (ecc->in_use)
          VW::finish_example(all, ecc);
    data.ec_seq.erase();
  }

  void finish_multiline_example(vw& all, cb_explore_adf& data, example& ec)
  {
    if (data.need_to_clear)
    {
      if (data.ec_seq.size() > 0)
      {
        output_example_seq(all, data);
        CB_ADF::global_print_newline(all);
      }
      clear_seq_and_finish_examples(all, data);
      data.need_to_clear = false;
    }
  }

  template <bool is_learn>
  void do_actual_learning(cb_explore_adf& data, base_learner& base)
  {
    data.known_cost = CB_ADF::get_observed_cost(data.ec_seq);

    switch (data.explore_type)
    {
    case EXPLORE_FIRST:
      predict_or_learn_first<is_learn>(data, base, data.ec_seq);
      break;
    case EPS_GREEDY:
      predict_or_learn_greedy<is_learn>(data, base, data.ec_seq);
      break;
    case SOFTMAX:
      predict_or_learn_softmax<is_learn>(data, base, data.ec_seq);
      break;
    case BAG_EXPLORE:
      predict_or_learn_bag<is_learn>(data, base, data.ec_seq);
      break;
    default:
      THROW("Unknown explorer type specified for contextual bandit learning: " << data.explore_type);
    }

  }


  template <bool is_learn>
  void predict_or_learn(cb_explore_adf& data, base_learner& base, example &ec)
  {
    vw* all = data.all;
    //data.base = &base;
    bool is_test_ec = CB::example_is_test(ec);
    bool need_to_break = VW::is_ring_example(*all, &ec) && (data.ec_seq.size() >= all->p->ring_size - 2);

    if ((CB_ALGS::example_is_newline_not_header(ec) && is_test_ec) || need_to_break)
    {
      data.ec_seq.push_back(&ec);
      do_actual_learning<is_learn>(data, base);
      // using flag to clear, because ec_seq is used in finish_example
      data.need_to_clear = true;
    }
    else
    {
      if (data.need_to_clear)    // should only happen if we're NOT driving
      {
        data.ec_seq.erase();
        data.need_to_clear = false;
      }
      data.ec_seq.push_back(&ec);
    }
  }

}


using namespace CB_EXPLORE_ADF;


base_learner* cb_explore_adf_setup(vw& all)
{ //parse and set arguments
  if (missing_option(all, true, "cb_explore_adf", "Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
    return nullptr;
  new_options(all, "CB_EXPLORE_ADF options")
    ("first", po::value<size_t>(), "tau-first exploration")
    ("epsilon", po::value<float>(), "epsilon-greedy exploration")
    ("bag", po::value<size_t>(), "bagging-based exploration")
    ("cover",po::value<size_t>() ,"Online cover based exploration")
    ("softmax", "softmax exploration")
    ("lambda", po::value<float>(), "parameter for softmax");
  add_options(all);

  po::variables_map& vm = all.vm;
  cb_explore_adf& data = calloc_or_throw<cb_explore_adf>();

  data.all = &all;
  if (count(all.args.begin(), all.args.end(), "--cb_adf") == 0)
    all.args.push_back("--cb_adf");

  all.delete_prediction = delete_action_scores;

  size_t problem_multiplier = 1;
  char type_string[10];

  if (vm.count("epsilon"))
    {
      data.epsilon = vm["epsilon"].as<float>();
      sprintf(type_string, "%f", data.epsilon);
      *all.file_options << " --epsilon "<<type_string;
    }
  if (vm.count("cover"))
    { data.cover_size = (uint32_t)vm["cover"].as<size_t>();
      data.cs_ldf_learner = all.cost_sensitive;
      sprintf(type_string, "%lu", data.cover_size);
      *all.file_options << " --cover " << type_string;

      if (!vm.count("epsilon"))
	data.epsilon = 0.05f;
    }
  else if (vm.count("bag"))
    {
      data.bag_size = (uint32_t)vm["bag"].as<size_t>();
      data.explore_type = BAG_EXPLORE;
      problem_multiplier = data.bag_size;
      sprintf(type_string, "%lu", data.bag_size);
      *all.file_options << " --bag "<<type_string;
    }
  else if (vm.count("first"))
    {
      data.tau = (uint32_t)vm["first"].as<size_t>();
      data.explore_type = EXPLORE_FIRST;
      sprintf(type_string, "%lu", data.tau);
      *all.file_options << " --first "<<type_string;
    }
  else if (vm.count("softmax"))
  {
    data.lambda = 1.0;
    if (vm.count("lambda"))
      data.lambda = (float)vm["lambda"].as<float>();
    data.explore_type = SOFTMAX;
    sprintf(type_string, "%f", data.lambda);
    *all.file_options << " --softmax --lambda "<<type_string;
  }
  else if (vm.count("epsilon"))
    data.explore_type = EPS_GREEDY;
  else //epsilon
    {
      data.epsilon = 0.05f;
      data.explore_type = EPS_GREEDY;
    }

  base_learner* base = setup_base(all);
  all.p->lp = CB::cb_label;

  learner<cb_explore_adf>& l = init_learner(&data, base, CB_EXPLORE_ADF::predict_or_learn<true>, CB_EXPLORE_ADF::predict_or_learn<false>, problem_multiplier);
  l.set_finish_example(CB_EXPLORE_ADF::finish_multiline_example);

  l.increment = base->increment;

  l.set_finish(CB_EXPLORE_ADF::finish);
  l.set_end_examples(CB_EXPLORE_ADF::end_examples);
  return make_base(l);
}

