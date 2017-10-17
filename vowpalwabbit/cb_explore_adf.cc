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

namespace CB_EXPLORE_ADF
{

struct cb_explore_adf
{ v_array<example*> ec_seq;
  v_array<action_score> action_probs;

  size_t explore_type;

  size_t tau;
  float epsilon;
  size_t bag_size;
  size_t cover_size;
  float psi;
  bool nounif;
  float lambda;
  uint64_t offset;
  bool greedify;

  size_t counter;

  bool need_to_clear;
  vw* all;
  LEARNER::base_learner* cs_ldf_learner;

  GEN_CS::cb_to_cs_adf gen_cs;
  COST_SENSITIVE::label cs_labels;
  v_array<CB::label> cb_labels;

  CB::label action_label;
  CB::label empty_label;

  COST_SENSITIVE::label cs_labels_2;

  v_array<COST_SENSITIVE::label> prepped_cs_labels;
};

template<class T> void swap(T& ele1, T& ele2)
{ T temp = ele2;
  ele2 = ele1;
  ele1 = temp;
}
template<bool is_learn>
void multiline_learn_or_predict(base_learner& base, v_array<example*>& examples, uint64_t offset, uint32_t id = 0)
{ for (example* ec : examples)
  { uint64_t old_offset = ec->ft_offset;
    ec->ft_offset = offset;
    if (is_learn)
      base.learn(*ec, id);
    else
      base.predict(*ec, id);
    ec->ft_offset = old_offset;
  }
}

example* test_adf_sequence(v_array<example*>& ec_seq)
{ uint32_t count = 0;
  example* ret = nullptr;
  for (size_t k = 0; k < ec_seq.size(); k++)
  { example *ec = ec_seq[k];

    if (ec->l.cb.costs.size() > 1)
      THROW("cb_adf: badly formatted example, only one cost can be known.");

    if (ec->l.cb.costs.size() == 1 && ec->l.cb.costs[0].cost != FLT_MAX)
    { ret = ec;
      count += 1;
    }

    if (CB::ec_is_example_header(*ec))
      if (k != 0)
        THROW("warning: example headers at position " << k << ": can only have in initial position!");
  }
  if (count == 0 || count == 1)
    return ret;
  else
    THROW("cb_adf: badly formatted example, only one line can have a cost");
}

template <bool is_learn>
void predict_or_learn_first(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
{ //Explore tau times, then act according to optimal.
  if (is_learn && data.gen_cs.known_cost.probability < 1 && test_adf_sequence(data.ec_seq) != nullptr)
    multiline_learn_or_predict<true>(base, examples, data.offset);
  else
    multiline_learn_or_predict<false>(base, examples, data.offset);

  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();

  if (data.tau)
  { float prob = 1.f / (float)num_actions;
    for (size_t i = 0; i < num_actions; i++)
      preds[i].score = prob;
    data.tau--;
  }
  else
  { for (size_t i = 1; i < num_actions; i++)
      preds[i].score = 0.;
    preds[0].score = 1.0;
  }
  CB_EXPLORE::safety(preds, data.epsilon, true);
}

  template <bool is_learn>
  void predict_or_learn_greedy(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
  { //Explore uniform random an epsilon fraction of the time.
    if (is_learn && test_adf_sequence(data.ec_seq) != nullptr)
      multiline_learn_or_predict<true>(base, examples, data.offset);
    else
      multiline_learn_or_predict<false>(base, examples, data.offset);
    
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
  if (num_actions == 0)
  {
    preds.erase();
    return;
  }

  data.action_probs.resize(num_actions);
  data.action_probs.erase();
  for (uint32_t i = 0; i < num_actions; i++)
    data.action_probs.push_back({ i,0. });
  float prob = 1.f / (float)data.bag_size;
  bool test_sequence = test_adf_sequence(data.ec_seq) == nullptr;
  for (uint32_t i = 0; i < data.bag_size; i++) 
  {
    // avoid updates to the random num generator
    // for greedify, always update first policy once
    uint32_t count = is_learn
      ? ((data.greedify && i == 0) ? 1 : BS::weight_gen(*data.all))
      : 0;
    if (is_learn && count > 0 && !test_sequence)
      multiline_learn_or_predict<true>(base, examples, data.offset, i);
    else
      multiline_learn_or_predict<false>(base, examples, data.offset, i);
    assert(preds.size() == num_actions);
    data.action_probs[preds[0].action].score += prob;
    if (is_learn && !test_sequence)
      for (uint32_t j = 1; j < count; j++)
        multiline_learn_or_predict<true>(base, examples, data.offset, i);
  }

  CB_EXPLORE::safety(data.action_probs, data.epsilon, true);
  qsort((void*) data.action_probs.begin(), data.action_probs.size(), sizeof(action_score), reverse_order);

  for (size_t i = 0; i < num_actions; i++)
    preds[i] = data.action_probs[i];
}
  
template <bool is_learn>
void predict_or_learn_cover(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
{ //Randomize over predictions from a base set of predictors
  //Use cost sensitive oracle to cover actions to form distribution.
  if (is_learn)
  { GEN_CS::gen_cs_example<false>(data.gen_cs, examples, data.cs_labels);
    multiline_learn_or_predict<true>(base, examples, data.offset);
  }
  else
  { GEN_CS::gen_cs_example_ips(examples, data.cs_labels);
    multiline_learn_or_predict<false>(base, examples, data.offset);
  }

  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();

  float additive_probability = 1.f / (float)data.cover_size;
  float min_prob = min(1.f / num_actions, 1.f / (float)sqrt(data.counter * num_actions));
  v_array<action_score>& probs = data.action_probs;
  probs.erase();
  for(uint32_t i = 0; i < num_actions; i++)
    probs.push_back({i,0.});

  probs[preds[0].action].score += additive_probability;

  uint32_t shared = CB::ec_is_example_header(*examples[0]) ? 1 : 0;

  float norm = min_prob * num_actions + (additive_probability - min_prob);
  for (size_t i = 1; i < data.cover_size; i++)
  { //Create costs of each action based on online cover
    if (is_learn)
    { data.cs_labels_2.costs.erase();
      if (shared > 0)
        data.cs_labels_2.costs.push_back(data.cs_labels.costs[0]);
      for (uint32_t j = 0; j < num_actions; j++)
      { float pseudo_cost = data.cs_labels.costs[j+shared].x - data.psi * min_prob / (max(probs[j].score, min_prob) / norm);
        data.cs_labels_2.costs.push_back({pseudo_cost,j,0.,0.});
      }
      GEN_CS::call_cs_ldf<true>(*(data.cs_ldf_learner), examples, data.cb_labels, data.cs_labels_2, data.prepped_cs_labels, data.offset, i+1);
    }
    else
      GEN_CS::call_cs_ldf<false>(*(data.cs_ldf_learner), examples, data.cb_labels, data.cs_labels, data.prepped_cs_labels, data.offset, i+1);

    uint32_t action = preds[0].action;
    if (probs[action].score < min_prob)
      norm += max(0, additive_probability - (min_prob - probs[action].score));
    else
      norm += additive_probability;
    probs[action].score += additive_probability;
  }

  CB_EXPLORE::safety(data.action_probs, min_prob * num_actions, !data.nounif);

  qsort((void*) probs.begin(), probs.size(), sizeof(action_score), reverse_order);
  for (size_t i = 0; i < num_actions; i++)
    preds[i] = probs[i];

  ++data.counter;
}

template <bool is_learn>
void predict_or_learn_softmax(cb_explore_adf& data, base_learner& base, v_array<example*>& examples)
{ if (is_learn && test_adf_sequence(data.ec_seq) != nullptr)
    multiline_learn_or_predict<true>(base, examples, data.offset);
  else
    multiline_learn_or_predict<false>(base, examples, data.offset);

  v_array<action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();
  float norm = 0.;
  float max_score = preds[0].score;
  for (size_t i = 1; i < num_actions; i++)
    if (max_score < preds[i].score)
      max_score = preds[i].score;

  for (size_t i = 0; i < num_actions; i++)
  { float prob = exp(data.lambda*(preds[i].score - max_score));
    preds[i].score = prob;
     norm += prob;
  }
  for (size_t i = 0; i < num_actions; i++)
    preds[i].score /= norm;
  CB_EXPLORE::safety(preds, data.epsilon, true);
}

void end_examples(cb_explore_adf& data)
{ if (data.need_to_clear)
    data.ec_seq.erase();
}

void finish(cb_explore_adf& data)
{ data.ec_seq.delete_v();
  data.action_probs.delete_v();
  data.cs_labels.costs.delete_v();
  data.cs_labels_2.costs.delete_v();
  data.cb_labels.delete_v();
  for(size_t i = 0; i < data.prepped_cs_labels.size(); i++)
    data.prepped_cs_labels[i].costs.delete_v();
  data.prepped_cs_labels.delete_v();
  data.gen_cs.pred_scores.costs.delete_v();
}


//Semantics: Currently we compute the IPS loss no matter what flags
//are specified. We print the first action and probability, based on
//ordering by scores in the final output.

void output_example(vw& all, cb_explore_adf& c, example& ec, v_array<example*>* ec_seq)
{ if (CB_ALGS::example_is_newline_not_header(ec)) return;

  size_t num_features = 0;

  float loss = 0.;
  ACTION_SCORE::action_scores preds = (*ec_seq)[0]->pred.a_s;

  for (size_t i = 0; i < (*ec_seq).size(); i++)
    if (!CB::ec_is_example_header(*(*ec_seq)[i]))
      num_features += (*ec_seq)[i]->num_features;

  bool is_test = false;
  if (c.gen_cs.known_cost.probability > 0)
  { for (uint32_t i = 0; i < preds.size(); i++)
    { float l = get_unbiased_cost(&c.gen_cs.known_cost, preds[i].action);
      loss += l*preds[i].score;
    }
  }
  else
    is_test = true;
  all.sd->update(ec.test_only, c.gen_cs.known_cost.probability > 0, loss, ec.weight, num_features);

  for (int sink : all.final_prediction_sink)
    print_action_score(sink, ec.pred.a_s, ec.tag);

  if (all.raw_prediction > 0)
  { string outputString;
    stringstream outputStringStream(outputString);
    v_array<CB::cb_class> costs = ec.l.cb.costs;

    for (size_t i = 0; i < costs.size(); i++)
    { if (i > 0) outputStringStream << ' ';
      outputStringStream << costs[i].action << ':' << costs[i].partial_prediction;
    }
    all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
  }

  CB::print_update(all, is_test, ec, ec_seq, true);
}

void output_example_seq(vw& all, cb_explore_adf& data)
{ if (data.ec_seq.size() > 0)
  { output_example(all, data, **(data.ec_seq.begin()), &(data.ec_seq));
    if (all.raw_prediction > 0)
      all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
  }
}


void clear_seq_and_finish_examples(vw& all, cb_explore_adf& data)
{ if (data.ec_seq.size() > 0)
    for (example* ecc : data.ec_seq)
      if (ecc->in_use)
        VW::finish_example(all, ecc);
  data.ec_seq.erase();
}

void finish_multiline_example(vw& all, cb_explore_adf& data, example& ec)
{ if (data.need_to_clear)
  { if (data.ec_seq.size() > 0)
    { output_example_seq(all, data);
      CB_ADF::global_print_newline(all);
    }
    clear_seq_and_finish_examples(all, data);
    data.need_to_clear = false;
  }
}

template <bool is_learn>
void do_actual_learning(cb_explore_adf& data, base_learner& base)
{ example* label_example=test_adf_sequence(data.ec_seq);
  data.gen_cs.known_cost = CB_ADF::get_observed_cost(data.ec_seq);

  if (label_example == nullptr || !is_learn)
  { if (label_example != nullptr)//extract label
    { data.action_label = label_example->l.cb;
      label_example->l.cb = data.empty_label;
    }
    switch (data.explore_type)
    { case EXPLORE_FIRST:
        predict_or_learn_first<false>(data, base, data.ec_seq);
        break;
      case EPS_GREEDY:
        predict_or_learn_greedy<false>(data, base, data.ec_seq);
        break;
      case SOFTMAX:
        predict_or_learn_softmax<false>(data, base, data.ec_seq);
        break;
      case BAG_EXPLORE:
        predict_or_learn_bag<false>(data, base, data.ec_seq);
        break;
      case COVER:
        predict_or_learn_cover<false>(data, base, data.ec_seq);
        break;
      default:
        THROW("Unknown explorer type specified for contextual bandit learning: " << data.explore_type);
    }
    if (label_example != nullptr)	//restore label
      label_example->l.cb = data.action_label;
  }
  else
  { /*	v_array<float> temp_probs;
    temp_probs = v_init<float>();
    do_actual_learning<false>(data,base);
    for (size_t i = 0; i < data.ec_seq[0]->pred.a_s.size(); i++)
    temp_probs.push_back(data.ec_seq[0]->pred.a_s[i].score);*/

    switch (data.explore_type)
    { case EXPLORE_FIRST:
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
      case COVER:
        predict_or_learn_cover<is_learn>(data, base, data.ec_seq);
        break;
      default:
        THROW("Unknown explorer type specified for contextual bandit learning: " << data.explore_type);
    }

    /*	for (size_t i = 0; i < temp_probs.size(); i++)
      if (temp_probs[i] != data.ec_seq[0]->pred.a_s[i].score)
        cout << "problem! " << temp_probs[i] << " != " << data.ec_seq[0]->pred.a_s[i].score << " for " << data.ec_seq[0]->pred.a_s[i].action << endl;
        temp_probs.delete_v();*/
  }
}

template <bool is_learn>
void predict_or_learn(cb_explore_adf& data, base_learner& base, example &ec)
{ vw* all = data.all;
  //data.base = &base;
  data.offset = ec.ft_offset;
  bool is_test_ec = CB::example_is_test(ec);
  bool need_to_break = VW::is_ring_example(*all, &ec) && (data.ec_seq.size() >= all->p->ring_size - 2);

  if ((CB_ALGS::example_is_newline_not_header(ec) && is_test_ec) || need_to_break)
  { data.ec_seq.push_back(&ec);
    if (data.ec_seq.size() == 1)
      cout << "Something is wrong---an example with no choice.  Do you have all 0 features? Or multiple empty lines?" << endl;
    else
      do_actual_learning<is_learn>(data, base);
    // using flag to clear, because ec_seq is used in finish_example
    data.need_to_clear = true;
  }
  else
  { if (data.need_to_clear)    // should only happen if we're NOT driving
    { data.ec_seq.erase();
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
  ("psi", po::value<float>(), "disagreement parameter for cover")
  ("nounif", "do not explore uniformly on zero-probability actions in cover")
  ("softmax", "softmax exploration")
  ("greedify", "always update first policy once in bagging")
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
  { data.epsilon = vm["epsilon"].as<float>();
    sprintf(type_string, "%f", data.epsilon);
    *all.file_options << " --epsilon "<<type_string;
  }
  if (vm.count("cover"))
  { data.cover_size = (uint32_t)vm["cover"].as<size_t>();
    data.explore_type = COVER;
    problem_multiplier = data.cover_size+1;
    *all.file_options << " --cover " << data.cover_size;

    data.psi = 1.0f;
    if (vm.count("psi"))
      data.psi = vm["psi"].as<float>();

    sprintf(type_string, "%f", data.psi);
    *all.file_options << " --psi " << type_string;
    if (vm.count("nounif"))
    { data.nounif = true;
      *all.file_options << " --nounif";
    }
  }
  else if (vm.count("bag"))
  { data.bag_size = (uint32_t)vm["bag"].as<size_t>();
    data.greedify = vm.count("greedify") > 0;
    data.explore_type = BAG_EXPLORE;
    problem_multiplier = data.bag_size;
    *all.file_options << " --bag "<< data.bag_size;
    if (data.greedify)
      *all.file_options << " --greedify";
  }
  else if (vm.count("first"))
  { data.tau = (uint32_t)vm["first"].as<size_t>();
    data.explore_type = EXPLORE_FIRST;
    *all.file_options << " --first "<< data.tau;
  }
  else if (vm.count("softmax"))
  { data.lambda = 1.0;
    if (vm.count("lambda"))
      data.lambda = (float)vm["lambda"].as<float>();
    data.explore_type = SOFTMAX;
    sprintf(type_string, "%f", data.lambda);
    *all.file_options << " --softmax --lambda "<<type_string;
  }
  else if (vm.count("epsilon"))
    data.explore_type = EPS_GREEDY;
  else //epsilon
  { data.epsilon = 0.05f;
    data.explore_type = EPS_GREEDY;
  }

  base_learner* base = setup_base(all);
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  learner<cb_explore_adf>& l = init_learner(&data, base, CB_EXPLORE_ADF::predict_or_learn<true>, CB_EXPLORE_ADF::predict_or_learn<false>, problem_multiplier, prediction_type::action_probs);

  //Extract from lower level reductions.
  data.gen_cs.scorer = all.scorer;
  data.cs_ldf_learner = all.cost_sensitive;
  data.gen_cs.cb_type = CB_TYPE_IPS;
  if (all.vm.count("cb_type"))
  { std::string type_string;
    type_string = all.vm["cb_type"].as<std::string>();

    if (type_string.compare("dr") == 0)
      data.gen_cs.cb_type = CB_TYPE_DR;
    else if (type_string.compare("ips") == 0)
      data.gen_cs.cb_type = CB_TYPE_IPS;
    else if (type_string.compare("mtr") == 0)
      if (vm.count("cover"))
      { all.trace_message << "warning: cover and mtr are not simultaneously supported yet, defaulting to ips" << endl;
        data.gen_cs.cb_type = CB_TYPE_IPS;
      }
      else
        data.gen_cs.cb_type = CB_TYPE_MTR;
    else
      all.trace_message << "warning: cb_type must be in {'ips','dr'}; resetting to ips." << std::endl;
  }

  l.set_finish_example(CB_EXPLORE_ADF::finish_multiline_example);
  l.set_finish(CB_EXPLORE_ADF::finish);
  l.set_end_examples(CB_EXPLORE_ADF::end_examples);
  return make_base(l);
}

