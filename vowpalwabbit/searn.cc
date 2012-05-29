#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "searn.h"
#include "gd.h"
#include "io.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "csoaa.h"
#include "v_hashmap.h"

// task-specific includes
#include "searn_sequencetask.h"

namespace SearnUtil
{
  using namespace std;

  string   audit_feature_space("history");
  uint32_t history_constant    = 8290741;

  void default_info(history_info* hinfo)
  {
    hinfo->bigrams           = false;
    hinfo->features          = 0;
    hinfo->bigram_features   = false;
    hinfo->length            = 1;
  }

  void* calloc_or_die(size_t nmemb, size_t size)
  {
    void* data = calloc(nmemb, size);
    if (data == NULL) {
      std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
      exit(-1);
    }
    return data;
  }

  void free_it(void*ptr)
  {
    if (ptr != NULL)
      free(ptr);
  }

  void add_policy_offset(example *ec, size_t max_action, size_t total_number_of_policies, size_t policy)
  {
    size_t amount = (policy * global.length() / max_action / total_number_of_policies) * global.stride;
    OAA::update_indicies(ec, amount);
  }

  void remove_policy_offset(example *ec, size_t max_action, size_t total_number_of_policies, size_t policy)
  {
    size_t amount = (policy * global.length() / max_action / total_number_of_policies) * global.stride;
    OAA::update_indicies(ec, -amount);
  }

