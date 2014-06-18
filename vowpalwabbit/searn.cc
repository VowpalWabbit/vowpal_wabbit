/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif
#include "searn.h"
#include "gd.h"
#include "parser.h"
#include "constant.h"
#include "multiclass.h"
#include "csoaa.h"
#include "cb.h"
#include "memory.h"
#include "v_hashmap.h"
#include "vw.h"
#include "rand48.h"
#include "beam.h"

#ifdef _WIN32
bool isfinite(float x)
{ return !infpattern(x);}
#endif

// task-specific includes
#include "searn_sequencetask.h"
#include "searn_entityrelationtask.h"

using namespace LEARNER;

namespace Searn
{
  using namespace std;

  const bool PRINT_UPDATE_EVERY_EXAMPLE =0;
  const bool PRINT_UPDATE_EVERY_PASS =0;
  const bool PRINT_CLOCK_TIME =0;

  searn_task* all_tasks[] = { &SequenceTask::task,
                              &ArgmaxTask::task,
                              &SequenceTask_DemoLDF::task,
                              &SequenceSpanTask::task,
			      &EntityRelationTask::task,
			      NULL };   // must NULL terminate!

  string   neighbor_feature_space("neighbor");

  uint32_t AUTO_HISTORY = 1, AUTO_HAMMING_LOSS = 2, EXAMPLES_DONT_CHANGE = 4, IS_LDF = 8;
  enum SearnState { NONE, INIT_TEST, INIT_TRAIN, LEARN, GET_TRUTH_STRING, BEAM_INIT, BEAM_ADVANCE, BEAM_PLAYOUT, FAST_FORWARD };

  int  random_policy(uint64_t, float, bool, int, bool, bool);

  struct snapshot_item {
    size_t index;
    size_t tag;
    void  *data_ptr;
    size_t data_size;  // sizeof(data_ptr)
    size_t pred_step;  // srn->t when snapshot is made
  };

  struct snapshot_item_ptr {
    size_t   start;  // inclusive
    size_t   end;    // inclusive
    uint32_t hash_value;
  };

  struct snapshot_item_result {
    uint32_t action; // the action taken at this position
    float    loss;   // the loss _before_ taking that action
    bool     on_training_path;
  };

  typedef v_hashmap<snapshot_item_ptr, snapshot_item_result> snapmap;
  // we map from a snapshot_item_ptr to the action taken at that position
  // here, priv->snapshot_data[sip.start .. sip.end] is the full snapshot

  struct beam_hyp {
    size_t t;           // the value of srn.t here
    size_t action_taken;// which action was this (i.e., how did we get here from parent?)
    float  incr_cost;   // how much did this recent action cost
    v_array<snapshot_item> snapshot;  // some information so we can restore the snapshot
    beam_hyp* parent;   // our parent hypothesis
    size_t num_actions; // how many actions are available now
    float*action_costs; // cost of each action
    bool   filled_in_prediction;   // has this been filled in properly by predict?
    bool   filled_in_snapshot;     // has this been filled in properly by snapshot?
    bool   pruned;      // we were pruned in search?
  };


  struct searn_private {
    vw* all;

    bool auto_history;          // do you want us to automatically add history features?
    bool auto_hamming_loss;     // if you're just optimizing hamming loss, we can do it for you!
    bool examples_dont_change;  // set to true if you don't do any internal example munging
    bool is_ldf;                // set to true if you'll generate LDF data

    size_t A;             // total number of actions, [1..A]; 0 means ldf
    SearnState state;           // current state of learning
    size_t learn_t;       // when LEARN, this is the t at which we're varying a
    uint32_t learn_a;     //   and this is the a we're varying it to
    size_t snapshot_is_equivalent_to_t;   // if we've finished snapshotting and are equiv up to this time step, then we can fast forward from there
    bool snapshot_could_match;
    size_t snapshot_last_found_pos;
    v_array<snapshot_item> snapshot_data;
    snapmap* snapshot_map;
    v_array<uint32_t> train_action;  // which actions did we actually take in the train (or test) pass?
    v_array<uint32_t> train_action_ids;  // these are the ids -- the same in non-ldf mode, but the index in ldf mode (while train_action is id.weight_index)
    v_array< void* > train_labels;  // which labels are valid at any given time
    v_array<uint32_t> rollout_action; // for auto_history, we need a space other than train_action for rollouts
    history_info hinfo;   // default history info for auto-history
    string *neighbor_features_string;
    v_array<int32_t> neighbor_features; // ugly encoding of neighbor feature requirements

    beam_hyp * cur_beam_hyp;
    v_array<snapshot_item> beam_restore_to_end;
    v_array<uint32_t> beam_final_action_sequence;
    bool beam_is_training;

    size_t   most_recent_snapshot_begin;
    size_t   most_recent_snapshot_end;
    uint32_t most_recent_snapshot_hash;
    float    most_recent_snapshot_loss;  // the loss up to the current point in time
    bool     snapshotted_since_predict;

    size_t   final_snapshot_begin; // this is a pointer to the last snapshot taken at INIT_TRAIN time
    size_t   final_snapshot_end;
    size_t   fast_forward_position; // where should we fast forward to?
    
    bool should_produce_string;
    stringstream *pred_string;
    stringstream *truth_string;
    stringstream *bad_string_stream;
    bool printed_output_header;

    size_t t;              // the current time step
    size_t T;              // the length of the (training) trajectory
    size_t loss_last_step; // at what time step did they last declare their loss?
    float  test_loss;      // total test loss for this example
    float  train_loss;     // total training loss for this example
    float  learn_loss;     // total loss for this "varied" example
    
    bool   loss_declared;  // have we declared a loss at all?

    v_array<float> learn_losses;  // losses for all (valid) actions at learn_t
    example learn_example_copy[MAX_BRANCHING_FACTOR];   // copy of example(s) at learn_t
    example*learn_example_ref;    // reference to example at learn_t, when there's not example munging
    size_t learn_example_len;     // number of example(s) at learn_t

    float  beta;                  // interpolation rate
    float  alpha; //parameter used to adapt beta for dagger (see above comment), should be in (0,1)

    short rollout_method; // 0=policy, 1=oracle, 2=none
    bool  trajectory_oracle; // if true, only construct trajectories using the oracle

    bool   allow_current_policy;  // should the current policy be used for training? true for dagger
    //bool   rollout_oracle; //if true then rollout are performed using oracle instead (optimal approximation discussed in searn's paper). this should be set to true for dagger
    bool   adaptive_beta; //used to implement dagger through searn. if true, beta = 1-(1-alpha)^n after n updates, and policy is mixed with oracle as \pi' = (1-beta)\pi^* + beta \pi
    bool   rollout_all_actions;   // by default we rollout all actions. This is set to false when searn is used with a contextual bandit base learner, where we rollout only one sampled action
    uint32_t current_policy;      // what policy are we training right now?
    //float exploration_temperature; // if <0, always choose policy action; if T>=0, choose according to e^{-prediction / T} -- done to avoid overfitting
    size_t beam_size;
    size_t kbest;
    bool   allow_unsafe_fast_forward;

    size_t num_features;
    uint32_t total_number_of_policies;
    bool do_snapshot;
    bool do_fastforward;
    float subsample_timesteps;

    size_t read_example_last_id;
    size_t passes_since_new_policy;
    size_t read_example_last_pass;
    size_t total_examples_generated;
    size_t total_predictions_made;

    bool hit_new_pass;

    size_t passes_per_policy;

    vector<example*> ec_seq;

    LEARNER::learner* base_learner;
    void* valid_labels;
    clock_t start_clock_time;

    example*empty_example;
  };

  string   audit_feature_space("history");
  uint32_t history_constant    = 8290743;
  uint32_t example_number = 0;

  uint32_t hash_example(example&ec, uint32_t seed) {
    uint32_t hash = seed;

    for (unsigned char* i=ec.indices.begin; i != ec.indices.end; i++)
      hash = uniform_hash((unsigned char*) ec.atomics[*i].begin,
                          sizeof(feature) * (ec.atomics[*i].end - ec.atomics[*i].begin),
                          hash );

    hash = uniform_hash( (unsigned char*) &ec.ft_offset,
                         sizeof(uint32_t),
                         hash );

    return hash;
  }


  void default_info(history_info* hinfo)
  {
    hinfo->bigrams           = false;
    hinfo->features          = 0;
    hinfo->bigram_features   = false;
    hinfo->length            = 1;
  }

  int random_policy(uint64_t seed, float beta, bool allow_current_policy, int current_policy, bool allow_optimal, bool reset_seed)
  {
    if(reset_seed) //reset_seed is false for contextual bandit, so that we only reset the seed if the base learner is not a contextual bandit learner, as this breaks the exploration.
      msrand48(seed * 2147483647);

    if (beta >= 1) {
      if (allow_current_policy) return (int)current_policy;
      if (current_policy > 0) return (((int)current_policy)-1);
      if (allow_optimal) return -1;
      std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
      return (int)current_policy;
    }

    int num_valid_policies = (int)current_policy + allow_optimal + allow_current_policy;
    int pid = -1;

    if (num_valid_policies == 0) {
      std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
      return (int)current_policy;
    } else if (num_valid_policies == 1) {
      pid = 0;
    } else {
      float r = frand48();
      pid = 0;

      if (r > beta) {
        r -= beta;
        while ((r > 0) && (pid < num_valid_policies-1)) {
          pid ++;
          r -= beta * powf(1.f - beta, (float)pid);
        }
      }
    }
    // figure out which policy pid refers to
    if (allow_optimal && (pid == num_valid_policies-1))
      return -1; // this is the optimal policy

    pid = (int)current_policy - pid;
    if (!allow_current_policy)
      pid--;

    return pid;
  }

  const float history_value = 1.;

  void add_history_to_example(vw&all, history_info &hinfo, example* ec, history h, size_t additional_offset=0)
  {
    uint64_t v0, v1, v, max_string_length = 0;
    uint32_t wpp = all.wpp << all.reg.stride_shift;
    if (hinfo.length == 0) return;
    if (h == NULL) {
      cerr << "error: got empty history in add_history_to_example" << endl;
      throw exception();
    }

    if (all.audit) {
      max_string_length = max((int)(ceil( log10((float)hinfo.length+1) )),
                              (int)(ceil( log10((float)10000000000+1) ))) + 1;   // max action id
    }

    for (uint32_t t=1; t<=hinfo.length; t++) {
      v0 = ((h[hinfo.length-t]+1) * quadratic_constant * (additional_offset+1) + t) * history_constant;
      cdbg << "v0 = " << v0 << " additional_offset = " << additional_offset << " h[] = " << h[hinfo.length-t] << endl;
      // add the basic history features
      feature temp = {history_value, (uint32_t) ( (v0*wpp) & all.reg.weight_mask )};
      ec->atomics[history_namespace].push_back(temp);

      if (all.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)((v0*wpp) & all.reg.weight_mask), history_value, true };
        a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)calloc_or_die(5 + 2*max_string_length, sizeof(char));
        sprintf(a_feature.feature, "ug@%d=%d", (int)t, (int)h[hinfo.length-t]);

