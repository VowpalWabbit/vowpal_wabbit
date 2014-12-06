/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <string.h>
#include "vw.h"
#include "search.h"
#include "v_hashmap.h"
#include "hash.h"
#include "rand48.h"
#include "cost_sensitive.h"
#include "multiclass.h"
#include "memory.h"
#include "constant.h"
#include "example.h"
#include "cb.h"
#include "gd.h" // for GD::foreach_feature
#include <math.h>
#include "search_sequencetask.h"
#include "search_multiclasstask.h"
#include "search_dep_parser.h"
#include "search_entityrelationtask.h"
#include "search_hooktask.h"
#include "csoaa.h"
#include "beam.h"

using namespace LEARNER;
using namespace std;
namespace CS = COST_SENSITIVE;
namespace MC = MULTICLASS;

namespace Search {
  search_task* all_tasks[] = { &SequenceTask::task,
                               &SequenceSpanTask::task,
                               &ArgmaxTask::task,
                               &SequenceTask_DemoLDF::task,
                               &MulticlassTask::task,
                               &DepParserTask::task,
                               &EntityRelationTask::task,
                               &HookTask::task,
                               NULL };   // must NULL terminate!

  const bool PRINT_UPDATE_EVERY_EXAMPLE =0;
  const bool PRINT_UPDATE_EVERY_PASS =0;
  const bool PRINT_CLOCK_TIME =0;

  string   neighbor_feature_space("neighbor");
  string   condition_feature_space("search_condition");

  uint32_t AUTO_CONDITION_FEATURES = 1, AUTO_HAMMING_LOSS = 2, EXAMPLES_DONT_CHANGE = 4, IS_LDF = 8;
  enum SearchState { INITIALIZE, INIT_TEST, INIT_TRAIN, LEARN, GET_TRUTH_STRING };
  enum RollMethod { POLICY, ORACLE, MIX_PER_STATE, MIX_PER_ROLL, NO_ROLLOUT };

  // a data structure to hold conditioning information
  struct prediction {
    ptag    me;     // the id of the current prediction (the one being memoized)
    size_t  cnt;    // how many variables are we conditioning on?
    ptag*   tags;   // which variables are they?
    action* acts;   // and which actions were taken at each?
    uint32_t hash;  // a hash of the above
  };

  // parameters for auto-conditioning
  struct auto_condition_settings {
    size_t max_bias_ngram_length;   // add a "bias" feature for each ngram up to and including this length. eg., if it's 1, then you get a single feature for each conditional
    size_t max_quad_ngram_length;   // add bias *times* input features for each ngram up to and including this length
    float  feature_value;           // how much weight should the conditional features get?
  };

  typedef v_array<action> action_prefix;
  
  struct search_private {
    vw* all;

    bool auto_condition_features;  // do you want us to automatically add conditioning features?
    bool auto_hamming_loss;        // if you're just optimizing hamming loss, we can do it for you!
    bool examples_dont_change;     // set to true if you don't do any internal example munging
    bool is_ldf;                   // user declared ldf
    
    v_array<int32_t> neighbor_features; // ugly encoding of neighbor feature requirements
    auto_condition_settings acset; // settings for auto-conditioning
    size_t history_length;         // value of --search_history_length, used by some tasks, default 1
    
    size_t A;                      // total number of actions, [1..A]; 0 means ldf
    size_t num_learners;           // total number of learners;
    bool cb_learner;               // do contextual bandit learning on action (was "! rollout_all_actions" which was confusing)
    SearchState state;             // current state of learning
    size_t learn_learner_id;       // we allow user to use different learners for different states
    int mix_per_roll_policy;       // for MIX_PER_ROLL, we need to choose a policy to use; this is where it's stored (-2 means "not selected yet")
    bool no_caching;               // turn off caching
    size_t rollout_num_steps;      // how many calls of "loss" before we stop really predicting on rollouts and switch to oracle (0 means "infinite")
    bool (*label_is_test)(void*);  // tell me if the label data from an example is test
    
    size_t t;                      // current search step
    size_t T;                      // length of root trajectory
    v_array<example> learn_ec_copy;// copy of example(s) at learn_t
    example* learn_ec_ref;         // reference to example at learn_t, when there's no example munging
    size_t learn_ec_ref_cnt;       // how many are there (for LDF mode only; otherwise 1)
    v_array<ptag> learn_condition_on;      // a copy of the tags used for conditioning at the training position
    v_array<action> learn_condition_on_act;// the actions taken
    v_array<char>   learn_condition_on_names;// the names of the actions
    v_array<action> learn_allowed_actions; // which actions were allowed at training time?
    v_array<action> ptag_to_action;// tag to action mapping for conditioning
    vector<action> test_action_sequence; // if test-mode was run, what was the corresponding action sequence; it's a vector cuz we might expose it to the library
    action learn_oracle_action;    // store an oracle action for debugging purposes
    
    polylabel* allowed_actions_cache;
    
    size_t loss_declared_cnt;      // how many times did run declare any loss (implicitly or explicitly)?
    v_array<action> train_trajectory; // the training trajectory
    v_array<action> current_trajectory;  // the current trajectory; only used in beam search mode
    size_t learn_t;                // what time step are we learning on?
    size_t learn_a_idx;            // what action index are we trying?
    bool done_with_all_actions;    // set to true when there are no more learn_a_idx to go

    float test_loss;               // loss incurred when run INIT_TEST
    float learn_loss;              // loss incurred when run LEARN
    float train_loss;              // loss incurred when run INIT_TRAIN
    
    bool last_example_was_newline; // used so we know when a block of examples has passed
    bool hit_new_pass;             // have we hit a new pass?

    // if we're printing to stderr we need to remember if we've printed the header yet
    // (i.e., we do this if we're driving)
    bool printed_output_header;

    // various strings for different search states
    bool should_produce_string;
    stringstream *pred_string;
    stringstream *truth_string;
    stringstream *bad_string_stream;

    // parameters controlling interpolation
    float  beta;                   // interpolation rate
    float  alpha;                  // parameter used to adapt beta for dagger (see above comment), should be in (0,1)

    RollMethod rollout_method;     // 0=policy, 1=oracle, 2=mix_per_state, 3=mix_per_roll
    RollMethod rollin_method;
    float subsample_timesteps;     // train at every time step or just a (random) subset?

    bool   allow_current_policy;   // should the current policy be used for training? true for dagger
    bool   adaptive_beta;          // used to implement dagger-like algorithms. if true, beta = 1-(1-alpha)^n after n updates, and policy is mixed with oracle as \pi' = (1-beta)\pi^* + beta \pi
    size_t passes_per_policy;      // if we're not in dagger-mode, then we need to know how many passes to train a policy

    uint32_t current_policy;       // what policy are we training right now?

    // various statistics for reporting
    size_t num_features;
    uint32_t total_number_of_policies;
    size_t read_example_last_id;
    size_t passes_since_new_policy;
    size_t read_example_last_pass;
    size_t total_examples_generated;
    size_t total_predictions_made;
    size_t total_cache_hits;

    vector<example*> ec_seq;  // the collected examples
    v_hashmap<unsigned char*, action> cache_hash_map;
    
    // for foreach_feature temporary storage for conditioning
    uint32_t dat_new_feature_idx;
    example* dat_new_feature_ec;
    stringstream dat_new_feature_audit_ss;
    size_t dat_new_feature_namespace;
    string* dat_new_feature_feature_space;
    float dat_new_feature_value;

    // to reduce memory allocation
    string rawOutputString;
    stringstream* rawOutputStringStream;
    CS::label ldf_test_label;
    v_array<action> condition_on_actions;
    v_array<size_t> timesteps;
    v_array<float> learn_losses;
    
    LEARNER::learner* base_learner;
    clock_t start_clock_time;

    example*empty_example;

    Beam::beam< action_prefix > *beam;
    size_t kbest;            // size of kbest list; 1 just means 1best
    float beam_initial_cost; // when we're doing a subsequent run, how much do we initially pay?
    action_prefix beam_actions; // on non-initial beam runs, what prefix of actions should we take?
    float beam_total_cost;
    
    search_task* task;    // your task!
  };

  string   audit_feature_space("conditional");
  uint32_t conditional_constant = 8290743;
  
  int random_policy(search_private& priv, bool allow_current, bool allow_optimal, bool advance_prng=true) {
    if (priv.beta >= 1) {
      if (allow_current) return (int)priv.current_policy;
      if (priv.current_policy > 0) return (((int)priv.current_policy)-1);
      if (allow_optimal) return -1;
      std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
      return (int)priv.current_policy;
    }

    int num_valid_policies = (int)priv.current_policy + allow_optimal + allow_current;
    int pid = -1;

    if (num_valid_policies == 0) {
      std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
      return (int)priv.current_policy;
    } else if (num_valid_policies == 1)
      pid = 0;
    else if (num_valid_policies == 2)
      pid = (advance_prng ? frand48() : frand48_noadvance()) >= priv.beta;
    else {
      // SPEEDUP this up in the case that beta is small!
      float r = (advance_prng ? frand48() : frand48_noadvance());
      pid = 0;

      if (r > priv.beta) {
        r -= priv.beta;
        while ((r > 0) && (pid < num_valid_policies-1)) {
          pid ++;
          r -= priv.beta * powf(1.f - priv.beta, (float)pid);
        }
      }
    }
    // figure out which policy pid refers to
    if (allow_optimal && (pid == num_valid_policies-1))
      return -1; // this is the optimal policy

    pid = (int)priv.current_policy - pid;
    if (!allow_current)
      pid--;

    return pid;
  }

  int select_learner(search_private& priv, int policy, size_t learner_id) {
    if (policy<0) return policy;  // optimal policy
    else          return (int) (policy*priv.num_learners+learner_id);
  }


