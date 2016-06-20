#include <float.h>
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "vw.h"
#include "cb_explore_adf.h"
#include "gen_cs_example.h"
#include "cb_algs.h"
#include "learner.h"
#include "mwt.h"
//#include "action_score.h"
#include "correctedMath.h"

using namespace LEARNER;
using namespace ACTION_SCORE;

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

  struct cb_explore_adf;

  void safety(v_array<float>& distribution, float min_prob);


  struct cb_explore_adf
  {

    v_array<example*> ec_seq;
    v_array<action_score> id_probs;

    size_t explore_type;

    size_t tau;
    float epsilon;
    size_t bag_size;
    size_t cover_size;
    float lambda;

    size_t counter;
    bool need_to_clear;
    vw* all;
    LEARNER::base_learner* scorer;
    CB::cb_class known_cost;
    bool learn_only;

  };

  template<class T> void swap(T& ele1, T& ele2)
  {
    T temp = ele2;
    ele2 = ele1;
    ele1 = temp;
  }

  template <bool is_learn>
  void predict_or_learn_first(cb_explore_adf& data, base_learner& base, v_array<example*>& examples, bool isTest, bool shared)
  { //Explore tau times, then act according to optimal.

    v_array<action_score>& preds = examples[0]->pred.a_s;
    data.id_probs.erase();

    size_t num_actions = examples.size() - 1;
    if (shared)
      num_actions--;
    if (preds.size() != num_actions)
      THROW("Received predictions of wrong size from CB base learner");

    if (!is_learn || !data.learn_only) {
      if (data.tau) {
        float prob = 1.0 / (float)num_actions;
        for (size_t i = 0; i < num_actions; i++) {
          action_score a_s;
          a_s.action = preds[i].action;
          a_s.score = prob;
          data.id_probs.push_back(a_s);
        }
        data.tau--;
      }
      else {
        for (size_t i = 0; i < num_actions; i++) {
          action_score a_s;
          a_s.action = preds[i].action;
          a_s.score = 0.;
          data.id_probs.push_back(a_s);
        }
        data.id_probs[0].score = 1.0;
      }
    }

    if (is_learn && data.known_cost.probability < 1)
      for (example* ec : examples)
        base.learn(*ec);

    for (size_t i = 0; i < num_actions; i++) {
      examples[0]->pred.a_s[i].score = data.id_probs[i].score;
      examples[0]->pred.a_s[i].action = data.id_probs[i].action;
    }

  }

  template <bool is_learn>
  void predict_or_learn_greedy(cb_explore_adf& data, base_learner& base, v_array<example*>& examples, bool isTest, bool shared)
  { //Explore uniform random an epsilon fraction of the time.

    v_array<action_score>& preds = examples[0]->pred.a_s;
    data.id_probs.erase();

    size_t num_actions = examples.size() - 1;
    if (shared)
      num_actions--;
    if (preds.size() != num_actions)
      THROW("Received predictions of wrong size from CB base learner");

    if(!is_learn || !data.learn_only) {
      float prob = data.epsilon/(float)num_actions;
      for(size_t i = 0;i < num_actions;i++) {
	action_score a_s;
	a_s.action = preds[i].action;
	a_s.score = prob;
	data.id_probs.push_back(a_s);
      }
      data.id_probs[0].score += (1 - data.epsilon);
    }


    if (is_learn)
      for (example* ec : examples)
        base.learn(*ec);

    for (size_t i = 0; i < num_actions; i++) {
      examples[0]->pred.a_s[i].score = data.id_probs[i].score;
      examples[0]->pred.a_s[i].action = data.id_probs[i].action;
    }

  }

  template <bool is_learn>
  void predict_or_learn_bag(cb_explore_adf& data, base_learner& base, v_array<example*>& examples, bool isTest, bool shared)
  { //Randomize over predictions from a base set of predictors

    v_array<action_score>& preds = examples[0]->pred.a_s;
    data.id_probs.erase();

    size_t num_actions = examples.size() - 1;
    if (shared)
      num_actions--;
    if (preds.size() != num_actions)
      THROW("Received predictions of wrong size from CB base learner");

    if (!is_learn || !data.learn_only) {
      for (size_t i = 0; i < num_actions; i++) {
        action_score a_s;
        a_s.action = preds[i].action;
        a_s.score = 0.;
        data.id_probs.push_back(a_s);
      }
      float prob = 1.0 / (float)data.bag_size;
      data.id_probs[0].score += prob;
      for (size_t i = 1; i < data.bag_size; i++) {
        for (example* ec : examples)
          base.predict(*ec, i);
        uint32_t chosen = preds[0].action;
        for (size_t i = 0; i < num_actions; i++)
          if (data.id_probs[i].action == chosen)
            data.id_probs[i].score += prob;
      }
    }

    if (is_learn)
      for (size_t i = 0; i < data.bag_size; i++)
      {
        uint32_t count = BS::weight_gen();
        for (uint32_t j = 0; j < count; j++)
          for (example* ec : examples)
            base.learn(*ec, i);
      }

    for (size_t i = 0; i < num_actions; i++) {
      examples[0]->pred.a_s[i].score = data.id_probs[i].score;
      examples[0]->pred.a_s[i].action = data.id_probs[i].action;
    }

  }


  template <bool is_learn>
  void predict_or_learn_softmax(cb_explore_adf& data, base_learner& base, v_array<example*>& examples, bool isTest, bool shared)
  {
    v_array<action_score>& preds = examples[0]->pred.a_s;
    data.id_probs.erase();

    size_t num_actions = examples.size() - 1;
    if (shared)
      num_actions--;
    if (preds.size() != num_actions)
      THROW("Received predictions of wrong size from CB base learner");
    float norm = 0.;
    float max_score = preds[0].score;

    if (!is_learn || !data.learn_only) {
      for (size_t i = 0; i < num_actions; i++) {
        float prob = exp(data.lambda*(preds[i].score - max_score));
        action_score a_s;
        a_s.action = preds[i].action;
        a_s.score = prob;
        data.id_probs.push_back(a_s);
        norm += prob;
      }
      for (size_t i = 0; i < num_actions; i++)
        data.id_probs[i].score /= norm;
    }


    if (is_learn) {
      for (example* ec : examples)
        base.learn(*ec);
    }

    for (size_t i = 0; i < num_actions; i++) {
      examples[0]->pred.a_s[i].score = data.id_probs[i].score;
      examples[0]->pred.a_s[i].action = data.id_probs[i].action;
    }
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
    data.id_probs.delete_v();
  }


  //Semantics: Currently we compute the IPS loss no matter what flags
  //are specified. We print the first action and probability, based on
  //ordering by scores in the final output.

  void output_example(vw& all, cb_explore_adf& c, example& ec, v_array<example*>* ec_seq)
  {

    if (CB_ADF::example_is_newline_not_header(ec)) return;

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
  void do_actual_learning(cb_explore_adf& data, base_learner& base)
  {
    bool isTest = test_adf_sequence(data);
    bool shared = CB::ec_is_example_header(*data.ec_seq[0]);
    data.known_cost = CB_ADF::get_observed_cost(data.ec_seq);

    for (example* ec : data.ec_seq) 
      base.predict(*ec);    

    switch (data.explore_type)
    {
    case EXPLORE_FIRST:
      predict_or_learn_first<is_learn>(data, base, data.ec_seq, isTest, shared);
      break;
    case EPS_GREEDY:
      predict_or_learn_greedy<is_learn>(data, base, data.ec_seq, isTest, shared);
      break;
    case SOFTMAX:
      predict_or_learn_softmax<is_learn>(data, base, data.ec_seq, isTest, shared);
      break;
    case BAG_EXPLORE:
      predict_or_learn_bag<is_learn>(data, base, data.ec_seq, isTest, shared);
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

    if ((CB_ADF::example_is_newline_not_header(ec) && is_test_ec) || need_to_break)
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
    ("softmax", po::value<float>(), "softmax exploration")
    ("learn_only", "for not calling predict when learn is true");
  add_options(all);

  po::variables_map& vm = all.vm;
  cb_explore_adf& data = calloc_or_throw<cb_explore_adf>();
  data.id_probs = v_init<action_score>();

  data.all = &all;
  if (count(all.args.begin(), all.args.end(), "--cb_adf") == 0)
    all.args.push_back("--cb_adf");

  if (vm.count("learn_only"))
    data.learn_only = true;
  else
    data.learn_only = false;

  all.delete_prediction = delete_action_scores;

  size_t problem_multiplier = 1;
  char type_string[10];

  if (vm.count("bag"))
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
      data.lambda = (float)vm["lambda"].as<size_t>();
    data.explore_type = SOFTMAX;
    sprintf(type_string, "%f", data.lambda);
    *all.file_options << " --softmax --lambda "<<type_string;
  }
  else
  {
    data.epsilon = 0.05f;
    if (vm.count("epsilon"))
      data.epsilon = vm["epsilon"].as<float>();
    data.explore_type = EPS_GREEDY;
    sprintf(type_string, "%f", data.epsilon);
    *all.file_options << " --epsilon "<<type_string;
  }

  base_learner* base = setup_base(all);
  all.p->lp = CB::cb_label;

  learner<cb_explore_adf>& l = init_learner(&data, base, CB_EXPLORE_ADF::predict_or_learn<true>, CB_EXPLORE_ADF::predict_or_learn<false>, problem_multiplier);
  l.set_finish_example(CB_EXPLORE_ADF::finish_multiline_example);

  l.increment = base->increment;
  data.scorer = all.scorer;

  l.set_finish(CB_EXPLORE_ADF::finish);
  l.set_end_examples(CB_EXPLORE_ADF::end_examples);
  return make_base(l);
}