        ec->audit_features[history_namespace].push_back(a_feature);
      }

      // add the bigram features
      if ((t > 1) && hinfo.bigrams) {
        v1 = (v0 * cubic_constant + (h[hinfo.length-t+1]+1) * (additional_offset+1)) * history_constant;

        feature temp = {history_value, (uint32_t) ( (v1*wpp) & all.reg.weight_mask )};
        ec->atomics[history_namespace].push_back(temp);

        if (all.audit) {
          audit_data a_feature = { NULL, NULL, (uint32_t)((v1*wpp) & all.reg.weight_mask), history_value, true };
          a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
          strcpy(a_feature.space, audit_feature_space.c_str());

          a_feature.feature = (char*)calloc_or_die(6 + 3*max_string_length, sizeof(char));
          sprintf(a_feature.feature, "bg@%d=%d-%d", (int)t-1, (int)h[hinfo.length-t], (int)h[hinfo.length-t+1]);

          ec->audit_features[history_namespace].push_back(a_feature);
        }

      }
    }

    string fstring;

    if (hinfo.features > 0) {
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) {
        int feature_index = 0;
        for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++) {

          if (all.audit) {
            if (feature_index >= (int)ec->audit_features[*i].size() ) {
              char buf[32];
              sprintf(buf, "{%d}", f->weight_index);
              fstring = string(buf);
            } else
              fstring = string(ec->audit_features[*i][feature_index].feature);
            feature_index++;
          }

          v = f->weight_index + history_constant;

          for (uint32_t t=1; t<=hinfo.features; t++) {
            v0 = ((h[hinfo.length-t]+1) * quadratic_constant * (additional_offset+1) + t) * history_constant;

            // add the history/feature pair
            feature temp = {history_value, (uint32_t) ( ((v0 + v)*wpp) & all.reg.weight_mask )};
            ec->atomics[history_namespace].push_back(temp);

            if (all.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)(((v+v0)*wpp) & all.reg.weight_mask), history_value, true };
              a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)calloc_or_die(8 + 2*max_string_length + fstring.length(), sizeof(char));
              sprintf(a_feature.feature, "ug+f@%d=%d=%s", (int)t, (int)h[hinfo.length-t], fstring.c_str());

              ec->audit_features[history_namespace].push_back(a_feature);
            }


            // add the bigram
            if ((t > 0) && hinfo.bigram_features) {
              v1 = (v0 * cubic_constant + (h[hinfo.length-t+1]+1) * (additional_offset+1)) * history_constant;

              feature temp = {history_value, (uint32_t) ( ((v + v1)*wpp) & all.reg.weight_mask )};
              ec->atomics[history_namespace].push_back(temp);

              if (all.audit) {
                audit_data a_feature = { NULL, NULL, (uint32_t)(((v+v1)*wpp) & all.reg.weight_mask), history_value, true };
                a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
                strcpy(a_feature.space, audit_feature_space.c_str());

                a_feature.feature = (char*)calloc_or_die(9 + 3*max_string_length + fstring.length(), sizeof(char));
                sprintf(a_feature.feature, "bg+f@%d=%d-%d=%s", (int)t-1, (int)h[hinfo.length-t], (int)h[hinfo.length-t+1], fstring.c_str());

                ec->audit_features[history_namespace].push_back(a_feature);
              }

            }
          }
        }
      }
    }

    ec->indices.push_back(history_namespace);
    ec->sum_feat_sq[history_namespace] += ec->atomics[history_namespace].size() * history_value;
    ec->total_sum_feat_sq += ec->sum_feat_sq[history_namespace];
    ec->num_features += ec->atomics[history_namespace].size();
  }

  void remove_history_from_example(vw&all, history_info &hinfo, example* ec)
  {
    if (hinfo.length == 0) return;

    if (ec->indices.size() == 0) {
      cerr << "internal error (bug): trying to remove history, but there are no namespaces!" << endl;
      return;
    }

    if (ec->indices.last() != history_namespace) {
      cerr << "internal error (bug): trying to remove history, but either it wasn't added, or something was added after and not removed!" << endl;
      return;
    }

    ec->num_features -= ec->atomics[history_namespace].size();
    ec->total_sum_feat_sq -= ec->sum_feat_sq[history_namespace] * history_value;
    ec->sum_feat_sq[history_namespace] = 0;
    ec->atomics[history_namespace].erase();
    if (all.audit) {
      if (ec->audit_features[history_namespace].begin != ec->audit_features[history_namespace].end) {
        for (audit_data *f = ec->audit_features[history_namespace].begin; f != ec->audit_features[history_namespace].end; f++) {
          if (f->alloced) {
            free(f->space);
            free(f->feature);
            f->alloced = false;
          }
        }
      }

      ec->audit_features[history_namespace].erase();
    }
    ec->indices.decr();
  }

  int choose_policy(searn& srn, bool allow_current, bool allow_optimal)
  {
    uint32_t seed = (uint32_t) srn.priv->read_example_last_id * 2147483 + (uint32_t)(srn.priv->t * 2147483647);
    return random_policy(seed, srn.priv->beta, allow_current, srn.priv->current_policy, allow_optimal, false); // srn.priv->rollout_all_actions);
  }

  size_t get_all_labels(void*dst, searn& srn, size_t num_ec, v_array<uint32_t> *yallowed)
  {
    if (srn.priv->rollout_all_actions) { // dst should be a COST_SENSITIVE::label*

      COST_SENSITIVE::label *ret = (COST_SENSITIVE::label*)dst;

      if (srn.priv->is_ldf) {
        if (ret->costs.size() > num_ec)
          ret->costs.resize(num_ec);
        else if (ret->costs.size() < num_ec)
          for (uint32_t i= (uint32_t) ret->costs.size(); i<num_ec; i++) {
            COST_SENSITIVE::wclass cost = { FLT_MAX, i, 0., 0. };
            ret->costs.push_back(cost);
          }
      } else { // is not LDF
        if (yallowed == NULL) { // any action is allowed
          if (ret->costs.size() != srn.priv->A) {
            ret->costs.erase();
            for (uint32_t i=1; i<=srn.priv->A; i++) {
              COST_SENSITIVE::wclass cost = { FLT_MAX, i, 0., 0. };
              ret->costs.push_back(cost);
            }
          }
        } else { // yallowed is not null
          ret->costs.erase();
          for (size_t i=0; i<yallowed->size(); i++) {
            COST_SENSITIVE::wclass cost = { FLT_MAX, (*yallowed)[i], 0., 0. };
            ret->costs.push_back(cost);
          }
        }
      }
      return ret->costs.size();
    } else { // dst should be a CB::label*
      // TODO: speed this up as above
      CB::label *ret = (CB::label*)dst;
      ret->costs.erase();
      if (srn.priv->is_ldf) {
        for (uint32_t i=0; i<num_ec; i++) {
          CB::cb_class cost = { FLT_MAX, i, 0. };
          ret->costs.push_back(cost);
        }
      } else { // is not LDF
        if (yallowed == NULL) {
          for (uint32_t i=1; i<=srn.priv->A; i++) {
            CB::cb_class cost = { FLT_MAX, i, 0. };
            ret->costs.push_back(cost);
          }
        } else {
          for (size_t i=0; i<yallowed->size(); i++) {
            CB::cb_class cost = { FLT_MAX, (*yallowed)[i], 0. };
            ret->costs.push_back(cost);
          }
        }
      }
      return ret->costs.size();
    }
  }

  uint32_t get_any_label(searn& srn, v_array<uint32_t> *yallowed) {
    if (srn.priv->is_ldf)
      return 0;
    else if (yallowed == NULL)
      return 1;
    else
      return (*yallowed)[0];
  }

  uint32_t sample_with_temperature_partial_prediction(example* ecs, size_t num_ec, float temp) {
    float total = 0.;
    for (size_t a=0; a<num_ec; a++)
      total += (float)exp(-1.0 * ecs[a].partial_prediction / temp);
    if ((total >= 0.) && isfinite(total)) {
      float r = frand48() * total;
      for (size_t a=0; a<num_ec; a++) {
        r -= (float)exp(-1.0 * ecs[a].partial_prediction / temp);
        if (r <= 0.)
          return (uint32_t)a;
      }
    }
    // something failed
    return 0;
  }

  uint32_t single_prediction_LDF(vw& all, learner& base, example* ecs, size_t num_ec, COST_SENSITIVE::label* valid_labels, size_t pol, bool allow_exploration)
  {
    assert(pol >= 0);
    searn *srn = (searn*)all.searnstr;
    COST_SENSITIVE::label test_label;
    COST_SENSITIVE::cs_label.default_label(&test_label);

    // TODO: modify this to handle contextual bandit base learner with ldf
    float best_prediction = 0;
    uint32_t best_action = 0;
    assert(num_ec == valid_labels->costs.size());
    for (uint32_t action=0; action<num_ec; action++) {
      cdbg << "predict: action=" << action << endl;
      void* old_label = ecs[action].ld;
      ecs[action].ld = &test_label;
      base.predict(ecs[action], pol);
      srn->priv->total_predictions_made++;
      srn->priv->num_features += ecs[action].num_features;
      srn->priv->empty_example->in_use = true;

      cdbg << "predict: empty_example" << endl;
      base.predict(*(srn->priv->empty_example));
      ecs[action].ld = old_label;
      cdbg << "predict: partial_prediction[" << action << "] = " << ecs[action].partial_prediction << endl;
      valid_labels->costs[action].partial_prediction = ecs[action].partial_prediction;

      if ((action == 0) ||
          (ecs[action].partial_prediction < best_prediction)) {
        cdbg << "best action = " << action << endl;
        best_prediction = ecs[action].partial_prediction;
        best_action     = action; // ((COST_SENSITIVE::label*)ecs[action].ld)->costs[0].weight_index;
      }
    }

    if ((srn->priv->state == INIT_TEST) && (all.raw_prediction > 0)) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (uint32_t action=0; action<num_ec; action++) {
        if (action != 0) outputStringStream << ' ';
        outputStringStream << action << ':' << ecs[action].partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), ecs[0].tag);
    }

    //if (allow_exploration && (srn->priv->exploration_temperature > 0.))
    //  best_action = sample_with_temperature_partial_prediction(ecs, num_ec, srn->priv->exploration_temperature);

    return best_action;
  }

  uint32_t sample_with_temperature_csoaa(void* l, float temp) {
    COST_SENSITIVE::label* ld = (COST_SENSITIVE::label*)l;
    float total = 0.;
    for (COST_SENSITIVE::wclass* c = ld->costs.begin; c != ld->costs.end; ++c)
      total += (float)exp(-1.0 * c->partial_prediction / temp);
    if ((total >= 0.) && isfinite(total)) {
      float r = frand48() * total;
      for (COST_SENSITIVE::wclass* c = ld->costs.begin; c != ld->costs.end; ++c) {
        r -= (float)exp(-1.0 * c->partial_prediction / temp);
        if (r <= 0.)
          return c->class_index;
      }
    }
    // something failed
    return ld->costs[0].class_index;
  }

  uint32_t sample_with_temperature_cb(void* l, float temp) {
    CB::label* ld = (CB::label*)l;
    float total = 0.;
    for (CB::cb_class* c = ld->costs.begin; c != ld->costs.end; ++c)
      total += (float)exp(-1.0 * c->partial_prediction / temp);
    if ((total >= 0.) && isfinite(total)) {
      float r = frand48() * total;
      for (CB::cb_class* c = ld->costs.begin; c != ld->costs.end; ++c) {
        r -= (float)exp(-1.0 * c->partial_prediction / temp);
        if (r <= 0.)
          return c->action;
      }
    }
    // something failed
    return ld->costs[0].action;
  }

  template <class T>
  uint32_t single_prediction_notLDF(vw& all, searn& srn, learner& base, example& ec, T* valid_labels, uint32_t pol, bool allow_exploration)
  {
    assert(pol >= 0);

    void* old_label = ec.ld;
    ec.ld = valid_labels;

    base.predict(ec, pol);
    srn.priv->total_predictions_made++;
    srn.priv->num_features += ec.num_features;
    T* ld = (T*)ec.ld;
    uint32_t final_prediction = ld->prediction;

    // if (allow_exploration && (srn.priv->exploration_temperature > 0.)) {
    //   if (srn.priv->rollout_all_actions)
    //     final_prediction = sample_with_temperature_csoaa(ld, srn.priv->exploration_temperature);
    //   else
    //     final_prediction = sample_with_temperature_cb(ld, srn.priv->exploration_temperature);
    // }

    if ((srn.priv->state == INIT_TEST) && (all.raw_prediction > 0) && (srn.priv->rollout_all_actions)) { // srn.priv->rollout_all_actions ==> this is not CB, so we have COST_SENSITIVE::labels
      string outputString;
      stringstream outputStringStream(outputString);
      COST_SENSITIVE::label *ld = (COST_SENSITIVE::label*)ec.ld;
      for (COST_SENSITIVE::wclass* c = ld->costs.begin; c != ld->costs.end; ++c) {
        if (c != ld->costs.begin) outputStringStream << ' ';
        outputStringStream << c->class_index << ':' << c->partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }

    ec.ld = old_label;

    return final_prediction;
  }

  template<class T> T choose_random(v_array<T> opts) {
    float r = frand48();
    assert(opts.size() > 0);
    return opts[(size_t)(r * (float)opts.size())];
  }

  // CLAIM (unproven): will return something along the training path _if available_
  bool get_most_recent_snapshot_action(searn_private* priv, uint32_t &action, float &loss, bool &was_on_training_path) {
    if (! priv->snapshotted_since_predict) return false;
    snapshot_item_ptr sip = { priv->most_recent_snapshot_begin,
                              priv->most_recent_snapshot_end,
                              priv->most_recent_snapshot_hash };
    snapshot_item_result res = priv->snapshot_map->get(sip, sip.hash_value);
    if (res.loss < 0.f) return false;
    cdbg << "found";
    action = res.action;
    loss   = res.loss;
    was_on_training_path = res.on_training_path;
    return true;
  }

  void set_most_recent_snapshot_action(searn_private* priv, uint32_t action, float loss) {
    if (priv->most_recent_snapshot_end == (size_t)-1) return;
    bool on_training_path = priv->state == INIT_TRAIN || priv->state == BEAM_INIT;
    cdbg << "set: " << priv->most_recent_snapshot_begin << "\t" << priv->most_recent_snapshot_end << "\t" << priv->most_recent_snapshot_hash << "\totp=" << on_training_path << endl;
    snapshot_item_ptr sip = { priv->most_recent_snapshot_begin,
                              priv->most_recent_snapshot_end,
                              priv->most_recent_snapshot_hash };
    snapshot_item_result &res = priv->snapshot_map->get(sip, sip.hash_value);
    if (res.loss < 0.f) { // not found!
      snapshot_item_result me = { action, loss, on_training_path };
      priv->snapshot_map->put_after_get(sip, sip.hash_value, me);
    } else {
      assert(false);
    }
  }

  template<class T> bool v_array_contains(v_array<T> &A, T x) {
    for (T* e = A.begin; e != A.end; ++e)
      if (*e == x)
        return true;
    return false;
  }



 
  template <class T>
  uint32_t single_action(vw& all, searn& srn, learner& base, example* ecs, size_t num_ec, T* valid_labels, int pol, v_array<uint32_t> *ystar, bool ystar_is_uint32t, bool allow_exploration, bool set_valid_labels_on_oracle=false) {
    uint32_t action;
    // cdbg << "pol=" << pol << endl; //  << " ystar.size()=" << ystar->size() << " ystar[0]=" << ((ystar->size() > 0) ? (*ystar)[0] : 0) << endl;

    float snapshot_loss = -1.;
    bool was_on_training_path = false;
    if (get_most_recent_snapshot_action(srn.priv, action, snapshot_loss, was_on_training_path)) {
      cdbg << "get: " << (srn.priv->state == LEARN) << " " << was_on_training_path << endl;
      if (srn.priv->allow_unsafe_fast_forward && (srn.priv->state == LEARN) && was_on_training_path && (srn.priv->t > srn.priv->learn_t)) { // TODO: if we're allowed to fastforward
        srn.priv->state = FAST_FORWARD;
        srn.priv->learn_loss = srn.priv->learn_loss + (srn.priv->train_loss - snapshot_loss);
        srn.priv->fast_forward_position = srn.priv->final_snapshot_begin;
        assert(srn.priv->final_snapshot_end >= srn.priv->final_snapshot_begin);
        assert(srn.priv->final_snapshot_end <  srn.priv->snapshot_data.size());
        cdbg << "fast_forward, t=" << srn.priv->t << " and learn_t=" << srn.priv->learn_t << endl;
      }
    } else { // no snapshot found
      if (pol == -1) { // optimal policy
        if (ystar_is_uint32t)
          action = *((uint32_t*)ystar);
        else if ((ystar == NULL) || (ystar->size() == 0)) { // TODO: choose according to current model!
          if (srn.priv->rollout_all_actions)
            action = choose_random<COST_SENSITIVE::wclass>(((COST_SENSITIVE::label*)valid_labels)->costs).class_index;
          else
            action = choose_random<CB::cb_class >(((CB::label   *)valid_labels)->costs).action;
        } else 
          action = choose_random<uint32_t>(*ystar); // TODO: choose according to current model!
        
        if (set_valid_labels_on_oracle && (ystar_is_uint32t || ((ystar != NULL) && (ystar->size() > 0)))) {
          assert(srn.priv->rollout_all_actions);  // TODO: deal with CB
          v_array<COST_SENSITIVE::wclass>* costs = &((COST_SENSITIVE::label*)valid_labels)->costs;
          for (size_t i=0; i<costs->size(); i++) {
            if (ystar_is_uint32t)
              costs->get(i).partial_prediction = (costs->get(i).class_index == action) ? 0.f : 1.f;
            else   // ystar is actually v_array<uint32_t>
              costs->get(i).partial_prediction = v_array_contains<uint32_t>(*ystar, costs->get(i).class_index) ? 0.f : 1.f;
          }
        }
      } else {        // learned policy
        if (!srn.priv->is_ldf) {  // single example (not LDF)
          if (srn.priv->hinfo.length>0) {cdbg << "add_history_to_example: srn.priv->t=" << srn.priv->t << " h=" << srn.priv->rollout_action.begin[srn.priv->t] << endl;}
          if (srn.priv->hinfo.length>0) {cdbg << "  rollout_action = ["; for (size_t i=0; i<srn.priv->t+1; i++) cdbg << " " << srn.priv->rollout_action.begin[i]; cdbg << " ], len=" << srn.priv->rollout_action.size() << endl;}
          if (srn.priv->auto_history) add_history_to_example(all, srn.priv->hinfo, ecs, srn.priv->rollout_action.begin+srn.priv->t);
          action = single_prediction_notLDF<T>(all, srn, base, *ecs, valid_labels, pol, allow_exploration);
          if (srn.priv->auto_history) remove_history_from_example(all, srn.priv->hinfo, ecs);
        } else { // LDF
          if (srn.priv->auto_history)
            for (size_t a=0; a<num_ec; a++) {
              cdbg << "class_index = " << ((COST_SENSITIVE::label*)ecs[a].ld)->costs[0].class_index << ":" << ((COST_SENSITIVE::label*)ecs[a].ld)->costs[0].x << endl;
              add_history_to_example(all, srn.priv->hinfo, &ecs[a], srn.priv->rollout_action.begin+srn.priv->t, a * history_constant);
            }
          //((OAA::mc_label*)ecs[a].ld)->label);
          action = single_prediction_LDF(all, base, ecs, num_ec, (COST_SENSITIVE::label*)valid_labels, pol, allow_exploration);
          if (srn.priv->auto_history)
            for (size_t a=0; a<num_ec; a++)
              remove_history_from_example(all, srn.priv->hinfo, &ecs[a]);
        }
      }
      set_most_recent_snapshot_action(srn.priv, action, srn.priv->most_recent_snapshot_loss);
    }
    return action;
  }

  void clear_snapshot(vw& all, searn& srn, bool free_data)
  {
    /*UNDOME*/cdbg << "clear_snapshot free_data=" << free_data << endl;
    if (free_data)
      for (size_t i=0; i<srn.priv->snapshot_data.size(); i++)
        free(srn.priv->snapshot_data[i].data_ptr);
    srn.priv->snapshot_data.erase();
    srn.priv->snapshot_map->clear();
  }

  void* copy_labels(searn &srn, void* l) {
    if (srn.priv->rollout_all_actions) {
      COST_SENSITIVE::label *ret = new COST_SENSITIVE::label();
      v_array<COST_SENSITIVE::wclass> costs = ((COST_SENSITIVE::label*)l)->costs;
      for (size_t i=0; i<costs.size(); i++) {
        COST_SENSITIVE::wclass c = { costs[i].x, costs[i].class_index, costs[i].partial_prediction, costs[i].wap_value };
        assert(costs[i].class_index <= srn.priv->A);
        ret->costs.push_back(c);
      }
      return ret;
    } else {
      CB::label *ret = new CB::label();
      v_array<CB::cb_class> costs = ((CB::label*)l)->costs;
      for (size_t i=0; i<costs.size(); i++) {
        CB::cb_class c = { costs[i].cost, costs[i].action, costs[i].probability };
        ret->costs.push_back(c);
      }
      return ret;
    }
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
        v.resize(2 * pos + 3);
        v.begin[pos] = item;
        v.end = v.begin + pos + 1;
      }
    }
  }

  // if not LDF:
  //   *ecs should be a pointer to THE example
  //   num_ec == 0
  //   yallowed:
  //     == NULL means ANY action is okay [1..M]
  //     != NULL means only the given actions are okay
  // if LDF:
  //   *ecs .. *(ecs+num_ec-1) should be valid actions
  //   num_ec > 0
  //   yallowed MUST be NULL (why would you give me an impossible action?)
  // in both cases:
  //   ec->ld is ignored
  //   ystar:
  //     == NULL (or empty) means we don't know the oracle label
  //     otherwise          means the oracle could do any of the listed actions
  template <class T>
  uint32_t searn_predict_without_loss(vw& all, learner& base, example* ecs, size_t num_ec, v_array<uint32_t> *yallowed, v_array<uint32_t> *ystar, bool ystar_is_uint32t)  // num_ec == 0 means normal example, >0 means ldf, yallowed==NULL means all allowed, ystar==NULL means don't know; ystar_is_uint32t means that the ystar ref is really just a uint32_t
  {
    searn* srn=(searn*)all.searnstr;

    // check ldf sanity
    if (!srn->priv->is_ldf) {
      assert(num_ec == 0); // searntask is trying to define an ldf example in a non-ldf problem
    } else { // is LDF
      assert(num_ec != 0); // searntask is trying to define a non-ldf example in an ldf problem" << endl;
      assert(yallowed == NULL); // searntask is trying to specify allowed actions in an ldf problem" << endl;
    }

    if (srn->priv->state == INIT_TEST) {
      int pol = choose_policy(*srn, true, false);
      //cerr << "(" << pol << ")";
      get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
      uint32_t a = single_action<T>(all, *srn, base, ecs, num_ec, (T*)srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, false);
      //uint32_t a_opt = single_action(all, *srn, ecs, num_ec, valid_labels, -1, ystar);
      cdbg << "predict @" << srn->priv->t << " pol=" << pol << " a=" << a << endl;
      uint32_t a_name = (! srn->priv->is_ldf) ? a : ((COST_SENSITIVE::label*)ecs[a].ld)->costs[0].class_index;
      if (srn->priv->auto_history) srn->priv->rollout_action.push_back(a_name);
      srn->priv->t++;
      return a;
    } else if ((srn->priv->state == GET_TRUTH_STRING) || (srn->priv->state == FAST_FORWARD)) {
      int pol = -1;
      get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
      uint32_t a = single_action<T>(all, *srn, base, ecs, num_ec, (T*)srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, false);
      srn->priv->t++;
      return a;
    } else if (srn->priv->state == INIT_TRAIN) {
      int pol = -1; // oracle
      if (!srn->priv->trajectory_oracle)
        pol = choose_policy(*srn, srn->priv->allow_current_policy, true);
      cdbg << "{" << pol << "," << srn->priv->trajectory_oracle << "}";
      get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
      uint32_t a = single_action<T>(all, *srn, base, ecs, num_ec, (T*)srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, true);
      //uint32_t a_opt = single_action(all, *srn, ecs, num_ec, valid_labels, -1, ystar);
      cdbg << "predict @" << srn->priv->t << " pol=" << pol << " a=" << a << endl;
      //assert((srn->priv->current_policy == 0) || (a == a_opt));
      //if (! ((srn->priv->current_policy == 0) || (a == a_opt))) { /*UNDOME*/cdbg << "FAIL!!!"<<endl;}
      srn->priv->train_action_ids.push_back(a);
      srn->priv->train_labels.push_back(copy_labels(*srn, srn->priv->valid_labels));
      uint32_t a_name = (! srn->priv->is_ldf) ? a : ((COST_SENSITIVE::label*)ecs[a].ld)->costs[0].class_index;
      srn->priv->train_action.push_back(a_name);
      if (srn->priv->auto_history) srn->priv->rollout_action.push_back(a_name);
      srn->priv->t++;
      return a;
    } else if (srn->priv->state == LEARN) {
      if (srn->priv->t < srn->priv->learn_t) {
        assert(srn->priv->t < srn->priv->train_action.size());
        srn->priv->t++;
        size_t a = srn->priv->train_action_ids[srn->priv->t - 1];
        return (uint32_t)a;
      } else if (srn->priv->t == srn->priv->learn_t) {
        if (srn->priv->learn_example_len == 0) {
          size_t num_to_copy = (num_ec == 0) ? 1 : num_ec;
          if (srn->priv->examples_dont_change) {
            srn->priv->learn_example_ref = ecs;
            srn->priv->learn_example_len = num_to_copy;
            cdbg << "examples_dont_change so setting ref to ecs, len=" << num_to_copy << " and t=" << srn->priv->t << endl;
          } else {
            size_t label_size = srn->priv->is_ldf ? sizeof(COST_SENSITIVE::label) : sizeof(MULTICLASS::mc_label);
            void (*label_copy_fn)(void*&,void*) = srn->priv->is_ldf ? COST_SENSITIVE::cs_label.copy_label : NULL;
            assert(num_to_copy < MAX_BRANCHING_FACTOR);
            cdbg << "copying " << num_to_copy << " items to learn_example_copy" << endl;
            for (size_t n=0; n<num_to_copy; n++)
              VW::copy_example_data(all.audit, srn->priv->learn_example_copy+n, &ecs[n], label_size, label_copy_fn);
            srn->priv->learn_example_len = num_to_copy;
          }
          cdbg << "copying example to " << srn->priv->learn_example_copy << endl;
        }
        // srn->priv->snapshot_is_equivalent_to_t = (size_t)-1;
        // srn->priv->snapshot_could_match = true;
        srn->priv->t++;
        uint32_t a_name = (! srn->priv->is_ldf) ? srn->priv->learn_a : ((COST_SENSITIVE::label*)ecs[srn->priv->learn_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) srn->priv->rollout_action.push_back(a_name);
        return srn->priv->learn_a;
      } else { // t > learn_t
        size_t this_a = 0;

        if (srn->priv->rollout_method == 1) { // rollout by oracle
          assert(ystar_is_uint32t);
          this_a = *(uint32_t*)ystar;
          //get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
          //this_a = single_action(all, *srn, base, ecs, num_ec, srn->priv->valid_labels, -1, ystar, ystar_is_uint32t, false);
          srn->priv->t++;
          //valid_labels.costs.erase(); valid_labels.costs.delete_v();
        } else if (srn->priv->rollout_method == 0) { // rollout by policy
          //if ((!srn->priv->do_fastforward) || (!srn->priv->snapshot_could_match) || (srn->priv->snapshot_is_equivalent_to_t == ((size_t)-1))) { // we haven't converged, continue predicting
            int pol = choose_policy(*srn, srn->priv->allow_current_policy, true);
            get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
            this_a = single_action(all, *srn, base, ecs, num_ec,(T*) srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, true);
            cdbg << "predict @" << srn->priv->t << " pol=" << pol << " a=" << this_a << endl;
            srn->priv->t++;
            //valid_labels.costs.erase(); valid_labels.costs.delete_v();

          //   srn->priv->snapshot_could_match = true;
          //   srn->priv->snapshot_is_equivalent_to_t = (size_t)-1;
          // } else {    // we can keep predicting using training trajectory
          //   srn->priv->snapshot_is_equivalent_to_t++;
          //   srn->priv->t = srn->priv->snapshot_is_equivalent_to_t;
          //   cdbg << "restoring previous prediction @ " << (srn->priv->t-1) << " = " << srn->priv->train_action_ids[srn->priv->t-1] << endl;
          //   this_a = srn->priv->train_action_ids[srn->priv->t - 1];
          // }
        } else { // rollout_method == 2 ==> rollout by NONE
          // TODO: implement me
          throw exception();
        }
        uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) srn->priv->rollout_action.push_back(a_name);
        return (uint32_t)this_a;
      }
    } else if (srn->priv->state == BEAM_INIT) {
      // our job is to fill in srn->priv->valid_labels and the corresponding action costs,
      // to collect a snapshot at the end, and otherwise to go as fast as possible!
      if (srn->priv->t == 0) { // collect action info
        bool allow_optimal = srn->priv->beam_is_training;
        bool allow_current = (! srn->priv->beam_is_training) || srn->priv->allow_current_policy;
        int pol = -1;
        if (!srn->priv->trajectory_oracle)
          pol = choose_policy(*srn, allow_current, allow_optimal);
        cdbg << "BEAM_INIT: pol = " << pol << ", beta = " << srn->priv->beta << endl;
        size_t num_actions = get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
        single_action<T>(all, *srn, base, ecs, num_ec, (T*)srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, false, true);
        // fill in relevant information
        srn->priv->cur_beam_hyp->num_actions = num_actions;
        srn->priv->cur_beam_hyp->filled_in_prediction = true;
        // TODO: check to see if valid_labels also containts costs!!!
      }
      srn->priv->t++;
      uint32_t this_a = get_any_label(*srn, yallowed);
      uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
      if (srn->priv->auto_history) push_at(srn->priv->rollout_action, a_name, srn->priv->t + srn->priv->hinfo.length - 1);
      cdbg << "A rollout_action.push_back(" << a_name << ", @ " << (srn->priv->t) << ")" << endl;
      if (srn->priv->hinfo.length>0) {cdbg << "  rollout_action = ["; for (size_t i=0; i<srn->priv->t+1; i++) cdbg << " " << srn->priv->rollout_action.begin[i]; cdbg << " ], len=" << srn->priv->rollout_action.size() << endl;}
      return this_a;
    } else if (srn->priv->state == BEAM_ADVANCE) {
      if (srn->priv->t + 1 == srn->priv->cur_beam_hyp->t) {
        srn->priv->t++;
        uint32_t this_a = (uint32_t)srn->priv->cur_beam_hyp->action_taken;
        get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
        if (!srn->priv->is_ldf) {
          this_a = ((COST_SENSITIVE::label*)srn->priv->valid_labels)->costs[this_a].class_index;
          cdbg << "valid_labels = ["; for (COST_SENSITIVE::wclass*wc=((COST_SENSITIVE::label*)srn->priv->valid_labels)->costs.begin; wc!= ((COST_SENSITIVE::label*)srn->priv->valid_labels)->costs.end; ++wc) cdbg << " " << wc->class_index; cdbg << " ]" << endl;
        }
        uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) push_at(srn->priv->rollout_action, a_name, srn->priv->t + srn->priv->hinfo.length - 1);
        cdbg << "B rollout_action.push_back(" << a_name << ", @ " << (srn->priv->t) << ")" << endl;
        if (srn->priv->hinfo.length>0) {cdbg << "  rollout_action = ["; for (size_t i=0; i<srn->priv->t+1; i++) cdbg << " " << srn->priv->rollout_action.begin[i]; cdbg << " ], len=" << srn->priv->rollout_action.size() << endl;}
        return this_a;
      } else if (srn->priv->t == srn->priv->cur_beam_hyp->t) {
        bool allow_current = (! srn->priv->beam_is_training) || srn->priv->allow_current_policy;
        bool allow_optimal = srn->priv->beam_is_training;
        int pol = -1;
        if (!srn->priv->trajectory_oracle)
          pol = choose_policy(*srn, allow_current, allow_optimal);
        cdbg << "BEAM_ADVANCE: pol = " << pol << ", beta = " << srn->priv->beta << endl;
        size_t num_actions = get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
        single_action<T>(all, *srn, base, ecs, num_ec, (T*)srn->priv->valid_labels, pol, ystar, ystar_is_uint32t, false, true);
        // fill in relevant information
        srn->priv->cur_beam_hyp->num_actions = num_actions;
        srn->priv->cur_beam_hyp->filled_in_prediction = true;
        srn->priv->t++;
        uint32_t this_a = get_any_label(*srn, yallowed);
        uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) push_at(srn->priv->rollout_action, a_name, srn->priv->t + srn->priv->hinfo.length - 1);
        cdbg << "C rollout_action.push_back(" << a_name << ", @ " << (srn->priv->t) << ")" << endl;
        if (srn->priv->hinfo.length>0) {cdbg << "  rollout_action = ["; for (size_t i=0; i<srn->priv->t+1; i++) cdbg << " " << srn->priv->rollout_action.begin[i]; cdbg << " ], len=" << srn->priv->rollout_action.size() << endl;}
        return this_a;
      } else {
        // TODO: check if auto history, etc., is necessary here
        srn->priv->t++;
        uint32_t this_a = get_any_label(*srn, yallowed);
        uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) push_at(srn->priv->rollout_action, a_name, srn->priv->t + srn->priv->hinfo.length - 1);
        cdbg << "D rollout_action.push_back(" << a_name << ", @ " << (srn->priv->t) << ")" << endl;
        //cdbg << "  rollout_action = ["; for (size_t i=0; i<srn->priv->t+1; i++) cdbg << " " << srn->priv->rollout_action.begin[i]; cdbg << " ], len=" << srn->priv->rollout_action.size() << endl;
        return this_a;
      }
    } else if (srn->priv->state == BEAM_PLAYOUT) {
      assert(srn->priv->beam_final_action_sequence.size() > 0);
      get_all_labels(srn->priv->valid_labels, *srn, num_ec, yallowed);
      srn->priv->t++;
      if (srn->priv->rollout_all_actions) {
        uint32_t this_a = srn->priv->beam_final_action_sequence.pop();
        if (!srn->priv->is_ldf)
          this_a = ((COST_SENSITIVE::label*)srn->priv->valid_labels)->costs[this_a].class_index;
        uint32_t a_name = (! srn->priv->is_ldf) ? (uint32_t)this_a : ((COST_SENSITIVE::label*)ecs[this_a].ld)->costs[0].class_index;
        if (srn->priv->auto_history) push_at(srn->priv->rollout_action, a_name, srn->priv->t - 1 + srn->priv->hinfo.length);
        return this_a;
      } else {
        throw exception();
      }
    } else {
      cerr << "fail: searn got into ill-defined state (" << (int)srn->priv->state << ")" << endl;
      throw exception();
    }
  }

  void searn_declare_loss(searn_private* priv, size_t predictions_since_last, float incr_loss)
  {
    vw* all = priv->all;
    searn* srn=(searn*)all->searnstr;

    /*
    if ((srn->priv->beam_size == 0) && (srn->priv->t != srn->priv->loss_last_step + predictions_since_last)) {
      cerr << "fail: searntask hasn't counted its predictions correctly.  current time step=" << srn->priv->t << ", last declaration at " << srn->priv->loss_last_step << ", declared # of predictions since then is " << predictions_since_last << endl;
      throw exception();
    }
    */

    srn->priv->loss_declared = true;

    srn->priv->loss_last_step = srn->priv->t;
    cdbg<<"new loss_last_step="<<srn->priv->t<<" incr_loss=" << incr_loss <<endl;
    if (srn->priv->state == INIT_TEST)
      srn->priv->test_loss += incr_loss;
    else if (srn->priv->state == INIT_TRAIN)
      srn->priv->train_loss += incr_loss;
    else if (srn->priv->state == LEARN)
      srn->priv->learn_loss += incr_loss;
    else if (srn->priv->state == BEAM_PLAYOUT)
      srn->priv->test_loss += incr_loss;
  }


  uint32_t searn_predict(searn_private* priv, example* ecs, size_t num_ec, v_array<uint32_t> *yallowed, v_array<uint32_t> *ystar, bool ystar_is_uint32t)  // num_ec == 0 means normal example, >0 means ldf, yallowed==NULL means all allowed, ystar==NULL means don't know; ystar_is_uint32t means that the ystar ref is really just a uint32_t
  {
    vw* all = priv->all;
    learner* base = priv->base_learner;
    searn* srn=(searn*)all->searnstr;
    uint32_t a;

    //bool found_ss = get_most_recent_snapshot_action(priv, a);

    //if (!found_ss) a = (uint32_t)-1;
    if (srn->priv->rollout_all_actions)
      a = searn_predict_without_loss<COST_SENSITIVE::label>(*all, *base, ecs, num_ec, yallowed, ystar, ystar_is_uint32t);
    else
      a = searn_predict_without_loss<CB::label            >(*all, *base, ecs, num_ec, yallowed, ystar, ystar_is_uint32t);

    //set_most_recent_snapshot_action(priv, a);
    priv->snapshotted_since_predict = false;

    if (priv->auto_hamming_loss) {
      float this_loss = 0.;
      if (ystar) {
        if (ystar_is_uint32t &&   // single allowed ystar
            (*((uint32_t*)ystar) != (uint32_t)-1) && // not a test example
            (*((uint32_t*)ystar) != a))  // wrong prediction
          this_loss = 1.;
        if ((!ystar_is_uint32t) && // many allowed ystar
            (!v_array_contains(*ystar, a)))
          this_loss = 1.;
      }
      searn_declare_loss(priv, 1, this_loss);
    }

    return a;
  }


  bool snapshot_binary_search_lt(v_array<snapshot_item> a, size_t desired_t, size_t tag, size_t &pos, size_t last_found_pos) {
    size_t hi  = a.size();
    if (hi == 0) return false;
    if (last_found_pos + 1 < hi) {
      pos = last_found_pos+1;
      if ((a[pos].pred_step == desired_t) && (a[pos].tag == tag)) return true;
      if (pos == hi - 1) { // this is the last item
        if ((a[pos].pred_step < desired_t) && (a[pos].tag == tag)) return true;
        else return false;
      }
      // not the last item
      if ((a[pos].pred_step <= desired_t) && (a[pos].tag == tag) &&
          ((a[pos+1].pred_step >  desired_t) ||
           ((a[pos+1].pred_step == desired_t) && (a[pos+1].tag > tag))))
        return true;
    }
    pos = hi >> 1;
    while (true) {
      if (hi <= 5) break;
      if ((a[pos].pred_step == desired_t) && (a[pos].tag == tag)) return true;
      if ((a[pos].pred_step >  desired_t) ||
          ((a[pos].pred_step == desired_t) && (a[pos].tag >  tag)))
        hi = pos;
      else break;

      pos = hi >> 1;
    }

    for (pos=hi-1; ; pos--) {
      if ((a[pos].pred_step <= desired_t) && (tag == a[pos].tag)) return true;
      if (pos == 0) return false;
    }
    return false;
  }


  bool snapshot_binary_search_eq(v_array<snapshot_item> a, size_t desired_index, size_t tag, size_t &pos, size_t last_found_pos) {
    size_t lo  = 0;  // the answer is always >= lo
    size_t hi  = a.size();  // the answer is always < hi
    if (hi == 0) return false;

    if (last_found_pos + 1 < hi) {
      pos = last_found_pos+1;
      if ((a[pos].index == desired_index) && (a[pos].tag == tag)) return true;
    } else
      pos = (lo + hi) >> 1;
    while (true) {
      if (lo == hi) return false;
      if ((a[pos].index == desired_index) && (a[pos].tag == tag)) return true;
      if (hi - lo <= 5) break;
      if ((a[pos].index <  desired_index) ||
          ((a[pos].index == desired_index) && (a[pos].tag <  tag)))
        lo = pos;
      else
        hi = pos;
      pos = (lo + hi) >> 1;
    }

    for (pos=lo; pos<hi; pos++) {
      if (a[pos].index > desired_index) return false;
      if ((a[pos].index == desired_index) && (a[pos].tag == tag)) return true;
    }
    return false;
  }

  void searn_snapshot_data(searn_private* priv, size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction) {
    if ((priv->state == NONE) || (priv->state == INIT_TEST) || (priv->state == GET_TRUTH_STRING) || (priv->state == BEAM_PLAYOUT))
      return;

    size_t i;
    if ((priv->state == LEARN) && (priv->t <= priv->learn_t) &&
        snapshot_binary_search_lt(priv->snapshot_data, priv->learn_t, tag, i, priv->snapshot_last_found_pos)) {
      // see if we can fast-forward as close as possible
      // TODO: if tag==1, then loss_last_step = pred_step ???
      // TODO: check to make sure last_found_pos is getting initialized properly
      priv->snapshot_last_found_pos = i;
      snapshot_item &item = priv->snapshot_data[i];
      assert(sizeof_data == item.data_size);

      memcpy(data_ptr, item.data_ptr, sizeof_data);
      priv->t = item.pred_step;
      return;
    }
    
    if ((priv->state == INIT_TRAIN) || (priv->state == LEARN)) {
      priv->snapshotted_since_predict = true;
      priv->most_recent_snapshot_end = priv->snapshot_data.size();
      cdbg << "end = " << priv->most_recent_snapshot_end << endl;

      void* new_data = NULL;
      if ((data_ptr != NULL) && (sizeof_data != 0)) {
        new_data = malloc(sizeof_data);
        memcpy(new_data, data_ptr, sizeof_data);
        if (used_for_prediction) {
          if (sizeof_data == 1 || sizeof_data == 2 || sizeof_data == 4 || sizeof_data == 8) {
            // fast hashing
            priv->most_recent_snapshot_hash *= 398401;
            if (sizeof_data == 1)      priv->most_recent_snapshot_hash += 4893107 * *(uint8_t*)data_ptr;
            else if (sizeof_data == 2) priv->most_recent_snapshot_hash += 4893107 * *(uint16_t*)data_ptr;
            else if (sizeof_data == 4) priv->most_recent_snapshot_hash += 4893107 * *(uint32_t*)data_ptr;
            else if (sizeof_data == 8) priv->most_recent_snapshot_hash +=  4893107 * (uint32_t)*(uint64_t*)data_ptr;
          } else {
            priv->most_recent_snapshot_hash = uniform_hash(data_ptr, sizeof_data, priv->most_recent_snapshot_hash);
          }
        }
      }
      snapshot_item item = { index, tag, new_data, sizeof_data, priv->t };
      priv->snapshot_data.push_back(item);
      //cerr << "priv->snapshot_data.push_back(item);" << endl;
      return;
    }

    if (priv->state == FAST_FORWARD) { // go to the end!
      snapshot_item &me = priv->snapshot_data[priv->fast_forward_position + tag];  // TODO: generalize or ensure that tags are +=1 each time, also this is broken if they don't auto-history
      assert(me.tag == tag);
      assert(me.data_size = sizeof_data);
      memcpy(data_ptr, me.data_ptr, sizeof_data);
      return;
    }
    
    if (priv->state == BEAM_INIT) {
      vw* all = priv->all;
      searn* srn=(searn*)all->searnstr;
      size_t cur_size = priv->snapshot_data.size();
      if ((cur_size > 0) && // only need to keep around the NEWEST set of snapshots
          (priv->snapshot_data[cur_size - 1].pred_step < priv->t))
        clear_snapshot(*all, *srn, true);

      void* new_data = malloc(sizeof_data);
      memcpy(new_data, data_ptr, sizeof_data);
      snapshot_item item = { index, tag, new_data, sizeof_data, priv->t };
      priv->snapshot_data.push_back(item);

      return;
    }

    if (priv->state == BEAM_ADVANCE) {
      /*UNDOME*/cdbg << "snapshot(BEAM_ADVANCE), srn.priv->t=" << priv->t << ", hyp.t=" << priv->cur_beam_hyp->t << " { cur_beam_hyp=" << priv->cur_beam_hyp << ", parent=" << priv->cur_beam_hyp->parent << " }" << endl;
      assert(priv->cur_beam_hyp->parent != NULL);
      if (priv->t < priv->cur_beam_hyp->t) {
        if (priv->cur_beam_hyp->parent->snapshot.size() == 0) {
          /*UNDOME*/cdbg << "skipping because parent snapshot is empty" << endl;
          assert(priv->cur_beam_hyp->parent->t == 0);
        } else {
          /*UNDOME*/cdbg << "skipping to desired position" << endl;
          assert(priv->cur_beam_hyp->parent->snapshot.size() > 0);
          size_t i, desired_index = priv->cur_beam_hyp->parent->snapshot[0].index;
          bool found = snapshot_binary_search_eq(priv->cur_beam_hyp->parent->snapshot, desired_index, tag, i, priv->snapshot_last_found_pos);
          if (! found) {
            cerr << "beam search failed (snapshot not found)" << endl;
            throw exception();
          }

          assert(sizeof_data == priv->cur_beam_hyp->parent->snapshot[i].data_size);
          memcpy(data_ptr, priv->cur_beam_hyp->parent->snapshot[i].data_ptr, sizeof_data);
          priv->t = priv->cur_beam_hyp->parent->snapshot[i].pred_step;
          cdbg << "  set data_ptr to " << *(uint32_t*)data_ptr << ", and priv->t to " << priv->t << " { cur_beam_hyp=" << priv->cur_beam_hyp << ", parent=" << priv->cur_beam_hyp->parent << " }" << endl;
        }
      } else if (priv->t == priv->cur_beam_hyp->t) {
        /*UNDOME*/cdbg << "recording index=" << index << " tag=" << tag << " data_ptr=" << *(uint32_t*)data_ptr << " { cur_beam_hyp=" << priv->cur_beam_hyp << ", parent=" << priv->cur_beam_hyp->parent << " }" << endl;
        void* new_data = malloc(sizeof_data);
        memcpy(new_data, data_ptr, sizeof_data);
        snapshot_item item = { index, tag, new_data, sizeof_data, priv->t };
        priv->cur_beam_hyp->snapshot.push_back(item);
        priv->cur_beam_hyp->filled_in_snapshot = true;
      } else {
        /*UNDOME*/cdbg << "fast forward to end" << endl;
        // fast foward to end
        size_t i;
        assert(priv->beam_restore_to_end.size() > 0);
        size_t end_index = priv->beam_restore_to_end[0].index;
        bool found = snapshot_binary_search_eq(priv->beam_restore_to_end, end_index, tag, i, priv->snapshot_last_found_pos);
        if (! found) {
          cerr << "beam search failed (fast-forward not found)" << endl;
          throw exception();
        }

        assert(sizeof_data == priv->beam_restore_to_end[i].data_size);
        memcpy(data_ptr, priv->beam_restore_to_end[i].data_ptr, sizeof_data);
        priv->t = priv->beam_restore_to_end[i].pred_step;
      }

      return;
    }

    cerr << "yikes, how did I get here? state = " << priv->state << endl;
    throw exception();
  }


  void searn_snapshot(searn_private* priv, size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction) {
    if (! priv->do_snapshot) return;

    if (tag == 1) {
      priv->most_recent_snapshot_hash  = 38429103;
      priv->most_recent_snapshot_begin = priv->snapshot_data.size();
      priv->most_recent_snapshot_end   = -1;
      cdbg << "end = -1   ***" << endl;
      priv->most_recent_snapshot_loss  = 0.f;
      if (priv->loss_declared)
        switch (priv->state) {
          case INIT_TEST : priv->most_recent_snapshot_loss = priv->test_loss;  break;
          case INIT_TRAIN: priv->most_recent_snapshot_loss = priv->train_loss; break;
          case LEARN     : priv->most_recent_snapshot_loss = priv->learn_loss; break;
          default        : break;
        }

      if (priv->state == INIT_TRAIN)
        priv->final_snapshot_begin = priv->most_recent_snapshot_begin;
      
      if (priv->auto_history) {
        size_t history_size = sizeof(uint32_t) * priv->hinfo.length;
        searn_snapshot_data(priv, index, 0, priv->rollout_action.begin + priv->t, history_size, true);
      }
    }

    searn_snapshot_data(priv, index, tag, data_ptr, sizeof_data, used_for_prediction);
    if (priv->state == INIT_TRAIN)
        priv->final_snapshot_end = priv->most_recent_snapshot_end;
  }


  inline bool cmp_size_t(const size_t a, const size_t b) { return a < b; }

  v_array<size_t> get_training_timesteps(vw& all, searn& srn)
  {
    v_array<size_t> timesteps;

    if (srn.priv->subsample_timesteps <= 0) {
      for (size_t t=0; t<srn.priv->T; t++)
        timesteps.push_back(t);
    } else if (srn.priv->subsample_timesteps < 1) {
      for (size_t t=0; t<srn.priv->T; t++)
        if (frand48() <= srn.priv->subsample_timesteps)
          timesteps.push_back(t);

      if (timesteps.size() == 0) // ensure at least one
        timesteps.push_back((size_t)(frand48() * srn.priv->T));
    } else {
      while ((timesteps.size() < (size_t)srn.priv->subsample_timesteps) &&
             (timesteps.size() < srn.priv->T)) {
        size_t t = (size_t)(frand48() * (float)srn.priv->T);
        if (! v_array_contains(timesteps, t))
          timesteps.push_back(t);
      }
      std::sort(timesteps.begin, timesteps.end, cmp_size_t);
    }

    return timesteps;
  }

  size_t labelset_size(searn&srn,void*l) {
    if (srn.priv->rollout_all_actions)
      return ((COST_SENSITIVE::label*)l)->costs.size();
    else
      return ((CB::label*)l)->costs.size();
  }

  size_t labelset_class_index(searn&srn, void*l, size_t i) {
    if (srn.priv->rollout_all_actions)
      return ((COST_SENSITIVE::label*)l)->costs[i].class_index;
    else
      return ((CB::label*)l)->costs[i].action;
  }

  bool should_print_update(vw& all, bool hit_new_pass=false)
  {
    //uncomment to print out final loss after all examples processed
    //commented for now so that outputs matches make test
    //if( parser_done(all.p)) return true;

    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS && hit_new_pass) return true;
    return (all.sd->weighted_examples >= all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  bool might_print_update(vw& all)
  {
    // basically do should_print_update but check me and the next
    // example because of off-by-ones

    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS) return true;
    return (all.sd->weighted_examples + 1. >= all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  void generate_training_example(vw& all, searn& srn, learner& base, example* ec, size_t len, void*labels, v_array<float> losses)
  {
    cdbg << "losses.size = " << losses.size() << endl; for (size_t i=0; i<losses.size(); i++) cdbg << "    " << losses[i]; cdbg << endl;
    assert(labelset_size(srn, labels) == losses.size());
    float min_loss = FLT_MAX;
    for (size_t i=0; i<losses.size(); i++)
      if (losses[i] < min_loss) min_loss = losses[i];
    cdbg << "losses.size = " << losses.size() << endl; for (size_t i=0; i<losses.size(); i++) cdbg << "    " << losses[i]; cdbg << endl;
    for (size_t i=0; i<losses.size(); i++)
      if (srn.priv->rollout_all_actions)
        ((COST_SENSITIVE::label*)labels)->costs[i].x = losses[i] - min_loss;
      else
        ((CB::label*)labels)->costs[i].cost = losses[i] - min_loss;
    cdbg << "losses.size = " << losses.size() << " = {"; for (size_t i=0; i<losses.size(); i++) cdbg << " " << ((COST_SENSITIVE::label*)labels)->costs[i].x; cdbg << " }" << endl;

    if (!srn.priv->is_ldf) { // not LDF
      void* old_label = ec[0].ld;
      ec[0].ld = labels;
      if (srn.priv->auto_history) add_history_to_example(all, srn.priv->hinfo, ec, srn.priv->rollout_action.begin+srn.priv->learn_t);
              if (srn.priv->hinfo.length>0) {cdbg << "add_history_to_example: srn.priv->learn_t=" << srn.priv->learn_t << " h=" << srn.priv->rollout_action.begin[srn.priv->learn_t] << endl;}
              if (srn.priv->hinfo.length>0) {cdbg << "  rollout_action = ["; for (size_t i=0; i<srn.priv->learn_t+1; i++) cdbg << " " << srn.priv->rollout_action.begin[i]; cdbg << " ], len=" << srn.priv->rollout_action.size() << endl;}
      ec[0].in_use = true;
      base.learn(ec[0], srn.priv->current_policy);
      if (srn.priv->auto_history) remove_history_from_example(all, srn.priv->hinfo, ec);
      ec[0].ld = old_label;
      srn.priv->total_examples_generated++;
    } else { // isLDF
    cdbg << "losses.size = " << losses.size() << endl; for (size_t i=0; i<losses.size(); i++) cdbg << "    " << losses[i]; cdbg << endl;
      for (size_t a=0; a<len; a++) {
        cdbg << "losses[" << a << "] = " << losses[a] << endl;
        //((OAA::mc_label*)ec[a]->ld)->weight = losses[a] - min_loss;
        COST_SENSITIVE::label* lab = (COST_SENSITIVE::label*)ec[a].ld;
        COST_SENSITIVE::cs_label.default_label(lab);
        COST_SENSITIVE::wclass c = { losses[a] - min_loss, (uint32_t)a, 0., 0. };
        lab->costs.push_back(c);
        cdbg << "learn t = " << srn.priv->learn_t << " cost = " << ((COST_SENSITIVE::label*)ec[a].ld)->costs[0].x << " action = " << ((COST_SENSITIVE::label*)ec[a].ld)->costs[0].class_index << endl;
        //cdbg << endl << "this_example = "; GD::print_audit_features(all, &ec[a]);
        if (srn.priv->auto_history)
          add_history_to_example(all, srn.priv->hinfo, &ec[a], srn.priv->rollout_action.begin+srn.priv->learn_t, a * history_constant);
        //((COST_SENSITIVE::label*)ec[a].ld)->costs[0].class_index);
        ec[a].in_use = true;
        base.learn(ec[a], srn.priv->current_policy);
      }
      cdbg << "learn: generate empty example" << endl;
      base.learn(*srn.priv->empty_example);
      //cdbg << "learn done " << repeat << endl;
      if (srn.priv->auto_history)
        for (size_t a=0; a<len; a++)
          remove_history_from_example(all, srn.priv->hinfo, &ec[a]);
      srn.priv->total_examples_generated++;
    }
  }

  void clear_rollout_actions(searn&srn) {
    /*UNDOME*/cdbg << "clear_rollout_actions" << endl;
    srn.priv->rollout_action.erase();
    for (size_t t=0; t<srn.priv->hinfo.length; t++)
      srn.priv->rollout_action.push_back(0);
  }


  void reset_searn_structure(searn&srn) {
    srn.priv->t = 0;
    srn.priv->T = 0;
    srn.priv->loss_last_step = 0;
    srn.priv->test_loss = 0.f;
    srn.priv->train_loss = 0.f;
    srn.priv->learn_loss = 0.f;
    srn.priv->num_features = 0;
    srn.priv->train_action.erase();
    srn.priv->train_action_ids.erase();
    srn.priv->loss_declared = false;
    if (srn.priv->auto_history) clear_rollout_actions(srn);

    srn.priv->snapshot_is_equivalent_to_t = (size_t)-1;
    srn.priv->snapshot_could_match = false;
    srn.priv->snapshot_last_found_pos = (size_t)-1;
  }

  void mark_hyp_pruned(void*data) { ((beam_hyp*)data)->pruned = true; }

  void compute_full_beam(vw&all, searn&srn, vector<example*>ec, v_array<beam_hyp>& hyp_pool, size_t& hyp_pool_id, Beam::beam* final_beam) {
    using namespace Beam;
    uint32_t DEFAULT_HASH = 0;

    beam* cur_beam   = new beam(srn.priv->beam_size);
    beam* next_beam  = new beam(srn.priv->beam_size);

    // initialize first beam
    {
      // in this call to structured_predict, we do the following:
      //   1) collect the number of actions & corresponding costs available at time 0
      //   2) collect the initial snapshot
      //   3) store the final snapshot so we can fast-foward to the end at will
      beam_hyp *hyp = hyp_pool.begin;

      hyp->t            = 0;
      hyp->parent       = NULL;
      hyp->action_taken = 0;     // irrelevant because parent==NULL
      hyp->action_costs = NULL;  // we will fill this in
      hyp->snapshot.erase();     // will be left empty
      hyp->incr_cost    = 0.;    // will be left as 0.
      hyp->num_actions  = 0;     // will get filled in
      hyp->filled_in_prediction = false; // will (hopefully) get filled in
      hyp->filled_in_snapshot   = false; // will *not* get filled in
      hyp->pruned = false;       // haven't been pruned yet

      reset_searn_structure(srn);
      srn.priv->state        = BEAM_INIT;
      srn.priv->cur_beam_hyp = hyp;
      srn.task->structured_predict(srn, ec);

      assert(hyp->filled_in_prediction);   // TODO: handle the case that structured_predict just returns or something else weird happens

      // collect the costs
      if (srn.priv->rollout_all_actions) { // TODO: handle CB
        v_array<COST_SENSITIVE::wclass>* costs = &((COST_SENSITIVE::label*)srn.priv->valid_labels)->costs;
        assert(hyp->num_actions == costs->size());
        cdbg << "action_costs =";
        hyp->action_costs = (float*)calloc_or_die(hyp->num_actions, sizeof(float));
        for (size_t i=0; i<hyp->num_actions; i++) {
          hyp->action_costs[i] = (costs->begin+i)->partial_prediction;
          cdbg << " " << hyp->action_costs[i];
        }
        cdbg << endl;
      }

      // collect the final snapshot
      copy_array(srn.priv->beam_restore_to_end, srn.priv->snapshot_data);

      bool added = cur_beam->insert(hyp, 0., DEFAULT_HASH);
      if (!added) hyp->pruned = true;
    }

    while (! cur_beam->empty() ) {
      /*UNDOME*/cdbg << "cur_beam is not empty" << endl;
      for (beam_element* be = cur_beam->begin(); be != cur_beam->end(); ++be) {
        /*UNDOME*/cdbg << "got be" << endl;
        beam_hyp* hyp = (beam_hyp*) be->data;
        for (size_t a=0; a<hyp->num_actions; a++) {
          /*UNDOME*/cdbg << "expanding hyp @ " << hyp << " { t=" << hyp->t << ", action_taken=" << hyp->action_taken << ", parent=" << hyp->parent << " a=" << a << "/" << hyp->num_actions << " }" << endl;
          if (hyp_pool.begin + hyp_pool_id + 1 >= hyp_pool.end_array) {
            assert(false);
            hyp_pool.resize(hyp_pool_id * 2 + 1, true);
          }
          beam_hyp *next = hyp_pool.begin + (++hyp_pool_id);
          next->t            = hyp->t + 1; // TODO: make this more flexible
          next->parent       = hyp;
          next->incr_cost    = hyp->action_costs[a];
          next->action_taken = a;     // which action did we take from parent->snapshot to get here?
          next->action_costs = NULL;  // we will fill this in
          next->num_actions  = 0;     // will get filled in
          next->snapshot.erase();     // will get filled in
          next->num_actions  = 0;     // will get filled in
          next->filled_in_prediction = false; // will (hopefully) get filled in
          next->filled_in_snapshot   = false; // will (hopefully) get filled in
          next->pruned       = false;

          reset_searn_structure(srn);
          srn.priv->state        = BEAM_ADVANCE;
          srn.priv->cur_beam_hyp = next;
          /*UNDOME*/cdbg << "======== BEAM_ADVANCE ==" << endl;
          srn.task->structured_predict(srn, ec);

          bool added = false;
          if (next->filled_in_snapshot) { // another snapshot was called
            // collect the costs
            if (srn.priv->rollout_all_actions) { // TODO: handle CB
              v_array<COST_SENSITIVE::wclass>* costs = &((COST_SENSITIVE::label*)srn.priv->valid_labels)->costs;
              assert(next->num_actions == costs->size());
              next->action_costs = (float*)calloc_or_die(next->num_actions, sizeof(float));
              for (size_t i=0; i<next->num_actions; i++)
                next->action_costs[i] = (costs->begin+i)->partial_prediction;
            }

            cdbg << "next_beam->insert(a=" << next->action_taken << ", cost=" << (be->cost + next->incr_cost) << ")" << endl;
            added = next_beam->insert(next, be->cost + next->incr_cost, DEFAULT_HASH);
            if (!added) next->pruned = true;
          } else {  // we reached the end of structured_predict
            cdbg << "final_beam->insert(a=" << next->action_taken << ", cost=" << (be->cost + next->incr_cost) << ")" << endl;
            added = final_beam->insert(next, be->cost + next->incr_cost, DEFAULT_HASH);
            if (!added) next->pruned = true;
          }
          /*UNDOME*/cdbg << "expansion added=" << added << ", filled_in_snapshot=" << next->filled_in_snapshot << endl;
          /*UNDOME*/cdbg << "a = " << a << " / " << hyp->num_actions << endl;
        }
        next_beam->maybe_compact(mark_hyp_pruned);
        final_beam->maybe_compact(mark_hyp_pruned);
      }

      // swap beams and finalize
      next_beam->compact(mark_hyp_pruned);
      beam* temp_beam = cur_beam;
      cur_beam = next_beam;
      next_beam = temp_beam;
      next_beam->erase();

      // debugging
      /*UNDOME*/cdbg << "NEXT BEAM =" << endl;
      for (beam_element* be = cur_beam->begin(); be != cur_beam->end(); ++be) {
        beam_hyp* hyp = (beam_hyp*) be->data;
        /*UNDOME*/cdbg << "\t{ cost=" << be->cost << " t=" << hyp->t << " action_taken=" << hyp->action_taken << " incr_cost=" << hyp->incr_cost << " num_actions=" << hyp->num_actions << " parent=" << hyp->parent << " }" << endl;
      }
    }

    // debug print the final beam
    /*UNDOME*/cdbg << "FINAL BEAM =" << endl;
    for (beam_element* be = final_beam->begin(); be != final_beam->end(); ++be) {
      beam_hyp* hyp = (beam_hyp*) be->data;
      /*UNDOME*/cdbg << "\tcost=" << be->cost;
      while (hyp != NULL) {
        /*UNDOME*/cdbg << " <- " << hyp->action_taken;
        hyp = hyp->parent;
      }
      /*UNDOME*/cdbg << endl;
    }

    // get the final info out
    final_beam->compact(mark_hyp_pruned);

    cur_beam->erase();
    next_beam->erase();
    delete cur_beam;
    delete next_beam;
  }


  void free_hyp_pool(v_array<beam_hyp> &hyp_pool, size_t &hyp_pool_id) {
    for (size_t i=0; i<hyp_pool_id; i++) {
      if (hyp_pool[i].action_costs)
        free(hyp_pool[i].action_costs);
      for (size_t j=0; j<hyp_pool[i].snapshot.size(); j++)
        if (hyp_pool[i].snapshot[j].data_ptr)
          free(hyp_pool[i].snapshot[j].data_ptr);
      hyp_pool[i].snapshot.delete_v();
    }
    hyp_pool.erase();
    hyp_pool_id = 0;
  }


  void beam_predict(vw&all, searn&srn, vector<example*>ec, v_array<beam_hyp> &hyp_pool, size_t &hyp_pool_id, bool is_learn) {
    using namespace Beam;

    if (might_print_update(all)) {
      reset_searn_structure(srn);
      srn.priv->state = GET_TRUTH_STRING;
      srn.priv->should_produce_string = true;
      srn.priv->truth_string->str("");
      srn.task->structured_predict(srn, ec);
    }

    beam* final_beam = new beam(max(1, min(srn.priv->beam_size, srn.priv->kbest)));  // at least 1, but otherwise the min of beam_size and kbest

    compute_full_beam(all, srn, ec, hyp_pool, hyp_pool_id, final_beam);

    if (srn.priv->should_produce_string && !is_learn) { // TODO: check if this is going to be used at all!!!
      /*UNDOME*/cdbg << "========== FINAL ROLLOUT(S) ==" <<endl;
      assert(final_beam->size() > 0);
      stringstream spred;
      stringstream* old_pred_string = srn.priv->pred_string;

      srn.priv->pred_string->str("");
      bool is_first = true;
      for (beam_element * be = final_beam->begin(); be != final_beam->end(); ++be) {
        beam_hyp* hyp = (beam_hyp*)be->data;
        assert(hyp);
        assert(hyp->parent);
        // TODO: proper k-best using reverse exact a* search
        srn.priv->beam_final_action_sequence.erase();
        for (; hyp->parent != NULL; hyp = hyp->parent)
          srn.priv->beam_final_action_sequence.push_back((uint32_t)hyp->action_taken);

        reset_searn_structure(srn);
        srn.priv->state = BEAM_PLAYOUT;
        srn.priv->pred_string = is_first ? old_pred_string : &spred;
        spred.str("");
        srn.task->structured_predict(srn, ec);

        if (all.final_prediction_sink.size() > 0)
          for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink) {
            if (srn.priv->kbest > 1) {
              stringstream output_string;
              output_string << be->cost << "\t" << srn.priv->pred_string->str();
              all.print_text(*sink, output_string.str(), ec[0]->tag);
            } else
              all.print_text(*sink, srn.priv->pred_string->str(), ec[0]->tag);
          }

        is_first = false;
      }
      srn.priv->pred_string = old_pred_string;

      if ((srn.priv->kbest > 1) && (all.final_prediction_sink.size() > 0))
        for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink)
          all.print_text(*sink, "", ec[0]->tag);
    }

    final_beam->erase();
    delete final_beam;
  }

  bool must_run_test(vw&all, vector<example*>ec) {
    return
        (all.final_prediction_sink.size() > 0) ||   // if we have to produce output, we need to run this
        might_print_update(all) ||                  // if we have to print and update to stderr
        (all.raw_prediction > 0) ||                 // we need raw predictions
        // or:
        //   it's not quiet AND
        //     current_pass == 0
        //     OR holdout is off
        //     OR it's a test example
        ( (! all.quiet) &&
          ( all.holdout_set_off ||                    // no holdout
            ec[0]->test_only ||
            (all.current_pass == 0)                   // we need error rates for progressive cost
            ) )
        ;
  }

  template <bool is_learn>
  void train_single_example(vw& all, searn& srn, vector<example*>ec) {
    // do an initial test pass to compute output (and loss)
    cdbg << "======================================== INIT TEST (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;

    reset_searn_structure(srn);
    srn.priv->state = INIT_TEST;

    if (must_run_test(all, ec)) {
      srn.priv->should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0) || (all.raw_prediction > 0);
      srn.priv->pred_string->str("");

      assert(srn.priv->truth_string != NULL);
      srn.task->structured_predict(srn, ec);
      srn.priv->should_produce_string = false;

      for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink)
        all.print_text((int)*sink, srn.priv->pred_string->str(), ec[0]->tag);

      if ((all.raw_prediction > 0) && (srn.priv->rollout_all_actions)) {
        all.print_text(all.raw_prediction, "", ec[0]->tag);
      }
    }

    if (is_learn && all.training && !ec[0]->test_only) {
      if (srn.priv->adaptive_beta)
        srn.priv->beta = 1.f - powf(1.f - srn.priv->alpha, (float)srn.priv->total_examples_generated);

      // do a pass over the data allowing oracle and snapshotting
      cdbg << "======================================== INIT TRAIN (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
      srn.priv->state = INIT_TRAIN;
      srn.priv->train_action.erase();
      srn.priv->train_action_ids.erase();
      if (srn.priv->auto_history) clear_rollout_actions(srn);
      srn.priv->t = 0;
      srn.priv->loss_last_step = 0;
      clear_snapshot(all, srn, true);

      srn.priv->snapshot_is_equivalent_to_t = (size_t)-1;
      srn.priv->snapshot_last_found_pos = (size_t)-1;
      srn.priv->snapshot_could_match = false;
      srn.priv->loss_declared = false;
      srn.priv->should_produce_string = false;

      srn.task->structured_predict(srn, ec);

      if ( (! srn.priv->loss_declared) &&   // no loss was declared
           (is_learn)                  &&   // and we're trying to learn
           (all.training)              &&   // in training mode
           (! ec[0]->test_only) )           // and not a test example
        cerr << "warning: no loss declared by task on something that looks like a training example!" << endl;

      if (srn.priv->t == 0) {
        clear_snapshot(all, srn, true);
        return;  // there was no data
      }

      srn.priv->T = srn.priv->t;

      // generate training examples on which to learn
      cdbg << "======================================== LEARN (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
      v_array<size_t> tset = get_training_timesteps(all, srn);
      cdbg << "tset ="; for (size_t*t=tset.begin; t!=tset.end; ++t) cdbg << " " << (*t); cdbg << endl;
      for (size_t tid=0; tid<tset.size(); tid++) {
        size_t t = tset[tid];
        void *aset = srn.priv->train_labels[t];
        srn.priv->learn_t = t;
        srn.priv->learn_losses.erase();

        cdbg << "t=" << tid << ", labelset_size=" << labelset_size(srn, aset) << endl;
        for (size_t i=0; i<labelset_size(srn, aset); i++) {
          if (srn.priv->auto_history) { // TODO: check if the push_many is actually used
            // startup the rollout at the train actions
            clear_rollout_actions(srn);
            //srn.priv->rollout_action.resize(srn.priv->hinfo.length + srn.priv->T);
            push_many(srn.priv->rollout_action, srn.priv->train_action.begin, t);
            //memcpy(srn.priv->rollout_action.begin + srn.priv->hinfo.length, srn.priv->train_action.begin, srn.priv->T * sizeof(uint32_t));
          }
          // srn.priv->snapshot_last_found_pos = (size_t)-1;

          size_t this_index = labelset_class_index(srn, aset, i);
          assert(this_index <= srn.priv->A);
          if (false && (this_index == srn.priv->train_action_ids[srn.priv->learn_t])) {  // TODO: fix this!
            srn.priv->learn_losses.push_back( srn.priv->train_loss );
            cdbg << srn.priv->train_loss << "* ";
          } else {
            srn.priv->t = 0;
            srn.priv->learn_a = (uint32_t)this_index;
            srn.priv->loss_last_step = 0;
            srn.priv->learn_loss = 0.f;
            srn.priv->learn_example_len = 0;
            cdbg << "learn_example_len = 0" << endl;
            cdbg << "learn_t = " << srn.priv->learn_t << " || learn_a = " << srn.priv->learn_a << endl;
            // srn.priv->snapshot_is_equivalent_to_t = (size_t)-1;
            // srn.priv->snapshot_could_match = true;
            srn.priv->state = LEARN;
            srn.task->structured_predict(srn, ec);

            srn.priv->learn_losses.push_back( srn.priv->learn_loss );
            cdbg << "total loss: " << srn.priv->learn_loss << endl;
          }
        }

        if (srn.priv->learn_example_len != 0) {
          example * ptr = srn.priv->examples_dont_change ? srn.priv->learn_example_ref : srn.priv->learn_example_copy;
          cdbg << "generate_training_example on " << srn.priv->learn_example_len << " learn_example_copy items" << endl;
          // cdbg << "losses:";
          // for (size_t n=0; n<srn.priv->learn_example_len; n++) {
          //   cdbg << " {";
          //   COST_SENSITIVE::label* ld = ((COST_SENSITIVE::label*)ptr[n].ld);
          //   for (size_t m=0; m<ld->costs.size(); m++)
          //     cdbg << " " << ld->costs[m].class_index << ":" << ld->costs[m].x;
          //     cdbg << " }";
          // }
          // cdbg << endl;
          generate_training_example(all, srn, *srn.priv->base_learner, ptr, srn.priv->learn_example_len, aset, srn.priv->learn_losses);
          // cdbg << "losses:";
          // for (size_t n=0; n<srn.priv->learn_example_len; n++) {
          //   cdbg << " {";
          //   COST_SENSITIVE::label* ld = ((COST_SENSITIVE::label*)ptr[n].ld);
          //   for (size_t m=0; m<ld->costs.size(); m++)
          //     cdbg << " " << ld->costs[m].class_index << ":" << ld->costs[m].x;
          //     cdbg << " }";
          // }
          // cdbg << endl;

          if (!srn.priv->examples_dont_change) {
            cdbg << "deleting labels for " << srn.priv->learn_example_len << " learn_example_copy items" << endl;
            for (size_t n=0; n<srn.priv->learn_example_len; n++)
              //cdbg << "free_example_data[" << n << "]: "; GD::print_audit_features(all, &srn.priv->learn_example_copy[n]);
	      if (srn.priv->is_ldf) COST_SENSITIVE::cs_label.delete_label(srn.priv->learn_example_copy[n].ld);
              else                  MULTICLASS::mc_label.delete_label(srn.priv->learn_example_copy[n].ld);
          }
        } else {
          cerr << "warning: searn did not generate an example for a given time-step" << endl;
        }
        cdbg << " | ";
      }
      tset.erase(); tset.delete_v();
      cdbg << endl;
    }

    clear_snapshot(all, srn, true);
    srn.priv->train_action.delete_v();
    srn.priv->train_action_ids.delete_v();
    srn.priv->rollout_action.delete_v();
    for (size_t i=0; i<srn.priv->train_labels.size(); i++) {
      if (srn.priv->rollout_all_actions) {
        ((COST_SENSITIVE::label*)srn.priv->train_labels[i])->costs.erase();
        ((COST_SENSITIVE::label*)srn.priv->train_labels[i])->costs.delete_v();
        delete ((COST_SENSITIVE::label*)srn.priv->train_labels[i]);
      } else {
        ((CB::label*)srn.priv->train_labels[i])->costs.erase();
        ((CB::label*)srn.priv->train_labels[i])->costs.delete_v();
        delete ((CB::label*)srn.priv->train_labels[i]);
      }
    }
    srn.priv->train_labels.erase();
    srn.priv->train_labels.delete_v();

    cdbg << "======================================== DONE (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;

  }


  void clear_seq(vw&all, searn& srn)
  {
    if (srn.priv->ec_seq.size() > 0)
      for (size_t i=0; i < srn.priv->ec_seq.size(); i++)
        VW::finish_example(all, srn.priv->ec_seq[i]);
    srn.priv->ec_seq.clear();
  }

  float safediv(float a,float b) { if (b == 0.f) return 0.f; else return (a/b); }

  void to_short_string(string in, size_t max_len, char*out) {
    for (size_t i=0; i<max_len; i++) {
      if (i >= in.length())
        out[i] = ' ';
      else if ((in[i] == '\n') || (in[i] == '\t'))    // TODO: maybe catch other characters?
        out[i] = ' ';
      else
        out[i] = in[i];
    }

    if (in.length() > max_len) {
      out[max_len-2] = '.';
      out[max_len-1] = '.';
    }
    out[max_len] = 0;
  }

