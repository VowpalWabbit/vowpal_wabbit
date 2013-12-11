/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "searn.h"
#include "gd.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "csoaa.h"
#include "cb.h"
#include "v_hashmap.h"
#include "vw.h"
#include "rand48.h"

// task-specific includes
#include "searn_sequencetask.h"

namespace SearnUtil
{
  using namespace std;

  string   audit_feature_space("history");
  uint32_t history_constant    = 8290743;
  uint32_t example_number = 0;


  void default_info(history_info* hinfo)
  {
    hinfo->bigrams           = false;
    hinfo->features          = 0;
    hinfo->bigram_features   = false;
    hinfo->length            = 1;
  }

  void* calloc_or_die(size_t nmemb, size_t size)
  {
    if (nmemb == 0 || size == 0)
      return NULL;

    void* data = calloc(nmemb, size);
    if (data == NULL) {
      std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
      throw exception();
    }
    return data;
  }

  void free_it(void*ptr)
  {
    if (ptr != NULL)
      free(ptr);
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

  void add_history_to_example(vw&all, history_info &hinfo, example* ec, history h)
  {
    uint64_t v0, v1, v, max_string_length = 0;
    uint32_t wpp = all.wpp * all.reg.stride;
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
      v0 = (h[hinfo.length-t] * quadratic_constant + t) * history_constant;
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
        v1 = (v0 * cubic_constant + h[hinfo.length-t+1]) * history_constant;

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
            v0 = (h[hinfo.length-t] * quadratic_constant + t) * history_constant;
          
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
              v1 = (v0 * cubic_constant + h[hinfo.length-t+1]) * history_constant;

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

  // size_t predict_with_history(vw&vw, example*ec, v_array<uint32_t>* ystar, history_info& hinfo, size_t*history) {
  //   add_history_to_example(vw, hinfo, ec, history);
  //   size_t prediction = ((Searn::searn*)vw.searnstr)->predict(vw, &ec, 0, NULL, ystar);
  //   remove_history_from_example(vw, hinfo, ec);
  //   return prediction;
  // }

}


namespace Searn {
  const char INIT_TEST  = 0;
  const char INIT_TRAIN = 1;
  const char LEARN      = 2;

  const bool PRINT_DEBUG_INFO =0;
  const bool PRINT_UPDATE_EVERY_EXAMPLE =0;
  const bool PRINT_UPDATE_EVERY_PASS =0;
  const bool PRINT_CLOCK_TIME =0;

  inline bool isLDF(searn& srn) { return (srn.A == 0); }

  int choose_policy(searn& srn, bool allow_current, bool allow_optimal)
  {
    uint32_t seed = /* srn.read_example_last_id * 2147483 + */ (uint32_t)(srn.t * 2147483647);
    return SearnUtil::random_policy(seed, srn.beta, allow_current, srn.current_policy, allow_optimal, srn.rollout_all_actions);
  }

  //CSOAA::label get_all_labels(searn& srn, size_t num_ec, v_array<uint32_t> *yallowed)
  void get_all_labels(void*dst, searn& srn, size_t num_ec, v_array<uint32_t> *yallowed)
  {
    if (srn.rollout_all_actions) { // dst should be a CSOAA::label*
      CSOAA::label *ret = (CSOAA::label*)dst;
      ret->costs.erase();
      if (isLDF(srn)) {
        for (uint32_t i=0; i<num_ec; i++) {
          CSOAA::wclass cost = { FLT_MAX, i, 0., 0. };
          ret->costs.push_back(cost);
        }
      } else { // is not LDF
        if (yallowed == NULL) {
          for (uint32_t i=1; i<=srn.A; i++) {
            CSOAA::wclass cost = { FLT_MAX, i, 0., 0. };
            ret->costs.push_back(cost);
          }
        } else {
          for (size_t i=0; i<yallowed->size(); i++) {
            CSOAA::wclass cost = { FLT_MAX, (*yallowed)[i], 0., 0. };
            ret->costs.push_back(cost);
          }
        }
      }
    } else { // dst should be a CB::label*
      CB::label *ret = (CB::label*)dst;
      ret->costs.erase();
      if (isLDF(srn)) {
        for (uint32_t i=0; i<num_ec; i++) {
          CB::cb_class cost = { FLT_MAX, i, 0. };
          ret->costs.push_back(cost);
        }
      } else { // is not LDF
        if (yallowed == NULL) {
          for (uint32_t i=1; i<=srn.A; i++) {
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
    }
  }

  uint32_t single_prediction_LDF(vw& all, learner& base, example** ecs, size_t num_ec, size_t pol)
  {
    assert(pol > 0);
    searn *srn = (searn*)all.searnstr;

    // TODO: modify this to handle contextual bandid base learner with ldf
    float best_prediction = 0;
    uint32_t best_action = 0;
    for (uint32_t action=0; action<num_ec; action++) {
      base.learn(ecs[action], pol);
      srn->total_predictions_made++;
      srn->num_features += ecs[action]->num_features;
      srn->empty_example->in_use = true;
      base.learn(srn->empty_example);

      if ((action == 0) || 
          ecs[action]->partial_prediction < best_prediction) {
        best_prediction = ecs[action]->partial_prediction;
        best_action     = action;
      }
    }

    return best_action;
  }

  uint32_t single_prediction_notLDF(vw& all, searn& srn, learner& base, example* ec, void*valid_labels, uint32_t pol)
  {
    assert(pol >= 0);

    void* old_label = ec->ld;
    ec->ld = valid_labels;

    base.learn(ec, pol);
    srn.total_predictions_made++;
    srn.num_features += ec->num_features;
    uint32_t final_prediction = (uint32_t)ec->final_prediction;

    if ((srn.state == INIT_TEST) && (all.raw_prediction > 0) && (srn.rollout_all_actions)) {
      string outputString;
      stringstream outputStringStream(outputString);
      CSOAA::label *ld = (CSOAA::label*)ec->ld;
      for (CSOAA::wclass* c = ld->costs.begin; c != ld->costs.end; ++c) {
        if (c != ld->costs.begin) outputStringStream << ' ';
        outputStringStream << c->weight_index << ':' << c->partial_prediction;
      }
      all.print_text(all.raw_prediction, outputStringStream.str(), ec->tag);
    }
    
    ec->ld = old_label;

    return final_prediction;
  }

  template<class T> T choose_random(v_array<T> opts) {
    float r = frand48();
    assert(opts.size() > 0);
    return opts[(size_t)(((float)opts.size()) * r)];
  }

  uint32_t single_action(vw& all, searn& srn, learner& base, example** ecs, size_t num_ec, void*valid_labels, int pol, v_array<uint32_t> *ystar, bool ystar_is_uint32t) {
    //cerr << "pol=" << pol << " ystar.size()=" << ystar->size() << " ystar[0]=" << ((ystar->size() > 0) ? (*ystar)[0] : 0) << endl;
    if (pol == -1) { // optimal policy
      if ((ystar == NULL) || ((! ystar_is_uint32t) && (ystar->size() == 0))) { // TODO: choose according to current model!
        if (srn.rollout_all_actions)
          return choose_random<CSOAA::wclass>(((CSOAA::label*)valid_labels)->costs).weight_index;
        else
          return choose_random<CB::cb_class >(((CB::label   *)valid_labels)->costs).weight_index;
      } else if (ystar_is_uint32t)
        return *((uint32_t*)ystar);
      else
        return choose_random<uint32_t>(*ystar);
    } else {        // learned policy
      if (!isLDF(srn)) {  // single example
        if (srn.auto_history) add_history_to_example(all, srn.hinfo, *ecs, srn.rollout_action.begin+srn.t);
        size_t action = single_prediction_notLDF(all, srn, base, *ecs, valid_labels, pol);
        if (srn.auto_history) remove_history_from_example(all, srn.hinfo, *ecs);
        return (uint32_t)action;
      } else {
        // TODO: auto-history for LDF
        return single_prediction_LDF(all, base, ecs, num_ec, pol);
      }
    }
  }

  void clear_snapshot(vw& all, searn& srn)
  {
    for (size_t i=0; i<srn.snapshot_data.size(); i++)
      free(srn.snapshot_data[i].data_ptr);
    srn.snapshot_data.erase();
  }

  void* copy_labels(searn &srn, void* l) {
    if (srn.rollout_all_actions) {
      CSOAA::label *ret = new CSOAA::label();
      v_array<CSOAA::wclass> costs = ((CSOAA::label*)l)->costs;
      for (size_t i=0; i<costs.size(); i++) {
        CSOAA::wclass c = { costs[i].x, costs[i].weight_index, costs[i].partial_prediction, costs[i].wap_value };
        ret->costs.push_back(c);
      }
      return ret;
    } else {
      CB::label *ret = new CB::label();
      v_array<CB::cb_class> costs = ((CB::label*)l)->costs;
      for (size_t i=0; i<costs.size(); i++) {
        CB::cb_class c = { costs[i].x, costs[i].weight_index, costs[i].prob_action };
        ret->costs.push_back(c);
      }
      return ret;
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
  uint32_t searn_predict_without_loss(vw& all, learner& base, example** ecs, size_t num_ec, v_array<uint32_t> *yallowed, v_array<uint32_t> *ystar, bool ystar_is_uint32t)  // num_ec == 0 means normal example, >0 means ldf, yallowed==NULL means all allowed, ystar==NULL means don't know; ystar_is_uint32t means that the ystar ref is really just a uint32_t
  {
    searn* srn=(searn*)all.searnstr;

    // check ldf sanity
    if (!isLDF(*srn)) {
      assert(num_ec == 0); // searntask is trying to define an ldf example in a non-ldf problem
    } else { // is LDF
      assert(num_ec != 0); // searntask is trying to define a non-ldf example in an ldf problem" << endl;
      assert(yallowed == NULL); // searntask is trying to specify allowed actions in an ldf problem" << endl;
    }

    if (srn->state == INIT_TEST) {
      int pol = choose_policy(*srn, true, false);
      //cerr << "(" << pol << ")";
      get_all_labels(srn->valid_labels, *srn, num_ec, yallowed);
      uint32_t a = single_action(all, *srn, base, ecs, num_ec, srn->valid_labels, pol, ystar, ystar_is_uint32t);
      //uint32_t a_opt = single_action(all, *srn, ecs, num_ec, valid_labels, -1, ystar);
      //clog << "predict @" << srn->t << " pol=" << pol << " a=" << a << endl;
      if (srn->auto_history) srn->rollout_action.push_back(a);
      srn->t++;
      return a;
    }
    if (srn->state == INIT_TRAIN) {
      int pol = choose_policy(*srn, srn->allow_current_policy, true);
      get_all_labels(srn->valid_labels, *srn, num_ec, yallowed);
      uint32_t a = single_action(all, *srn, base, ecs, num_ec, srn->valid_labels, pol, ystar, ystar_is_uint32t);
      //uint32_t a_opt = single_action(all, *srn, ecs, num_ec, valid_labels, -1, ystar);
      //clog << "predict @" << srn->t << " pol=" << pol << " a=" << a << endl;
      //assert((srn->current_policy == 0) || (a == a_opt));
      //if (! ((srn->current_policy == 0) || (a == a_opt))) { cerr << "FAIL!!!"<<endl;}
      srn->train_action.push_back(a);
      srn->train_labels.push_back(copy_labels(*srn, srn->valid_labels));
      if (srn->auto_history) srn->rollout_action.push_back(a);
      srn->t++;
      return a;
    }
    if (srn->state == LEARN) {
      if (srn->t < srn->learn_t) {
        assert(srn->t < srn->train_action.size());
        srn->t++;
        size_t a = srn->train_action[srn->t - 1];
        return (uint32_t)a;
      } else if (srn->t == srn->learn_t) {
        if (srn->learn_example_copy == NULL) {
          size_t num_to_copy = (num_ec == 0) ? 1 : num_ec;
          srn->learn_example_len = num_to_copy;
          // TODO: move the calloc outside
          srn->learn_example_copy = (example**)SearnUtil::calloc_or_die(num_to_copy, sizeof(example*));
          for (size_t n=0; n<num_to_copy; n++) {
            if (srn->examples_dont_change)
              srn->learn_example_copy[n] = ecs[n];
            else {
              srn->learn_example_copy[n] = alloc_example(sizeof(OAA::mc_label));
              VW::copy_example_data(all.audit, srn->learn_example_copy[n], ecs[n], sizeof(OAA::mc_label), NULL);
            }
          }
          //cerr << "copying example to " << srn->learn_example_copy << endl;
        }
        srn->snapshot_is_equivalent_to_t = (size_t)-1;
        srn->snapshot_could_match = true;
        srn->t++;
        if (srn->auto_history) srn->rollout_action.push_back(srn->learn_a);
        return srn->learn_a;
      } else { // t > learn_t
        size_t this_a = 0;
        if (srn->rollout_oracle) {
          get_all_labels(srn->valid_labels, *srn, num_ec, yallowed);
          this_a = single_action(all, *srn, base, ecs, num_ec, srn->valid_labels, -1, ystar, ystar_is_uint32t);
          srn->t++;
          //valid_labels.costs.erase(); valid_labels.costs.delete_v();
        } else if ((!srn->do_fastforward) || (!srn->snapshot_could_match) || (srn->snapshot_is_equivalent_to_t == ((size_t)-1))) { // we haven't converged, continue predicting
          int pol = choose_policy(*srn, srn->allow_current_policy, true);
          get_all_labels(srn->valid_labels, *srn, num_ec, yallowed);
          this_a = single_action(all, *srn, base, ecs, num_ec, srn->valid_labels, pol, ystar, ystar_is_uint32t);
          //clog << "predict @" << srn->t << " pol=" << pol << " a=" << a << endl;
          srn->t++;
          //valid_labels.costs.erase(); valid_labels.costs.delete_v();

          srn->snapshot_could_match = true;
          srn->snapshot_is_equivalent_to_t = (size_t)-1;
        } else {    // we can keep predicting using training trajectory
          srn->snapshot_is_equivalent_to_t++;
          srn->t = srn->snapshot_is_equivalent_to_t;
          //clog << "restoring previous prediction @ " << (srn->t-1) << " = " << srn->train_action[srn->t-1] << endl;
          this_a = srn->train_action[srn->t - 1];
        }
        if (srn->auto_history) srn->rollout_action.push_back((uint32_t)this_a);
        return (uint32_t)this_a;
      }
      assert(false);
    }
    cerr << "fail: searn got into ill-defined state (" << (int)srn->state << ")" << endl;
    throw exception();
  }

  void searn_declare_loss(vw& all, size_t predictions_since_last, float incr_loss)
  {
    searn* srn=(searn*)all.searnstr;
    if (srn->t != srn->loss_last_step + predictions_since_last) {
      cerr << "fail: searntask hasn't counted its predictions correctly.  current time step=" << srn->t << ", last declaration at " << srn->loss_last_step << ", declared # of predictions since then is " << predictions_since_last << endl;
      throw exception();
    }
    srn->loss_last_step = srn->t;
    //clog<<"new loss_last_step="<<srn->t<<endl;
    if (srn->state == INIT_TEST)
      srn->test_loss += incr_loss;
    else if (srn->state == INIT_TRAIN)
      srn->train_loss += incr_loss;
    else
      srn->learn_loss += incr_loss;
  }

  template<class T> bool v_array_contains(v_array<T> &A, T x) {
    for (T* e = A.begin; e != A.end; ++e)
      if (*e == x)
        return true;
    return false;
  }


  uint32_t searn_predict(vw& all, learner& base, example** ecs, size_t num_ec, v_array<uint32_t> *yallowed, v_array<uint32_t> *ystar, bool ystar_is_uint32t)  // num_ec == 0 means normal example, >0 means ldf, yallowed==NULL means all allowed, ystar==NULL means don't know; ystar_is_uint32t means that the ystar ref is really just a uint32_t
  {
    searn* srn=(searn*)all.searnstr;
    uint32_t a = searn_predict_without_loss(all, base, ecs, num_ec, yallowed, ystar, ystar_is_uint32t);

    if (srn->auto_hamming_loss) {
      float this_loss = 0.;
      if (ystar) {
        if (ystar_is_uint32t &&   // single allowed ystar
            (*((uint32_t*)ystar) != (uint32_t)-1) && // not a test example
            (*((uint32_t*)ystar) != a))
          this_loss = 1.;
        if ((!ystar_is_uint32t) && // many allowed ystar
            (!v_array_contains(*ystar, a)))
          this_loss = 1.;
      }
      searn_declare_loss(all, 1, this_loss);
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

  

  void searn_snapshot(vw& all, size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction)
  {
    searn* srn=(searn*)all.searnstr;
    if (! srn->do_snapshot) return;
    assert(tag > 0);

    //if (srn->state == INIT_TRAIN) return;
    //if (srn->state == LEARN) return;
    
    //clog << "snapshot called with:   { index=" << index << ", tag=" << tag << ", data_ptr=" << *(size_t*)data_ptr << ", t=" << srn->t << ", u4p=" << used_for_prediction << " }" << endl;
    

    if (srn->state == INIT_TEST) return;
    if (srn->state == INIT_TRAIN) {  // training means "record snapshots"
      if ((srn->snapshot_data.size() > 0) &&
          ((srn->snapshot_data.last().index > index) ||
           ((srn->snapshot_data.last().index == index) && (srn->snapshot_data.last().tag > tag)))) 
        cerr << "warning: trying to snapshot in a non-monotonic order! ignoring this snapshot" << endl;
      else {
        // if we're doing auto-history and this is the first snapshot of a given index, we need to also snapshot the relevant piece of history
        if (srn->auto_history &&
            ((srn->snapshot_data.size() == 0) ||
             ((srn->snapshot_data.size() > 0) && (srn->snapshot_data.last().index < index)))) {
          size_t history_size = srn->hinfo.length * sizeof(uint32_t);
          void* history_data = malloc(history_size);  // TODO: free these!
          memcpy(history_data, srn->rollout_action.begin + srn->t, history_size);
          snapshot_item item = { index, 0, history_data, history_size, srn->t };
          srn->snapshot_data.push_back(item);
          //cerr << "ss t=" << srn->t << " 
        }

        void* new_data = malloc(sizeof_data);
        memcpy(new_data, data_ptr, sizeof_data);
        snapshot_item item = { index, tag, new_data, sizeof_data, srn->t };
        srn->snapshot_data.push_back(item);
      }
      return;
    }

    // ELSE, this is TEST mode
    if (srn->t <= srn->learn_t) {  // RESTORE up to certain point
      //cerr << "index=" << index << " tag=" << tag << endl;

      // otherwise, we're restoring snapshots -- we want to find the index of largest value that has .t<=learn_t
      size_t i;
      bool found;
      found = snapshot_binary_search_lt(srn->snapshot_data, srn->learn_t, tag, i, srn->snapshot_last_found_pos);
      // size_t i2;
      // bool found2;
      // found2 = snapshot_linear_search_lt(srn->snapshot_data, srn->learn_t, tag, i2);
      // assert(found == found2);
      // assert(i == i2);
      if (!found) return;  // can't do anything

      if (tag == 1) {
        // restore our own stuff
        srn->loss_last_step = srn->snapshot_data[i].pred_step;
        
        if (srn->auto_history) {
          assert((i > 0) && (srn->snapshot_data[i-1].pred_step == srn->learn_t) && (srn->snapshot_data[i-1].tag == 0));
          // restore i-1 as history
          snapshot_item item = srn->snapshot_data[i-1];
          assert(item.data_size == srn->hinfo.length * sizeof(uint32_t));
          if (srn->rollout_action.size() < srn->learn_t + srn->hinfo.length)
            srn->rollout_action.resize(srn->learn_t + srn->hinfo.length);
          //assert(srn->rollout_action.size() >= srn->learn_t + srn->hinfo.length);
          memcpy(srn->rollout_action.begin + srn->learn_t, item.data_ptr, item.data_size);
        }
      }
      
      // if (srn->auto_history && (i > 0) && (srn->snapshot_data[i-1].pred_step == srn->learn_t) && (srn->snapshot_data[i-1].tag == 0)) {
      //   // restore i-1 as history
      //   snapshot_item item = srn->snapshot_data[i-1];
      //   assert(item.data_size == srn->hinfo.length * sizeof(uint32_t));
      //   memcpy(srn->rollout_action.begin + srn->learn_t, item.data_ptr, item.data_size);
      // }


      
      srn->snapshot_last_found_pos = i;
      snapshot_item item = srn->snapshot_data[i];

      /*
      //cerr << "restoring snapshot @ " << item.pred_step << " (learn_t=" << srn->learn_t << ") with " << index << "." << tag << ", value=" << *(size_t*)item.data_ptr << endl;
      cerr << "would restore snapshot: { index=" << item.index << ", tag=" << item.tag << ", data_ptr=";
      if (item.tag == 1) { cerr << *(size_t*)item.data_ptr; }
      if (item.tag == 2) {
      size_t *tmp = (size_t*)item.data_ptr;
      cerr << tmp[0];
      }
      if (item.tag == 3) { cerr << *(float*)item.data_ptr; }
      cerr << ", pred_step=" << item.pred_step << ", moving from t=" << srn->t << " }" << endl;
      */
      assert(sizeof_data == item.data_size);

      memcpy(data_ptr, item.data_ptr, sizeof_data);
      srn->t = item.pred_step;
    } else if (srn->do_fastforward) { // can we FAST FORWARD to end???
      if (srn->rollout_oracle) return;
      if (! srn->snapshot_could_match) return; // already hosed
      if (! used_for_prediction) return; // we don't care if it matches or not
      size_t i;
      bool found;
      found = snapshot_binary_search_eq(srn->snapshot_data, index, tag, i, srn->snapshot_last_found_pos);

      // bool found2; size_t i2;
      // found2 = snapshot_linear_search_eq(srn->snapshot_data, index, tag, i2);
      // assert(found == found2);
      // assert(i == i2);
      
      if (!found) return; // can't do anything

      
      srn->snapshot_last_found_pos = i;
      //clog << "a" << index << "/" << tag << " ";
      snapshot_item item = srn->snapshot_data[i];
      bool matches = memcmp(item.data_ptr, data_ptr, sizeof_data) == 0;
      if (matches) {
        // TODO: make sure it's the right number of snapshots!!!
        srn->snapshot_is_equivalent_to_t = item.pred_step;

        if (srn->auto_history && (i>0) && (srn->snapshot_data[i-1].index == index) && (srn->snapshot_data[i-1].tag == 0)) {
          item = srn->snapshot_data[i-1];
          if (srn->rollout_action.size() >= srn->t + srn->hinfo.length) {
            matches = memcmp(item.data_ptr, srn->rollout_action.begin + srn->t, sizeof_data) == 0;
            if (!matches)
              srn->snapshot_could_match = false;
          }
        }
      } else {
        srn->snapshot_could_match = false;
      }
    }
  }

  inline bool cmp_size_t(const size_t a, const size_t b) { return a < b; }

  v_array<size_t> get_training_timesteps(vw& all, searn& srn)
  {
    v_array<size_t> timesteps;

    if (srn.subsample_timesteps <= 0) {
      for (size_t t=0; t<srn.T; t++)
        timesteps.push_back(t);
    } else if (srn.subsample_timesteps < 1) {
      for (size_t t=0; t<srn.T; t++)
        if (frand48() <= srn.subsample_timesteps)
          timesteps.push_back(t);

      if (timesteps.size() == 0) // ensure at least one
        timesteps.push_back((size_t)(frand48() * srn.T));
    } else {
      while ((timesteps.size() < (size_t)srn.subsample_timesteps) &&
             (timesteps.size() < srn.T)) {
        size_t t = (size_t)(frand48() * (float)srn.T);
        if (! v_array_contains(timesteps, t))
          timesteps.push_back(t);
      }
      std::sort(timesteps.begin, timesteps.end, cmp_size_t);
    }
      
    return timesteps;
  }

  size_t labelset_size(searn&srn,void*l) {
    if (srn.rollout_all_actions)
      return ((CSOAA::label*)l)->costs.size();
    else
      return ((CB::label*)l)->costs.size();
  }

  size_t labelset_weight_index(searn&srn, void*l, size_t i) {
    if (srn.rollout_all_actions)
      return ((CSOAA::label*)l)->costs[i].weight_index;
    else
      return ((CB::label*)l)->costs[i].weight_index;
  }

  bool should_print_update(vw& all, bool hit_new_pass=false)
  {
    //uncomment to print out final loss after all examples processed
    //commented for now so that outputs matches make test
    //if( parser_done(all.p)) return true;

    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS && hit_new_pass) return true;
    return (all.sd->weighted_examples > all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  bool might_print_update(vw& all)
  {
    // basically do should_print_update but check me and the next
    // example because of off-by-ones

    if (PRINT_UPDATE_EVERY_EXAMPLE) return true;
    if (PRINT_UPDATE_EVERY_PASS) return true;
    return (all.sd->weighted_examples + 1. >= all.sd->dump_interval) && !all.quiet && !all.bfgs;
  }

  void generate_training_example(vw& all, searn& srn, learner& base, example** ec, size_t len, void*labels, v_array<float> losses)
  {
    assert(labelset_size(srn, labels) == losses.size());
    float min_loss = FLT_MAX;
    for (size_t i=0; i<losses.size(); i++)
      if (losses[i] < min_loss) min_loss = losses[i];
    for (size_t i=0; i<losses.size(); i++)
      if (srn.rollout_all_actions)
        ((CSOAA::label*)labels)->costs[i].x = losses[i] - min_loss;
      else
        ((CB::label*)labels)->costs[i].x = losses[i] - min_loss;

    if (!isLDF(srn)) {
      void* old_label = ec[0]->ld;
      ec[0]->ld = labels;
      base.learn(ec[0], srn.current_policy);
      ec[0]->ld = old_label;
      srn.total_examples_generated++;
    } else { // isLDF
      //TODO
    }
  }

  void clear_rollout_actions(searn&srn) {
    srn.rollout_action.erase();
    for (size_t t=0; t<srn.hinfo.length; t++)
      srn.rollout_action.push_back(0);
  }


  void train_single_example(vw& all, searn& srn, example**ec, size_t len)
  {
    // do an initial test pass to compute output (and loss)
    // TODO: don't do this if we don't need it!
    //clog << "======================================== INIT TEST (" << srn.current_policy << "," << srn.read_example_last_pass << ") ========================================" << endl;

    srn.state = INIT_TEST;
    srn.t = 0;
    srn.T = 0;
    srn.loss_last_step = 0;
    srn.test_loss = 0.f;
    srn.train_loss = 0.f;
    srn.learn_loss = 0.f;
    srn.learn_example_copy = NULL;
    srn.learn_example_len  = 0;
    srn.num_features = 0;
    srn.train_action.erase();
    if (srn.auto_history) clear_rollout_actions(srn);

    srn.snapshot_is_equivalent_to_t = (size_t)-1;
    srn.snapshot_could_match = false;
    srn.snapshot_last_found_pos = (size_t)-1;

    if ((all.final_prediction_sink.size() > 0) ||   // if we have to produce output, we need to run this
        might_print_update(all) ||                  // if we have to print and update to stderr
        (!all.training) ||                          // if we're just testing
        (all.current_pass == 0) ||                  // we need error rates for progressive cost
        (all.holdout_set_off) ||                    // no holdout
        (ec[0]->test_only) ||                       // it's a holdout example
        (all.raw_prediction > 0)                    // we need raw predictions
        ) {

      srn.should_produce_string = might_print_update(all) || (all.final_prediction_sink.size() > 0);
      if (srn.should_produce_string) {  // TODO: don't do this all the time!!!
        srn.truth_string->str("");  // erase contents
        srn.pred_string->str("");
      }
      
      assert(srn.truth_string != NULL);
      srn.task->structured_predict(srn, ec, len,
                                   srn.should_produce_string ? srn.pred_string  : NULL,
                                   srn.should_produce_string ? srn.truth_string : NULL);

      for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; ++sink)
        all.print_text((int)*sink, srn.pred_string->str(), ec[0]->tag);

      if ((all.raw_prediction > 0) && (srn.rollout_all_actions)) {
        all.print_text(all.raw_prediction, "", ec[0]->tag);
      }
    }
    
    if ((srn.t > 0) && all.training) {
      if (srn.adaptive_beta)
        srn.beta = 1.f - powf(1.f - srn.alpha, (float)srn.total_examples_generated);

      // do a pass over the data allowing oracle and snapshotting
      //clog << "======================================== INIT TRAIN (" << srn.current_policy << "," << srn.read_example_last_pass << ") ========================================" << endl;
      srn.state = INIT_TRAIN;
      srn.train_action.erase();
      if (srn.auto_history)
        clear_rollout_actions(srn);
      srn.t = 0;
      srn.loss_last_step = 0;
      clear_snapshot(all, srn);

      srn.snapshot_is_equivalent_to_t = (size_t)-1;
      srn.snapshot_last_found_pos = (size_t)-1;
      srn.snapshot_could_match = false;
      srn.task->structured_predict(srn, ec, len, NULL, NULL);

      if (srn.t == 0) {
        clear_snapshot(all, srn);
        return;  // there was no data
      }

      srn.T = srn.t;

      // generate training examples on which to learn
      //clog << "======================================== LEARN (" << srn.current_policy << "," << srn.read_example_last_pass << ") ========================================" << endl;
      srn.state = LEARN;
      v_array<size_t> tset = get_training_timesteps(all, srn);
      for (size_t tid=0; tid<tset.size(); tid++) {
        size_t t = tset[tid];
        void *aset = srn.train_labels[t];
        srn.learn_t = t;
        srn.learn_losses.erase();

        for (size_t i=0; i<labelset_size(srn, aset); i++) {
          if (srn.auto_history) {
            // startup the rollout at the train actions
            clear_rollout_actions(srn);
            //srn.rollout_action.resize(srn.hinfo.length + srn.T);
            push_many(srn.rollout_action, srn.train_action.begin, t);
            //memcpy(srn.rollout_action.begin + srn.hinfo.length, srn.train_action.begin, srn.T * sizeof(uint32_t));
          }
          srn.snapshot_last_found_pos = (size_t)-1;

          size_t this_index = labelset_weight_index(srn, aset, i);
          if (this_index == srn.train_action[srn.learn_t])
            srn.learn_losses.push_back( srn.train_loss );
          else {
            srn.t = 0;
            srn.learn_a = (uint32_t)this_index;
            srn.loss_last_step = 0;
            srn.learn_loss = 0.f;

            //clog << "learn_t = " << srn.learn_t << " || learn_a = " << srn.learn_a << endl;
            srn.snapshot_is_equivalent_to_t = (size_t)-1;
            srn.snapshot_could_match = true;
            srn.task->structured_predict(srn, ec, len, NULL, NULL);

            srn.learn_losses.push_back( srn.learn_loss );
            //clog << "total loss: " << srn.learn_loss << endl;
          }
        }

        if (srn.learn_example_copy != NULL) {
          generate_training_example(all, srn, *srn.base_learner, srn.learn_example_copy, srn.learn_example_len, aset, srn.learn_losses);

          if (!srn.examples_dont_change)
            for (size_t n=0; n<srn.learn_example_len; n++) {
              dealloc_example(OAA::delete_label, *srn.learn_example_copy[n]);
              free(srn.learn_example_copy[n]);
            }
          free(srn.learn_example_copy);
          srn.learn_example_copy = NULL;
          srn.learn_example_len  = 0;
        } else {
          cerr << "warning: searn did not generate an example for a given time-step" << endl;
        }
      }
      tset.erase(); tset.delete_v();
    }
    
    clear_snapshot(all, srn);
    srn.train_action.erase();
    srn.train_action.delete_v();
    srn.rollout_action.erase();
    srn.rollout_action.delete_v();
    for (size_t i=0; i<srn.train_labels.size(); i++) {
      if (srn.rollout_all_actions) {
        ((CSOAA::label*)srn.train_labels[i])->costs.erase();
        ((CSOAA::label*)srn.train_labels[i])->costs.delete_v();
        delete ((CSOAA::label*)srn.train_labels[i]);
      } else {
        ((CB::label*)srn.train_labels[i])->costs.erase();
        ((CB::label*)srn.train_labels[i])->costs.delete_v();
        delete ((CB::label*)srn.train_labels[i]);
      }
    }
    srn.train_labels.erase();
    srn.train_labels.delete_v();
    
    //clog << "======================================== DONE (" << srn.current_policy << "," << srn.read_example_last_pass << ") ========================================" << endl;

  }


  void clear_seq(vw&all, searn& srn)
  {
    if (srn.ec_seq.size() > 0) 
      for (example** ecc=srn.ec_seq.begin; ecc!=srn.ec_seq.end; ecc++) {
	VW::finish_example(all, *ecc);
      }
    srn.ec_seq.erase();
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

void print_update(vw& all, searn* srn)
  {
    if (!srn->printed_output_header && !all.quiet) {
      const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";
      fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
      fprintf(stderr, header_fmt, "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
      cerr.precision(5);
      srn->printed_output_header = true;
    }

    if (!should_print_update(all, srn->hit_new_pass))
      return;

    char true_label[21];
    char pred_label[21];
    to_short_string(srn->truth_string->str(), 20, true_label);
    to_short_string(srn->pred_string->str() , 20, pred_label);

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
            (long unsigned int)srn->num_features,
            (int)srn->read_example_last_pass,
            (int)srn->current_policy,
            (long unsigned int)srn->total_predictions_made,
            (long unsigned int)srn->total_examples_generated);

    if (PRINT_CLOCK_TIME) {
      size_t num_sec = (size_t)(((float)(clock() - srn->start_clock_time)) / CLOCKS_PER_SEC);
      fprintf(stderr, " %15lusec", num_sec);
    }

    if (!all.holdout_set_off && all.current_pass >= 1)
      fprintf(stderr, " h");
    
    fprintf(stderr, "\n");

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    all.sd->dump_interval *= 2;
  }

  void add_neighbor_features(searn& srn) {
    size_t neighbor_constant = 8349204823;
    if (srn.neighbor_features.size() == 0) return;
    uint32_t wpp = srn.all->wpp * srn.all->reg.stride;

    for (int32_t n=0; n<(int32_t)srn.ec_seq.size(); n++) {
      example*me = srn.ec_seq[n];
      //cerr << "o=" << me->num_features << endl;
      for (int32_t*enc=srn.neighbor_features.begin; enc!=srn.neighbor_features.end; ++enc) {
        int32_t offset = (*enc) >> 24;
        size_t  old_ns = (*enc) & 0xFF;
        size_t  enc_offset = wpp * ((2 * (size_t)(*enc)) + ((*enc < 0) ? 1 : 0));

        // TODO: auditing :P
        if ((n + offset >= 0) && (n + offset < (int32_t)srn.ec_seq.size())) { // we're okay on position
          example*you = srn.ec_seq[n+offset];
          size_t  you_size = you->atomics[old_ns].size();

          if (you_size > 0) {
            if (me->atomics[neighbor_namespace].size() == 0) {
              me->indices.push_back(neighbor_namespace);
            }

            me->atomics[neighbor_namespace].resize(me->atomics[neighbor_namespace].size() + you_size + 1);
            for (feature*f = you->atomics[old_ns].begin; f != you->atomics[old_ns].end; ++f) {
              feature f2 = { (*f).x, (uint32_t)( ((*f).weight_index * neighbor_constant + enc_offset) & srn.all->reg.weight_mask ) };
              me->atomics[neighbor_namespace].push_back(f2);
            }
            //push_many(me->atomics[neighbor_namespace], you->atomics[old_ns].begin, you_size);
            //cerr << "copying " << you_size << " features" << endl;
            me->sum_feat_sq[neighbor_namespace] += you->sum_feat_sq[old_ns];
            me->total_sum_feat_sq += you->sum_feat_sq[old_ns];
            me->num_features += you_size;
          }
        } else {
          // TODO: add dummy features for <s> or </s>
        }
      }
    }
  }

  void del_neighbor_features(searn& srn) {
    if (srn.neighbor_features.size() == 0) return;

    for (int32_t n=0; n<(int32_t)srn.ec_seq.size(); n++) {
      example*me = srn.ec_seq[n];
      //cerr << "n=" << me->num_features;
      for (int32_t*enc=srn.neighbor_features.begin; enc!=srn.neighbor_features.end; ++enc) {
        int32_t offset = (*enc) >> 24;
        size_t  old_ns = (*enc) & 0xFF;

        if ((n + offset >= 0) && (n + offset < (int32_t)srn.ec_seq.size())) { // we're okay on position
          example*you = srn.ec_seq[n+offset];
          size_t  you_size = you->atomics[old_ns].size();

          if (you_size > 0) {
            if (me->atomics[neighbor_namespace].size() == you_size) {
              char last_idx = me->indices.pop();
              assert(last_idx == (char)neighbor_namespace);
              //cerr << "erasing new ns '" << (char)neighbor_namespace << "' of size " << me->atomics[neighbor_namespace].size() << endl;
              me->atomics[neighbor_namespace].erase();
            } else {
              me->atomics[neighbor_namespace].end -= you_size;
              //cerr << "erasing " << you_size << " features" << endl;
            }
            
            me->sum_feat_sq[neighbor_namespace] -= you->sum_feat_sq[old_ns];
            me->total_sum_feat_sq -= you->sum_feat_sq[old_ns];
            me->num_features -= you_size;
          }
        } else {
          // TODO: add dummy features for <s> or </s>
        }
      }
      //cerr << " " << me->num_features << endl;
    }
  }



  void do_actual_learning(vw&all, searn& srn)
  {
    if (srn.ec_seq.size() == 0)
      return;  // nothing to do :)

    add_neighbor_features(srn);
    train_single_example(all, srn, srn.ec_seq.begin, srn.ec_seq.size());
    del_neighbor_features(srn);

    if (srn.ec_seq[0]->test_only) {
      all.sd->weighted_holdout_examples += 1.f;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += 1.f;
      all.sd->weighted_holdout_examples_since_last_pass += 1.f;
      all.sd->holdout_sum_loss += srn.test_loss;
      all.sd->holdout_sum_loss_since_last_dump += srn.test_loss;
      all.sd->holdout_sum_loss_since_last_pass += srn.test_loss;//since last pass
    } else {
      all.sd->weighted_examples += 1.f;
      all.sd->total_features += srn.num_features;
      all.sd->sum_loss += srn.test_loss;
      all.sd->sum_loss_since_last_dump += srn.test_loss;
      all.sd->example_number++;
    }
  }

  void searn_learn(void*d, learner& base, example*ec) {
    searn *srn = (searn*)d;
    vw* all = srn->all;
    srn->base_learner = &base;
    bool is_real_example = true;
    if (example_is_newline(ec) || srn->ec_seq.size() >= all->p->ring_size - 2) { 
      if (srn->ec_seq.size() >= all->p->ring_size - 2) { // give some wiggle room
	std::cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << std::endl;
      }

      do_actual_learning(*all, *srn);
      clear_seq(*all, *srn);
      srn->hit_new_pass = false;
      
      //VW::finish_example(*all, ec);
      is_real_example = false;
    } else {
      srn->ec_seq.push_back(ec);
    }
    
    if (is_real_example) {
      srn->read_example_last_id = ec->example_counter;
    }
  }

  void end_pass(void* d) {
    searn *srn = (searn*)d;
    vw* all = srn->all;
    srn->hit_new_pass = true;
    srn->read_example_last_pass++;
    srn->passes_since_new_policy++;
    if (srn->passes_since_new_policy >= srn->passes_per_policy) {
      srn->passes_since_new_policy = 0;
      if(all->training)
        srn->current_policy++;
      if (srn->current_policy > srn->total_number_of_policies) {
        std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
        srn->current_policy = srn->total_number_of_policies;
      }
      //reset searn_trained_nb_policies in options_from_file so it is saved to regressor file later
      std::stringstream ss;
      ss << srn->current_policy;
      VW::cmd_string_replace_value(all->options_from_file,"--searn_trained_nb_policies", ss.str());
    }
  }

  void finish_example(vw& all, void* d, example* ec) {
    searn *srn = (searn*)d;

    if (ec->end_pass || example_is_newline(ec) || srn->ec_seq.size() >= all.p->ring_size - 2) { 
      print_update(all, srn);
      VW::finish_example(all, ec);
    }
  }

  void end_examples(void* d) {
    searn* srn = (searn*)d;
    vw* all    = srn->all;

    do_actual_learning(*all, *srn);

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << ((srn->passes_since_new_policy == 0) ? srn->current_policy : (srn->current_policy+1));
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_trained_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_trained_nb_policies", ss1.str()); 
      ss2 << srn->total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_total_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_total_nb_policies", ss2.str());
    }
  }

  void searn_initialize(vw& all, searn& srn)
  {
    srn.predict_f = searn_predict;
    srn.declare_loss_f = searn_declare_loss;
    srn.snapshot_f = searn_snapshot;

    srn.examples_dont_change = false;
    
    srn.beta = 0.5;
    srn.allow_current_policy = false;
    srn.rollout_oracle = false;
    srn.adaptive_beta = false;
    srn.alpha = 1e-6f;
    srn.num_features = 0;
    srn.current_policy = 0;
    srn.state = 0;
    srn.do_snapshot = true;
    srn.do_fastforward = true;
    srn.rollout_all_actions = true;

    srn.neighbor_features_string = new string();
    
    srn.passes_per_policy = 1;     //this should be set to the same value as --passes for dagger

    srn.task = NULL;
    srn.task_data = NULL;
    
    srn.read_example_last_id = 0;
    srn.passes_since_new_policy = 0;
    srn.read_example_last_pass = 0;
    srn.total_examples_generated = 0;
    srn.total_predictions_made = 0;
    srn.hit_new_pass = false;
    srn.subsample_timesteps = 0.;
    
    srn.total_number_of_policies = 1;

    srn.truth_string = new stringstream();
    srn.pred_string  = new stringstream();
    srn.should_produce_string = false;

    srn.printed_output_header = false;

    srn.auto_history = false;
    srn.auto_hamming_loss = false;
    srn.examples_dont_change = false;
    
    srn.empty_example = alloc_example(sizeof(OAA::mc_label));
    OAA::default_label(srn.empty_example->ld);
    srn.empty_example->in_use = true;
  }

  void searn_finish(void* d)
  {
    searn *srn = (searn*)d;
    vw* all = srn->all;
    //cerr << "searn_finish" << endl;

    delete srn->truth_string;
    delete srn->pred_string;
    delete srn->neighbor_features_string;
    srn->neighbor_features.erase();
    srn->neighbor_features.delete_v();
    
    if (srn->rollout_all_actions) { // dst should be a CSOAA::label*
      ((CSOAA::label*)srn->valid_labels)->costs.erase();
      ((CSOAA::label*)srn->valid_labels)->costs.delete_v();
    } else {
      ((CB::label*)srn->valid_labels)->costs.erase();
      ((CB::label*)srn->valid_labels)->costs.delete_v();
    }
    
    if (srn->rollout_all_actions) // labels are CSOAA
      delete (CSOAA::label*)srn->valid_labels;
    else // labels are CB
      delete (CB::label*)srn->valid_labels;

    dealloc_example(OAA::delete_label, *(srn->empty_example));
    free(srn->empty_example);
    
    srn->ec_seq.delete_v();

    clear_snapshot(*all, *srn);
    srn->snapshot_data.delete_v();

    for (size_t i=0; i<srn->train_labels.size(); i++) {
      if (srn->rollout_all_actions) {
        ((CSOAA::label*)srn->train_labels[i])->costs.erase();
        ((CSOAA::label*)srn->train_labels[i])->costs.delete_v();
        delete ((CSOAA::label*)srn->train_labels[i]);
      } else {
        ((CB::label*)srn->train_labels[i])->costs.erase();
        ((CB::label*)srn->train_labels[i])->costs.delete_v();
        delete ((CB::label*)srn->train_labels[i]);
      }
    }
    srn->train_labels.erase(); srn->train_labels.delete_v();
    srn->train_action.erase(); srn->train_action.delete_v();
    srn->rollout_action.erase(); srn->rollout_action.delete_v();
    srn->learn_losses.erase(); srn->learn_losses.delete_v();

    if (srn->task->finish != NULL) {
      srn->task->finish(*srn);
      free(srn->task);
    }
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

  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string) {
    if (vm_file.count(opt_name)) { // loaded from regressor file
      ret = vm_file[opt_name].as<T>();
      if (vm.count(opt_name) && !equal(ret, vm[opt_name].as<T>())) {
        if (default_to_cmdline)
          ret = vm[opt_name].as<T>();
        std::cerr << mismatch_error_string << ret << endl;
      }
    } else if (vm.count(opt_name)) {
      ret = vm[opt_name].as<T>();
      stringstream ss;
      ss << " --" << opt_name << " " << ret;
      all.options_from_file.append(ss.str());
    } else if (strlen(required_error_string)>0) {
      std::cerr << required_error_string << endl;
      throw exception();
    }
  }  

  void check_option(bool& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string) {
    if (vm_file.count(opt_name)) { // loaded from regressor file
      ret = true;
      if (!vm.count(opt_name)) {
        if (default_to_cmdline)
          ret = false;
        std::cerr << mismatch_error_string << ret << endl;
      }
    } else if (vm.count(opt_name)) {
      ret = true;
      stringstream ss;
      ss << " " << opt_name;
      all.options_from_file.append(ss.str());
    } else {
      ret = false;
    }
  }  

  void setup_searn_options(po::options_description& desc, vw&vw, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(vw.options_from_file_argc, vw.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);
  }


  void handle_history_options(vw& vw, history_info &hinfo, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    po::options_description desc("Searn[sequence] options");
    desc.add_options()
      ("searn_history",  po::value<size_t>(), "length of history to use")
      ("searn_features", po::value<size_t>(), "length of history to pair with observed features")
      ("searn_bigrams",                       "use bigrams from history")
      ("searn_bigram_features",               "use bigrams from history paired with observed features");

    setup_searn_options(desc, vw, opts, vm, vm_file);
    
    check_option<size_t>(hinfo.length, vw, vm, vm_file, "searn_history", false, size_equal,
                         "warning: you specified a different value for --searn_history than the one loaded from regressor. proceeding with loaded value: ", "");
    
    check_option<size_t>(hinfo.features, vw, vm, vm_file, "searn_features", false, size_equal,
                         "warning: you specified a different value for --searn_features than the one loaded from regressor. proceeding with loaded value: ", "");
    
    check_option        (hinfo.bigrams, vw, vm, vm_file, "searn_bigrams", false,
                         "warning: you specified --searn_bigrams but that wasn't loaded from regressor. proceeding with loaded value: ");
    
    check_option        (hinfo.bigram_features, vw, vm, vm_file, "searn_bigram_features", false,
                         "warning: you specified --searn_bigram_features but that wasn't loaded from regressor. proceeding with loaded value: ");
  }

  void parse_neighbor_features(searn&srn) {
    srn.neighbor_features.erase();
    size_t len = srn.neighbor_features_string->length();
    if (len == 0) return;

    char * cstr = new char [len+1];
    strcpy(cstr, srn.neighbor_features_string->c_str());

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
      srn.neighbor_features.push_back(enc);
      
      p = strtok(NULL, ",");
    }
    cmd.erase();
    cmd.delete_v();
    
    delete cstr;
  }

  learner* setup(vw&all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    searn* srn = (searn*)calloc(1,sizeof(searn));
    srn->all = &all;

    searn_initialize(all, *srn);

    po::options_description desc("Searn options");
    desc.add_options()
      ("searn_task", po::value<string>(), "the searn task")
      ("searn_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
      ("searn_beta", po::value<float>(), "interpolation rate for policies")
      ("searn_allow_current_policy", "allow searn labeling to use the current policy")
      ("searn_rollout_oracle", "allow searn/dagger to do rollouts with the oracle when estimating cost-to-go")
      ("searn_as_dagger", po::value<float>(), "sets options to make searn operate as dagger. parameter is the sliding autonomy rate (rate at which beta tends to 1).")
      ("searn_total_nb_policies", po::value<size_t>(), "if we are going to train the policies through multiple separate calls to vw, we need to specify this parameter and tell vw how many policies are eventually going to be trained")
      ("searn_no_snapshot", "turn off snapshotting capabilities")
      ("searn_no_fastforward", "turn off fastforwarding (note: fastforwarding requires snapshotting)")
      ("searn_subsample_timesteps", po::value<float>(), "instead of training at all timesteps, use a subset v. if v<=0, train everywhere. if v in (0,1), train on a random v% (>=1 always selected). if v>=1, train on precisely v steps per example")
      ("searn_neighbor_features", po::value<string>(), "copy features from neighboring lines. argument looks like: '-1:a,+2' meaning copy previous line namespace a and next next line from namespace _unnamed_, where ',' separates them");
    po::options_description add_desc_file("Searn options only available in regressor file");
    add_desc_file.add_options()("searn_trained_nb_policies", po::value<size_t>(), "the number of trained policies in the regressor file");

    po::options_description desc_file;
    desc_file.add(desc).add(add_desc_file);

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(all.options_from_file_argc, all.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc_file).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);
  
    std::string task_string;

    check_option<float >(srn->beta, all, vm, vm_file, "searn_beta", false, float_equal,
                         "warning: you specified a different value through --searn_beta than the one loaded from predictor. using loaded value of: ", "");
    check_option<string>(task_string, all, vm, vm_file, "searn_task", false, string_equal,
                         "warning: specified --searn_task different than the one loaded from regressor. using loaded value of: ",
                         "error: you must specify a task using --searn_task");
    check_option<size_t>(srn->A, all, vm, vm_file, "searn", false, size_equal,
                         "warning: you specified a different number of actions through --searn than the one loaded from predictor. using loaded value of: ", "");
    check_option<string>(*srn->neighbor_features_string, all, vm, vm_file, "searn_neighbor_features", false, string_equal,
                         "warning: you specified a different feature structure with --searn_neighbor_features than the one loaded from predictor. using loaded value of: ", "");

    parse_neighbor_features(*srn);
    
    if (vm.count("searn_subsample_timesteps"))     srn->subsample_timesteps  = vm["searn_subsample_timesteps"].as<float>();
    if (vm.count("searn_passes_per_policy"))       srn->passes_per_policy    = vm["searn_passes_per_policy"].as<size_t>();
    if (vm.count("searn_allow_current_policy"))    srn->allow_current_policy = true;
    if (vm.count("searn_rollout_oracle"))          srn->rollout_oracle       = true;
    if (vm.count("searn_no_snapshot"))             srn->do_snapshot          = false;
    if (vm.count("searn_no_fastforward"))          srn->do_fastforward       = false;
    if (vm.count("searn_as_dagger")) {
      srn->allow_current_policy = true;
      srn->passes_per_policy = all.numpasses;
      if (srn->current_policy > 1) srn->current_policy = 1;
      srn->adaptive_beta = true;
      srn->alpha = vm["searn_as_dagger"].as<float>();
    }

    //check if the base learner is contextual bandit, in which case, we dont rollout all actions.
    if (vm.count("cb") || vm_file.count("cb")) {
      srn->rollout_all_actions = false;
      srn->valid_labels = new CB::label();
    } else {
      srn->rollout_all_actions = true;
      srn->valid_labels = new CSOAA::label();
    }
    
    //if we loaded a regressor with -i option, --searn_trained_nb_policies contains the number of trained policies in the file
    // and --searn_total_nb_policies contains the total number of policies in the file
    if ( vm_file.count("searn_total_nb_policies") ) {
      srn->current_policy = (uint32_t)vm_file["searn_trained_nb_policies"].as<size_t>();
      srn->total_number_of_policies = (uint32_t)vm_file["searn_total_nb_policies"].as<size_t>();
      if (vm.count("searn_total_nb_policies") && (uint32_t)vm["searn_total_nb_policies"].as<size_t>() != srn->total_number_of_policies)
        std::cerr << "warning: --searn_total_nb_policies doesn't match the total number of policies stored in initial predictor. Using loaded value of: " << srn->total_number_of_policies << endl;
    } else if (vm.count("searn_total_nb_policies"))
      srn->total_number_of_policies = (uint32_t)vm["searn_total_nb_policies"].as<size_t>();

    ensure_param(srn->beta , 0.0, 1.0, 0.5, "warning: searn_beta must be in (0,1); resetting to 0.5");
    ensure_param(srn->alpha, 0.0, 1.0, 1e-6f, "warning: searn_as_dagger must be in (0,1); resetting to 0.001");

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    uint32_t tmp_number_of_policies = srn->current_policy; 
    if( all.training )
      tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)srn->passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls, 
    //so only set total_number_of_policies to computed value if it is larger
    //clog << "current_policy=" << srn->current_policy << " tmp_number_of_policies=" << tmp_number_of_policies << " total_number_of_policies=" << srn->total_number_of_policies << endl;
    if( tmp_number_of_policies > srn->total_number_of_policies ) {
      srn->total_number_of_policies = tmp_number_of_policies;
      if( srn->current_policy > 0 ) //we loaded a file but total number of policies didn't match what is needed for training
        std::cerr << "warning: you're attempting to train more classifiers than was allocated initially. Likely to cause bad performance." << endl;
    }

    //current policy currently points to a new policy we would train
    //if we are not training and loaded a bunch of policies for testing, we need to subtract 1 from current policy
    //so that we only use those loaded when testing (as run_prediction is called with allow_current to true)
    if( !all.training && srn->current_policy > 0 )
      srn->current_policy--;

    std::stringstream ss1, ss2;
    ss1 << srn->current_policy;           VW::cmd_string_replace_value(all.options_from_file,"--searn_trained_nb_policies", ss1.str()); 
    ss2 << srn->total_number_of_policies; VW::cmd_string_replace_value(all.options_from_file,"--searn_total_nb_policies",   ss2.str());

    //clog << "searn increment = " << srn->increment <<  " " << all.reg.stride << endl;
    //clog << "searn current_policy = " << srn->current_policy << " total_number_of_policies = " << srn->total_number_of_policies << endl;
    
    if (task_string.compare("sequence") == 0) {
      searn_task* mytask = (searn_task*)calloc(1, sizeof(searn_task));
      mytask->initialize = SequenceTask::initialize;
      mytask->finish = SequenceTask::finish;
      mytask->structured_predict = SequenceTask::structured_predict;
      all.p->emptylines_separate_examples = true;
      srn->task = mytask;
    } else if (task_string.compare("sequencespan") == 0) {
      searn_task* mytask = (searn_task*)calloc(1, sizeof(searn_task));
      mytask->initialize = SequenceSpanTask::initialize;
      mytask->finish = SequenceSpanTask::finish;
      mytask->structured_predict = SequenceSpanTask::structured_predict;
      all.p->emptylines_separate_examples = true;
      srn->task = mytask;
    } else {
      cerr << "fail: unknown task for --searn_task: " << task_string << endl;
      throw exception();
    }

    // default to OAA labels unless the task wants to override this!
    *(all.p->lp) = OAA::mc_label_parser; 
    srn->task->initialize(*srn, srn->A, opts, vm, vm_file);

    // set up auto-history if they want it
    if (srn->auto_history) {
      default_info(&srn->hinfo);

      handle_history_options(all, srn->hinfo, opts, vm, vm_file);
      
      if (srn->hinfo.length < srn->hinfo.features)
        srn->hinfo.length = srn->hinfo.features;
      
      if (srn->hinfo.length == 0)
        srn->auto_history = false;
    } else {
      srn->hinfo.length = 0;
      srn->hinfo.features = 0;
      srn->hinfo.bigrams = false;
      srn->hinfo.bigram_features = false;
    }
    
    if (!srn->allow_current_policy) // if we're not dagger
      all.check_holdout_every_n_passes = srn->passes_per_policy;

    all.searnstr = srn;

    srn->start_clock_time = clock();

    learner* l = new learner(srn, searn_learn, all.l, srn->total_number_of_policies);
    l->set_finish_example(finish_example);
    l->set_end_examples(end_examples);
    l->set_finish(searn_finish);
    l->set_end_pass(end_pass);
    
    return l;
  }
}

// ./vw -k -c -d train.f2.gz --passes 1 --searn_passes_per_policy 10 --searn_task sequence --searn 9 --searn_as_dagger 1e-6 --holdout_off -f foo --searn_neighbor_features -2,-1,1,2
