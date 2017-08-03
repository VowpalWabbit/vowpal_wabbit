#include "reductions.h"
#include "cb_algs.h"
#include "../explore/cpp/MWTExplorer.h"
#include "vw.h"
#include "cb_adf.h"
#include "cb_explore_adf.h"
#include "rand48.h"

//Do evaluation of nonstationary policies.
//input = contextual bandit label
//output = chosen ranking

using namespace LEARNER;
using namespace CB_ALGS;
using namespace MultiWorldTesting;
using namespace MultiWorldTesting::SingleAction;
using namespace std;

namespace EXPLORE_EVAL
{

struct explore_eval
{ CB::cb_class known_cost;
  v_array<example*> ec_seq;
  bool need_to_clear;
  vw* all;
  uint64_t offset;
  CB::label action_label;
  CB::label empty_label;
  size_t example_counter;

  size_t update_count;
  size_t violations;
  float multiplier;

  bool fixed_multiplier;
};

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

void end_examples(explore_eval& data)
{ if (data.need_to_clear)
    data.ec_seq.erase();
}

void finish(explore_eval& data)
{ data.ec_seq.delete_v();
  if (!data.all->quiet)
    { data.all->trace_message << "update count = " << data.update_count << endl;
      if (data.violations > 0)
		  data.all->trace_message << "violation count = " << data.violations << endl;
      if (!data.fixed_multiplier)
		  data.all->trace_message << "final multiplier = " << data.multiplier << endl;
    }
}
  
//Semantics: Currently we compute the IPS loss no matter what flags
//are specified. We print the first action and probability, based on
//ordering by scores in the final output.

void output_example(vw& all, explore_eval& c, example& ec, v_array<example*>* ec_seq)
{ if (example_is_newline_not_header(ec)) return;

  size_t num_features = 0;

  float loss = 0.;
  ACTION_SCORE::action_scores preds = (*ec_seq)[0]->pred.a_s;

  for (size_t i = 0; i < (*ec_seq).size(); i++)
    if (!CB::ec_is_example_header(*(*ec_seq)[i]))
      num_features += (*ec_seq)[i]->num_features;
  
  bool is_test = false;
  if (c.known_cost.probability > 0)
  { for (uint32_t i = 0; i < preds.size(); i++)
    { float l = get_unbiased_cost(&c.known_cost, preds[i].action);
      loss += l*preds[i].score;
    }
  }
  else
    is_test = true;
  all.sd->update( ec.test_only, !is_test, loss, ec.weight, num_features);
  
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

void output_example_seq(vw& all, explore_eval& data)
{ if (data.ec_seq.size() > 0)
  { output_example(all, data, **(data.ec_seq.begin()), &(data.ec_seq));
    if (all.raw_prediction > 0)
      all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
  }
}


void clear_seq_and_finish_examples(vw& all, explore_eval& data)
{ if (data.ec_seq.size() > 0)
    for (example* ecc : data.ec_seq)
      if (ecc->in_use)
        VW::finish_example(all, ecc);
  data.ec_seq.erase();
}

void finish_multiline_example(vw& all, explore_eval& data, example& ec)
{ if (data.need_to_clear)
  { if (data.ec_seq.size() > 0)
    { output_example_seq(all, data);
      CB_ADF::global_print_newline(all);
    }
    clear_seq_and_finish_examples(all, data);
    data.need_to_clear = false;
  }
}

template <bool is_learn> void do_actual_learning(explore_eval& data, base_learner& base)
{ example* label_example=CB_EXPLORE_ADF::test_adf_sequence(data.ec_seq);

  if (label_example != nullptr)//extract label
  { data.action_label = label_example->l.cb;
    label_example->l.cb = data.empty_label;
  }
  multiline_learn_or_predict<false>(base, data.ec_seq, data.offset);
  
  if (label_example != nullptr)	//restore label
    label_example->l.cb = data.action_label;
  
  data.known_cost = CB_ADF::get_observed_cost(data.ec_seq);
  if (label_example != nullptr && is_learn)
    {
      ACTION_SCORE::action_scores& a_s = data.ec_seq[0]->pred.a_s;
      
      float action_probability = 0;
      for (size_t i =0 ; i < a_s.size(); i++)
	if (data.known_cost.action == a_s[i].action)
	  action_probability = a_s[i].score;
      
      float threshold = action_probability / data.known_cost.probability;

      if (!data.fixed_multiplier)
	data.multiplier = min(data.multiplier, 1/threshold);
      else
	threshold *= data.multiplier;
      
      if (threshold > 1. + 1e-6)
	data.violations++;
      
      if (merand48(data.all->random_state) < threshold)
	{
	  example* ec_found = nullptr;
	  for (example*& ec : data.ec_seq)
	    {
	      if (ec->l.cb.costs.size() == 1 &&
		  ec->l.cb.costs[0].cost != FLT_MAX &&
		  ec->l.cb.costs[0].probability > 0)
		ec_found = ec;
	      if (threshold > 1)
		ec->weight *= threshold;
	    }
	  ec_found->l.cb.costs[0].probability = action_probability;
	  
	  multiline_learn_or_predict<true>(base, data.ec_seq, data.offset);

	  if (threshold > 1)
	    {
	      float inv_threshold = 1.f / threshold;
	      for (example*& ec : data.ec_seq)
		ec->weight *= inv_threshold;
	    }	    
	  ec_found->l.cb.costs[0].probability = data.known_cost.probability;
	  data.update_count++;
	}
    }
}
  
template <bool is_learn>
void predict_or_learn(explore_eval& data, base_learner& base, example &ec)
{ vw* all = data.all;
  //data.base = &base;
  data.offset = ec.ft_offset;
  bool is_test_ec = CB::example_is_test(ec);
  bool need_to_break = VW::is_ring_example(*all, &ec) && (data.ec_seq.size() >= all->p->ring_size - 2);

  if ((CB_ALGS::example_is_newline_not_header(ec) && is_test_ec) || need_to_break)
  { data.ec_seq.push_back(&ec);
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

using namespace EXPLORE_EVAL;

base_learner* explore_eval_setup(vw& all)
{ //parse and set arguments
  if (missing_option(all, true, "explore_eval", "Evaluate explore_eval adf policies"))
    return nullptr;
  new_options(all, "Explore evaluation options")
  ("multiplier", po::value<float>(), "the multiplier needed to make all rejection sample probabilities <= 1");
  add_options(all);
  

  explore_eval& data = calloc_or_throw<explore_eval>();

  data.all = &all;

  if (all.vm.count("multiplier") > 0)
    { data.multiplier = all.vm["multiplier"].as<float>();
      data.fixed_multiplier = true;
    }
  else
    data.multiplier = 1;
  
  if (count(all.args.begin(), all.args.end(), "--cb_explore_adf") == 0)
    all.args.push_back("--cb_explore_adf");
  
  all.delete_prediction = nullptr;
  
  base_learner* base = setup_base(all);
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  learner<explore_eval>& l = init_learner(&data, base, predict_or_learn<true>, predict_or_learn<false>, 1, prediction_type::action_probs);

  l.set_finish_example(finish_multiline_example);
  l.set_finish(finish);
  l.set_end_examples(end_examples);
  return make_base(l);
}