void print_update(vw& all, searn& srn)
  {
    if (!srn.priv->printed_output_header && !all.quiet) {
      const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";
      fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
      fprintf(stderr, header_fmt, "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
      cerr.precision(5);
      srn.priv->printed_output_header = true;
    }

    if (!should_print_update(all, srn.priv->hit_new_pass))
      return;

    char true_label[21];
    char pred_label[21];
    to_short_string(srn.priv->truth_string->str(), 20, true_label);
    to_short_string(srn.priv->pred_string->str() , 20, pred_label);

    float avg_loss = 0.;
    float avg_loss_since = 0.;
    if (!all.holdout_set_off && all.current_pass >= 1) {
      avg_loss       = safediv((float)all.sd->holdout_sum_loss, (float)all.sd->weighted_holdout_examples);
      avg_loss_since = safediv((float)all.sd->holdout_sum_loss_since_last_dump, (float)all.sd->weighted_holdout_examples_since_last_dump);

      all.sd->weighted_holdout_examples_since_last_dump = 0;
      all.sd->holdout_sum_loss_since_last_dump = 0.0;
    } else {
      avg_loss       = safediv((float)all.sd->sum_loss, (float)all.sd->weighted_examples);
      avg_loss_since = safediv((float)all.sd->sum_loss_since_last_dump, (float) (all.sd->weighted_examples - all.sd->old_weighted_examples));
    }

    fprintf(stderr, "%-10.6f %-10.6f %8ld %15f   [%s] [%s] %8lu %5d %5d %15lu %15lu",
            avg_loss,
            avg_loss_since,
            (long int)all.sd->example_number,
            all.sd->weighted_examples,
            true_label,
            pred_label,
            (long unsigned int)srn.priv->num_features,
            (int)srn.priv->read_example_last_pass,
            (int)srn.priv->current_policy,
            (long unsigned int)srn.priv->total_predictions_made,
            (long unsigned int)srn.priv->total_examples_generated);

    if (PRINT_CLOCK_TIME) {
      size_t num_sec = (size_t)(((float)(clock() - srn.priv->start_clock_time)) / CLOCKS_PER_SEC);
      fprintf(stderr, " %15lusec", num_sec);
    }

    //fprintf(stderr, " beta=%g", srn.priv->beta);

    if (!all.holdout_set_off && all.current_pass >= 1)
      fprintf(stderr, " h");

    fprintf(stderr, "\n");

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    fflush(stderr);
    VW::update_dump_interval(all);
  }

  void add_neighbor_features(searn& srn) {
    size_t neighbor_constant = 8349204823;
    vw*all = srn.priv->all;
    if (srn.priv->neighbor_features.size() == 0) return;
    uint32_t wpp = srn.priv->all->wpp << srn.priv->all->reg.stride_shift;

    for (int32_t n=0; n<(int32_t)srn.priv->ec_seq.size(); n++) {
      example*me = srn.priv->ec_seq[n];
      cdbg << "add " << n << " n=" << me->num_features << endl;
      for (int32_t*enc=srn.priv->neighbor_features.begin; enc!=srn.priv->neighbor_features.end; ++enc) {
        int32_t offset = (*enc) >> 24;
        size_t  old_ns = (*enc) & 0xFF;
        size_t  enc_offset = wpp * ((2 * (size_t)(*enc)) + ((*enc < 0) ? 1 : 0));

        cdbg << "old_ns = " << old_ns << endl;

        if ((n + offset >= 0) && (n + offset < (int32_t)srn.priv->ec_seq.size())) { // we're okay on position
          example*you = srn.priv->ec_seq[n+offset];
          size_t  you_size = you->atomics[old_ns].size();

          if (you_size > 0) {
            if (me->atomics[neighbor_namespace].size() == 0)
              me->indices.push_back(neighbor_namespace);

            me->atomics[neighbor_namespace].resize(me->atomics[neighbor_namespace].size() + you_size + 1);
            for (feature*f = you->atomics[old_ns].begin; f != you->atomics[old_ns].end; ++f) {
              feature f2 = { (*f).x, (uint32_t)( ((*f).weight_index * neighbor_constant + enc_offset) & srn.priv->all->reg.weight_mask ) };
              me->atomics[neighbor_namespace].push_back(f2);
              cdbg << "_";
            }

            if (all->audit && (all->current_pass==0)) {
              assert(you->atomics[old_ns].size() == you->audit_features[old_ns].size());
              for (audit_data*f = you->audit_features[old_ns].begin; f != you->audit_features[old_ns].end; ++f) {
                uint32_t wi = (uint32_t)((*f).weight_index * neighbor_constant + enc_offset) & srn.priv->all->reg.weight_mask;
                audit_data f2 = { NULL, NULL, wi, f->x, true };

                f2.space = (char*) calloc_or_die(neighbor_feature_space.length()+1, sizeof(char));
                strcpy(f2.space, neighbor_feature_space.c_str());

                f2.feature = (char*) calloc_or_die( strlen(f->feature) + 6, sizeof(char) );
                f2.feature[0] = '@';
                f2.feature[1] = (offset > 0) ? '+' : '-';
                f2.feature[2] = (char)(abs(offset) + '0');
                f2.feature[3] = (char)old_ns;
                f2.feature[4] = '=';
                strcpy(f2.feature+5, f->feature);

                me->audit_features[neighbor_namespace].push_back(f2);
                cdbg << "+" << "[" << me->audit_features[neighbor_namespace].size() << "]";
              }

            }
            //cdbg << "copying " << you_size << " features" << endl;
            me->sum_feat_sq[neighbor_namespace] += you->sum_feat_sq[old_ns];
            me->total_sum_feat_sq += you->sum_feat_sq[old_ns];
            me->num_features += you_size;
          }
        } else if ((n + offset == -1) || (n + offset == (int32_t)srn.priv->ec_seq.size())) { // handle <s> and </s>
          size_t bias  = constant * ((n + offset < 0) ? 2 : 3);
          uint32_t fid = ((uint32_t)(( bias * neighbor_constant + enc_offset))) & srn.priv->all->reg.weight_mask;

          if (me->atomics[neighbor_namespace].size() == 0)
            me->indices.push_back(neighbor_namespace);

          feature f = { 1., fid };
          me->atomics[neighbor_namespace].push_back(f);
          cdbg << ".";

          if (all->audit && (all->current_pass==0)) {
            audit_data f2 = { NULL, NULL, fid, 1., true };

            f2.space = (char*) calloc_or_die(neighbor_feature_space.length()+1, sizeof(char));
            strcpy(f2.space, neighbor_feature_space.c_str());

            f2.feature = (char*) calloc_or_die(4, sizeof(char) );
            f2.feature[0] = 'b';
            f2.feature[1] = '@';
            f2.feature[2] = (offset > 0) ? '+' : '-';
            f2.feature[3] = 0;

            me->audit_features[neighbor_namespace].push_back(f2);
            cdbg << "+" << "{" << me->audit_features[neighbor_namespace].size() << "}";
          }

          me->sum_feat_sq[neighbor_namespace] += 1.;
          me->total_sum_feat_sq += 1.;
          me->num_features += 1;
        }
      }
      cdbg << "audit=" << me->audit_features[neighbor_namespace].size() << ", atomics=" << me->atomics[neighbor_namespace].size() << endl;
      cdbg << "add n'=" << me->num_features << endl;
    }
  }

  void del_neighbor_features(searn& srn) {
    if (srn.priv->neighbor_features.size() == 0) return;
    vw*all = srn.priv->all;

    for (int32_t n=0; n<(int32_t)srn.priv->ec_seq.size(); n++) {
      example*me = srn.priv->ec_seq[n];
      cdbg << "del n=" << me->num_features;
      size_t total_size = 0;
      float total_sfs = 0.;

      for (int32_t*enc=srn.priv->neighbor_features.begin; enc!=srn.priv->neighbor_features.end; ++enc) {
        int32_t offset = (*enc) >> 24;
        size_t  old_ns = (*enc) & 0xFF;

        if ((n + offset >= 0) && (n + offset < (int32_t)srn.priv->ec_seq.size())) { // we're okay on position
          example*you = srn.priv->ec_seq[n+offset];
          total_size += you->atomics[old_ns].size();
          total_sfs  += you->sum_feat_sq[old_ns];
        } else if ((n + offset == -1) || (n + offset == (int32_t)srn.priv->ec_seq.size())) {
          total_size += 1;
          total_sfs += 1;
        }
      }

      if (total_size > 0) {
        if (me->atomics[neighbor_namespace].size() == total_size) {
          char last_idx = me->indices.pop();
          if (last_idx != (char)neighbor_namespace) {
            cerr << "error: some namespace was added after the neighbor namespace" << endl;
            throw exception();
          }
          cdbg << "erasing new ns '" << (char)neighbor_namespace << "' of size " << me->atomics[neighbor_namespace].size() << endl;
          me->atomics[neighbor_namespace].erase();
        } else {
          cerr << "warning: neighbor namespace seems to be the wrong size? (total_size=" << total_size << " but ns.size=" << me->atomics[neighbor_namespace].size() << ")" << endl;
          assert(false);
          me->atomics[neighbor_namespace].end -= total_size;
          cdbg << "erasing " << total_size << " features" << endl;
        }

        if (all->audit && (all->current_pass == 0)) {
          assert(total_size == me->audit_features[neighbor_namespace].size());

          for (audit_data*ad = me->audit_features[neighbor_namespace].begin; ad != me->audit_features[neighbor_namespace].end; ++ad)
            if (ad->alloced) {
              free(ad->space);
              free(ad->feature);
            }

          me->audit_features[neighbor_namespace].end -= total_size;
        }

        me->sum_feat_sq[neighbor_namespace] -= total_sfs;
        me->total_sum_feat_sq -= total_sfs;
        me->num_features -= total_size;
      } else {
        // TODO: add dummy features for <s> or </s>
      }
      cdbg<< "del n'=" << me->num_features << endl;
    }
  }


  void train_single_example_beam(vw&all, searn&srn, v_array<beam_hyp> &hyp_pool, size_t hyp_pool_size) {
    searn_private* priv = srn.priv;
    
    cdbg << "======================================== BEAM LEARN (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
    priv->state = LEARN;
    priv->should_produce_string = false;
    for (size_t hyp_id = 0; hyp_id < hyp_pool_size; hyp_id++) {
      beam_hyp me = hyp_pool[hyp_id];
      if (me.pruned) continue;
      if (me.num_actions == 0) continue;
      cdbg << "=== hyp_id = " << hyp_id << " ===" << endl;
      cdbg << "me = {" << endl << "  t = " << me.t << endl << "  action_taken = " << me.action_taken << endl << "  parent = " << me.parent << endl << "  parent.action_taken = " << ((me.parent == NULL) ? -1 : me.parent->action_taken) << endl << "}" << endl;

      priv->learn_t = me.t;
      priv->learn_losses.erase();

      COST_SENSITIVE::label aset;

      // TODO: prune training steps ala get_training_timesteps
      cdbg << "t=" << me.t << ", labelset_size=" << me.num_actions << endl;
      for (uint32_t aid=0; aid<me.num_actions; aid++) {
        COST_SENSITIVE::wclass my_class = { 0., aid+1, 0., 0. }; // TODO: make this valid for LDF
        aset.costs.push_back( my_class );

        if (srn.priv->auto_history)
          clear_rollout_actions(srn);

        priv->train_action.resize(me.t);       priv->train_action.end     = priv->train_action.begin     + me.t;
        priv->train_action_ids.resize(me.t);   priv->train_action_ids.end = priv->train_action_ids.begin + me.t;
        beam_hyp *h = &me;
        for (size_t t=0; t<me.t; t++) {
          assert(h != NULL);
          priv->train_action[me.t - t - 1] = (uint32_t)h->action_taken+1;
          priv->train_action_ids[me.t - t - 1] = (uint32_t)h->action_taken+1;  // TODO: make this valid for ldf
          cdbg << "set train_action[" << (me.t-t-1) << "] = " << h->action_taken+1 << endl;
          h = h->parent;
        }
        assert(h != NULL);
        assert(h->action_taken == 0);
        assert(h->parent == NULL);
        cdbg << "train_action.size = " << priv->train_action.size() << endl;
        for (size_t t=0; t<me.t; t++)
          priv->rollout_action.push_back(priv->train_action[t]);

        priv->t = 0;
        priv->learn_a = (uint32_t)aid+1;  // TODO: make this LDF compatible (by remembering what the actions were!)
        priv->loss_last_step = 0;
        priv->learn_loss = 0.f;
        priv->learn_example_len = 0;

        cdbg << "learn_t = " << srn.priv->learn_t << " || learn_a = " << srn.priv->learn_a << endl;
        srn.task->structured_predict(srn, priv->ec_seq);

        priv->learn_losses.push_back( priv->learn_loss );
        cdbg << "total loss: " << srn.priv->learn_loss << endl;
      }

      if (priv->learn_example_len != 0) {
        example * ptr = priv->examples_dont_change ? priv->learn_example_ref : priv->learn_example_copy;
        cdbg << "generate_training_example on " << priv->learn_example_len << " learn_example_copy items" << endl;

        if (priv->adaptive_beta)
          priv->beta = 1.f - powf(1.f - priv->alpha, (float)priv->total_examples_generated);
        generate_training_example(all, srn, *priv->base_learner, ptr, priv->learn_example_len, &aset, priv->learn_losses);

        if (!priv->examples_dont_change) {
          cdbg << "deleting labels for " << priv->learn_example_len << " learn_example_copy items" << endl;
          for (size_t n=0; n<priv->learn_example_len; n++)
            //cdbg << "free_example_data[" << n << "]: "; GD::print_audit_features(all, &priv->learn_example_copy[n]);
            if (priv->is_ldf) COST_SENSITIVE::cs_label.delete_label(priv->learn_example_copy[n].ld);
            else                  MULTICLASS::mc_label.delete_label(priv->learn_example_copy[n].ld);
        }
      } else {
        cerr << "warning: searn did not generate an example for a given time-step" << endl;
      }
    }
  }

  template <bool is_learn>
  void do_actual_learning(vw&all, searn& srn)
  {
    if (srn.priv->ec_seq.size() == 0)
      return;  // nothing to do :)

    add_neighbor_features(srn);

    // if we're going to have to print to the screen, generate the "truth" string
    cdbg << "======================================== GET TRUTH STRING (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
    if (might_print_update(all)) {
      reset_searn_structure(srn);
      srn.priv->state = GET_TRUTH_STRING;
      srn.priv->should_produce_string = true;
      srn.priv->truth_string->str("");
      srn.task->structured_predict(srn, srn.priv->ec_seq);
    }

    if (srn.priv->beam_size == 0)
      train_single_example<is_learn>(all, srn, srn.priv->ec_seq);
    else {
      v_array<beam_hyp> hyp_pool;
      size_t hyp_pool_id = 0;
      float cached_test_loss = 0.;

      if (srn.priv->adaptive_beta)
        srn.priv->beta = 1.f - powf(1.f - srn.priv->alpha, (float)srn.priv->total_examples_generated);

      if (must_run_test(all, srn.priv->ec_seq)) {
        cdbg << "======================================== BEAM TEST (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
        srn.priv->beam_is_training = false;
        srn.priv->should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0) || (all.raw_prediction > 0);
        srn.priv->pred_string->str("");
        hyp_pool.resize(10000, true);
        beam_predict(all, srn, srn.priv->ec_seq, hyp_pool, hyp_pool_id, false);
        cached_test_loss = srn.priv->test_loss;
        free_hyp_pool(hyp_pool, hyp_pool_id);
      }

      srn.priv->beam_is_training = is_learn && all.training && !srn.priv->ec_seq[0]->test_only;
      if (srn.priv->beam_is_training) {
        cdbg << "======================================== BEAM TRAIN (" << srn.priv->current_policy << "," << srn.priv->read_example_last_pass << ") ========================================" << endl;
        hyp_pool.resize(10000, true);
        srn.priv->should_produce_string = false;
        beam_predict(all, srn, srn.priv->ec_seq, hyp_pool, hyp_pool_id, true);
        train_single_example_beam(all, srn, hyp_pool, hyp_pool_id);
        free_hyp_pool(hyp_pool, hyp_pool_id);
      }
      
      hyp_pool.delete_v();
      srn.priv->test_loss = cached_test_loss;
    }

    del_neighbor_features(srn);

    if (srn.priv->ec_seq[0]->test_only) {
      all.sd->weighted_holdout_examples += 1.f;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += 1.f;
      all.sd->weighted_holdout_examples_since_last_pass += 1.f;
      all.sd->holdout_sum_loss += srn.priv->test_loss;
      all.sd->holdout_sum_loss_since_last_dump += srn.priv->test_loss;
      all.sd->holdout_sum_loss_since_last_pass += srn.priv->test_loss;//since last pass
    } else {
      all.sd->weighted_examples += 1.f;
      all.sd->total_features += srn.priv->num_features;
      all.sd->sum_loss += srn.priv->test_loss;
      all.sd->sum_loss_since_last_dump += srn.priv->test_loss;
      all.sd->example_number++;
    }
  }

  template <bool is_learn>
  void searn_predict_or_learn(searn& srn, learner& base, example& ec) {
    vw* all = srn.priv->all;
    srn.priv->base_learner = &base;
    bool is_real_example = true;

    if (example_is_newline(ec) || srn.priv->ec_seq.size() >= all->p->ring_size - 2) {
      if (srn.priv->ec_seq.size() >= all->p->ring_size - 2) { // give some wiggle room
	std::cerr << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << std::endl;
      }

      do_actual_learning<is_learn>(*all, srn);
      clear_seq(*all, srn);
      srn.priv->hit_new_pass = false;

      //VW::finish_example(*all, ec);
      is_real_example = false;
    } else {
      srn.priv->ec_seq.push_back(&ec);
    }

    if (is_real_example) {
      srn.priv->read_example_last_id = ec.example_counter;
    }
  }

  void end_pass(searn& srn) {
    vw* all = srn.priv->all;
    srn.priv->hit_new_pass = true;
    srn.priv->read_example_last_pass++;
    srn.priv->passes_since_new_policy++;

    if (srn.priv->passes_since_new_policy >= srn.priv->passes_per_policy) {
      srn.priv->passes_since_new_policy = 0;
      if(all->training)
        srn.priv->current_policy++;
      if (srn.priv->current_policy > srn.priv->total_number_of_policies) {
        std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
        srn.priv->current_policy = srn.priv->total_number_of_policies;
      }
      //reset searn_trained_nb_policies in options_from_file so it is saved to regressor file later
      std::stringstream ss;
      ss << srn.priv->current_policy;
      VW::cmd_string_replace_value(all->file_options,"--search_trained_nb_policies", ss.str());
    }
  }

  void finish_example(vw& all, searn& srn, example& ec) {
    if (ec.end_pass || example_is_newline(ec) || srn.priv->ec_seq.size() >= all.p->ring_size - 2) {
      print_update(all, srn);
      VW::finish_example(all, &ec);
    }
  }

  void end_examples(searn& srn) {
    vw* all    = srn.priv->all;

    do_actual_learning<true>(*all, srn);

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << ((srn.priv->passes_since_new_policy == 0) ? srn.priv->current_policy : (srn.priv->current_policy+1));
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --search_trained_nb_policies
      VW::cmd_string_replace_value(all->file_options,"--search_trained_nb_policies", ss1.str());
      ss2 << srn.priv->total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --search_total_nb_policies
      VW::cmd_string_replace_value(all->file_options,"--search_total_nb_policies", ss2.str());
    }
  }

  bool searn_should_generate_output(searn_private* priv) { return priv->should_produce_string; }
  stringstream& searn_output_streamstream(searn_private* priv) {
    if (! priv->should_produce_string)
      return *(priv->bad_string_stream);
    else if (priv->state == GET_TRUTH_STRING)
      return *(priv->truth_string);
    else
      return *(priv->pred_string);
  }

  void  searn_set_options(searn_private* priv, uint32_t opts) {
    if (priv->state != NONE) {
      cerr << "error: task cannot set options except in initialize function!" << endl;
      throw exception();
    }
    if ((opts & AUTO_HISTORY)         != 0) priv->auto_history = true;
    if ((opts & AUTO_HAMMING_LOSS)    != 0) priv->auto_hamming_loss = true;
    if ((opts & EXAMPLES_DONT_CHANGE) != 0) priv->examples_dont_change = true;
    if ((opts & IS_LDF)               != 0) priv->is_ldf = true;
  }

  bool snapshot_item_ptr_eq0(void*ss_data, snapshot_item_ptr &a, snapshot_item_ptr &b) {
    // TODO: what to do if not used for prediction?
    if ((a.start == b.start) && (a.end == b.end)) return true;
    v_array<snapshot_item> *ss = (v_array<snapshot_item>*)ss_data;
    if (a.hash_value != b.hash_value) return false;
    if (a.end - a.start != b.end - b.start) return false;
    if (a.end >= ss->size()) return false;
    if (b.end >= ss->size()) return false;
    for (size_t i=0; i<=a.end-a.start; i++) {
      size_t a_ds = ss->get(a.start + i).data_size;
      if (a_ds != ss->get(b.start + i).data_size) return false;
      if (memcmp(ss->get(a.start + i).data_ptr, ss->get(b.start + i).data_ptr, a_ds) != 0) return false;
    }
    return true;
  }

  void print_snapshot_item(void*ss_data, snapshot_item_ptr& a) {
    v_array<snapshot_item> *ss = (v_array<snapshot_item>*)ss_data;
    cerr << "{ st=" << a.start << ", en=" << a.end << ", h=" << a.hash_value << ", d = [";
    for (size_t i=0; i<=a.end-a.start; i++) {
      snapshot_item me = ss->get(a.start+i);
      cerr << " (" << i << "): idx=" << me.index << " tag=" << me.tag << " dat=" << (*(size_t*)me.data_ptr);
    }
    cerr << " ]";
  }
    
  bool snapshot_item_ptr_eq(void*ss_data, snapshot_item_ptr &a, snapshot_item_ptr &b) {
    bool r = snapshot_item_ptr_eq0(ss_data, a, b);
    // cerr << "snapshot_item_ptr_eq:" << endl << "\t";
    // print_snapshot_item(ss_data, a);
    // cerr << endl << " vs\t";
    // print_snapshot_item(ss_data, b);
    // cerr << endl << " = " << r << endl;
    return r;
  }

  void searn_initialize(vw& all, searn& srn)
  {
    srn.priv->examples_dont_change = false;

    srn.priv->beta = 0.5;
    srn.priv->alpha = 1e-10f;
    srn.priv->allow_current_policy = false;
    srn.priv->rollout_method = 0;
    srn.priv->trajectory_oracle = false;
    srn.priv->adaptive_beta = false;
    srn.priv->num_features = 0;
    srn.priv->current_policy = 0;
    srn.priv->state = NONE;
    srn.priv->do_snapshot = true;
    srn.priv->do_fastforward = true;
    srn.priv->rollout_all_actions = true;
    //srn.priv->exploration_temperature = -1.0; // don't explore
    srn.priv->beam_size = 0; // 0 ==> no beam
    srn.priv->kbest = 0; // 0 or 1 means just 1 best
    srn.priv->allow_unsafe_fast_forward = true;

    srn.priv->neighbor_features_string = new string();

    srn.priv->passes_per_policy = 1;     //this should be set to the same value as --passes for dagger

    srn.task = NULL;
    srn.task_data = NULL;

    srn.priv->read_example_last_id = 0;
    srn.priv->passes_since_new_policy = 0;
    srn.priv->read_example_last_pass = 0;
    srn.priv->total_examples_generated = 0;
    srn.priv->total_predictions_made = 0;
    srn.priv->hit_new_pass = false;
    srn.priv->subsample_timesteps = 0.;

    srn.priv->total_number_of_policies = 1;

    srn.priv->truth_string = new stringstream();
    srn.priv->pred_string  = new stringstream();
    srn.priv->bad_string_stream = new stringstream();
    srn.priv->bad_string_stream->clear(srn.priv->bad_string_stream->badbit);

    srn.priv->should_produce_string = false;

    srn.priv->printed_output_header = false;

    srn.priv->auto_history = false;
    srn.priv->auto_hamming_loss = false;
    srn.priv->examples_dont_change = false;
    srn.priv->is_ldf = false;

    snapshot_item_result def_snapshot_result = { 0, -1.f };
    srn.priv->snapshot_map = new snapmap(102341, def_snapshot_result, snapshot_item_ptr_eq, &srn.priv->snapshot_data);

    srn.priv->empty_example = alloc_examples(sizeof(COST_SENSITIVE::label), 1);
    COST_SENSITIVE::cs_label.default_label(srn.priv->empty_example->ld);
    srn.priv->empty_example->in_use = true;
  }

  void searn_finish(searn& srn)
  {
    vw* all = srn.priv->all;
    cdbg << "searn_finish" << endl;

    delete srn.priv->truth_string;
    delete srn.priv->pred_string;
    delete srn.priv->bad_string_stream;
    delete srn.priv->neighbor_features_string;
    srn.priv->neighbor_features.erase();
    srn.priv->neighbor_features.delete_v();

    if (srn.priv->rollout_all_actions) { // dst should be a COST_SENSITIVE::label*
      ((COST_SENSITIVE::label*)srn.priv->valid_labels)->costs.erase();
      ((COST_SENSITIVE::label*)srn.priv->valid_labels)->costs.delete_v();
    } else {
      ((CB::label*)srn.priv->valid_labels)->costs.erase();
      ((CB::label*)srn.priv->valid_labels)->costs.delete_v();
    }

    if (srn.priv->rollout_all_actions) // labels are COST_SENSITIVE
      delete (COST_SENSITIVE::label*)srn.priv->valid_labels;
    else // labels are CB
      delete (CB::label*)srn.priv->valid_labels;

    dealloc_example(COST_SENSITIVE::cs_label.delete_label, *(srn.priv->empty_example));
    free(srn.priv->empty_example);

    srn.priv->ec_seq.clear();

    clear_snapshot(*all, srn, true);
    srn.priv->snapshot_data.delete_v();

    for (size_t i=0; i<srn.priv->train_labels.size(); i++) {
      if (srn.priv->rollout_all_actions) {
        ((COST_SENSITIVE::label*)srn.priv->train_labels[i])->costs.erase();
        ((COST_SENSITIVE::label*)srn.priv->train_labels[i])->costs.delete_v();
        delete ((COST_SENSITIVE::label*)srn.priv->train_labels[i]);
      } else {
        ((CB::label*)srn.priv->train_labels[i])->costs.erase();
        ((CB::label*)srn.priv->train_labels[i])->costs.delete_v();
        delete ((CB::label*)srn.priv->train_labels[i]);
      }
    }

    // destroy copied examples if we needed them
    if (! srn.priv->examples_dont_change) {
      void (*delete_label)(void*) = srn.priv->is_ldf ? COST_SENSITIVE::cs_label.delete_label : MULTICLASS::mc_label.delete_label;

      for (size_t n=0; n<MAX_BRANCHING_FACTOR; n++)
        dealloc_example(delete_label, srn.priv->learn_example_copy[n]);
    }


    if (srn.task->finish != NULL) {
      srn.task->finish(srn);
    }

    srn.priv->train_labels.delete_v();
    srn.priv->train_action.delete_v();
    srn.priv->train_action_ids.delete_v();
    srn.priv->rollout_action.delete_v();
    srn.priv->learn_losses.delete_v();

    srn.priv->beam_restore_to_end.delete_v();
    srn.priv->beam_final_action_sequence.delete_v();

    delete srn.priv->snapshot_map;
    delete srn.priv;
  }

  void ensure_param(float &v, float lo, float hi, float def, const char* string) {
    if ((v < lo) || (v > hi)) {
      cerr << string << endl;
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

  void handle_history_options(vw& vw, history_info &hinfo, po::variables_map& vm) {
    po::options_description history_options("history options");
    history_options.add_options()
      ("search_history",  po::value<size_t>(), "length of history to use")
      ("search_features", po::value<size_t>(), "length of history to pair with observed features")
      ("search_bigrams",                       "use bigrams from history")
      ("search_bigram_features",               "use bigrams from history paired with observed features");

    vm = add_options(vw, history_options);

    check_option<size_t>(hinfo.length, vw, vm, "search_history", false, size_equal,
                         "warning: you specified a different value for --search_history than the one loaded from regressor. proceeding with loaded value: ", "");

    check_option<size_t>(hinfo.features, vw, vm, "search_features", false, size_equal,
                         "warning: you specified a different value for --search_features than the one loaded from regressor. proceeding with loaded value: ", "");

    check_option        (hinfo.bigrams, vw, vm, "search_bigrams", false,
                         "warning: you specified --search_bigrams but that wasn't loaded from regressor. proceeding with loaded value: ");

    check_option        (hinfo.bigram_features, vw, vm, "search_bigram_features", false,
                         "warning: you specified --search_bigram_features but that wasn't loaded from regressor. proceeding with loaded value: ");
  }

  v_array<COST_SENSITIVE::label> read_allowed_transitions(uint32_t A, const char* filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
      cerr << "error: could not read file " << filename << " (" << strerror(errno) << "); assuming all transitions are valid" << endl;
      throw exception();
    }

    bool* bg = (bool*)malloc((A+1)*(A+1) * sizeof(bool));
    int rd,from,to,count=0;
    while ((rd = fscanf(f, "%d:%d", &from, &to)) > 0) {
      if ((from < 0) || (from > (int)A)) { cerr << "warning: ignoring transition from " << from << " because it's out of the range [0," << A << "]" << endl; }
      if ((to   < 0) || (to   > (int)A)) { cerr << "warning: ignoring transition to "   << to   << " because it's out of the range [0," << A << "]" << endl; }
      bg[from * (A+1) + to] = true;
      count++;
    }
    fclose(f);

    v_array<COST_SENSITIVE::label> allowed;

    for (size_t from=0; from<A; from++) {
      v_array<COST_SENSITIVE::wclass> costs;

      for (size_t to=0; to<A; to++)
        if (bg[from * (A+1) + to]) {
          COST_SENSITIVE::wclass c = { FLT_MAX, (uint32_t)to, 0., 0. };
          costs.push_back(c);
        }

      COST_SENSITIVE::label ld = { costs, 0 };
      allowed.push_back(ld);
    }
    free(bg);

    cerr << "read " << count << " allowed transitions from " << filename << endl;

    return allowed;
  }


  void parse_neighbor_features(searn&srn) {
    srn.priv->neighbor_features.erase();
    size_t len = srn.priv->neighbor_features_string->length();
    if (len == 0) return;

    char * cstr = new char [len+1];
    strcpy(cstr, srn.priv->neighbor_features_string->c_str());

    char * p = strtok(cstr, ",");
    v_array<substring> cmd;
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
        cerr << "warning: ignoring malformed neighbor specification: '" << p << "'" << endl;
      }
      int32_t enc = (posn << 24) | (ns & 0xFF);
      srn.priv->neighbor_features.push_back(enc);

      p = strtok(NULL, ",");
    }
    cmd.erase();
    cmd.delete_v();

    delete cstr;
  }

  learner* setup(vw&all, po::variables_map& vm)
  {
    searn* srn = (searn*)calloc_or_die(1,sizeof(searn));
    srn->priv = new searn_private();
    srn->priv->all = &all;
    srn->all = &all;

    searn_initialize(all, *srn);

    po::options_description searn_opts("Searn options");
    searn_opts.add_options()
        ("search_task",              po::value<string>(), "the search task")
        ("search_interpolation",     po::value<string>(), "at what level should interpolation happen? [*data|policy]")
        ("search_rollout",           po::value<string>(), "how should rollouts be executed?           [*policy|oracle|none]")
        ("search_trajectory",        po::value<string>(), "how should past trajectories be generated? [*policy|oracle]")

        ("search_passes_per_policy", po::value<size_t>(), "number of passes per policy (only valid for search_interpolation=policy)     [def=1]")
        ("search_beta",              po::value<float>(),  "interpolation rate for policies (only valid for search_interpolation=policy) [def=0.5]")

        ("search_alpha",             po::value<float>(),  "annealed beta = 1-(1-alpha)^t (only valid for search_interpolation=data)     [def=1e-10]")

        ("search_total_nb_policies", po::value<size_t>(), "if we are going to train the policies through multiple separate calls to vw, we need to specify this parameter and tell vw how many policies are eventually going to be trained")

        ("search_trained_nb_policies", po::value<size_t>(), "the number of trained policies in a file")

        ("search_allowed_transitions",po::value<string>(),"read file of allowed transitions [def: all transitions are allowed]")
        ("search_subsample_time",    po::value<float>(),  "instead of training at all timesteps, use a subset. if value in (0,1), train on a random v%. if v>=1, train on precisely v steps per example")
        ("search_neighbor_features", po::value<string>(), "copy features from neighboring lines. argument looks like: '-1:a,+2' meaning copy previous line namespace a and next next line from namespace _unnamed_, where ',' separates them")
        ("search_beam", po::value<size_t>(), "size of beam -- currently only usable in test mode, not for learning")
        ("search_kbest", po::value<size_t>(), "return kbest lists -- currently only usable in test mode, requires beam >= kbest size")

        ("search_no_unsafe_fastforward",                  "turn off efficiency gains from (very slightly) unsafe fastforwarding")
        ("search_no_snapshot",                            "turn off snapshotting capabilities")
        ("search_no_fastforward",                         "turn off fastforwarding (note: fastforwarding requires snapshotting)");

    vm = add_options(all, searn_opts);

    std::string task_string;
    std::string interpolation_string = "data";
    std::string rollout_string = "policy";
    std::string trajectory_string = "policy";

    check_option<string>(task_string, all, vm, "search_task", false, string_equal,
                         "warning: specified --search_task different than the one loaded from regressor. using loaded value of: ",
                         "error: you must specify a task using --search_task");
    check_option<string>(interpolation_string, all, vm, "search_interpolation", false, string_equal,
                         "warning: specified --search_interpolation different than the one loaded from regressor. using loaded value of: ", "");
    check_option<string>(rollout_string, all, vm, "search_rollout", false, string_equal,
                         "warning: specified --search_rollout different than the one loaded from regressor. using loaded value of: ", "");
    check_option<string>(trajectory_string, all, vm, "search_trajectory", false, string_equal,
                         "warning: specified --search_trajectory different than the one loaded from regressor. using loaded value of: ", "");

    if (vm.count("search_passes_per_policy"))       srn->priv->passes_per_policy    = vm["search_passes_per_policy"].as<size_t>();
    if (vm.count("search_beta"))                    srn->priv->beta                 = vm["search_beta"             ].as<float>();

    if (vm.count("search_alpha"))                   srn->priv->alpha                = vm["search_alpha"            ].as<float>();

    if (vm.count("search_subsample_time"))          srn->priv->subsample_timesteps  = vm["search_subsample_time"].as<float>();

    check_option<string>(*srn->priv->neighbor_features_string, all, vm, "search_neighbor_features", false, string_equal,
                         "warning: you specified a different feature structure with --search_neighbor_features than the one loaded from predictor. using loaded value of: ", "");
    parse_neighbor_features(*srn);

    if (vm.count("search_beam"))                    srn->priv->beam_size            = vm["search_beam"].as<size_t>();
    if (vm.count("search_kbest"))                   srn->priv->kbest                = vm["search_kbest"].as<size_t>();

    if (vm.count("search_no_snapshot"))             srn->priv->do_snapshot          = false;
    if (vm.count("search_no_fastforward"))          srn->priv->do_fastforward       = false;
    if (vm.count("search_no_unsafe_fastforward"))   srn->priv->allow_unsafe_fast_forward = false;

    if (interpolation_string.compare("data") == 0) { // run as dagger
      srn->priv->adaptive_beta = true;
      srn->priv->allow_current_policy = true;
      srn->priv->passes_per_policy = all.numpasses;
      if (srn->priv->current_policy > 1) srn->priv->current_policy = 1;
    } else if (interpolation_string.compare("policy") == 0) {
    } else {
      cerr << "error: --search_interpolation must be 'data' or 'policy'" << endl;
      throw exception();
    }

    if (rollout_string.compare("policy") == 0) {
      srn->priv->rollout_method = 0;
    } else if (rollout_string.compare("oracle") == 0) {
      srn->priv->rollout_method = 1;
    } else if (rollout_string.compare("none") == 0) {
      srn->priv->rollout_method = 2;
    } else {
      cerr << "error: --search_rollout must be 'policy', 'oracle' or 'none'" << endl;
      throw exception();
    }

    if (trajectory_string.compare("policy") == 0) {
      srn->priv->trajectory_oracle = false;
    } else if (trajectory_string.compare("oracle") == 0) {
      srn->priv->trajectory_oracle = true;
    } else {
      cerr << "error: --search_trajectory must be 'policy' or 'oracle'" << endl;
      throw exception();
    }

    //check_option<float >(srn->priv->exploration_temperature, all, vm, "search_exploration_temperature", false, float_equal,
    //                     "warning: you specified a different value through --search_exploration_temperature than the one loaded from predictor. using loaded value of: ", "");
    check_option<size_t>(srn->priv->A, all, vm, "search", false, size_equal,
                         "warning: you specified a different number of actions through --search than the one loaded from predictor. using loaded value of: ", "");

    //if (vm.count("search_allow_current_policy"))    srn->priv->allow_current_policy = true;
    //if (vm.count("search_rollout_oracle"))          srn->priv->rollout_oracle       = true;

    if (srn->priv->beam_size == 1)
      cerr << "warning: setting searn_beam=1 is kind of a weird thing to do -- just don't use a beam at all" << endl;
    // if ((srn->priv->beam_size > 0) && all.training) {
    //   cerr << "error: cannot currently train with beam" << endl;
    //   throw exception();
    // }
    if ((srn->priv->beam_size > 0) && ((!srn->priv->do_snapshot) || (!srn->priv->do_fastforward))) {
      cerr << "error: beam>0 requires snapshotting and fastforwarding" << endl;
      throw exception();
    }
    if ((srn->priv->kbest > 1) && (srn->priv->kbest < srn->priv->beam_size)) {
      cerr << "error: kbest must be at least equal to beam_size" << endl;
      throw exception();
    }

    //check if the base learner is contextual bandit, in which case, we dont rollout all actions.
    if (vm.count("cb")) {
      srn->priv->rollout_all_actions = false;
      srn->priv->valid_labels = new CB::label();
    } else {
      srn->priv->rollout_all_actions = true;
      srn->priv->valid_labels = new COST_SENSITIVE::label();
    }

    //if we loaded a regressor with -i option, --search_trained_nb_policies contains the number of trained policies in the file
    // and --search_total_nb_policies contains the total number of policies in the file
    if (vm.count("search_total_nb_policies"))
      srn->priv->total_number_of_policies = (uint32_t)vm["search_total_nb_policies"].as<size_t>();

    ensure_param(srn->priv->beta , 0.0, 1.0, 0.5, "warning: search_beta must be in (0,1); resetting to 0.5");
    ensure_param(srn->priv->alpha, 0.0, 1.0, 1e-10f, "warning: search_alpha must be in (0,1); resetting to 1e-10");

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    uint32_t tmp_number_of_policies = srn->priv->current_policy;
    if( all.training )
      tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)srn->priv->passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls,
    //so only set total_number_of_policies to computed value if it is larger
    cdbg << "current_policy=" << srn->priv->current_policy << " tmp_number_of_policies=" << tmp_number_of_policies << " total_number_of_policies=" << srn->priv->total_number_of_policies << endl;
    if( tmp_number_of_policies > srn->priv->total_number_of_policies ) {
      srn->priv->total_number_of_policies = tmp_number_of_policies;
      if( srn->priv->current_policy > 0 ) //we loaded a file but total number of policies didn't match what is needed for training
        std::cerr << "warning: you're attempting to train more classifiers than was allocated initially. Likely to cause bad performance." << endl;
    }

    //current policy currently points to a new policy we would train
    //if we are not training and loaded a bunch of policies for testing, we need to subtract 1 from current policy
    //so that we only use those loaded when testing (as run_prediction is called with allow_current to true)
    if( !all.training && srn->priv->current_policy > 0 )
      srn->priv->current_policy--;

    std::stringstream ss1, ss2;
    ss1 << srn->priv->current_policy;           VW::cmd_string_replace_value(all.file_options,"--search_trained_nb_policies", ss1.str());
    ss2 << srn->priv->total_number_of_policies; VW::cmd_string_replace_value(all.file_options,"--search_total_nb_policies",   ss2.str());

    cdbg << "search current_policy = " << srn->priv->current_policy << " total_number_of_policies = " << srn->priv->total_number_of_policies << endl;

    srn->task = NULL;
    for (searn_task** mytask = all_tasks; *mytask != NULL; mytask++)
      if (task_string.compare((*mytask)->task_name) == 0) {
        srn->task = *mytask;
        break;
      }
    if (srn->task == NULL) {
      cerr << "fail: unknown task for --search_task: " << task_string << endl;
      throw exception();
    }
    all.p->emptylines_separate_examples = true;

    // default to OAA labels unless the task wants to override this!
    all.p->lp = MULTICLASS::mc_label;
    srn->task->initialize(*srn, srn->priv->A, vm);

    if (vm.count("search_allowed_transitions"))     read_allowed_transitions((uint32_t)srn->priv->A, vm["search_allowed_transitions"].as<string>().c_str());

    // set up auto-history if they want it
    if (srn->priv->auto_history) {
      default_info(&srn->priv->hinfo);

      handle_history_options(all, srn->priv->hinfo, vm);

      if (srn->priv->hinfo.length < srn->priv->hinfo.features)
        srn->priv->hinfo.length = srn->priv->hinfo.features;

      if (srn->priv->hinfo.length == 0)
        srn->priv->auto_history = false;
    } else {
      srn->priv->hinfo.length = 0;
      srn->priv->hinfo.features = 0;
      srn->priv->hinfo.bigrams = false;
      srn->priv->hinfo.bigram_features = false;
    }

    // set up copied examples if we need them
    if (! srn->priv->examples_dont_change) {
      size_t label_size = srn->priv->is_ldf ? sizeof(COST_SENSITIVE::label) : sizeof(MULTICLASS::mc_label);
      for (size_t n=0; n<MAX_BRANCHING_FACTOR; n++)
        srn->priv->learn_example_copy[n].ld = calloc_or_die(1, label_size);
    }

    if (!srn->priv->allow_current_policy) // if we're not dagger
      all.check_holdout_every_n_passes = srn->priv->passes_per_policy;

    all.searnstr = srn;

    srn->priv->start_clock_time = clock();

    learner* l = new learner(srn, all.l, srn->priv->total_number_of_policies);
    l->set_learn<searn, searn_predict_or_learn<true> >();
    l->set_predict<searn, searn_predict_or_learn<false> >();
    l->set_finish_example<searn,finish_example>();
    l->set_end_examples<searn,end_examples>();
    l->set_finish<searn,searn_finish>();
    l->set_end_pass<searn,end_pass>();

    return l;
  }


  // the interface:
  uint32_t searn::predictLDF(example* ecs, size_t ec_len, v_array<uint32_t>* ystar, v_array<uint32_t>* yallowed) // for LDF
  { return searn_predict(this->priv, ecs, ec_len, yallowed, ystar, false); }

  uint32_t searn::predictLDF(example* ecs, size_t ec_len, uint32_t one_ystar, v_array<uint32_t>* yallowed) // for LDF
  { if (one_ystar == (uint32_t)-1) // test example
      return searn_predict(this->priv, ecs, ec_len, yallowed, NULL, false);
    else
      return searn_predict(this->priv, ecs, ec_len, yallowed, (v_array<uint32_t>*)&one_ystar, true);
  }

  uint32_t searn::predict(example* ec, v_array<uint32_t>* ystar, v_array<uint32_t>* yallowed) // for not LDF
  { return searn_predict(this->priv, ec, 0, yallowed, ystar, false); }

  uint32_t searn::predict(example* ec, uint32_t one_ystar, v_array<uint32_t>* yallowed) // for not LDF
  { if (one_ystar == (uint32_t)-1) // test example
      return searn_predict(this->priv, ec, 0, yallowed, NULL, false);
    else
      return searn_predict(this->priv, ec, 0, yallowed, (v_array<uint32_t>*)&one_ystar, true);
  }

  void     searn::loss(float incr_loss, size_t predictions_since_last)
  { searn_declare_loss(this->priv, predictions_since_last, incr_loss); }

  void     searn::snapshot(size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction)
  { searn_snapshot(this->priv, index, tag, data_ptr, sizeof_data, used_for_prediction); }

  stringstream& searn::output() {
    return searn_output_streamstream(this->priv);
  }

  void  searn::set_options(uint32_t opts) {
    searn_set_options(this->priv, opts);
  }
}