  bool should_print_update(vw& all, bool hit_new_pass=false) {
    //uncomment to print out final loss after all examples processed
    //commented for now so that outputs matches make test
    //if( parser_done(all.p)) return true;
    
    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS && hit_new_pass) return true;
    return (all.sd->weighted_examples >= all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  
  bool might_print_update(vw& all) {
    // basically do should_print_update but check me and the next
    // example because of off-by-ones

    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS) return true;  // SPEEDUP: make this better
    return (all.sd->weighted_examples + 1. >= all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  bool must_run_test(vw&all, vector<example*>ec, bool is_test_ex) {
    return
        (all.final_prediction_sink.size() > 0) ||   // if we have to produce output, we need to run this
        might_print_update(all) ||                  // if we have to print and update to stderr
        (all.raw_prediction > 0) ||                 // we need raw predictions
        // or:
        //   it's not quiet AND
        //     current_pass == 0
        //     OR holdout is off
        //     OR it's a test example
        ( //   (! all.quiet) &&  // had to disable this because of library mode!
          (! is_test_ex) &&
          ( all.holdout_set_off ||                    // no holdout
            ec[0]->test_only ||
            (all.current_pass == 0)                   // we need error rates for progressive cost
            ) )
        ;
  }

  void clear_seq(vw&all, search_private& priv) {
    if (priv.ec_seq.size() > 0)
      for (size_t i=0; i < priv.ec_seq.size(); i++)
        VW::finish_example(all, priv.ec_seq[i]);
    priv.ec_seq.clear();
  }

  float safediv(float a,float b) { if (b == 0.f) return 0.f; else return (a/b); }

  void to_short_string(string in, size_t max_len, char*out) {
    for (size_t i=0; i<max_len; i++)
      out[i] = ((i >= in.length()) || (in[i] == '\n') || (in[i] == '\t')) ? ' ' : in[i];

    if (in.length() > max_len) {
      out[max_len-2] = '.';
      out[max_len-1] = '.';
    }
    out[max_len] = 0;
  }

  void number_to_natural(size_t big, char* c) {
    if      (big > 9999999999) sprintf(c, "%dg", (int)(big / 1000000000));
    else if (big >    9999999) sprintf(c, "%dm", (int)(big /    1000000));
    else if (big >       9999) sprintf(c, "%dk", (int)(big /       1000));
    else                       sprintf(c, "%d",  (int)(big));
  }
  
  void print_update(search_private& priv) {
    vw& all = *priv.all;
    if (!priv.printed_output_header && !all.quiet) {
      const char * header_fmt = "%-10s %-10s %8s%24s %22s %5s %5s  %7s  %7s  %7s  %-8s\n";
      fprintf(stderr, header_fmt, "average", "since", "instance", "current true",  "current predicted", "cur",  "cur", "predic", "cache", "examples", "");
      fprintf(stderr, header_fmt, "loss",    "last",  "counter",  "output prefix",  "output prefix",    "pass", "pol", "made",    "hits",  "gener", "beta");
      std::cerr.precision(5);
      priv.printed_output_header = true;
    }

    if (!should_print_update(all, priv.hit_new_pass))
      return;

    char true_label[21];
    char pred_label[21];
    to_short_string(priv.truth_string->str(), 20, true_label);
    to_short_string(priv.pred_string->str() , 20, pred_label);

    float avg_loss = 0.;
    float avg_loss_since = 0.;
    bool use_heldout_loss = (!all.holdout_set_off && all.current_pass >= 1) && (all.sd->weighted_holdout_examples > 0);
    if (use_heldout_loss) {
      avg_loss       = safediv((float)all.sd->holdout_sum_loss, (float)all.sd->weighted_holdout_examples);
      avg_loss_since = safediv((float)all.sd->holdout_sum_loss_since_last_dump, (float)all.sd->weighted_holdout_examples_since_last_dump);

      all.sd->weighted_holdout_examples_since_last_dump = 0;
      all.sd->holdout_sum_loss_since_last_dump = 0.0;
    } else {
      avg_loss       = safediv((float)all.sd->sum_loss, (float)all.sd->weighted_examples);
      avg_loss_since = safediv((float)all.sd->sum_loss_since_last_dump, (float) (all.sd->weighted_examples - all.sd->old_weighted_examples));
    }

    char inst_cntr[9];  number_to_natural(all.sd->example_number, inst_cntr);
    char total_pred[8]; number_to_natural(priv.total_predictions_made, total_pred);
    char total_cach[8]; number_to_natural(priv.total_cache_hits, total_cach);
    char total_exge[8]; number_to_natural(priv.total_examples_generated, total_exge);
    
    fprintf(stderr, "%-10.6f %-10.6f %8s  [%s] [%s] %5d %5d  %7s  %7s  %7s  %-8f",
            avg_loss,
            avg_loss_since,
            inst_cntr,
            true_label,
            pred_label,
            (int)priv.read_example_last_pass,
            (int)priv.current_policy,
            total_pred,
            total_cach,
            total_exge,
            priv.beta);

    if (PRINT_CLOCK_TIME) {
      size_t num_sec = (size_t)(((float)(clock() - priv.start_clock_time)) / CLOCKS_PER_SEC);
      fprintf(stderr, " %15lusec", num_sec);
    }

    if (use_heldout_loss)
      fprintf(stderr, " h");

    fprintf(stderr, "\n");

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    fflush(stderr);
    VW::update_dump_interval(all);
  }

  void add_new_feature(search_private& priv, float val, uint32_t idx) {
    size_t mask = priv.all->reg.weight_mask;
    size_t ss   = priv.all->reg.stride_shift;
    size_t idx2 = ((idx & mask) >> ss) & mask;
    feature f = { val * priv.dat_new_feature_value,
                  (uint32_t) (((priv.dat_new_feature_idx + idx2) << ss) ) };
    priv.dat_new_feature_ec->atomics[priv.dat_new_feature_namespace].push_back(f);
    priv.dat_new_feature_ec->sum_feat_sq[priv.dat_new_feature_namespace] += f.x * f.x;
    if (priv.all->audit) {
      audit_data a = { NULL, NULL, f.weight_index, f.x, true };
      a.space   = (char*)calloc_or_die(priv.dat_new_feature_feature_space->length()+1, sizeof(char));
      a.feature = (char*)calloc_or_die(priv.dat_new_feature_audit_ss.str().length() + 32, sizeof(char));
      strcpy(a.space, priv.dat_new_feature_feature_space->c_str());
      int num = sprintf(a.feature, "fid=%lu_", (idx & mask) >> ss);
      strcpy(a.feature+num, priv.dat_new_feature_audit_ss.str().c_str());
      priv.dat_new_feature_ec->audit_features[priv.dat_new_feature_namespace].push_back(a);
    }
  }

  void del_features_in_top_namespace(search_private& priv, example& ec, size_t ns) {
    if ((ec.indices.size() == 0) || (ec.indices.last() != ns)) {
      std::cerr << "internal error (bug): expecting top namespace to be '" << ns << "' but it was ";
      if (ec.indices.size() == 0) std::cerr << "empty";
      else std::cerr << (size_t)ec.indices.last();
      std::cerr << endl;
      throw exception();
    }
    ec.num_features -= ec.atomics[ns].size();
    ec.total_sum_feat_sq -= ec.sum_feat_sq[ns];
    ec.sum_feat_sq[ns] = 0;
    ec.indices.decr();
    ec.atomics[ns].erase();
    if (priv.all->audit) {
      for (size_t i=0; i<ec.audit_features[ns].size(); i++)
        if (ec.audit_features[ns][i].alloced) {
          free(ec.audit_features[ns][i].space);
          free(ec.audit_features[ns][i].feature);
        }
      ec.audit_features[ns].erase();
    }
  }
  
  void add_neighbor_features(search_private& priv) {
    vw& all = *priv.all;
    if (priv.neighbor_features.size() == 0) return;

    for (size_t n=0; n<priv.ec_seq.size(); n++) {  // iterate over every example in the sequence
      example& me = *priv.ec_seq[n];
      for (size_t n_id=0; n_id < priv.neighbor_features.size(); n_id++) {
        int32_t offset = priv.neighbor_features[n_id] >> 24;
        size_t  ns     = priv.neighbor_features[n_id] & 0xFF;

        priv.dat_new_feature_ec = &me;
        priv.dat_new_feature_value = 1.;
        priv.dat_new_feature_idx = priv.neighbor_features[n_id] * 13748127;
        priv.dat_new_feature_namespace = neighbor_namespace;
        if (priv.all->audit) {
          priv.dat_new_feature_feature_space = &neighbor_feature_space;
          priv.dat_new_feature_audit_ss.str("");
          priv.dat_new_feature_audit_ss << '@' << ((offset > 0) ? '+' : '-') << (char)(abs(offset) + '0');
          if (ns != ' ') priv.dat_new_feature_audit_ss << (char)ns;
        }

        //cerr << "n=" << n << " offset=" << offset << endl;
        if (n + offset < 0) // add <s> feature
          add_new_feature(priv, 1., 925871901 << priv.all->reg.stride_shift);
        else if (n + offset >= priv.ec_seq.size()) // add </s> feature
          add_new_feature(priv, 1., 3824917 << priv.all->reg.stride_shift);
        else { // this is actually a neighbor
          example& other = *priv.ec_seq[n + offset];
          GD::foreach_feature<search_private,add_new_feature>(all.reg.weight_vector, all.reg.weight_mask, other.atomics[ns].begin, other.atomics[ns].end, priv, me.ft_offset);
        }
      }

      size_t sz = me.atomics[neighbor_namespace].size();
      if ((sz > 0) && (me.sum_feat_sq[neighbor_namespace] > 0.)) {
        me.indices.push_back(neighbor_namespace);
        me.total_sum_feat_sq += me.sum_feat_sq[neighbor_namespace];
        me.num_features += sz;
      } else {
        me.atomics[neighbor_namespace].erase();
        if (priv.all->audit) me.audit_features[neighbor_namespace].erase();
    }
    }
  }

  void del_neighbor_features(search_private& priv) {
    if (priv.neighbor_features.size() == 0) return;
    for (size_t n=0; n<priv.ec_seq.size(); n++)
      del_features_in_top_namespace(priv, *priv.ec_seq[n], neighbor_namespace);
  }

  void reset_search_structure(search_private& priv) {
    // NOTE: make sure do NOT reset priv.learn_a_idx
    priv.t = 0;
    priv.loss_declared_cnt = 0;
    priv.done_with_all_actions = false;
    priv.test_loss = 0.;
    priv.learn_loss = 0.;
    priv.train_loss = 0.;
    priv.num_features = 0;
    priv.should_produce_string = false;
    priv.mix_per_roll_policy = -2;
    if (priv.adaptive_beta) {
      float x = - log1pf(- priv.alpha) * (float)priv.total_examples_generated;
      static const float log_of_2 = (float)0.6931471805599453;
      priv.beta = (x <= log_of_2) ? -expm1f(-x) : (1-expf(-x)); // numerical stability
      //float priv_beta = 1.f - powf(1.f - priv.alpha, (float)priv.total_examples_generated);
      //assert( fabs(priv_beta - priv.beta) < 1e-2 );
      if (priv.beta > 1) priv.beta = 1;
    }
    priv.ptag_to_action.erase();
    if (priv.beam)
      priv.current_trajectory.erase();
    
    if (! priv.cb_learner) { // was: if rollout_all_actions
      uint32_t seed = (uint32_t)(priv.read_example_last_id * 147483 + 4831921) * 2147483647;
      msrand48(seed);
    }
  }

  void search_declare_loss(search_private& priv, float loss) {
    priv.loss_declared_cnt++;
    switch (priv.state) {
      case INIT_TEST:  priv.test_loss  += loss; break;
      case INIT_TRAIN: priv.train_loss += loss; break;
      case LEARN:
        if ((priv.rollout_num_steps == 0) || (priv.loss_declared_cnt <= priv.rollout_num_steps))
          priv.learn_loss += loss;
        break;
      default: break; // get rid of the warning about missing cases (danger!)
    }
  }

  size_t random(size_t max) { return (size_t)(frand48() * (float)max); }
  
  action choose_oracle_action(search_private& priv, size_t ec_cnt, const action* oracle_actions, size_t oracle_actions_cnt, const action* allowed_actions, size_t allowed_actions_cnt) {
    cdbg << "choose_oracle_action from oracle_actions = ["; for (size_t i=0; i<oracle_actions_cnt; i++) cdbg << " " << oracle_actions[i]; cdbg << " ]" << endl;
    return ( oracle_actions_cnt > 0) ?  oracle_actions[random(oracle_actions_cnt )] :
           (allowed_actions_cnt > 0) ? allowed_actions[random(allowed_actions_cnt)] :
           (action)random(ec_cnt);
  }

  void add_example_conditioning(search_private& priv, example& ec, const ptag* condition_on, size_t condition_on_cnt, const char* condition_on_names, const action* condition_on_actions) {
    if (condition_on_cnt == 0) return;

    uint32_t extra_offset=0;
    if (priv.is_ldf) 
      if (ec.l.cs.costs.size() > 0)
        extra_offset = 3849017 * ec.l.cs.costs[0].class_index;
    
    size_t I = condition_on_cnt;
    size_t N = max(priv.acset.max_bias_ngram_length, priv.acset.max_quad_ngram_length);
    for (size_t i=0; i<I; i++) { // position in conditioning
      uint32_t fid = 71933 + 8491087 * extra_offset;
      if (priv.all->audit) {
        priv.dat_new_feature_audit_ss.str("");
        priv.dat_new_feature_audit_ss.clear();
        priv.dat_new_feature_feature_space = &condition_feature_space;
      }

      for (size_t n=0; n<N; n++) { // length of ngram
        if (i + n >= I) break; // no more ngrams
        // we're going to add features for the ngram condition_on_actions[i .. i+N]
        char name = condition_on_names[i+n];
        fid = fid * 328901 + 71933 * ((condition_on_actions[i+n] + 349101) * (name + 38490137));

        priv.dat_new_feature_ec  = &ec;
        priv.dat_new_feature_idx = fid * quadratic_constant;
        priv.dat_new_feature_namespace = conditioning_namespace;
        priv.dat_new_feature_value = priv.acset.feature_value;

        if (priv.all->audit) {
          if (n > 0) priv.dat_new_feature_audit_ss << ',';
          if ((33 <= name) && (name <= 126)) priv.dat_new_feature_audit_ss << name;
          else priv.dat_new_feature_audit_ss << '#' << (int)name;
          priv.dat_new_feature_audit_ss << '=' << condition_on_actions[i+n];
        }
        
        // add the single bias feature
        if (n < priv.acset.max_bias_ngram_length)
          add_new_feature(priv, 1., 4398201 << priv.all->reg.stride_shift);

        // add the quadratic features
        if (n < priv.acset.max_quad_ngram_length)
          GD::foreach_feature<search_private,uint32_t,add_new_feature>(*priv.all, ec, priv);
      }
    }

    size_t sz = ec.atomics[conditioning_namespace].size();
    if ((sz > 0) && (ec.sum_feat_sq[conditioning_namespace] > 0.)) {
      ec.indices.push_back(conditioning_namespace);
      ec.total_sum_feat_sq += ec.sum_feat_sq[conditioning_namespace];
      ec.num_features += sz;
    } else {
      ec.atomics[conditioning_namespace].erase();
      if (priv.all->audit) ec.audit_features[conditioning_namespace].erase();
    }
  }

  void del_example_conditioning(search_private& priv, example& ec) {
    if ((ec.indices.size() > 0) && (ec.indices.last() == conditioning_namespace))
      del_features_in_top_namespace(priv, ec, conditioning_namespace);
  }
  
  size_t cs_get_costs_size(bool isCB, polylabel& ld) {
    return isCB ? ld.cb.costs.size()
                : ld.cs.costs.size();
  }

  uint32_t cs_get_cost_index(bool isCB, polylabel& ld, size_t k) {
    return isCB ? ld.cb.costs[k].action
                : ld.cs.costs[k].class_index;
  }

  float cs_get_cost_partial_prediction(bool isCB, polylabel& ld, size_t k) {
    return isCB ? ld.cb.costs[k].partial_prediction
                : ld.cs.costs[k].partial_prediction;
  }

  void cs_costs_erase(bool isCB, polylabel& ld) {
    if (isCB) ld.cb.costs.erase();
    else      ld.cs.costs.erase();
  }

  void cs_costs_resize(bool isCB, polylabel& ld, size_t new_size) {
    if (isCB) ld.cb.costs.resize(new_size);
    else      ld.cs.costs.resize(new_size);
  }

  void cs_cost_push_back(bool isCB, polylabel& ld, uint32_t index, float value) {
    if (isCB) { CB::cb_class cost = { value, index, 0., 0. }; ld.cb.costs.push_back(cost); }
    else      { CS::wclass   cost = { value, index, 0., 0. }; ld.cs.costs.push_back(cost); }
  }
  
  polylabel& allowed_actions_to_ld(search_private& priv, size_t ec_cnt, const action* allowed_actions, size_t allowed_actions_cnt) {
    bool isCB = priv.cb_learner;
    polylabel& ld = *priv.allowed_actions_cache;
    uint32_t num_costs = (uint32_t)cs_get_costs_size(isCB, ld);

    if (priv.is_ldf) {  // LDF version easier
      if (num_costs > ec_cnt)
        cs_costs_resize(isCB, ld, ec_cnt);
      else if (num_costs < ec_cnt)
        for (action k = num_costs; k < ec_cnt; k++)
          cs_cost_push_back(isCB, ld, k, FLT_MAX);
      
    } else { // non-LDF version
      if ((allowed_actions == NULL) || (allowed_actions_cnt == 0)) { // any action is allowed
        if (num_costs != priv.A) {  // if there are already A-many actions, they must be the right ones, unless the user did something stupid like putting duplicate allowed_actions...
          cs_costs_erase(isCB, ld);
          for (action k = 0; k < priv.A; k++)
            cs_cost_push_back(isCB, ld, k+1, FLT_MAX);  //+1 because MC is 1-based
        }
      } else { // we need to peek at allowed_actions
        cs_costs_erase(isCB, ld);
        for (size_t i = 0; i < allowed_actions_cnt; i++)
          cs_cost_push_back(isCB, ld, allowed_actions[i], FLT_MAX);
      }
    }

    return ld;
  }

  template<class T> bool array_contains(T target, const T*A, size_t n) {
    if (A == NULL) return false;
    for (size_t i=0; i<n; i++)
      if (A[i] == target) return true;
    return false;
  }
  
  void allowed_actions_to_losses(search_private& priv, size_t ec_cnt, const action* allowed_actions, size_t allowed_actions_cnt, const action* oracle_actions, size_t oracle_actions_cnt, v_array<float>& losses) {
    if (priv.is_ldf)  // LDF version easier
      for (action k=0; k<ec_cnt; k++)
        losses.push_back( array_contains<action>(k, oracle_actions, oracle_actions_cnt) ? 0.f : 1.f );
    else { // non-LDF
      if ((allowed_actions == NULL) || (allowed_actions_cnt == 0))  // any action is allowed
        for (action k=1; k<=priv.A; k++)
          losses.push_back( array_contains<action>(k, oracle_actions, oracle_actions_cnt) ? 0.f : 1.f );
      else
        for (size_t i=0; i<allowed_actions_cnt; i++) {
          action k = allowed_actions[i];
          losses.push_back( array_contains<action>(k, oracle_actions, oracle_actions_cnt) ? 0.f : 1.f );
        }
    }
  }
  
  action single_prediction_notLDF(search_private& priv, example& ec, int policy, const action* allowed_actions, size_t allowed_actions_cnt) {
    vw& all = *priv.all;
    
    polylabel old_label = ec.l;
    ec.l = allowed_actions_to_ld(priv, 1, allowed_actions, allowed_actions_cnt);
    priv.base_learner->predict(ec, policy);
    uint32_t act = ec.pred.multiclass;

    // in beam search mode, go through alternatives and add them as back-ups
    if (priv.beam) {
      float act_cost = 0;
      size_t K = cs_get_costs_size(priv.cb_learner, ec.l);
      for (size_t k = 0; k < K; k++)
        if (cs_get_cost_index(priv.cb_learner, ec.l, k) == act) {
          act_cost = cs_get_cost_partial_prediction(priv.cb_learner, ec.l, k);
          break;
        }

      priv.beam_total_cost += act_cost;
      size_t new_len = priv.current_trajectory.size() + 1;
      for (size_t k = 0; k < K; k++) {
        action k_act = cs_get_cost_index(priv.cb_learner, ec.l, k);
        if (k_act == act) continue;  // skip the taken action
        float delta_cost = cs_get_cost_partial_prediction(priv.cb_learner, ec.l, k) - act_cost + priv.beam_initial_cost;   // TODO: is delta_cost the right cost?
        // construct the action prefix
        action_prefix* px = new v_array<action>;
	*px = v_init<action>();
        px->resize(new_len);
        px->end = px->begin + new_len;
        memcpy(px->begin, priv.current_trajectory.begin, sizeof(action) * (new_len-1));
        px->begin[new_len-1] = k_act;
        uint32_t px_hash = uniform_hash(px->begin, sizeof(action) * new_len, 3419);
        if (! priv.beam->insert(px, delta_cost, px_hash)) {
          px->delete_v();  // SPEEDUP: could be more efficient by reusing for next action
          delete px;
        }
      }
    }
    
    // generate raw predictions if necessary
    if ((priv.state == INIT_TEST) && (all.raw_prediction > 0)) {
      priv.rawOutputStringStream->str("");
      for (size_t k = 0; k < cs_get_costs_size(priv.cb_learner, ec.l); k++) {
        if (k > 0) (*priv.rawOutputStringStream) << ' ';
        (*priv.rawOutputStringStream) << cs_get_cost_index(priv.cb_learner, ec.l, k) << ':' << cs_get_cost_partial_prediction(priv.cb_learner, ec.l, k);
      }
      all.print_text(all.raw_prediction, priv.rawOutputStringStream->str(), ec.tag);
    }
    
    ec.l = old_label;

    priv.total_predictions_made++;
    priv.num_features += ec.num_features;

    return act;
  }

  action single_prediction_LDF(search_private& priv, example* ecs, size_t ec_cnt, int policy) {
    CS::cs_label.default_label(&priv.ldf_test_label);
    CS::wclass wc = { 0., 1, 0., 0. };
    priv.ldf_test_label.costs.push_back(wc);

    // keep track of best (aka chosen) action
    float  best_prediction = 0.;
    action best_action = 0;

    size_t start_K = (priv.is_ldf && CSOAA_AND_WAP_LDF::LabelDict::ec_is_example_header(ecs[0])) ? 1 : 0;

    for (action a= (uint32_t)start_K; a<ec_cnt; a++) {
      cdbg << "== single_prediction_LDF a=" << a << "==" << endl;
      if (start_K > 0)
        CSOAA_AND_WAP_LDF::LabelDict::add_example_namespaces_from_example(ecs[a], ecs[0]);
        
      polylabel old_label = ecs[a].l;
      ecs[a].l.cs = priv.ldf_test_label;
      priv.base_learner->predict(ecs[a], policy);

      priv.empty_example->in_use = true;
      priv.base_learner->predict(*priv.empty_example);

      if ((a == start_K) || (ecs[a].partial_prediction < best_prediction)) {
        best_prediction = ecs[a].partial_prediction;
        best_action     = a;
      }
      
      priv.num_features += ecs[a].num_features;
      ecs[a].l = old_label;
      if (start_K > 0)
        CSOAA_AND_WAP_LDF::LabelDict::del_example_namespaces_from_example(ecs[a], ecs[0]);
    }

    if (priv.beam) {
      priv.beam_total_cost += best_prediction;
      size_t new_len = priv.current_trajectory.size() + 1;
      for (size_t k=start_K; k<ec_cnt; k++) {
        if (k == best_action) continue;
        float delta_cost = ecs[k].partial_prediction - best_prediction + priv.beam_initial_cost;
        action_prefix* px = new v_array<action>;
	*px = v_init<action>();
        px->resize(new_len);
        px->end = px->begin + new_len;
        memcpy(px->begin, priv.current_trajectory.begin, sizeof(action) * (new_len-1));
        px->begin[new_len-1] = (uint32_t)k;  // TODO: k or ld[k]?
        uint32_t px_hash = uniform_hash(px->begin, sizeof(action) * new_len, 3419);
        if (! priv.beam->insert(px, delta_cost, px_hash)) {
          px->delete_v();  // SPEEDUP: could be more efficient by reusing for next action
          delete px;
        }
      }
    }
    
    priv.total_predictions_made++;
    return best_action;
  }

  int choose_policy(search_private& priv, bool advance_prng=true) {
    RollMethod method = (priv.state == INIT_TEST ) ? POLICY :
                        (priv.state == LEARN     ) ? priv.rollout_method :
                        (priv.state == INIT_TRAIN) ? priv.rollin_method :
                        NO_ROLLOUT;   // this should never happen
    switch (method) {
      case POLICY:
        return random_policy(priv, priv.allow_current_policy || (priv.state == INIT_TEST), false, advance_prng);

      case ORACLE:
        return -1;

      case MIX_PER_STATE:
        return random_policy(priv, priv.allow_current_policy, true, advance_prng);

      case MIX_PER_ROLL:
        if (priv.mix_per_roll_policy == -2) // then we have to choose one!
          priv.mix_per_roll_policy = random_policy(priv, priv.allow_current_policy, true, advance_prng);
        return priv.mix_per_roll_policy;

    case NO_ROLLOUT:
    default:
        std::cerr << "internal error (bug): trying to rollin or rollout with NO_ROLLOUT" << endl;
        throw exception();
    }
  }

  template<class T> void cdbg_print_array(string str, v_array<T>& A) { cdbg << str << " = ["; for (size_t i=0; i<A.size(); i++) cdbg << " " << A[i]; cdbg << " ]" << endl; }
  template<class T> void cerr_print_array(string str, v_array<T>& A) { cerr << str << " = ["; for (size_t i=0; i<A.size(); i++) cerr << " " << A[i]; cerr << " ]" << endl; }
  
  template<class T>
  void ensure_size(v_array<T>& A, size_t sz) {
    if ((size_t)(A.end_array - A.begin) < sz) 
      A.resize(sz*2+1, true);
    A.end = A.begin + sz;
  }

  template<class T> void push_at(v_array<T>& v, T item, size_t pos) {
    if (v.size() > pos)
      v.begin[pos] = item;
    else {
      if (v.end_array > v.begin + pos) {
        // there's enough memory, just not enough filler
        v.begin[pos] = item;
        v.end = v.begin + pos + 1;
      } else {
        // there's not enough memory
        v.resize(2 * pos + 3, true);
        v.begin[pos] = item;
        v.end = v.begin + pos + 1;
      }
    }
  }
  
  void record_action(search_private& priv, ptag mytag, action a) {
    if (mytag == 0) return;
    push_at(priv.ptag_to_action, a, mytag);
  }

  bool cached_item_equivalent(unsigned char*& A, unsigned char*& B) {
    size_t sz_A = *A;
    size_t sz_B = *B;
    if (sz_A != sz_B) return false;
    return memcmp(A, B, sz_A) == 0;
  }

  void free_key(unsigned char* mem, action a) { free(mem); }
  void clear_cache_hash_map(search_private& priv) {
    priv.cache_hash_map.iter(free_key);
    priv.cache_hash_map.clear();
  }
  
  // returns true if found and do_store is false. if do_store is true, always returns true.
  bool cached_action_store_or_find(search_private& priv, ptag mytag, const ptag* condition_on, const char* condition_on_names, const action* condition_on_actions, size_t condition_on_cnt, int policy, size_t learner_id, action &a, bool do_store) {
    if (priv.no_caching) return do_store;
    if (mytag == 0) return do_store; // don't attempt to cache when tag is zero

    size_t sz  = sizeof(size_t) + sizeof(ptag) + sizeof(int) + sizeof(size_t) + sizeof(size_t) + condition_on_cnt * (sizeof(ptag) + sizeof(action) + sizeof(char));
    if (sz % 4 != 0) sz = 4 * (sz / 4 + 1); // make sure sz aligns to 4 so that uniform_hash does the right thing

    unsigned char* item = (unsigned char*)calloc(sz, 1);
    unsigned char* here = item;
    *here = (unsigned char)sz; here += sizeof(size_t);
    *here = mytag;             here += sizeof(ptag);
    *here = policy;            here += sizeof(int);
    *here = (unsigned char)learner_id;        here += sizeof(size_t);
    *here = (unsigned char)condition_on_cnt;  here += (unsigned char)sizeof(size_t);
    for (size_t i=0; i<condition_on_cnt; i++) {
      *here = condition_on[i];         here += sizeof(ptag);
      *here = condition_on_actions[i]; here += sizeof(action);
      *here = condition_on_names[i];   here += sizeof(char);  // SPEEDUP: should we align this at 4?
    }
    uint32_t hash = uniform_hash(item, sz, 3419);

    if (do_store) {
      priv.cache_hash_map.put(item, hash, a);
      return true;
    } else { // its a find
      a = priv.cache_hash_map.get(item, hash);
      free(item);
      return a != (action)-1;
    }
  }

  void generate_training_example(search_private& priv, v_array<float>& losses, bool add_conditioning=true) {
    // should we really subtract out min-loss?
    float min_loss = FLT_MAX, max_loss = -FLT_MAX;
    size_t num_min = 0;
    for (size_t i=0; i<losses.size(); i++) {
      if (losses[i] < min_loss) { min_loss = losses[i]; num_min = 1; }
      else if (losses[i] == min_loss) num_min++;
      if (losses[i] > max_loss) { max_loss = losses[i]; }
    }
    
    int learner = select_learner(priv, priv.current_policy, priv.learn_learner_id);
    
    if (!priv.is_ldf) {   // not LDF
      // since we're not LDF, it should be the case that ec_ref_cnt == 1
      // and learn_ec_ref[0] is a pointer to a single example
      assert(priv.learn_ec_ref_cnt == 1);
      assert(priv.learn_ec_ref != NULL);

      polylabel labels = allowed_actions_to_ld(priv, priv.learn_ec_ref_cnt, priv.learn_allowed_actions.begin, priv.learn_allowed_actions.size());
      cdbg_print_array("learn_allowed_actions", priv.learn_allowed_actions);
      //bool any_gt_1 = false;
      for (size_t i=0; i<losses.size(); i++) {
        losses[i] = losses[i] - min_loss;
        if (priv.cb_learner) labels.cb.costs[i].cost = losses[i];
        else                 labels.cs.costs[i].x    = losses[i];
      }

      example& ec = priv.learn_ec_ref[0];
      polylabel old_label = ec.l;
      ec.l = labels;
      ec.in_use = true;
      if (add_conditioning) add_example_conditioning(priv, ec, priv.learn_condition_on.begin, priv.learn_condition_on.size(), priv.learn_condition_on_names.begin, priv.learn_condition_on_act.begin);
      priv.base_learner->learn(ec, learner);
      if (add_conditioning) del_example_conditioning(priv, ec);
      ec.l = old_label;
      priv.total_examples_generated++;
    } else {              // is  LDF
      assert(losses.size() == priv.learn_ec_ref_cnt);
      size_t start_K = (priv.is_ldf && CSOAA_AND_WAP_LDF::LabelDict::ec_is_example_header(priv.learn_ec_ref[0])) ? 1 : 0;
      for (action a= (uint32_t)start_K; a<priv.learn_ec_ref_cnt; a++) {
        example& ec = priv.learn_ec_ref[a];

        CS::label& lab = ec.l.cs;
        if (lab.costs.size() == 0) {
          CS::wclass wc = { 0., 1, 0., 0. };
          lab.costs.push_back(wc);
        }
        lab.costs[0].x = losses[a] - min_loss;
        ec.in_use = true;
        if (add_conditioning) add_example_conditioning(priv, ec, priv.learn_condition_on.begin, priv.learn_condition_on.size(), priv.learn_condition_on_names.begin, priv.learn_condition_on_act.begin);
        priv.base_learner->learn(ec, learner);
        cdbg << "generate_training_example called learn on action a=" << a << ", costs.size=" << lab.costs.size() << " ec=" << &ec << endl;
        priv.total_examples_generated++;
      }
      priv.base_learner->learn(*priv.empty_example, learner);
      cdbg << "generate_training_example called learn on empty_example" << endl;

      for (action a= (uint32_t)start_K; a<priv.learn_ec_ref_cnt; a++) {
        example& ec = priv.learn_ec_ref[a];
        if (add_conditioning) 
          del_example_conditioning(priv, ec);
      }
    }
  }

  bool search_predictNeedsExample(search_private& priv) {
    // this is basically copied from the logic of search_predict()
    switch (priv.state) {
      case INITIALIZE: return false;
      case GET_TRUTH_STRING: return false;
      case INIT_TEST:
        if (priv.beam && (priv.t < priv.beam_actions.size()))
          return false;
        return true;
      case INIT_TRAIN:
        break;
      case LEARN:
        if (priv.t < priv.learn_t) return false;
        if (priv.t == priv.learn_t) return true;  // SPEEDUP: we really only need it on the last learn_a, but this is hard to know...
        // t > priv.learn_t
        if ((priv.rollout_num_steps > 0) && (priv.loss_declared_cnt >= priv.rollout_num_steps)) return false; // skipping
        break;
    }

    int pol = choose_policy(priv, false); // choose a policy but don't advance prng
    return (pol != -1);
  }
  
  // note: ec_cnt should be 1 if we are not LDF
  action search_predict(search_private& priv, example* ecs, size_t ec_cnt, ptag mytag, const action* oracle_actions, size_t oracle_actions_cnt, const ptag* condition_on, const char* condition_on_names, const action* allowed_actions, size_t allowed_actions_cnt, size_t learner_id) {
    size_t condition_on_cnt = condition_on_names ? strlen(condition_on_names) : 0;
    size_t t = priv.t;
    priv.t++;

    // make sure parameters come in pairs correctly
    assert((oracle_actions  == NULL) == (oracle_actions_cnt  == 0));
    assert((condition_on    == NULL) == (condition_on_names  == NULL));
    assert((allowed_actions == NULL) == (allowed_actions_cnt == 0));
    
    // if we're just after the string, choose an oracle action
    if (priv.state == GET_TRUTH_STRING)
      return choose_oracle_action(priv, ec_cnt, oracle_actions, oracle_actions_cnt, allowed_actions, allowed_actions_cnt);

    // if we're in LEARN mode and before learn_t, return the train action
    if ((priv.state == LEARN) && (t < priv.learn_t)) {
      assert(t < priv.train_trajectory.size());
      return priv.train_trajectory[t];
    }

    if (priv.beam && (priv.state == INIT_TEST) && (t < priv.beam_actions.size()))
      return priv.beam_actions[t];

    // for LDF, # of valid actions is ec_cnt; otherwise it's either allowed_actions_cnt or A
    size_t valid_action_cnt = priv.is_ldf ? ec_cnt :
                              (allowed_actions_cnt > 0) ? allowed_actions_cnt : priv.A;

    // if we're in LEARN mode and _at_ learn_t, then:
    //   - choose the next action
    //   - decide if we're done
    //   - if we are, then copy/mark the example ref
    if ((priv.state == LEARN) && (t == priv.learn_t)) {
      action a = (action)priv.learn_a_idx;
      priv.loss_declared_cnt = 0;
      
      priv.learn_a_idx++;
      priv.learn_loss = 0.;  // don't include "past cost"

      // check to see if we're done with available actions
      if (priv.learn_a_idx >= valid_action_cnt) {
        priv.done_with_all_actions = true;
        priv.learn_learner_id = learner_id;

        // set reference or copy example(s)
        if (oracle_actions_cnt > 0) priv.learn_oracle_action = oracle_actions[0];
        priv.learn_ec_ref_cnt = ec_cnt;
        if (priv.examples_dont_change)
          priv.learn_ec_ref = ecs;
        else {
          size_t label_size = priv.is_ldf ? sizeof(CS::label) : sizeof(MC::multiclass);
          void (*label_copy_fn)(void*,void*) = priv.is_ldf ? CS::cs_label.copy_label : NULL;
          
          ensure_size(priv.learn_ec_copy, ec_cnt);
          for (size_t i=0; i<ec_cnt; i++) 
            VW::copy_example_data(priv.all->audit, priv.learn_ec_copy.begin+i, ecs+i, label_size, label_copy_fn);

          priv.learn_ec_ref = priv.learn_ec_copy.begin;
        }

        // copy conditioning stuff and allowed actions
        if (priv.auto_condition_features) {
          ensure_size(priv.learn_condition_on,     condition_on_cnt);
          ensure_size(priv.learn_condition_on_act, condition_on_cnt);

          priv.learn_condition_on.end = priv.learn_condition_on.begin + condition_on_cnt;   // allow .size() to be used in lieu of _cnt

          memcpy(priv.learn_condition_on.begin, condition_on, condition_on_cnt * sizeof(ptag));
          
          for (size_t i=0; i<condition_on_cnt; i++)
            push_at(priv.learn_condition_on_act, ((1 <= condition_on[i]) && (condition_on[i] < priv.ptag_to_action.size())) ? priv.ptag_to_action[condition_on[i]] : 0, i);

          if (condition_on_names == NULL) {
            ensure_size(priv.learn_condition_on_names, 1);
            priv.learn_condition_on_names[0] = 0;
          } else {
            ensure_size(priv.learn_condition_on_names, strlen(condition_on_names)+1);
            strcpy(priv.learn_condition_on_names.begin, condition_on_names);
          }
        }

        ensure_size(priv.learn_allowed_actions, allowed_actions_cnt);
        memcpy(priv.learn_allowed_actions.begin, allowed_actions, allowed_actions_cnt*sizeof(action));
        cdbg_print_array("in LEARN, learn_allowed_actions", priv.learn_allowed_actions);
      }

      assert((allowed_actions_cnt == 0) || (a < allowed_actions_cnt));
      return (allowed_actions_cnt > 0) ? allowed_actions[a] : priv.is_ldf ? a : (a+1);
    }

    if ((priv.state == LEARN) && (t > priv.learn_t) && (priv.rollout_num_steps > 0) && (priv.loss_declared_cnt >= priv.rollout_num_steps)) {
      cdbg << "... skipping" << endl;
      if (priv.is_ldf) return 0;
      else if (allowed_actions_cnt > 0) return allowed_actions[0];
      else return 1;
    }

    
    if ((priv.state == INIT_TRAIN) ||
        (priv.state == INIT_TEST) ||
        ((priv.state == LEARN) && (t > priv.learn_t))) {
      // we actually need to run the policy
      
      int policy = choose_policy(priv);
      action a;

      cdbg << "executing policy " << policy << endl;
      
      bool gte_here = (priv.state == INIT_TRAIN) && (priv.rollout_method == NO_ROLLOUT) && (oracle_actions_cnt > 0);
      
      if (policy == -1)
        a = choose_oracle_action(priv, ec_cnt, oracle_actions, oracle_actions_cnt, allowed_actions, allowed_actions_cnt);

      if ((policy >= 0) || gte_here) {
        int learner = select_learner(priv, policy, learner_id);

        ensure_size(priv.condition_on_actions, condition_on_cnt);
        for (size_t i=0; i<condition_on_cnt; i++)
          priv.condition_on_actions[i] = ((1 <= condition_on[i]) && (condition_on[i] < priv.ptag_to_action.size())) ? priv.ptag_to_action[condition_on[i]] : 0;

        if (cached_action_store_or_find(priv, mytag, condition_on, condition_on_names, priv.condition_on_actions.begin, condition_on_cnt, policy, learner_id, a, false))
          // if this succeeded, 'a' has the right action
          priv.total_cache_hits++;
        else { // we need to predict, and then cache
          size_t start_K = (priv.is_ldf && CSOAA_AND_WAP_LDF::LabelDict::ec_is_example_header(ecs[0])) ? 1 : 0;
          if (priv.auto_condition_features)
            for (size_t n=start_K; n<ec_cnt; n++)
              add_example_conditioning(priv, ecs[n], condition_on, condition_on_cnt, condition_on_names, priv.condition_on_actions.begin);

          if (policy >= 0)   // only make a prediction if we're going to use the output
            a = priv.is_ldf ? single_prediction_LDF(priv, ecs, ec_cnt, learner)
                            : single_prediction_notLDF(priv, *ecs, learner, allowed_actions, allowed_actions_cnt);
          
          if (gte_here) {
            cdbg << "INIT_TRAIN, NO_ROLLOUT, at least one oracle_actions" << endl;
            // we can generate a training example _NOW_ because we're not doing rollouts
            v_array<float> losses = v_init<float>(); // SPEEDUP: move this to data structure
            allowed_actions_to_losses(priv, ec_cnt, allowed_actions, allowed_actions_cnt, oracle_actions, oracle_actions_cnt, losses);
            cdbg_print_array("losses", losses);
            priv.learn_ec_ref = ecs;
            priv.learn_ec_ref_cnt = ec_cnt;
            ensure_size(priv.learn_allowed_actions, allowed_actions_cnt);
            memcpy(priv.learn_allowed_actions.begin, allowed_actions, allowed_actions_cnt * sizeof(action));
            generate_training_example(priv, losses, false);
            losses.delete_v();
          }
          
          if (priv.auto_condition_features)
            for (size_t n=start_K; n<ec_cnt; n++)
              del_example_conditioning(priv, ecs[n]);

          cached_action_store_or_find(priv, mytag, condition_on, condition_on_names, priv.condition_on_actions.begin, condition_on_cnt, policy, learner_id, a, true);
        }
      }

      if (priv.state == INIT_TRAIN)
        priv.train_trajectory.push_back(a); // note the action for future reference
      
      return a;
    }

    std::cerr << "error: predict called in unknown state" << endl;
    throw exception();
  }
  
  inline bool cmp_size_t(const size_t a, const size_t b) { return a < b; }
  void get_training_timesteps(search_private& priv, v_array<size_t>& timesteps) {
    timesteps.erase();
    
    // if there's no subsampling to do, just return [0,T)
    if (priv.subsample_timesteps <= 0)
      for (size_t t=0; t<priv.T; t++)
        timesteps.push_back(t);

    // if subsample in (0,1) then pick steps with that probability, but ensuring there's at least one!
    else if (priv.subsample_timesteps < 1) {
      for (size_t t=0; t<priv.T; t++)
        if (frand48() <= priv.subsample_timesteps)
          timesteps.push_back(t);

      if (timesteps.size() == 0) // ensure at least one
        timesteps.push_back((size_t)(frand48() * priv.T));
    }

    // finally, if subsample >= 1, then pick (int) that many uniformly at random without replacement; could use an LFSR but why? :P
    else {
      while ((timesteps.size() < (size_t)priv.subsample_timesteps) &&
             (timesteps.size() < priv.T)) {
        size_t t = (size_t)(frand48() * (float)priv.T);
        if (! v_array_contains(timesteps, t))
          timesteps.push_back(t);
      }
      std::sort(timesteps.begin, timesteps.end, cmp_size_t);
    }
  }

  void free_action_prefix(action_prefix* px) {
    px->delete_v();
    delete px;
  }

  void free_action_prefix_string_pair(pair<action_prefix*,string>* p) {
    p->first->delete_v();
    delete p->first;
    delete p;
  }
  
  void final_beam_insert(search_private&priv, Beam::beam< pair<action_prefix*,string> >& beam, float cost, vector<action>& seq) {
    action_prefix* final = new action_prefix;  // TODO: can we memcpy/push_many?
    *final = v_init<action>();
    for (size_t i=0; i<seq.size(); i++)
      final->push_back(seq[i]);
    pair<action_prefix*,string>* p = priv.should_produce_string ? new pair<action_prefix*,string>(final, priv.pred_string->str()) : new pair<action_prefix*,string>(final, "");
    uint32_t final_hash = uniform_hash(final->begin, sizeof(action)*final->size(), 3419);
    if (!beam.insert(p, cost, final_hash)) {
      final->delete_v();
      delete final;
      delete p;
    }
  }
    
  
  void beam_predict(search& sch) {
    search_private& priv = *sch.priv;
    vw&all = *priv.all;
    bool old_no_caching = priv.no_caching;   // caching is incompatible with generating beam rollouts
    priv.no_caching = true;

    priv.beam->erase(free_action_prefix);
    clear_cache_hash_map(priv);

    reset_search_structure(priv);
    priv.beam_actions.erase();
    priv.state = INIT_TEST;
    priv.should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0) || (all.raw_prediction > 0);

    // do the initial prediction
    priv.beam_initial_cost = 0.;
    priv.beam_total_cost = 0.;
    priv.test_action_sequence.clear();
    if (priv.should_produce_string) priv.pred_string->str("");
    priv.task->run(sch, priv.ec_seq);
    if (all.raw_prediction > 0) all.print_text(all.raw_prediction, "end of initial beam prediction", priv.ec_seq[0]->tag);

    Beam::beam< pair<action_prefix*, string> > final_beam(max(1,priv.kbest));
    final_beam_insert(priv, final_beam, priv.beam_total_cost, priv.test_action_sequence);
    
    for (size_t beam_run=1; beam_run<priv.beam->get_beam_size(); beam_run++) {
      priv.beam->compact(free_action_prefix);
      Beam::beam_element<action_prefix>* item = priv.beam->pop_best_item();
      if (item != NULL) {
        reset_search_structure(priv);
        priv.beam_actions.erase();
        priv.state = INIT_TEST;
        priv.should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0) || (all.raw_prediction > 0);
        if (priv.should_produce_string) priv.pred_string->str("");
        priv.beam_initial_cost = item->cost;
        priv.beam_total_cost   = priv.beam_initial_cost;
        push_many(priv.beam_actions, item->data->begin, item->data->size());
        priv.test_action_sequence.clear();
        priv.task->run(sch, priv.ec_seq);
        if (all.raw_prediction > 0) all.print_text(all.raw_prediction, "end of next beam prediction", priv.ec_seq[0]->tag);
        final_beam_insert(priv, final_beam, priv.beam_total_cost, priv.test_action_sequence);
      }
    }

    final_beam.compact(free_action_prefix_string_pair);
    Beam::beam_element< pair<action_prefix*,string> >* best = final_beam.begin();
    while ((best != final_beam.end()) && !best->active) ++best;
    if (best != final_beam.end()) {
      // store in beam_actions the actions for this so that subsequent calls to ->_run() produce it!
      priv.beam_actions.erase();
      push_many(priv.beam_actions, best->data->first->begin, best->data->first->size());
    }

    if (all.final_prediction_sink.begin != all.final_prediction_sink.end) {  // need to produce prediction output
      v_array<char> new_tag = v_init<char>();
      for (; best != final_beam.end(); ++best)
        if (best->active) {
          new_tag.erase();
          new_tag.resize(50, true);
          int len = sprintf(new_tag.begin, "%-10.6f\t", best->cost);
          new_tag.end = new_tag.begin + len;
          push_many(new_tag, priv.ec_seq[0]->tag.begin, priv.ec_seq[0]->tag.size());
          for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink)
            all.print_text((int)*sink, best->data->second, new_tag);
        }
      new_tag.delete_v();
    }