  int random_policy(short unsigned int seed, float beta, bool allow_current_policy, int current_policy, bool allow_optimal)
  {
    seed48(&seed);

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
      float r = drand48();
      pid = 0;
      if (r > beta) {
        r -= beta;
        while ((r > 0) && (pid < num_valid_policies-1)) {
          pid ++;
          r -= beta * powf(1. - beta, (float)pid);
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

  void add_history_to_example(history_info *hinfo, example* ec, history h)
  {
    size_t v0, v, max_string_length = 0;
    size_t total_length = max(hinfo->features, hinfo->length);

    if (global.audit) {
      max_string_length = max((int)(ceil( log10((float)total_length+1) )),
                              (int)(ceil( log10((float)MAX_ACTION_ID+1) ))) + 1;
    }

    for (size_t t=1; t<=total_length; t++) {
      v0 = (h[hinfo->length-t] * quadratic_constant + t) * quadratic_constant + history_constant;

      // add the basic history features
      feature temp = {1., (uint32_t) ( (2*v0) & global.parse_mask )};
      push(ec->atomics[history_namespace], temp);

      if (global.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & global.parse_mask), 1., true };
        a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)calloc_or_die(5 + 2*max_string_length, sizeof(char));
        sprintf(a_feature.feature, "ug@%d=%d", (int)t, (int)h[hinfo->length-t]);

        push(ec->audit_features[history_namespace], a_feature);
      }

      // add the bigram features
      if ((t > 1) && hinfo->bigrams) {
        v0 = ((v0 - history_constant) * quadratic_constant + h[hinfo->length-t+1]) * quadratic_constant + history_constant;

        feature temp = {1., (uint32_t) ( (2*v0) & global.parse_mask )};
        push(ec->atomics[history_namespace], temp);

        if (global.audit) {
          audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & global.parse_mask), 1., true };
          a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
          strcpy(a_feature.space, audit_feature_space.c_str());

          a_feature.feature = (char*)calloc_or_die(6 + 3*max_string_length, sizeof(char));
          sprintf(a_feature.feature, "bg@%d=%d-%d", (int)t-1, (int)h[hinfo->length-t], (int)h[hinfo->length-t+1]);

          push(ec->audit_features[history_namespace], a_feature);
        }

      }
    }

    string fstring;

    if (hinfo->features > 0) {
      for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
        int feature_index = 0;
        for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++) {

          if (global.audit) {
            if (!ec->audit_features[*i].index() >= feature_index) {
              char buf[32];
              sprintf(buf, "{%d}", f->weight_index);
              fstring = string(buf);
            } else 
              fstring = string(ec->audit_features[*i][feature_index].feature);
            feature_index++;
          }

          v = f->weight_index + history_constant;

          for (size_t t=1; t<=hinfo->features; t++) {
            v0 = (h[hinfo->length-t] * quadratic_constant + t) * quadratic_constant;
          
            // add the history/feature pair
            feature temp = {1., (uint32_t) ( (2*(v0 + v)) & global.parse_mask )};
            push(ec->atomics[history_namespace], temp);

            if (global.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & global.parse_mask), 1., true };
              a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)calloc_or_die(8 + 2*max_string_length + fstring.length(), sizeof(char));
              sprintf(a_feature.feature, "ug+f@%d=%d=%s", (int)t, (int)h[hinfo->length-t], fstring.c_str());

              push(ec->audit_features[history_namespace], a_feature);
            }


            // add the bigram
            if ((t > 0) && hinfo->bigram_features) {
              v0 = (v0 + h[hinfo->length-t+1]) * quadratic_constant;

              feature temp = {1., (uint32_t) ( (2*(v + v0)) & global.parse_mask )};
              push(ec->atomics[history_namespace], temp);

              if (global.audit) {
                audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & global.parse_mask), 1., true };
                a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
                strcpy(a_feature.space, audit_feature_space.c_str());

                a_feature.feature = (char*)calloc_or_die(9 + 3*max_string_length + fstring.length(), sizeof(char));
                sprintf(a_feature.feature, "bg+f@%d=%d-%d=%s", (int)t-1, (int)h[hinfo->length-t], (int)h[hinfo->length-t+1], fstring.c_str());

                push(ec->audit_features[history_namespace], a_feature);
              }

            }
          }
        }
      }
    }

    push(ec->indices, history_namespace);
    ec->sum_feat_sq[history_namespace] += ec->atomics[history_namespace].index();
    ec->total_sum_feat_sq += ec->sum_feat_sq[history_namespace];
    ec->num_features += ec->atomics[history_namespace].index();
  }

  void remove_history_from_example(example* ec)
  {
    if (ec->indices.index() == 0) {
      cerr << "internal error (bug): trying to remove history, but there are no namespaces!" << endl;
      return;
    }

    if (ec->indices.last() != history_namespace) {
      cerr << "internal error (bug): trying to remove history, but either it wasn't added, or something was added after and not removed!" << endl;
      return;
    }

    ec->num_features -= ec->atomics[history_namespace].index();
    ec->total_sum_feat_sq -= ec->sum_feat_sq[history_namespace];
    ec->sum_feat_sq[history_namespace] = 0;
    ec->atomics[history_namespace].erase();
    if (global.audit) {
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

}

namespace Searn
{
  // task stuff
  search_task task;
  bool is_singleline;
  bool is_ldf;
  bool has_hash;
  bool constrainted_actions;

  // options
  size_t max_action           = 1;
  size_t max_rollout          = INT_MAX;
  size_t passes_per_policy    = 1;
  float  beta                 = 0.5;
  size_t gamma                = 1.;
  bool   do_recombination     = false;
  bool   allow_current_policy = false;

  struct rollout_item {
    state *st;
    bool  is_finished;
    bool  alive;
    size_t hash;
  };

  // memory
  rollout_item* rollout;
  v_array<example*> ec_seq = v_array<example*>();
  example** global_example_set = NULL;
  example* empty_example = NULL;  // TODO: fill this in!
  v_array<CSOAA::wclass>loss_vector = v_array<CSOAA::wclass>();
  v_array<void*>old_labels = v_array<void*>();
  CSOAA::label testall_labels = { v_array<CSOAA::wclass>() };
  CSOAA::label allowed_labels = { v_array<CSOAA::wclass>() };


  // we need a hashmap that maps from STATES to ACTIONS
  v_hashmap<state,action> past_states = v_hashmap<state,action>(0);  // 0 is an invalid action

  // tracking of example
  size_t read_example_this_loop   = 0;
  size_t read_example_last_id     = 0;
  size_t passes_since_new_policy  = 0;
  size_t read_example_last_pass   = 0;
  size_t total_examples_generated = 0;

  // variables
  size_t current_policy           = 0;
  size_t total_number_of_policies = 1;


  void (*base_learner)(example*) = NULL;
  void (*base_finish)() = NULL;


  void parse_args(po::variables_map& vm, void (*base_l)(example*), void (*base_f)())
  {
    // parse variables
    std::string task_string = vm["searn"].as<std::string>();

    if (task_string.compare("sequence") == 0)
      task =
        { SequenceTask::final,
          SequenceTask::loss,
          SequenceTask::step, 
          SequenceTask::oracle,
          SequenceTask::copy,
          SequenceTask::finish, 
          CSOAA::cs_label_parser,
          NULL,  // start_state
          SequenceTask::start_state_multiline,
          SequenceTask::cs_example,
          NULL,  // cs_example_ldf
          SequenceTask::initialize,
          NULL,  // finalize
          NULL,  // equivalent
          NULL,  // hash
          NULL   // allowed
        };
    else {
      std::cerr << "error: unknown search task '" << task_string << "'" << std::endl;
      exit(-1);
    }


    if (vm.count("searn_max_action"))              max_action           = vm["searn_max_action"].as<size_t>();
    else {
      std::cerr << "error: you must specify --searn_max_action on the command line" << std::endl;
      exit(-1);
    }

    if (vm.count("searn_rollout"))                 max_rollout          = vm["searn_rollout"].as<size_t>();
    if (vm.count("searn_passes_per_policy"))       passes_per_policy    = vm["searn_passes_per_policy"].as<size_t>();
    if (vm.count("searn_beta"))                    beta                 = vm["searn_beta"].as<float>();
    if (vm.count("searn_gamma"))                   gamma                = vm["searn_gamma"].as<float>();
    if (vm.count("searn_recombine"))               do_recombination     = true;
    if (vm.count("searn_allow_current_policy"))    allow_current_policy = true;

    if (beta <= 0 || beta >= 1) {
      std::cerr << "warning: searn_beta must be in (0,1); resetting to 0.5" << std::endl;
      beta = 0.5;
    }

    if (gamma <= 0 || gamma > 1) {
      std::cerr << "warning: searn_gamma must be in (0,1); resetting to 1.0" << std::endl;
      gamma = 1.0;
    }

    total_number_of_policies = (int)ceil(((float)global.numpasses) / ((float)passes_per_policy));

    if (task.initialize != NULL)
      if (!task.initialize(vm)) {
        std::cerr << "error: task did not initialize properly" << std::endl;
        exit(-1);
      }

    // check to make sure task is valid and set up our variables
    if (task.final  == NULL ||
        task.loss   == NULL ||
        task.step   == NULL ||
        task.oracle == NULL ||
        task.copy   == NULL ||
        task.finish == NULL ||
        ((task.start_state == NULL) == (task.start_state_multiline == NULL)) ||
        ((task.cs_example  == NULL) == (task.cs_ldf_example        == NULL))) {
      std::cerr << "error: searn task malformed" << std::endl;
      exit(-1);
    }

    is_singleline  = (task.start_state != NULL);
    is_ldf         = (task.cs_example  == NULL);
    has_hash       = (task.hash        != NULL);
    constrainted_actions = (task.allowed != NULL);

    if (do_recombination && (task.hash == NULL)) {
      std::cerr << "warning: cannot do recombination when hashing is unavailable -- turning off recombination" << std::endl;
      do_recombination = false;
    }
    if (do_recombination)
      past_states.set_equivalent(task.equivalent);

    if (is_ldf && !constrainted_actions) {
      std::cerr << "error: LDF requires allowed" << std::endl;
      exit(-1);
    }

    base_learner = base_l;
    base_finish  = base_f;
  }

  void clear_seq()
  {
    if (ec_seq.index() > 0) 
      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++) {
        free_example(*ecc);
      }
    ec_seq.erase();
  }

  void initialize_memory()
  {
    // initialize searn's memory
    rollout = (rollout_item*)SearnUtil::calloc_or_die(max_action, sizeof(rollout_item));
    global_example_set = (example**)SearnUtil::calloc_or_die(max_action, sizeof(example*));
    for (size_t k=0; k<max_action; k++)
      global_example_set[k] = (example*)SearnUtil::calloc_or_die(1, sizeof(example*));

    for (size_t k=1; k<=max_action; k++) {
      CSOAA::wclass cost = { FLT_MAX, k, 0. };
      push(testall_labels.costs, cost);
    }
  }
  
  void free_memory()
  {
    SearnUtil::free_it(rollout);

    loss_vector.erase();
    SearnUtil::free_it(loss_vector.begin);

    old_labels.erase();
    SearnUtil::free_it(old_labels.begin);

    clear_seq();
    SearnUtil::free_it(ec_seq.begin);

    for (size_t k=0; k<max_action; k++)
      SearnUtil::free_it(global_example_set[k]);
    SearnUtil::free_it(global_example_set);

    SearnUtil::free_it(testall_labels.costs.begin);
    SearnUtil::free_it(allowed_labels.costs.begin);
  }

  bool is_test_example(example*ec)
  {
    // TODO
    return true;
  }

  size_t searn_predict(state s0, size_t step, bool allow_oracle)
  {
    int policy = SearnUtil::random_policy(has_hash ? task.hash(s0) : step, beta, allow_current_policy, current_policy, allow_oracle);
    if (policy == -1)
      return task.oracle(s0);

    example *ec;

    if (!is_ldf) {
      task.cs_example(s0, ec, true);
      SearnUtil::add_policy_offset(ec, max_action, total_number_of_policies, policy);

      void* old_label = ec->ld;
      ec->ld = (void*)&testall_labels;
      if (task.allowed != NULL) {  // we need to check which actions are allowed
        allowed_labels.costs.erase();
        bool all_allowed = true;
        for (size_t k=1; k<=max_action; k++)
          if (task.allowed(s0, k)) {
            CSOAA::wclass cost = { FLT_MAX, k, 0. };
            push(allowed_labels.costs, cost);
          } else
            all_allowed = false;

        if (!all_allowed)
          ec->ld = (void*)&allowed_labels;
      }
      base_learner(ec);

      ec->ld = old_label;

      SearnUtil::remove_policy_offset(ec, max_action, total_number_of_policies, policy);
      task.cs_example(s0, ec, false);

      return (size_t)ec->final_prediction;
    } else {  // is_ldf
      float best_prediction = 0;
      size_t best_action = 0;
      for (size_t action=1; action <= max_action; action++) {
        if (!task.allowed(s0, action))
          break;   // for LDF, there are no more actions

        task.cs_ldf_example(s0, action, ec, true);
        SearnUtil::add_policy_offset(ec, max_action, total_number_of_policies, policy);
        base_learner(ec);
        SearnUtil::remove_policy_offset(ec, max_action, total_number_of_policies, policy);
        task.cs_ldf_example(s0, action, ec, false);
        if (action == 1 || 
            ec->partial_prediction > best_prediction) {
          best_prediction = ec->partial_prediction;
          best_action     = action;
        }
      }
      if (best_action < 1) {
        std::cerr << "warning: internal error on search -- could not find an available action; quitting!" << std::endl;
        exit(-1);
      }
      return best_action;
    }
  }

  void parallel_rollout(state s0)
  {
    // first, make K copies of s0 and step them
    bool all_finished = true;
    for (size_t k=1; k<=max_action; k++) 
      rollout[k-1].alive = false;

    for (size_t k=1; k<=max_action; k++) {
      // in the case of LDF, we might run out of actions early
      if (!task.allowed(s0, k)) {
        if (is_ldf) break;
        else continue;
      }
      rollout[k-1].alive = true;
      task.copy(s0, rollout[k-1].st);
      task.step(rollout[k-1].st, k);
      rollout[k-1].is_finished = task.final(rollout[k-1].st);
      if (do_recombination) rollout[k-1].hash = task.hash(rollout[k-1].st);
      all_finished = all_finished && rollout[k-1].is_finished;
    }

    // now, complete all rollouts
    if (!all_finished) {
      for (size_t step=1; step<max_rollout; step++) {
        all_finished = true;
        for (size_t k=1; k<=max_action; k++) {
          if (rollout[k-1].is_finished) continue;
          
          size_t action = 0;
          if (do_recombination)
            action = past_states.get(rollout[k-1].st, rollout[k-1].hash);

          if (action == 0) {  // this means we didn't find it or we're not recombining
            action = searn_predict(*rollout[k-1].st, step, true);
            if (do_recombination) {
              // we need to make a copy of the state
              state copy;
              task.copy(rollout[k-1].st, &copy);
              past_states.put_after_get(copy, rollout[k-1].hash, action);
            }
          }          
          
          task.step(*rollout[k-1].st, action);
          rollout[k-1].is_finished = task.final(rollout[k-1].st);
          if (do_recombination) rollout[k-1].hash = task.hash(rollout[k-1].st);
          all_finished = all_finished && rollout[k-1].is_finished;
        }
        if (all_finished) break;
      }
    }

    // finally, compute losses and free copies
    float min_loss = 0;
    loss_vector.erase();
    for (size_t k=1; k<=max_action; k++) {
      if (!rollout[k-1].alive)
        break;

      float l = task.loss(*rollout[k-1].st);
      if ((l == FLT_MAX) && (!rollout[k-1].is_finished) && (max_rollout < INT_MAX)) {
        std::cerr << "error: you asked for short rollouts, but your task does not support pre-final losses" << std::endl;
        exit(-1);
      }

      CSOAA::wclass temp = { l, k, 0. };
      push(loss_vector, temp);
      if ((k == 1) || (l < min_loss)) { min_loss = l; }

      task.finish(rollout[k-1].st);
    }

    // subtract the smallest loss
    for (size_t k=1; k<=max_action; k++)
      if (rollout[k-1].alive)
        loss_vector[k-1].x -= min_loss;
  }

  void generate_state_example(state s0)
  {
    // start by doing rollouts so we can get costs
    parallel_rollout(s0);
    if (loss_vector.index() <= 1) {
      // nothing interesting to do!
      return;
    }

    // now, generate training examples
    if (!is_ldf) {
      total_examples_generated++;

      example* ec;
      CSOAA::label ld = { loss_vector };

      task.cs_example(s0, ec, true);
      void* old_label = ec->ld;
      ec->ld = (void*)&ld;
      SearnUtil::add_policy_offset(ec, max_action, total_number_of_policies, current_policy);
      base_learner(ec);
      SearnUtil::remove_policy_offset(ec, max_action, total_number_of_policies, current_policy);
      ec->ld = old_label;
      task.cs_example(s0, ec, false);
    } else { // is_ldf
      old_labels.erase();
      for (size_t k=1; k<=max_action; k++) {
        if (!rollout[k-1].alive) break;

        total_examples_generated++;

        OAA::mc_label ld = { k, loss_vector[k-1].x };

        task.cs_ldf_example(s0, k, global_example_set[k-1], true);
        push(old_labels, global_example_set[k-1]->ld);
        global_example_set[k-1]->ld = (void*)&ld;
        SearnUtil::add_policy_offset(global_example_set[k-1], max_action, total_number_of_policies, current_policy);
        base_learner(global_example_set[k-1]);
      }

      base_learner(empty_example);

      for (size_t k=1; k<=max_action; k++) {
        if (!rollout[k-1].alive) break;
        SearnUtil::remove_policy_offset(global_example_set[k-1], max_action, total_number_of_policies, current_policy);
        global_example_set[k-1]->ld = old_labels[k-1];
        task.cs_ldf_example(s0, k, global_example_set[k-1], false);
      }
    }

  }

  void run_prediction(state s0, bool allow_oracle)
  {
    int step = 1;
    while (!task.final(s0)) {
      size_t action = searn_predict(s0, step, allow_oracle);
      task.step(s0, action);
      step++;
    }
  }

  void hm_free_state_copies(state s, action a) { task.finish(&s); }

  void do_actual_learning()
  {
    // there are two cases:
    //   * is_singleline --> look only at ec_seq[0]
    //   * otherwise     --> look at everything

    if (ec_seq.index() == 0)
      return;

    // generate the start state
    state s0;
    if (is_singleline)
      task.start_state(ec_seq[0], &s0);
    else
      task.start_state_multiline(ec_seq.begin, ec_seq.index(), &s0);

    // if this is a test-example, just run it!
    if (is_test_example(ec_seq[0])) {
      run_prediction(s0, false);
      task.finish(&s0);
      return;
    }

    // training examples only get here
    int step = 1;
    while (!task.final(s0)) {
      // first, make a prediction (we don't want to bias ourselves if
      // we're using the current policy to predict)
      size_t action = searn_predict(s0, step, true);

      // generate training example for the current state
      generate_state_example(s0);

      // take the prescribed step
      task.step(s0, action);

      step++;
    }

    if (do_recombination) {  // we need to free a bunch of memory
      past_states.iter(&hm_free_state_copies);
      past_states.clear();
    }
  }

  void process_next_example(example *ec)
  {
    bool is_real_example = true;

    if (is_singleline) {
      if (ec_seq.index() == 0)
        push(ec_seq, ec);
      else
        ec_seq[0] = ec;

      do_actual_learning();
    } else {  
      // is multiline
      if (ec_seq.index() >= global.ring_size - 2) { // give some wiggle room
        std::cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << std::endl;
        do_actual_learning();
        clear_seq();
      }

      if (CSOAA_LDF::example_is_newline(ec)) {
        do_actual_learning();
        clear_seq();
        CSOAA_LDF::global_print_newline();
        free_example(ec);
        is_real_example = false;
      } else {
        push(ec_seq, ec);
      }
    }

    // for both single and multiline
    if (is_real_example) {
      read_example_this_loop++;
      read_example_last_id = ec->example_counter;
      if (ec->pass != read_example_last_pass) {
        read_example_last_pass = ec->pass;
        passes_since_new_policy++;
        if (passes_since_new_policy >= passes_per_policy) {
          passes_since_new_policy = 0;
          current_policy++;
          if (current_policy > total_number_of_policies) {
            std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
            current_policy = total_number_of_policies;
          }
        }
      }
    }
  }

  void drive_searn()
  {
    // initialize everything
    initialize_memory();

    example* ec = NULL;
    read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example()) != NULL) { // semiblocking operation
        process_next_example(ec);
      } else if (parser_done()) {
        if (!is_singleline)
          do_actual_learning();
        break;
      }
    }

    // free everything
    if (task.finalize != NULL)
      task.finalize();
    free_memory();
    base_finish();
  }
}