// ./vw -k -c -d train.f2.gz --passes 1 --search_passes_per_policy 10 --search_task sequence --search 9 --search_as_dagger 1e-6 --holdout_off -f foo --search_neighbor_features -2,-1,1,2

// ./vw -k -c -d seqtest --passes 5 --search_as_dagger 1e-8 --holdout_off --search 7 --search_task sequencespan

/*
  NON-LDF TRAIN

  ./vw -k -c -d z2 --passes 50 --invariant --search_as_dagger 1e-8 --search_task sequence --search 5 --holdout_off -f z2.model

  NON-LDF NO BEAM

  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence --search 5 -i z2.model -p out

  NON-LDF BEAM 1

  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence --search 5 -i z2.model -p out --search_beam 1

  NON-LDF BEAM 100

  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence --search 5 -i z2.model -p out --search_beam 100 --search_kbest 100



  LDF TRAIN
  ./vw -k -c -d z2 --passes 50 --invariant --search_as_dagger 1e-8 --search_task sequence_demoldf --search 5 --holdout_off -f z2.model --audit --csoaa_ldf m

  LDF NO BEAM
  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence_demoldf --search 5 -i z2.model --audit --csoaa_ldf m -p out

  LDF BEAM 1

  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence_demoldf --search 5 -i z2.model --audit --csoaa_ldf m -p out --search_beam 1

  LDF BEAM 100

  ./vw -k -c -d z2 -t --search_as_dagger 1e-8 --search_task sequence_demoldf --search 5 -i z2.model --audit --csoaa_ldf m -p out --search_beam 100 --search_kbest 100


  === SPAN ===

  TRAIN

  ./vw -k -c -d seqtest --passes 50 --invariant --search_as_dagger 1e-8 --search_task sequencespan --search 7 --holdout_off -f seqtest.model

  NO BEAM

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out

  BEAM 1

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out --search_beam 1

  BEAM 100

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out --search_beam 100 --search_kbest 100


  === SPAN WITH BILOU ===

  TRAIN

  ./vw -k -c -d seqtest --passes 50 --invariant --search_as_dagger 1e-8 --search_task sequencespan --search 7 --holdout_off -f seqtest.model --search_bilou

  NO BEAM

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out --search_bilou

  BEAM 1

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out --search_bilou --search_beam 1

  BEAM 100

  ./vw -k -c -d seqtest -t --search_as_dagger 1e-8 --search_task sequencespan --search 7 -i seqtest.model -p out --search_beam 100 --search_kbest 100 --search_bilou


 */