    final_beam.erase(free_action_prefix_string_pair);

    priv.no_caching = old_no_caching;
  }
  
  template <bool is_learn>
  void train_single_example(search& sch, bool is_test_ex) {
    search_private& priv = *sch.priv;
    vw&all = *priv.all;
    bool ran_test = false;  // we must keep track so that even if we skip test, we still update # of examples seen

    clear_cache_hash_map(priv);
    
    // do an initial test pass to compute output (and loss)
    if (must_run_test(all, priv.ec_seq, is_test_ex)) {
      cdbg << "======================================== INIT TEST (" << priv.current_policy << "," << priv.read_example_last_pass << ") ========================================" << endl;

      ran_test = true;
      
      if (priv.beam)
        beam_predict(sch);
      
      // do the prediction
      reset_search_structure(priv);
      priv.state = INIT_TEST;
      priv.should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0) || (all.raw_prediction > 0);
      priv.pred_string->str("");
      priv.test_action_sequence.clear();
      priv.task->run(sch, priv.ec_seq);

      // accumulate loss
      if (! is_test_ex) { // we cannot accumulate loss on test examples!
        if (priv.ec_seq[0]->test_only) {
          all.sd->weighted_holdout_examples += 1.f;//test weight seen
          all.sd->weighted_holdout_examples_since_last_dump += 1.f;
          all.sd->weighted_holdout_examples_since_last_pass += 1.f;
          all.sd->holdout_sum_loss += priv.test_loss;
          all.sd->holdout_sum_loss_since_last_dump += priv.test_loss;
          all.sd->holdout_sum_loss_since_last_pass += priv.test_loss;//since last pass
        } else {
          all.sd->weighted_examples += 1.f;
          all.sd->total_features += priv.num_features;
          all.sd->sum_loss += priv.test_loss;
          all.sd->sum_loss_since_last_dump += priv.test_loss;
          all.sd->example_number++;
        }
      }
      
      // generate output
      for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink)
      if (priv.beam)
        all.print_text((int)*sink, "", priv.ec_seq[0]->tag);
      else
        all.print_text((int)*sink, priv.pred_string->str(), priv.ec_seq[0]->tag);

      if (all.raw_prediction > 0)
        all.print_text(all.raw_prediction, "", priv.ec_seq[0]->tag);
    }

    // if we're not training, then we're done!
    if ((!is_learn) || is_test_ex || priv.ec_seq[0]->test_only || (!priv.all->training))
      return;

    // SPEEDUP: if the oracle was never called, we can skip this!
    
    // do a pass over the data allowing oracle
    cdbg << "======================================== INIT TRAIN (" << priv.current_policy << "," << priv.read_example_last_pass << ") ========================================" << endl;
    reset_search_structure(priv);
    priv.state = INIT_TRAIN;
    priv.train_trajectory.erase();  // this is where we'll store the training sequence
    priv.task->run(sch, priv.ec_seq);

    if (!ran_test) {  // was  && !priv.ec_seq[0]->test_only) { but we know it's not test_only
      all.sd->weighted_examples += 1.f;
      all.sd->total_features += priv.num_features;
      all.sd->sum_loss += priv.test_loss;
      all.sd->sum_loss_since_last_dump += priv.test_loss;
      all.sd->example_number++;
    }
    
    // if there's nothing to train on, we're done!
    if ((priv.loss_declared_cnt == 0) || (priv.t == 0) || (priv.rollout_method == NO_ROLLOUT))
      return;
    
    // otherwise, we have some learn'in to do!
    // RollMethod old_rollout = priv.rollout_method;
    // for (size_t num_learn_passes=0; num_learn_passes<=(all.current_pass>0); num_learn_passes++){ // all.current_pass
    //   //if ((frand48() < 0.5) /* &(all.current_pass > 0) */)  priv.rollout_method = ORACLE;
    //   if (num_learn_passes == 0) priv.rollout_method = ORACLE;
    //   else priv.rollout_method = old_rollout;
      
    cdbg << "======================================== LEARN (" << priv.current_policy << "," << priv.read_example_last_pass << ") ========================================" << endl;
    priv.T = priv.t;
    get_training_timesteps(priv, priv.timesteps);
    priv.learn_losses.erase();
    cdbg_print_array("timesteps", priv.timesteps);
    for (size_t tid=0; tid<priv.timesteps.size(); tid++) {
      priv.learn_a_idx = 0;
      priv.done_with_all_actions = false;
      // for each action, roll out to get a loss
      while (! priv.done_with_all_actions) {
        reset_search_structure(priv);
        priv.beam_actions.erase();
        priv.state = LEARN;
        priv.learn_t = priv.timesteps[tid];
        cdbg << "learn_t = " << priv.learn_t << ", learn_a_idx = " << priv.learn_a_idx << endl;
        priv.task->run(sch, priv.ec_seq);
        priv.learn_losses.push_back( priv.learn_loss );  // SPEEDUP: should we just put this in a CS structure from the get-go?
        cdbg_print_array("learn_losses", priv.learn_losses);
      }
      // now we can make a training example
      generate_training_example(priv, priv.learn_losses);
      if (! priv.examples_dont_change)
        for (size_t n=0; n<priv.learn_ec_copy.size(); n++) {
          if (sch.priv->is_ldf) CS::cs_label.delete_label(&priv.learn_ec_copy[n].l.cs);
          else                  MC::mc_label.delete_label(&priv.learn_ec_copy[n].l.multi);
        }
      priv.learn_losses.erase();
    }
    // }
    // priv.rollout_method = old_rollout;
  }
    
  
  template <bool is_learn>
  void do_actual_learning(vw&all, search& sch) {
    search_private& priv = *sch.priv;

    if (priv.ec_seq.size() == 0)
      return;  // nothing to do :)

    bool is_test_ex = false;
    for (size_t i=0; i<priv.ec_seq.size(); i++)
      if (priv.label_is_test(&priv.ec_seq[i]->l)) { is_test_ex = true; break; }
    
    if (priv.task->run_setup) priv.task->run_setup(sch, priv.ec_seq);
    
    // if we're going to have to print to the screen, generate the "truth" string
    cdbg << "======================================== GET TRUTH STRING (" << priv.current_policy << "," << priv.read_example_last_pass << ") ========================================" << endl;
    if (might_print_update(all)) {
      if (is_test_ex)
        priv.truth_string->str("**test**");
      else {
        reset_search_structure(*sch.priv);
        priv.beam_actions.erase();
        priv.state = GET_TRUTH_STRING;
        priv.should_produce_string = true;
        priv.truth_string->str("");
        priv.task->run(sch, priv.ec_seq);
      }
    }

    add_neighbor_features(priv);
    train_single_example<is_learn>(sch, is_test_ex);
    del_neighbor_features(priv);

    if (priv.task->run_takedown) priv.task->run_takedown(sch, priv.ec_seq);
  }

  template <bool is_learn>
  void search_predict_or_learn(search& sch, learner& base, example& ec) {
    search_private& priv = *sch.priv;
    vw* all = priv.all;
    priv.base_learner = &base;
    bool is_real_example = true;

    if (example_is_newline(ec) || priv.ec_seq.size() >= all->p->ring_size - 2) {
      if (priv.ec_seq.size() >= all->p->ring_size - 2) // -2 to give some wiggle room
        std::cerr << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << std::endl;

      do_actual_learning<is_learn>(*all, sch);

      priv.hit_new_pass = false;
      priv.last_example_was_newline = true;
      is_real_example = false;
    } else {
      if (priv.last_example_was_newline)
        priv.ec_seq.clear();
      priv.ec_seq.push_back(&ec);
      priv.last_example_was_newline = false;
    }

    if (is_real_example)
      priv.read_example_last_id = ec.example_counter;
  }

  void end_pass(search& sch) {
    search_private& priv = *sch.priv;
    vw* all = priv.all;
    priv.hit_new_pass = true;
    priv.read_example_last_pass++;
    priv.passes_since_new_policy++;

    if (priv.passes_since_new_policy >= priv.passes_per_policy) {
      priv.passes_since_new_policy = 0;
      if(all->training)
        priv.current_policy++;
      if (priv.current_policy > priv.total_number_of_policies) {
        std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
        priv.current_policy = priv.total_number_of_policies;
      }
      //reset search_trained_nb_policies in options_from_file so it is saved to regressor file later
      std::stringstream ss;
      ss << priv.current_policy;
      VW::cmd_string_replace_value(all->file_options,"--search_trained_nb_policies", ss.str());
    }
  }

  void finish_example(vw& all, search& sch, example& ec) {
    if (ec.end_pass || example_is_newline(ec) || sch.priv->ec_seq.size() >= all.p->ring_size - 2) {
      print_update(*sch.priv);
      VW::finish_example(all, &ec);
      clear_seq(all, *sch.priv);
    }
  }

  void end_examples(search& sch) {
    search_private& priv = *sch.priv;
    vw* all    = priv.all;

    do_actual_learning<true>(*all, sch);

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << ((priv.passes_since_new_policy == 0) ? priv.current_policy : (priv.current_policy+1));
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --search_trained_nb_policies
      VW::cmd_string_replace_value(all->file_options,"--search_trained_nb_policies", ss1.str());
      ss2 << priv.total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --search_total_nb_policies
      VW::cmd_string_replace_value(all->file_options,"--search_total_nb_policies", ss2.str());
    }
  }

  bool mc_label_is_test(void* lab) {
	  if (MC::label_is_test((MC::multiclass*)lab) > 0)
		  return true;
	  else
		  return false;
  }
  
  void search_initialize(vw* all, search& sch) {
    search_private& priv = *sch.priv;
    priv.all = all;
    
    priv.auto_condition_features = false;
    priv.auto_hamming_loss = false;
    priv.examples_dont_change = false;
    priv.is_ldf = false;

    priv.label_is_test = mc_label_is_test;
    
    priv.A = 1;
    priv.num_learners = 1;
    priv.cb_learner = false;
    priv.state = INITIALIZE;
    priv.learn_learner_id = 0;
    priv.mix_per_roll_policy = -2;

    priv.t = 0;
    priv.T = 0;
    priv.learn_ec_ref = NULL;
    priv.learn_ec_ref_cnt = 0;
    //priv.allowed_actions_cache = NULL;
    
    priv.loss_declared_cnt = 0;
    priv.learn_t = 0;
    priv.learn_a_idx = 0;
    priv.done_with_all_actions = false;

    priv.test_loss = 0.;
    priv.learn_loss = 0.;
    priv.train_loss = 0.;
    
    priv.last_example_was_newline = false;
    priv.hit_new_pass = false;

    priv.printed_output_header = false;

    priv.should_produce_string = false;
    priv.pred_string  = new stringstream();
    priv.truth_string = new stringstream();
    priv.bad_string_stream = new stringstream();
    priv.bad_string_stream->clear(priv.bad_string_stream->badbit);
        
    priv.beta = 0.5;
    priv.alpha = 1e-10f;

    priv.rollout_method = MIX_PER_ROLL;
    priv.rollin_method  = MIX_PER_ROLL;
    priv.subsample_timesteps = 0.;
    
    priv.allow_current_policy = true;
    priv.adaptive_beta = true;
    priv.passes_per_policy = 1;     //this should be set to the same value as --passes for dagger

    priv.current_policy = 0;

    priv.num_features = 0;
    priv.total_number_of_policies = 1;
    priv.read_example_last_id = 0;
    priv.passes_per_policy = 0;
    priv.read_example_last_pass = 0;
    priv.total_examples_generated = 0;
    priv.total_predictions_made = 0;
    priv.total_cache_hits = 0;

    priv.history_length = 1;
    priv.acset.max_bias_ngram_length = 1;
    priv.acset.max_quad_ngram_length = 0;
    priv.acset.feature_value = 1.;

    priv.cache_hash_map.set_default_value((action)-1);
    priv.cache_hash_map.set_equivalent(cached_item_equivalent);
    
    priv.task = NULL;
    sch.task_data = NULL;

    priv.empty_example = alloc_examples(sizeof(CS::label), 1);
    CS::cs_label.default_label(&priv.empty_example->l.cs);
    priv.empty_example->in_use = true;

    priv.rawOutputStringStream = new stringstream(priv.rawOutputString);
  }

  void search_finish(search& sch) {
    search_private& priv = *sch.priv;
    cdbg << "search_finish" << endl;

    clear_cache_hash_map(priv);

    delete priv.truth_string;
    delete priv.pred_string;
    delete priv.bad_string_stream;
    priv.neighbor_features.delete_v();
    priv.timesteps.delete_v();
    priv.learn_losses.delete_v();
    priv.condition_on_actions.delete_v();
    priv.learn_allowed_actions.delete_v();
    priv.ldf_test_label.costs.delete_v();
    
    if (priv.beam) {
      priv.beam->erase(free_action_prefix);
      delete priv.beam;
    }
    priv.beam_actions.delete_v();
    
    if (priv.cb_learner)
      priv.allowed_actions_cache->cb.costs.delete_v();
    else
      priv.allowed_actions_cache->cs.costs.delete_v();

    priv.train_trajectory.delete_v();
    priv.current_trajectory.delete_v();
    priv.ptag_to_action.delete_v();
    
    dealloc_example(CS::cs_label.delete_label, *(priv.empty_example));
    free(priv.empty_example);

    priv.ec_seq.clear();

    // destroy copied examples if we needed them
    if (! priv.examples_dont_change) {
      void (*delete_label)(void*) = priv.is_ldf ? CS::cs_label.delete_label : MC::mc_label.delete_label;
      for(example*ec = priv.learn_ec_copy.begin; ec!=priv.learn_ec_copy.end; ++ec)
        dealloc_example(delete_label, *ec);
      priv.learn_ec_copy.delete_v();
    }
    priv.learn_condition_on_names.delete_v();
    priv.learn_condition_on.delete_v();
    priv.learn_condition_on_act.delete_v();
    
    if (priv.task->finish != NULL) {
      priv.task->finish(sch);
    }

    free(priv.allowed_actions_cache);
    delete priv.rawOutputStringStream;
    delete sch.priv;
  }

  void ensure_param(float &v, float lo, float hi, float def, const char* string) {
    if ((v < lo) || (v > hi)) {
      std::cerr << string << endl;
      v = def;
    }
  }

  bool string_equal(string a, string b) { return a.compare(b) == 0; }
  bool float_equal(float a, float b) { return fabs(a-b) < 1e-6; }
  bool uint32_equal(uint32_t a, uint32_t b) { return a==b; }
  bool size_equal(size_t a, size_t b) { return a==b; }

  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string) {
    if (vm.count(opt_name)) {
      ret = vm[opt_name].as<T>();
      stringstream ss;
      ss << " --" << opt_name << " " << ret;
      all.file_options.append(ss.str());
    } else if (strlen(required_error_string)>0) {
      std::cerr << required_error_string << endl;
      if (! vm.count("help"))
        throw exception();
    }
  }

  void check_option(bool& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string) {
    if (vm.count(opt_name)) {
      ret = true;
      stringstream ss;
      ss << " --" << opt_name;
      all.file_options.append(ss.str());
    } else
      ret = false;
  }

  void handle_condition_options(vw& vw, auto_condition_settings& acset, po::variables_map& vm) {
    po::options_description condition_options("Search Auto-conditioning Options");
    condition_options.add_options()
        ("search_max_bias_ngram_length",   po::value<size_t>(), "add a \"bias\" feature for each ngram up to and including this length. eg., if it's 1 (default), then you get a single feature for each conditional")
        ("search_max_quad_ngram_length",   po::value<size_t>(), "add bias *times* input features for each ngram up to and including this length (def: 0)")
        ("search_condition_feature_value", po::value<float> (), "how much weight should the conditional features get? (def: 1.)");

    vm = add_options(vw, condition_options);

    check_option<size_t>(acset.max_bias_ngram_length, vw, vm, "search_max_bias_ngram_length", false, size_equal,
                         "warning: you specified a different value for --search_max_bias_ngram_length than the one loaded from regressor. proceeding with loaded value: ", "");

    check_option<size_t>(acset.max_quad_ngram_length, vw, vm, "search_max_quad_ngram_length", false, size_equal,
                         "warning: you specified a different value for --search_max_quad_ngram_length than the one loaded from regressor. proceeding with loaded value: ", "");

    check_option<float> (acset.feature_value, vw, vm, "search_condition_feature_value", false, float_equal,
                         "warning: you specified a different value for --search_condition_feature_value than the one loaded from regressor. proceeding with loaded value: ", "");
  }

  v_array<CS::label> read_allowed_transitions(action A, const char* filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
      std::cerr << "error: could not read file " << filename << " (" << strerror(errno) << "); assuming all transitions are valid" << endl;
      throw exception();
    }

    bool* bg = (bool*)malloc((A+1)*(A+1) * sizeof(bool));
    int rd,from,to,count=0;
    while ((rd = fscanf(f, "%d:%d", &from, &to)) > 0) {
      if ((from < 0) || (from > (int)A)) { std::cerr << "warning: ignoring transition from " << from << " because it's out of the range [0," << A << "]" << endl; }
      if ((to   < 0) || (to   > (int)A)) { std::cerr << "warning: ignoring transition to "   << to   << " because it's out of the range [0," << A << "]" << endl; }
      bg[from * (A+1) + to] = true;
      count++;
    }
    fclose(f);

    v_array<CS::label> allowed = v_init<CS::label>();

    for (size_t from=0; from<A; from++) {
      v_array<CS::wclass> costs = v_init<CS::wclass>();

      for (size_t to=0; to<A; to++)
        if (bg[from * (A+1) + to]) {
          CS::wclass c = { FLT_MAX, (action)to, 0., 0. };
          costs.push_back(c);
        }

      CS::label ld = { costs };
      allowed.push_back(ld);
    }
    free(bg);

    std::cerr << "read " << count << " allowed transitions from " << filename << endl;

    return allowed;
  }


  void parse_neighbor_features(string& nf_string, search&sch) {
    search_private& priv = *sch.priv;
    priv.neighbor_features.erase();
    size_t len = nf_string.length();
    if (len == 0) return;

    char * cstr = new char [len+1];
    strcpy(cstr, nf_string.c_str());

    char * p = strtok(cstr, ",");
    v_array<substring> cmd = v_init<substring>();
    while (p != 0) {
      cmd.erase();
      substring me = { p, p+strlen(p) };
      tokenize(':', me, cmd, true);

      int32_t posn = 0;
      char ns = ' ';
      if (cmd.size() == 1) {
        posn = int_of_substring(cmd[0]);
        ns   = ' ';
      } else if (cmd.size() == 2) {
        posn = int_of_substring(cmd[0]);
        ns   = (cmd[1].end > cmd[1].begin) ? cmd[1].begin[0] : ' ';
      } else {
        std::cerr << "warning: ignoring malformed neighbor specification: '" << p << "'" << endl;
      }
      int32_t enc = (posn << 24) | (ns & 0xFF);
      priv.neighbor_features.push_back(enc);

      p = strtok(NULL, ",");
    }
    cmd.delete_v();

    delete[] cstr;
  }

  learner* setup(vw&all, po::variables_map& vm) {
    search* sch = (search*)calloc_or_die(1,sizeof(search));
    sch->priv = new search_private();
    search_initialize(&all, *sch);
    search_private& priv = *sch->priv;

    po::options_description search_opts("Search Options");
    search_opts.add_options()
        ("search_task",              po::value<string>(), "the search task (use \"--search_task list\" to get a list of available tasks)")
        ("search_interpolation",     po::value<string>(), "at what level should interpolation happen? [*data|policy]")
        ("search_rollout",           po::value<string>(), "how should rollouts be executed?           [policy|oracle|*mix_per_state|mix_per_roll|none]")
        ("search_rollin",            po::value<string>(), "how should past trajectories be generated? [policy|oracle|*mix_per_state|mix_per_roll]")

        ("search_passes_per_policy", po::value<size_t>(), "number of passes per policy (only valid for search_interpolation=policy)     [def=1]")
        ("search_beta",              po::value<float>(),  "interpolation rate for policies (only valid for search_interpolation=policy) [def=0.5]")

        ("search_alpha",             po::value<float>(),  "annealed beta = 1-(1-alpha)^t (only valid for search_interpolation=data)     [def=1e-10]")

        ("search_total_nb_policies", po::value<size_t>(), "if we are going to train the policies through multiple separate calls to vw, we need to specify this parameter and tell vw how many policies are eventually going to be trained")

        ("search_trained_nb_policies", po::value<size_t>(), "the number of trained policies in a file")

        ("search_allowed_transitions",po::value<string>(),"read file of allowed transitions [def: all transitions are allowed]")
        ("search_subsample_time",    po::value<float>(),  "instead of training at all timesteps, use a subset. if value in (0,1), train on a random v%. if v>=1, train on precisely v steps per example")
        ("search_neighbor_features", po::value<string>(), "copy features from neighboring lines. argument looks like: '-1:a,+2' meaning copy previous line namespace a and next next line from namespace _unnamed_, where ',' separates them")
        ("search_rollout_num_steps", po::value<size_t>(), "how many calls of \"loss\" before we stop really predicting on rollouts and switch to oracle (def: 0 means \"infinite\")")
        ("search_history_length",    po::value<size_t>(), "some tasks allow you to specify how much history their depend on; specify that here [def: 1]")

        ("search_no_caching",                             "turn off the built-in caching ability (makes things slower, but technically more safe)")
        ("search_beam",              po::value<size_t>(), "use beam search (arg = beam size, default 0 = no beam)")
        ("search_kbest",             po::value<size_t>(), "size of k-best list to produce (must be <= beam size)")
        ;

    bool has_hook_task = false;
    for (size_t i=0; i<all.args.size()-1; i++)
      if (all.args[i] == "--search_task" && all.args[i+1] == "hook")
        has_hook_task = true;
    if (has_hook_task)
      for (int i = (int)all.args.size()-2; i >= 0; i--)
        if (all.args[i] == "--search_task" && all.args[i+1] != "hook")
          all.args.erase(all.args.begin() + i, all.args.begin() + i + 2);

    vm = add_options(all, search_opts);
 
    std::string task_string;
    std::string interpolation_string = "data";
    std::string rollout_string = "mix_per_state";
    std::string rollin_string = "mix_per_state";

    check_option<string>(task_string, all, vm, "search_task", false, string_equal,
                         "warning: specified --search_task different than the one loaded from regressor. using loaded value of: ",
                         "error: you must specify a task using --search_task");
      
    check_option<string>(interpolation_string, all, vm, "search_interpolation", false, string_equal,
                         "warning: specified --search_interpolation different than the one loaded from regressor. using loaded value of: ", "");

    if (vm.count("search_passes_per_policy"))       priv.passes_per_policy    = vm["search_passes_per_policy"].as<size_t>();

    if (vm.count("search_alpha"))                   priv.alpha                = vm["search_alpha"            ].as<float>();
    if (vm.count("search_beta"))                    priv.beta                 = vm["search_beta"             ].as<float>();

    if (vm.count("search_subsample_time"))          priv.subsample_timesteps  = vm["search_subsample_time"].as<float>();
    if (vm.count("search_no_caching"))              priv.no_caching           = true;
    if (vm.count("search_rollout_num_steps"))       priv.rollout_num_steps    = vm["search_rollout_num_steps"].as<size_t>();

    if (vm.count("search_beam"))
      priv.beam = new Beam::beam<action_prefix>(vm["search_beam"].as<size_t>());  // TODO: pruning, kbest, equivalence testing
    else
      priv.beam = NULL;

    priv.kbest = 1;
    if (vm.count("search_kbest")) {
      priv.kbest = max(1, vm["search_kbest"].as<size_t>());
      if (priv.kbest > priv.beam->get_beam_size()) {
        cerr << "warning: kbest set greater than beam size; shrinking back to " << priv.beam->get_beam_size() << endl;
        priv.kbest = priv.beam->get_beam_size();
      }
    }
    
    priv.A = vm["search"].as<size_t>();

    string neighbor_features_string;
    check_option<string>(neighbor_features_string, all, vm, "search_neighbor_features", false, string_equal,
                         "warning: you specified a different feature structure with --search_neighbor_features than the one loaded from predictor. using loaded value of: ", "");
    parse_neighbor_features(neighbor_features_string, *sch);

    if (interpolation_string.compare("data") == 0) { // run as dagger
      priv.adaptive_beta = true;
      priv.allow_current_policy = true;
      priv.passes_per_policy = all.numpasses;
      if (priv.current_policy > 1) priv.current_policy = 1;
    } else if (interpolation_string.compare("policy") == 0) {
    } else {
      std::cerr << "error: --search_interpolation must be 'data' or 'policy'" << endl;
      throw exception();
    }

    if (vm.count("search_rollout")) rollout_string = vm["search_rollout"].as<string>();
    if (vm.count("search_rollin" )) rollin_string  = vm["search_rollin" ].as<string>();
    
    if      (rollout_string.compare("policy") == 0)          priv.rollout_method = POLICY;
    else if (rollout_string.compare("oracle") == 0)          priv.rollout_method = ORACLE;
    else if (rollout_string.compare("mix_per_state") == 0)   priv.rollout_method = MIX_PER_STATE;
    else if (rollout_string.compare("mix_per_roll") == 0)    priv.rollout_method = MIX_PER_ROLL;
    else if (rollout_string.compare("none") == 0)          { priv.rollout_method = NO_ROLLOUT; priv.no_caching = true; cerr << "no rollout!" << endl; }
    else {
      std::cerr << "error: --search_rollout must be 'policy', 'oracle', 'mix_per_state', 'mix_per_roll' or 'none'" << endl;
      throw exception();
    }

    if      (rollin_string.compare("policy") == 0)         priv.rollin_method = POLICY;
    else if (rollin_string.compare("oracle") == 0)         priv.rollin_method = ORACLE;
    else if (rollin_string.compare("mix_per_state") == 0)  priv.rollin_method = MIX_PER_STATE;
    else if (rollin_string.compare("mix_per_roll") == 0)   priv.rollin_method = MIX_PER_ROLL;
    else {
      std::cerr << "error: --search_rollin must be 'policy', 'oracle', 'mix_per_state' or 'mix_per_roll'" << endl;
      throw exception();
    }
    
    check_option<size_t>(priv.A, all, vm, "search", false, size_equal,
                         "warning: you specified a different number of actions through --search than the one loaded from predictor. using loaded value of: ", "");

    check_option<size_t>(priv.history_length, all, vm, "search_history_length", false, size_equal,
                         "warning: you specified a different history length through --search_history_length than the one loaded from predictor. using loaded value of: ", "");
    
    //check if the base learner is contextual bandit, in which case, we dont rollout all actions.
    priv.allowed_actions_cache = (polylabel*)calloc_or_die(1,sizeof(polylabel));
    if (vm.count("cb")) {
      priv.cb_learner = true;
      CB::cb_label.default_label(priv.allowed_actions_cache);
    } else {
      priv.cb_learner = false;
      CS::cs_label.default_label(priv.allowed_actions_cache);
    }

    //if we loaded a regressor with -i option, --search_trained_nb_policies contains the number of trained policies in the file
    // and --search_total_nb_policies contains the total number of policies in the file
    if (vm.count("search_total_nb_policies"))
      priv.total_number_of_policies = (uint32_t)vm["search_total_nb_policies"].as<size_t>();

    ensure_param(priv.beta , 0.0, 1.0, 0.5, "warning: search_beta must be in (0,1); resetting to 0.5");
    ensure_param(priv.alpha, 0.0, 1.0, 1e-10f, "warning: search_alpha must be in (0,1); resetting to 1e-10");

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    uint32_t tmp_number_of_policies = priv.current_policy;
    if( all.training )
      tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)priv.passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls,
    //so only set total_number_of_policies to computed value if it is larger
    cdbg << "current_policy=" << priv.current_policy << " tmp_number_of_policies=" << tmp_number_of_policies << " total_number_of_policies=" << priv.total_number_of_policies << endl;
    if( tmp_number_of_policies > priv.total_number_of_policies ) {
      priv.total_number_of_policies = tmp_number_of_policies;
      if( priv.current_policy > 0 ) //we loaded a file but total number of policies didn't match what is needed for training
        std::cerr << "warning: you're attempting to train more classifiers than was allocated initially. Likely to cause bad performance." << endl;
    }

    //current policy currently points to a new policy we would train
    //if we are not training and loaded a bunch of policies for testing, we need to subtract 1 from current policy
    //so that we only use those loaded when testing (as run_prediction is called with allow_current to true)
    if( !all.training && priv.current_policy > 0 )
      priv.current_policy--;

    std::stringstream ss1, ss2;
    ss1 << priv.current_policy;           VW::cmd_string_replace_value(all.file_options,"--search_trained_nb_policies", ss1.str());
    ss2 << priv.total_number_of_policies; VW::cmd_string_replace_value(all.file_options,"--search_total_nb_policies",   ss2.str());

    cdbg << "search current_policy = " << priv.current_policy << " total_number_of_policies = " << priv.total_number_of_policies << endl;

    if (task_string.compare("list") == 0) {
      std::cerr << endl << "available search tasks:" << endl;
      for (search_task** mytask = all_tasks; *mytask != NULL; mytask++)
        std::cerr << "  " << (*mytask)->task_name << endl;
      std::cerr << endl;
      exit(0);
    }
    for (search_task** mytask = all_tasks; *mytask != NULL; mytask++)
      if (task_string.compare((*mytask)->task_name) == 0) {
        priv.task = *mytask;
        sch->task_name = (*mytask)->task_name;
        break;
      }
    if (priv.task == NULL) {
      if (! vm.count("help")) {
        std::cerr << "fail: unknown task for --search_task '" << task_string << "'; use --search_task list to get a list" << endl;
        throw exception();
      }
    }
    all.p->emptylines_separate_examples = true;

    // default to OAA labels unless the task wants to override this (which they can do in initialize)
    all.p->lp = MC::mc_label;
    if (priv.task)
      priv.task->initialize(*sch, priv.A, vm);

    if (vm.count("search_allowed_transitions"))     read_allowed_transitions((action)priv.A, vm["search_allowed_transitions"].as<string>().c_str());
    
    // set up auto-history if they want it
    if (priv.auto_condition_features) {
      handle_condition_options(all, priv.acset, vm);

      // turn off auto-condition if it's irrelevant
      if (((priv.acset.max_bias_ngram_length == 0) && (priv.acset.max_quad_ngram_length == 0)) ||
          (priv.acset.feature_value == 0.f)) {
        std::cerr << "warning: turning off AUTO_CONDITION_FEATURES because settings make it useless" << endl;
        priv.auto_condition_features = false;
      }
    }

    if (!priv.allow_current_policy) // if we're not dagger
      all.check_holdout_every_n_passes = priv.passes_per_policy;

    all.searchstr = sch;

    priv.start_clock_time = clock();

    learner* l = new learner(sch, all.l, priv.total_number_of_policies);
    l->set_learn<search, search_predict_or_learn<true> >();
    l->set_predict<search, search_predict_or_learn<false> >();
    l->set_finish_example<search,finish_example>();
    l->set_end_examples<search,end_examples>();
    l->set_finish<search,search_finish>();
    l->set_end_pass<search,end_pass>();

    return l;
  }

  float action_hamming_loss(action a, const action* A, size_t sz) {
    if (sz == 0) return 0.;   // latent variables have zero loss
    for (size_t i=0; i<sz; i++)
      if (a == A[i]) return 0.;
    return 1.;
  }
  
  // the interface:
  bool search::is_ldf() { return this->priv->is_ldf; }

  action search::predict(example& ec, ptag mytag, const action* oracle_actions, size_t oracle_actions_cnt, const ptag* condition_on, const char* condition_on_names, const action* allowed_actions, size_t allowed_actions_cnt, size_t learner_id) {
    action a = search_predict(*this->priv, &ec, 1, mytag, oracle_actions, oracle_actions_cnt, condition_on, condition_on_names, allowed_actions, allowed_actions_cnt, learner_id);
    if (priv->beam) priv->current_trajectory.push_back(a);
    if (priv->state == INIT_TEST) priv->test_action_sequence.push_back(a);
    if (mytag != 0) push_at(priv->ptag_to_action, a, mytag);
    if (this->priv->auto_hamming_loss)
      loss(action_hamming_loss(a, oracle_actions, oracle_actions_cnt));
    cdbg << "predict returning " << a << endl;
    return a;
  }

  action search::predictLDF(example* ecs, size_t ec_cnt, ptag mytag, const action* oracle_actions, size_t oracle_actions_cnt, const ptag* condition_on, const char* condition_on_names, size_t learner_id) {
    action a = search_predict(*this->priv, ecs, ec_cnt, mytag, oracle_actions, oracle_actions_cnt, condition_on, condition_on_names, NULL, 0, learner_id);
    if (priv->beam) priv->current_trajectory.push_back(a);
    if (priv->state == INIT_TEST) priv->test_action_sequence.push_back(a);
    if ((mytag != 0) && ecs[a].l.cs.costs.size() > 0)
      push_at(priv->ptag_to_action, ecs[a].l.cs.costs[0].class_index, mytag);
    if (this->priv->auto_hamming_loss)
      loss(action_hamming_loss(a, oracle_actions, oracle_actions_cnt));
    cdbg << "predict returning " << a << endl;
    return a;
  }

  void search::loss(float loss) { search_declare_loss(*this->priv, loss); }

  bool search::predictNeedsExample() { return search_predictNeedsExample(*this->priv); }
  
  stringstream& search::output() {
    if      (!this->priv->should_produce_string    ) return *(this->priv->bad_string_stream);
    else if ( this->priv->state == GET_TRUTH_STRING) return *(this->priv->truth_string);
    else                                             return *(this->priv->pred_string);
  }

  void  search::set_options(uint32_t opts) {
    if (this->priv->state != INITIALIZE) {
      std::cerr << "error: task cannot set options except in initialize function!" << endl;
      throw exception();
    }
    if ((opts & AUTO_CONDITION_FEATURES) != 0) this->priv->auto_condition_features = true;
    if ((opts & AUTO_HAMMING_LOSS)       != 0) this->priv->auto_hamming_loss = true;
    if ((opts & EXAMPLES_DONT_CHANGE)    != 0) this->priv->examples_dont_change = true;
    if ((opts & IS_LDF)                  != 0) this->priv->is_ldf = true;
  }

  void search::set_label_parser(label_parser&lp, bool (*is_test)(void*)) {
    if (this->priv->state != INITIALIZE) {
      std::cerr << "error: task cannot set label parser except in initialize function!" << endl;
      throw exception();
    }
    this->priv->all->p->lp = lp;
    this->priv->label_is_test = is_test;
  }

  void search::get_test_action_sequence(vector<action>& V) {
    V.clear();
    for (size_t i=0; i<this->priv->test_action_sequence.size(); i++)
      V.push_back(this->priv->test_action_sequence[i]);
  }

  
  void search::set_num_learners(size_t num_learners) { this->priv->num_learners = num_learners; }
  void search::add_program_options(po::variables_map& vm, po::options_description& opts) { vm = add_options( *this->priv->all, opts ); }

  size_t search::get_mask() { return this->priv->all->reg.weight_mask;}
  size_t search::get_stride_shift() { return this->priv->all->reg.stride_shift;}
  uint32_t search::get_history_length() { return (uint32_t)this->priv->history_length; }
  
  // predictor implementation
  predictor::predictor(search& sch, ptag my_tag) : is_ldf(false), my_tag(my_tag), ec(NULL), ec_cnt(0), ec_alloced(false), oracle_is_pointer(false), allowed_is_pointer(false), learner_id(0), sch(sch) { 
    oracle_actions = v_init<action>(); 
    condition_on_tags = v_init<ptag>();
    condition_on_names = v_init<char>();
    allowed_actions = v_init<action>();
  }

  void predictor::free_ec() {
    if (ec_alloced) {
      if (is_ldf)
        for (size_t i=0; i<ec_cnt; i++)
          dealloc_example(CS::cs_label.delete_label, ec[i]);
      else
        dealloc_example(NULL, *ec);
      free(ec);
    }
  }
  
  predictor::~predictor() {
    if (! oracle_is_pointer) oracle_actions.delete_v();
    if (! allowed_is_pointer) allowed_actions.delete_v();
    free_ec();
    condition_on_tags.delete_v();
    condition_on_names.delete_v();
  }

  predictor& predictor::set_input(example&input_example) {
    free_ec();
    is_ldf = false;
    ec = &input_example;
    ec_cnt = 1;
    ec_alloced = false;
    return *this;
  }

  predictor& predictor::set_input(example*input_example, size_t input_length) {
    free_ec();
    is_ldf = true;
    ec = input_example;
    ec_cnt = input_length;
    ec_alloced = false;
    return *this;
  }

  void predictor::set_input_length(size_t input_length) {
    is_ldf = true;
    if (ec_alloced) ec = (example*)realloc(ec, input_length * sizeof(example));
    else            ec = (example*)calloc(input_length, sizeof(example));
    ec_cnt = input_length;
    ec_alloced = true;
  }
  void predictor::set_input_at(size_t posn, example&ex) {
    if (!ec_alloced) { std::cerr << "call to set_input_at without previous call to set_input_length" << endl; throw exception(); }
    if (posn >= ec_cnt) { std::cerr << "call to set_input_at with too large a position" << endl; throw exception(); }
    VW::copy_example_data(false, ec+posn, &ex, CS::cs_label.label_size, CS::cs_label.copy_label); // TODO: the false is "audit"
  }
  
  void predictor::make_new_pointer(v_array<action>& A, size_t new_size) {
    size_t old_size      = A.size();
    action* old_pointer  = A.begin;
    A.begin     = (action*)calloc_or_die(new_size, sizeof(action));
    A.end       = A.begin + new_size;
    A.end_array = A.end;
    memcpy(A.begin, old_pointer, old_size * sizeof(action));
  }

  predictor& predictor::add_to(v_array<action>& A, bool& A_is_ptr, action a, bool clear_first) {
    if (A_is_ptr) { // we need to make our own memory
      if (clear_first)
        A.end = A.begin;
      size_t new_size = clear_first ? 1 : (A.size() + 1);
      make_new_pointer(A, new_size);
      A_is_ptr = false;
      A[new_size-1] = a;
    } else { // we've already allocated our own memory
      if (clear_first) A.erase();
      A.push_back(a);
    }
    return *this;
  }    

  predictor& predictor::add_to(v_array<action>&A, bool& A_is_ptr, action*a, size_t action_count, bool clear_first) {
    size_t old_size = A.size();
    if (old_size > 0) {
      if (A_is_ptr) { // we need to make our own memory
        if (clear_first) {
          A.end = A.begin;
          old_size = 0;
        }
        size_t new_size = old_size + action_count;
        make_new_pointer(A, new_size);
        A_is_ptr = false;
        memcpy(A.begin + old_size, a, action_count * sizeof(action));
      } else { // we already have our own memory
        if (clear_first) A.erase();
        push_many<action>(A, a, action_count);
      }
    } else { // old_size == 0, clear_first is irrelevant
      if (! A_is_ptr)
        A.delete_v(); // avoid memory leak

      A.begin = a;
      A.end   = a + action_count;
      A.end_array = A.end;
      A_is_ptr = true;
    }
    return *this;
  }

  predictor& predictor::erase_oracles() { if (oracle_is_pointer) oracle_actions.end = oracle_actions.begin; else oracle_actions.erase(); return *this; }
  predictor& predictor::add_oracle(action a) { return add_to(oracle_actions, oracle_is_pointer, a, false); }
  predictor& predictor::add_oracle(action*a, size_t action_count) { return add_to(oracle_actions, oracle_is_pointer, a, action_count, false); }
  predictor& predictor::add_oracle(v_array<action>& a) { return add_to(oracle_actions, oracle_is_pointer, a.begin, a.size(), false); }
  
  predictor& predictor::set_oracle(action a) { return add_to(oracle_actions, oracle_is_pointer, a, true); }
  predictor& predictor::set_oracle(action*a, size_t action_count) { return add_to(oracle_actions, oracle_is_pointer, a, action_count, true); }
  predictor& predictor::set_oracle(v_array<action>& a) { return add_to(oracle_actions, oracle_is_pointer, a.begin, a.size(), true); }

  predictor& predictor::erase_alloweds() { if (allowed_is_pointer) allowed_actions.end = allowed_actions.begin; else allowed_actions.erase(); return *this; }
  predictor& predictor::add_allowed(action a) { return add_to(allowed_actions, allowed_is_pointer, a, false); }
  predictor& predictor::add_allowed(action*a, size_t action_count) { return add_to(allowed_actions, allowed_is_pointer, a, action_count, false); }
  predictor& predictor::add_allowed(v_array<action>& a) { return add_to(allowed_actions, allowed_is_pointer, a.begin, a.size(), false); }
  
  predictor& predictor::set_allowed(action a) { return add_to(allowed_actions, allowed_is_pointer, a, true); }
  predictor& predictor::set_allowed(action*a, size_t action_count) { return add_to(allowed_actions, allowed_is_pointer, a, action_count, true); }
  predictor& predictor::set_allowed(v_array<action>& a) { return add_to(allowed_actions, allowed_is_pointer, a.begin, a.size(), true); }
  
  predictor& predictor::add_condition(ptag tag, char name) { condition_on_tags.push_back(tag); condition_on_names.push_back(name); return *this; }
  predictor& predictor::set_condition(ptag tag, char name) { condition_on_tags.erase(); condition_on_names.erase(); return add_condition(tag, name); }

  predictor& predictor::add_condition_range(ptag hi, ptag count, char name0) {
    if (count == 0) return *this;
    for (ptag i=0; i<count; i++) {
      if (i > hi) break;
      char name = name0 + i;
      condition_on_tags.push_back(hi-i);
      condition_on_names.push_back(name);
    }
    return *this;
  }
  predictor& predictor::set_condition_range(ptag hi, ptag count, char name0) { condition_on_tags.erase(); condition_on_names.erase(); return add_condition_range(hi, count, name0); }

  predictor& predictor::set_learner_id(size_t id) { learner_id = id; return *this; }

  predictor& predictor::set_tag(ptag tag) { my_tag = tag; return *this; }

  action predictor::predict() {
    const action* orA = oracle_actions.size() == 0 ? NULL : oracle_actions.begin;
    const ptag*   cOn = condition_on_names.size() == 0 ? NULL : condition_on_tags.begin;
    const char*   cNa = NULL;
    if (condition_on_names.size() > 0) {
      condition_on_names.push_back((char)0);  // null terminate
      cNa = condition_on_names.begin;
    }
    const action* alA = (allowed_actions.size() == 0) ? NULL : allowed_actions.begin;
    action p = is_ldf ? sch.predictLDF(ec, ec_cnt, my_tag, orA, oracle_actions.size(), cOn, cNa, learner_id)
                      : sch.predict(*ec, my_tag, orA, oracle_actions.size(), cOn, cNa, alA, allowed_actions.size(), learner_id);

    if (condition_on_names.size() > 0)
      condition_on_names.pop();  // un-null-terminate
    return p;
  }
}

// ./vw --search 5 -k -c --search_task sequence -d test_seq --passes 10 -f test_seq.model --holdout_off
// ./vw -i test_seq.model -t -d test_seq --search_beam 2 -p /dev/stdout -r /dev/stdout

// ./vw --search 5 --csoaa_ldf m -k -c --search_task sequence_demoldf -d test_seq --passes 10 -f test_seq.model --holdout_off --search_history_length 0
// ./vw -i test_seq.model -t -d test_seq -p /dev/stdout -r /dev/stdout


// TODO: raw predictions in LDF mode