/* TODO LIST:

 * write documentation
 * pull munge/unmunge out of structured_predict
 * allow optional functions in searn tasks
 * label constraints
 * hypothesis recombination
 * beam at train
 * coreference
 
time ./vw -k -c -d pos.gz --search_as_dagger 1e-8 --search_task sequence --search 45 --holdout_off

real	1m40.899s
user	1m41.810s
sys	0m0.256s

down to:

real	0m54.234s
user	0m55.195s
sys	0m0.208s

./vw -k -c -d pos.gz --search_as_dagger 1e-6 --search_task sequence --search 45 --holdout_off

./vw -k -c -d pos.gz --search_as_dagger 1e-8 --search_task sequence --search 45 --holdout_off -f m
./vw -d pos.gz -t --search_task sequence --search 45 -i m -p output


last fast commit 2b46732522fe912835cca0252af3bfdd4720f2c0
  time ./vw -k -c -d pos.gz --search_as_dagger 1e-8 --search_task sequence --search 45 --holdout_off
  4.764648   4.622070      32768    32768.000000   [27 5 14 17 5 4 17 ..] [27 12 14 17 5 4 17..]      144     0     0          783265          783206
  real	1m8.606s

next slow commit ab38c2fedee0ba7ca86bd4cf4d145a27d4a6f5d5
  time ./vw -k -c -d pos.gz --search_as_dagger 1e-8 --search_task sequence --search 45 --holdout_off
  4.761292   4.621216      32768    32768.000000   [27 5 14 17 5 4 17 ..] [27 12 14 17 5 4 17..]      144     0     0          783267          783206
  real	3m13.804s
*/
