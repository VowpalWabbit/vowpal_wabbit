/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
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
#include "cb.h"
#include "v_hashmap.h"
#include "beam.h"
#include "vw.h"

// task-specific includes
#include "searn_sequencetask.h"

#ifdef _WIN32
inline void srand48(long l) { srand(l); }
inline double drand48() { return rand() / (double)RAND_MAX; }
#endif

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
    if (nmemb == 0 || size == 0)
      return NULL;

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

  void add_policy_offset(vw&all, example *ec, size_t increment, size_t policy)
  {
    update_example_indicies(all.audit, ec, policy * increment);
  }

  void remove_policy_offset(vw&all, example *ec, size_t increment, size_t policy)
  {
    update_example_indicies(all.audit, ec, -policy * increment);
  }

  int random_policy(long int seed, float beta, bool allow_current_policy, int current_policy, bool allow_optimal, bool reset_seed)
  {
    if(reset_seed) //reset_seed is false for contextual bandit, so that we only reset the seed if the base learner is not a contextual bandit learner, as this breaks the exploration.
      srand48(seed);

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
      float r = (float)drand48();
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

  void add_history_to_example(vw&all, history_info *hinfo, example* ec, history h)
  {
    size_t v0, v, max_string_length = 0;
    size_t total_length = max(hinfo->features, hinfo->length);

    if (total_length == 0) return;
    if (h == NULL) {
      cerr << "error: got empty history in add_history_to_example" << endl;
      exit(-1);
    }

    if (all.audit) {
      max_string_length = max((int)(ceil( log10((float)total_length+1) )),
                              (int)(ceil( log10((float)MAX_ACTION_ID+1) ))) + 1;
    }

    for (size_t t=1; t<=total_length; t++) {
      v0 = (h[hinfo->length-t] * quadratic_constant + t) * quadratic_constant + history_constant;

      // add the basic history features
      feature temp = {1., (uint32_t) ( (2*v0) & all.parse_mask )};
      push(ec->atomics[history_namespace], temp);

      if (all.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & all.parse_mask), 1., true };
        a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)calloc_or_die(5 + 2*max_string_length, sizeof(char));
        sprintf(a_feature.feature, "ug@%d=%d", (int)t, (int)h[hinfo->length-t]);

        push(ec->audit_features[history_namespace], a_feature);
      }

      // add the bigram features
      if ((t > 1) && hinfo->bigrams) {
        v0 = ((v0 - history_constant) * quadratic_constant + h[hinfo->length-t+1]) * quadratic_constant + history_constant;

        feature temp = {1., (uint32_t) ( (2*v0) & all.parse_mask )};
        push(ec->atomics[history_namespace], temp);

        if (all.audit) {
          audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & all.parse_mask), 1., true };
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

          if (all.audit) {
            if (feature_index >= (int)ec->audit_features[*i].index() ) {
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
            feature temp = {1., (uint32_t) ( (2*(v0 + v)) & all.parse_mask )};
            push(ec->atomics[history_namespace], temp);

            if (all.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & all.parse_mask), 1., true };
              a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)calloc_or_die(8 + 2*max_string_length + fstring.length(), sizeof(char));
              sprintf(a_feature.feature, "ug+f@%d=%d=%s", (int)t, (int)h[hinfo->length-t], fstring.c_str());

              push(ec->audit_features[history_namespace], a_feature);
            }


            // add the bigram
            if ((t > 0) && hinfo->bigram_features) {
              v0 = (v0 + h[hinfo->length-t+1]) * quadratic_constant;

              feature temp = {1., (uint32_t) ( (2*(v + v0)) & all.parse_mask )};
              push(ec->atomics[history_namespace], temp);

              if (all.audit) {
                audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & all.parse_mask), 1., true };
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

  void remove_history_from_example(vw&all, history_info *hinfo, example* ec)
  {
    size_t total_length = max(hinfo->features, hinfo->length);
    if (total_length == 0) return;

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

}

namespace Searn
{
  // task stuff
  search_task task;
  bool is_singleline;
  bool is_ldf;
  bool has_hash;
  bool constrainted_actions;
  size_t input_label_size;

  // options
  size_t max_action           = 1;
  size_t max_rollout          = INT_MAX;
  size_t passes_per_policy    = 1;     //this should be set to the same value as --passes for dagger
  float  beta                 = 0.5;
  float gamma                = 1.;
  bool   do_recombination     = true;
  bool   allow_current_policy = false; //this should be set to true for dagger
  bool   rollout_oracle       = false; //if true then rollout are performed using oracle instead (optimal approximation discussed in searn's paper). this should be set to true for dagger
  bool   adaptive_beta        = false; //used to implement dagger through searn. if true, beta = 1-(1-alpha)^n after n updates, and policy is mixed with oracle as \pi' = (1-beta)\pi^* + beta \pi
  float  alpha                = 0.001f; //parameter used to adapt beta for dagger (see above comment), should be in (0,1)
  bool   rollout_all_actions  = true;  //by default we rollout all actions. This is set to false when searn is used with a contextual bandit base learner, where we rollout only one sampled action

  // debug stuff
  bool PRINT_DEBUG_INFO             = 0;
  bool PRINT_UPDATE_EVERY_EXAMPLE   = 0 | PRINT_DEBUG_INFO;


  // rollout
  struct rollout_item {
    state st;
    bool  is_finished;
    bool  alive;
    size_t hash;
  };

  // memory
  rollout_item* rollout;
  v_array<example*> ec_seq = v_array<example*>();
  example** global_example_set = NULL;
  example* empty_example = NULL;
  OAA::mc_label empty_label;
  v_array<CSOAA::wclass>loss_vector = v_array<CSOAA::wclass>();
  v_array<CB::cb_class>loss_vector_cb = v_array<CB::cb_class>();
  v_array<void*>old_labels = v_array<void*>();
  v_array<OAA::mc_label>new_labels = v_array<OAA::mc_label>();
  CSOAA::label testall_labels = { v_array<CSOAA::wclass>() };
  CSOAA::label allowed_labels = { v_array<CSOAA::wclass>() };
  CB::label testall_labels_cb = { v_array<CB::cb_class>() };
  CB::label allowed_labels_cb = { v_array<CB::cb_class>() };

  // we need a hashmap that maps from STATES to ACTIONS
  v_hashmap<state,action> *past_states = NULL;
  v_array<state> unfreed_states = v_array<state>();

  // tracking of example
  size_t read_example_this_loop   = 0;
  size_t read_example_last_id     = 0;
  size_t passes_since_new_policy  = 0;
  size_t read_example_last_pass   = 0;
  size_t total_examples_generated = 0;
  size_t total_predictions_made   = 0;
  int    searn_num_features       = 0;

  // variables
  size_t current_policy           = 0;
  size_t total_number_of_policies = 1;
  size_t increment                = 0; //for policy offset

  void (*base_learner)(void*, example*) = NULL;
  void (*base_finish)(void*) = NULL;

  void simple_print_example_features(vw&all, example *ec)
  {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      {
        feature* end = ec->atomics[*i].end;
        for (feature* f = ec->atomics[*i].begin; f!= end; f++) {
          cerr << "\t" << f->weight_index << ":" << f->x << ":" << all.reg.weight_vectors[f->weight_index & all.weight_mask];
        }
      }
    cerr << endl;
  }

  void simple_print_costs(CSOAA::label *c)
  {
    for (CSOAA::wclass *f = c->costs.begin; f != c->costs.end; f++) {
      clog << "\t" << f->weight_index << ":" << f->x << "::" << f->partial_prediction;
    }
    clog << endl;
  }

  bool should_print_update(vw& all)
  {
    //uncomment to print out final loss after all examples processed
    //commented for now so that outputs matches make test
    //if( parser_done(all.p)) return true;

    if (!(all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)) {
      if (!PRINT_UPDATE_EVERY_EXAMPLE) return false;
    }
    return true;
  }

  std::vector<action> empty_action_vector = std::vector<action>();

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

  void global_print_label(vw& all, example*ec, state s0, std::vector<action> last_action_sequence)
  {
    if (!task.to_string)
      return;

    string str = task.to_string(s0, false, last_action_sequence);
    for (size_t i=0; i<all.final_prediction_sink.index(); i++) {
      int f = all.final_prediction_sink[i];
      all.print_text(f, str, ec->tag);
    }
  }

  void print_update(vw& all, state s0, std::vector<action> last_action_sequence)
  {
    if (!should_print_update(all))
      return;

    char true_label[21];
    char pred_label[21];
    if (task.to_string) {
      to_short_string(task.to_string(s0, true , empty_action_vector ), 20, true_label);
      to_short_string(task.to_string(s0, false, last_action_sequence), 20, pred_label);
    } else {
      to_short_string("", 20, true_label);
      to_short_string("", 20, pred_label);
    }

    fprintf(stderr, "%-10.6f %-10.6f %8ld %15f   [%s] [%s] %8lu %5d %5d %15lu %15lu\n",
            all.sd->sum_loss/all.sd->weighted_examples,
            all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
            (long int)all.sd->example_number,
            all.sd->weighted_examples,
            true_label,
            pred_label,
            (long unsigned int)searn_num_features,
            (int)read_example_last_pass,
            (int)current_policy,
            (long unsigned int)total_predictions_made,
            (long unsigned int)total_examples_generated);

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    all.sd->dump_interval *= 2;
  }



  void clear_seq(vw&all)
  {
    if (ec_seq.index() > 0) 
      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++) {
	VW::finish_example(all, *ecc);
      }
    ec_seq.erase();
  }

  void free_unfreed_states()
  {
    while (!unfreed_states.empty()) {
      state s = unfreed_states.pop();
      task.finish(s);
    }
  }

  void initialize_memory()
  {
    // initialize searn's memory
    rollout = (rollout_item*)SearnUtil::calloc_or_die(max_action, sizeof(rollout_item));
    global_example_set = (example**)SearnUtil::calloc_or_die(max_action, sizeof(example*));

    for (size_t k=1; k<=max_action; k++) {
      CSOAA::wclass cost = { FLT_MAX, k, 0. };
      push(testall_labels.costs, cost);
      CB::cb_class cost_cb = { FLT_MAX, k, 0. };
      push(testall_labels_cb.costs, cost_cb);
    }

    empty_example = alloc_example(sizeof(OAA::mc_label));
    OAA::default_label(empty_example->ld);
    //    cerr << "create: empty_example->ld = " << empty_example->ld << endl;
    empty_example->in_use = true;
  }
  
  void free_memory(vw&all)
  {
    dealloc_example(NULL, *empty_example);
    free(empty_example);

    SearnUtil::free_it(rollout);

    loss_vector.erase();
    SearnUtil::free_it(loss_vector.begin);

    old_labels.erase();
    SearnUtil::free_it(old_labels.begin);

    new_labels.erase();
    SearnUtil::free_it(new_labels.begin);

    free_unfreed_states();
    unfreed_states.erase();
    SearnUtil::free_it(unfreed_states.begin);

    clear_seq(all);
    SearnUtil::free_it(ec_seq.begin);

    SearnUtil::free_it(global_example_set);

    SearnUtil::free_it(testall_labels.costs.begin);
    SearnUtil::free_it(testall_labels_cb.costs.begin);
    SearnUtil::free_it(allowed_labels.costs.begin);
    SearnUtil::free_it(allowed_labels_cb.costs.begin);

    if (do_recombination) {
      delete past_states;
      past_states = NULL;
    }
  }



  void learn(void*in, example *ec)
  {
    //vw*all = (vw*)in;
    // TODO
  }

  void finish(void*in)
  {
    vw*all = (vw*)in;
    // free everything
    if (task.finalize != NULL)
      task.finalize();
    free_memory(*all);
    base_finish(all);
  }

  void parse_flags(vw&all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    po::options_description desc("Searn options");
    desc.add_options()
      ("searn_task", po::value<string>(), "the searn task")
      ("searn_rollout", po::value<size_t>(), "maximum rollout length")
      ("searn_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
      ("searn_beta", po::value<float>(), "interpolation rate for policies")
      ("searn_gamma", po::value<float>(), "discount rate for policies")
      ("searn_recombine", "allow searn labeling to use the current policy")
      ("searn_allow_current_policy", "allow searn labeling to use the current policy")
      ("searn_rollout_oracle", "allow searn/dagger to do rollouts with the oracle when estimating cost-to-go")
      ("searn_as_dagger", po::value<float>(), "sets options to make searn operate as dagger. parameter is the sliding autonomy rate (rate at which beta tends to 1).")
      ("searn_total_nb_policies", po::value<size_t>(), "if we are going to train the policies through multiple separate calls to vw, we need to specify this parameter and tell vw how many policies are eventually going to be trained");

    po::options_description add_desc_file("Searn options only available in regressor file");
    add_desc_file.add_options()
      ("searn_trained_nb_policies", po::value<size_t>(), "the number of trained policies in the regressor file");

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
    if(vm_file.count("searn_task")) {//we loaded searn task flag from regressor file    
      task_string = vm_file["searn_task"].as<std::string>();
      if(vm.count("searn_task") && task_string.compare(vm["searn_task"].as<std::string>()) != 0 )
      {
        std::cerr << "warning: specified --searn_task different than the one loaded from regressor. Pursuing with loaded value of: " << task_string << endl;
      }
    }
    else {
      if (vm.count("searn_task") == 0) {
        cerr << "must specify --searn_task" << endl;
        exit(-1);
      }
      task_string = vm["searn_task"].as<std::string>();

      //append the searn task to options_from_file so it is saved in the regressor file later
      all.options_from_file.append(" --searn_task ");
      all.options_from_file.append(task_string);
    }

    if (task_string.compare("sequence") == 0) {
      task.final = SequenceTask::final;
      task.loss = SequenceTask::loss;
      task.step = SequenceTask::step; 
      task.oracle = SequenceTask::oracle;
      task.copy = SequenceTask::copy;
      task.finish = SequenceTask::finish; 
      task.searn_label_parser = OAA::mc_label_parser;
      task.is_test_example = SequenceTask::is_test_example;
      input_label_size = sizeof(OAA::mc_label);
      task.start_state = NULL;
      task.start_state_multiline = SequenceTask::start_state_multiline;
      if (1) {
        task.cs_example = SequenceTask::cs_example;
        task.cs_ldf_example = NULL;
      } else {
        task.cs_example = NULL;
        task.cs_ldf_example = SequenceTask::cs_ldf_example;
      }
      task.initialize = SequenceTask::initialize;
      task.finalize = NULL;
      task.equivalent = SequenceTask::equivalent;
      task.hash = SequenceTask::hash;
      task.allowed = SequenceTask::allowed;
      task.to_string = SequenceTask::to_string;
    } else {
      std::cerr << "error: unknown search task '" << task_string << "'" << std::endl;
      exit(-1);
    }

    *(all.p->lp)=task.searn_label_parser;

    if(vm_file.count("searn")) { //we loaded searn flag from regressor file 
      max_action = vm_file["searn"].as<size_t>();
      if( vm.count("searn") && vm["searn"].as<size_t>() != max_action )
        std::cerr << "warning: you specified a different number of actions through --searn than the one loaded from predictor. Pursuing with loaded value of: " << max_action << endl;
    }
    else {
      max_action = vm["searn"].as<size_t>();

      //append searn with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --searn " << max_action;
      all.options_from_file.append(ss.str());
    }

    if(vm_file.count("searn_beta")) { //we loaded searn_beta flag from regressor file 
      beta = vm_file["searn_beta"].as<float>();
      if (vm.count("searn_beta") && vm["searn_beta"].as<float>() != beta )
        std::cerr << "warning: you specified a different value through --searn_beta than the one loaded from predictor. Pursuing with loaded value of: " << beta << endl;

    }
    else {
      if (vm.count("searn_beta")) beta = vm["searn_beta"].as<float>();

      //append searn_beta to options_from_file so it is saved in the regressor file later
      std::stringstream ss;
      ss << " --searn_beta " << beta;
      all.options_from_file.append(ss.str());
    }

    if (vm.count("searn_rollout"))                 max_rollout          = vm["searn_rollout"].as<size_t>();
    if (vm.count("searn_passes_per_policy"))       passes_per_policy    = vm["searn_passes_per_policy"].as<size_t>();
      
    if (vm.count("searn_gamma"))                   gamma                = vm["searn_gamma"].as<float>();
    if (vm.count("searn_norecombine"))             do_recombination     = false;
    if (vm.count("searn_allow_current_policy"))    allow_current_policy = true;
    if (vm.count("searn_rollout_oracle"))    	   rollout_oracle       = true;

    //check if the base learner is contextual bandit, in which case, we dont rollout all actions.
    if ( vm.count("cb") || vm_file.count("cb") ) rollout_all_actions = false;

    //if we loaded a regressor with -i option, --searn_trained_nb_policies contains the number of trained policies in the file
    // and --searn_total_nb_policies contains the total number of policies in the file
    if ( vm_file.count("searn_total_nb_policies") )
    {
	current_policy = vm_file["searn_trained_nb_policies"].as<size_t>();
	total_number_of_policies = vm_file["searn_total_nb_policies"].as<size_t>();
        if (vm.count("searn_total_nb_policies") && vm["searn_total_nb_policies"].as<size_t>() != total_number_of_policies)
          std::cerr << "warning: --searn_total_nb_policies doesn't match the total number of policies stored in initial predictor. Using loaded value of: " << total_number_of_policies << endl;
    }
    else if (vm.count("searn_total_nb_policies"))
    {
	total_number_of_policies = vm["searn_total_nb_policies"].as<size_t>();
    }

    if (vm.count("searn_as_dagger"))
    {
      //overide previously loaded options to set searn as dagger
      allow_current_policy = true;
      passes_per_policy = all.numpasses;
      //rollout_oracle = true;
      if( current_policy > 1 ) 
        current_policy = 1;

      //indicate to adapt beta for each update
      adaptive_beta = true;
      alpha = vm["searn_as_dagger"].as<float>();
    }

    if (beta <= 0 || beta >= 1) {
      std::cerr << "warning: searn_beta must be in (0,1); resetting to 0.5" << std::endl;
      beta = 0.5;
    }

    if (gamma <= 0 || gamma > 1) {
      std::cerr << "warning: searn_gamma must be in (0,1); resetting to 1.0" << std::endl;
      gamma = 1.0;
    }

    if (alpha < 0 || alpha > 1) {
      std::cerr << "warning: searn_adaptive_beta must be in (0,1); resetting to 0.001" << std::endl;
      alpha = 0.001f;
    }

    if (task.initialize != NULL)
      if (!task.initialize(all, opts, vm, vm_file)) {
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
    if (do_recombination) {
      // 0 is an invalid action
      past_states = new v_hashmap<state,action>(1023, 0, task.equivalent);
    }

    if (is_ldf && !constrainted_actions) {
      std::cerr << "error: LDF requires allowed" << std::endl;
      exit(-1);
    }

    all.searn = true;

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    size_t tmp_number_of_policies = current_policy; 
    if( all.training )
	tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls, 
    //so only set total_number_of_policies to computed value if it is larger
    if( tmp_number_of_policies > total_number_of_policies )
    {
	total_number_of_policies = tmp_number_of_policies;
        if( current_policy > 0 ) //we loaded a file but total number of policies didn't match what is needed for training
        {
          std::cerr << "warning: you're attempting to train more classifiers than was allocated initially. Likely to cause bad performance." << endl;
        }  
    }

    //current policy currently points to a new policy we would train
    //if we are not training and loaded a bunch of policies for testing, we need to subtract 1 from current policy
    //so that we only use those loaded when testing (as run_prediction is called with allow_current to true)
    if( !all.training && current_policy > 0 )
	current_policy--;

    //std::cerr << "Current Policy: " << current_policy << endl;
    //std::cerr << "Total Number of Policies: " << total_number_of_policies << endl;

    std::stringstream ss1;
    std::stringstream ss2;
    ss1 << current_policy;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_trained_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--searn_trained_nb_policies", ss1.str()); 
    ss2 << total_number_of_policies;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_total_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--searn_total_nb_policies", ss2.str());

    all.base_learner_nb_w *= total_number_of_policies;
    increment = (all.length() / all.base_learner_nb_w ) * all.stride;

    all.driver = drive;
    base_learner = all.learn;
    all.learn = learn;
    base_finish = all.finish;
    all.finish = finish;
  }

  size_t searn_predict(vw&all, state s0, size_t step, bool allow_oracle, bool allow_current, v_array< pair<size_t,float> >* partial_predictions)  // TODO: partial_predictions
  {
    int policy = SearnUtil::random_policy(has_hash ? task.hash(s0) : step, beta, allow_current, current_policy, allow_oracle, rollout_all_actions);
    if (PRINT_DEBUG_INFO) { cerr << "predicing with policy " << policy << " (allow_oracle=" << allow_oracle << ", allow_current=" << allow_current << "), current_policy=" << current_policy << endl; }
    if (policy == -1) {
      return task.oracle(s0);
    }

    example *ec;

    if (!is_ldf) {
      task.cs_example(all, s0, ec, true);
      SearnUtil::add_policy_offset(all, ec, increment, policy);

      void* old_label = ec->ld;
      if(rollout_all_actions) { //this means we have a cost-sensitive base learner
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
      }
      else { //if we have a contextual bandit base learner
        ec->ld = (void*)&testall_labels_cb;
        if (task.allowed != NULL) {  // we need to check which actions are allowed
          allowed_labels_cb.costs.erase();
          bool all_allowed = true;
          for (size_t k=1; k<=max_action; k++)
            if (task.allowed(s0, k)) {
              CB::cb_class cost = { FLT_MAX, k, 0. };
              push(allowed_labels_cb.costs, cost);
            } else
              all_allowed = false;

          if (!all_allowed)
            ec->ld = (void*)&allowed_labels_cb;
        }
      }
      base_learner(&all,ec);  total_predictions_made++;  searn_num_features += ec->num_features;
      size_t final_prediction = (size_t)(*(OAA::prediction_t*)&(ec->final_prediction));
      ec->ld = old_label;

      SearnUtil::remove_policy_offset(all, ec, increment, policy);
      task.cs_example(all, s0, ec, false);

      return final_prediction;
    } else {  // is_ldf
      //TODO: modify this to handle contextual bandit base learner with ldf
      float best_prediction = 0;
      size_t best_action = 0;
      for (size_t action=1; action <= max_action; action++) {
        if (!task.allowed(s0, action))
          break;   // for LDF, there are no more actions

        task.cs_ldf_example(all, s0, action, ec, true);
        //cerr << "created example: " << ec << ", label: " << ec->ld << endl;
        SearnUtil::add_policy_offset(all, ec, increment, policy);
        base_learner(&all,ec);  total_predictions_made++;  searn_num_features += ec->num_features;
        //cerr << "base_learned on example: " << ec << endl;
        empty_example->in_use = true;
        base_learner(&all,empty_example);
        //cerr << "base_learned on empty example: " << empty_example << endl;
        SearnUtil::remove_policy_offset(all, ec, increment, policy);

        if (action == 1 || 
            ec->partial_prediction < best_prediction) {
          best_prediction = ec->partial_prediction;
          best_action     = action;
        }
        //cerr << "releasing example: " << ec << ", label: " << ec->ld << endl;
        task.cs_ldf_example(all, s0, action, ec, false);
      }

      if (best_action < 1) {
        std::cerr << "warning: internal error on search -- could not find an available action; quitting!" << std::endl;
        exit(-1);
      }
      return best_action;
    }
  }

  float single_rollout(vw&all, state s0, size_t action)
  {
    //first check if action is valid for current state
    if( action < 1 || action > max_action || (task.allowed && !task.allowed(s0,action)) )
    {
	std::cerr << "warning: asked to rollout an unallowed action: " << action << "; not performing rollout." << std::endl;
	return 0;
    }
    
    //copy state and step it with current action
    rollout[action-1].alive = true;
    rollout[action-1].st = task.copy(s0);
    task.step(rollout[action-1].st, action);
    rollout[action-1].is_finished = task.final(rollout[action-1].st);
    if (do_recombination) rollout[action-1].hash = task.hash(rollout[action-1].st);

    //if not finished complete rollout
    if (!rollout[action-1].is_finished) {
      for (size_t step=1; step<max_rollout; step++) {
        size_t act_tmp = 0;
        if (do_recombination)
          act_tmp = past_states->get(rollout[action-1].st, rollout[action-1].hash);

        if (act_tmp == 0) {  // this means we didn't find it or we're not recombining
          if( !rollout_oracle )
            act_tmp = searn_predict(all, rollout[action-1].st, step, true, allow_current_policy, NULL);
	  else
            act_tmp = task.oracle(rollout[action-1].st);

          if (do_recombination) {
            // we need to make a copy of the state
            state copy = task.copy(rollout[action-1].st);
            past_states->put_after_get(copy, rollout[action-1].hash, act_tmp);
            push(unfreed_states, copy);
          }
        }          
          
        task.step(rollout[action-1].st, act_tmp);
        rollout[action-1].is_finished = task.final(rollout[action-1].st);
        if (do_recombination) rollout[action-1].hash = task.hash(rollout[action-1].st);
        if (rollout[action-1].is_finished) break;
      }
    }

    // finally, compute losses and free copies
    float l = task.loss(rollout[action-1].st);
    if ((l == FLT_MAX) && (!rollout[action-1].is_finished) && (max_rollout < INT_MAX)) {
      std::cerr << "error: you asked for short rollouts, but your task does not support pre-final losses" << std::endl;
      exit(-1);
    }
    task.finish(rollout[action-1].st);

    return l;
  }

  void parallel_rollout(vw&all, state s0)
  {
    // first, make K copies of s0 and step them
    bool all_finished = true;
    for (size_t k=1; k<=max_action; k++) 
      rollout[k-1].alive = false;
    
    for (size_t k=1; k<=max_action; k++) {
      // in the case of LDF, we might run out of actions early
      if (task.allowed && !task.allowed(s0, k)) {
        if (is_ldf) break;
        else continue;
      }
      rollout[k-1].alive = true;
      rollout[k-1].st = task.copy(s0);
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
            action = past_states->get(rollout[k-1].st, rollout[k-1].hash);

          if (action == 0) {  // this means we didn't find it or we're not recombining
            if( !rollout_oracle )
              action = searn_predict(all, rollout[k-1].st, step, true, allow_current_policy, NULL);
	    else
              action = task.oracle(rollout[k-1].st);

            if (do_recombination) {
              // we need to make a copy of the state
              state copy = task.copy(rollout[k-1].st);
              past_states->put_after_get(copy, rollout[k-1].hash, action);
              push(unfreed_states, copy);
            }
          }          
          
          task.step(rollout[k-1].st, action);
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

      float l = task.loss(rollout[k-1].st);
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

  size_t uniform_exploration(state s0, float& prob_sampled_action)
  {
    //find how many valid actions
    size_t nb_allowed_actions = max_action;
    if( task.allowed ) {  
      for (size_t k=1; k<=max_action; k++) {
        if( !task.allowed(s0,k) ) {
          nb_allowed_actions--;
          if (is_ldf) {
            nb_allowed_actions = k-1;
            break;
          }
        }
      }
    }

    size_t action = (size_t)(drand48() * nb_allowed_actions) + 1;
    if( task.allowed && nb_allowed_actions < max_action && !is_ldf) {
      //need to adjust action to the corresponding valid action
      for (size_t k=1; k<=action; k++) {
        if( !task.allowed(s0,k) ) action++;
      }
    }
    prob_sampled_action = (float) (1.0/nb_allowed_actions);
    return action;
  }

  void get_contextual_bandit_loss_vector(vw&all, state s0)
  {
    float prob_sampled = 1.;
    size_t act = uniform_exploration(s0,prob_sampled);
    float loss = single_rollout(all,s0,act);

    loss_vector_cb.erase();
    for (size_t k=1; k<=max_action; k++) {
      if( task.allowed && !task.allowed(s0,k))
	break;
      
      CB::cb_class temp;
      temp.x = FLT_MAX;
      temp.weight_index = k;
      temp.prob_action = 0.;
      if( act == k ) {
        temp.x = loss;
        temp.prob_action = prob_sampled;
      }
      push(loss_vector_cb, temp);
    }
  }

  void generate_state_example(vw&all, state s0)
  {
    // start by doing rollouts so we can get costs
    loss_vector.erase();
    loss_vector_cb.erase();
    if( rollout_all_actions ) {
      parallel_rollout(all, s0);      
    }
    else {
      get_contextual_bandit_loss_vector(all, s0);
    }

    if (loss_vector.index() <= 1 && loss_vector_cb.index() == 0) {
      // nothing interesting to do!
      return;
    }

    // now, generate training examples
    if (!is_ldf) {
      total_examples_generated++;

      example* ec;
      task.cs_example(all, s0, ec, true);
      void* old_label = ec->ld;

      if(rollout_all_actions) {
        CSOAA::label ld = { loss_vector };
        ec->ld = (void*)&ld;
      } 
      else {
        CB::label ld = { loss_vector_cb };
        ec->ld = (void*)&ld;
      }
      SearnUtil::add_policy_offset(all, ec, increment, current_policy);
      base_learner(&all,ec);
      SearnUtil::remove_policy_offset(all, ec, increment, current_policy);
      ec->ld = old_label;
      task.cs_example(all, s0, ec, false);
    } else { // is_ldf
      //TODO: support ldf with contextual bandit base learner
      old_labels.erase();
      new_labels.erase();

      for (size_t k=1; k<=max_action; k++) {
        if (rollout[k-1].alive) {
          OAA::mc_label ld = { k, loss_vector[k-1].x };
          push(new_labels, ld);
        } else {
          OAA::mc_label ld = { k, 0. };
          push(new_labels, ld);
        }
      }

      //      cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;

      for (size_t k=1; k<=max_action; k++) {
        if (!rollout[k-1].alive) break;

        total_examples_generated++;

        task.cs_ldf_example(all, s0, k, global_example_set[k-1], true);
        push(old_labels, global_example_set[k-1]->ld);
        global_example_set[k-1]->ld = (void*)(&new_labels[k-1]);
        SearnUtil::add_policy_offset(all, global_example_set[k-1], increment, current_policy);
        if (PRINT_DEBUG_INFO) { cerr << "add_policy_offset, max_action=" << max_action << ", total_number_of_policies=" << total_number_of_policies << ", current_policy=" << current_policy << endl;}
        base_learner(&all,global_example_set[k-1]);
      }

      //      cerr << "============================ (empty = " << empty_example << ")" << endl;
      empty_example->in_use = true;
      base_learner(&all,empty_example);

      for (size_t k=1; k<=max_action; k++) {
        if (!rollout[k-1].alive) break;
        SearnUtil::remove_policy_offset(all, global_example_set[k-1], increment, current_policy);
        global_example_set[k-1]->ld = old_labels[k-1];
        task.cs_ldf_example(all, s0, k, global_example_set[k-1], false);
      }
      //      cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    }

  }

  void run_prediction(vw&all, state s0, bool allow_oracle, bool allow_current, bool track_actions, std::vector<action>* action_sequence)
  {
    int step = 1;
    while (!task.final(s0)) {
      size_t action = searn_predict(all, s0, step, allow_oracle, allow_current, NULL);
      if (track_actions)
        action_sequence->push_back(action);

      task.step(s0, action);
      step++;
    }
  }
  
  struct beam_info_struct {
    vw&all;
    bool allow_oracle;
    bool allow_current;
  };

  void run_prediction_beam_iter(Beam::beam*b, size_t bucket_id, state s0, float cur_loss, void*args)
  {
    beam_info_struct* bi = (beam_info_struct*)args;

    if (task.final(s0)) return;

    v_array< pair<size_t,float> > partial_predictions;
    searn_predict(bi->all, s0, bucket_id, bi->allow_oracle, bi->allow_current, &partial_predictions);
    for (size_t i=0; i<partial_predictions.index(); i++) {
      state s1 = task.copy(s0);
      float new_loss = cur_loss + partial_predictions[i].second;
      size_t action = partial_predictions[i].first;
      task.step( s1, action );
      b->put( task.bucket(s1), s1, action, new_loss );
    }
  }

  void run_prediction_beam(vw&all, size_t max_beam_size, state s0, bool allow_oracle, bool allow_current, bool track_actions, std::vector<action>* action_sequence)
  {
    Beam::beam *b = new Beam::beam(task.equivalent, task.hash, max_beam_size);

    beam_info_struct bi = { all, allow_oracle, allow_current };

    b->put(task.bucket(s0), s0, 0, 0.);
    size_t current_bucket = 0;
    while (true) {
      current_bucket = b->get_next_bucket(current_bucket);
      if (current_bucket == 0) break;
      b->iterate(current_bucket, run_prediction_beam_iter, &bi);
    }

    if (track_actions && (action_sequence != NULL))
      b->get_best_output(action_sequence);

    delete b;
  }

  //  void hm_free_state_copies(state s, action a) { 
  //    task.finish(s);
  //  }


  void do_actual_learning(vw&all)
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

    state s0copy = NULL;
    bool  is_test = task.is_test_example(ec_seq.begin, ec_seq.index());
    if (!is_test) {
      s0copy = task.copy(s0);
      all.sd->example_number++;
      all.sd->total_features    += searn_num_features;
      all.sd->weighted_examples += 1.;
    }
    bool will_print = is_test || should_print_update(all);

    searn_num_features = 0;
    std::vector<action> action_sequence;
    if (will_print)
      action_sequence = std::vector<action>();

    // if we are using adaptive beta, update it to take into account the latest updates
    if( adaptive_beta ) beta = 1.f - powf(1.f - alpha,(float)total_examples_generated);
    
    run_prediction(all, s0, false, true, will_print, &action_sequence);

    if (is_test) {
      global_print_label(all, ec_seq[0], s0, action_sequence);
    }

    if (!is_test) {
      float loss = task.loss(s0);
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
    }

    print_update(all, s0, action_sequence);
    
    task.finish(s0);

    if (is_test || !all.training)
      return;

    s0 = s0copy;

    // training examples only get here
    int step = 1;
    while (!task.final(s0)) {
      // if we are using adaptive beta, update it to take into account the latest updates
      if( adaptive_beta ) beta = 1.f - powf(1.f - alpha,(float)total_examples_generated);

      // first, make a prediction (we don't want to bias ourselves if
      // we're using the current policy to predict)
      size_t action = searn_predict(all, s0, step, true, allow_current_policy, NULL);

      // generate training example for the current state
      generate_state_example(all, s0);

      // take the prescribed step
      task.step(s0, action);

      step++;
    }
    task.finish(s0);
    if (do_recombination) {  // we need to free a bunch of memory
      //      past_states->iter(&hm_free_state_copies);
      free_unfreed_states();
      past_states->clear();
    }
  }

  void process_next_example(vw&all, example *ec)
  {
    bool is_real_example = true;

    if (is_singleline) {
      if (ec_seq.index() == 0)
        push(ec_seq, ec);
      else
        ec_seq[0] = ec;

      do_actual_learning(all);
    } else {  
      // is multiline
      if (ec_seq.index() >= all.p->ring_size - 2) { // give some wiggle room
        std::cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << std::endl;
        do_actual_learning(all);
        clear_seq(all);
      }

      if (OAA::example_is_newline(ec)) {
        do_actual_learning(all);
        clear_seq(all);
        //CSOAA_LDF::global_print_newline(all);
	VW::finish_example(all, ec);
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
          if(all.training)
            current_policy++;
          if (current_policy > total_number_of_policies) {
            std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
            current_policy = total_number_of_policies;
          }
          //reset searn_trained_nb_policies in options_from_file so it is saved to regressor file later
          std::stringstream ss;
          ss << current_policy;
          VW::cmd_string_replace_value(all.options_from_file,"--searn_trained_nb_policies", ss.str());
        }
      }
    }
  }

  void drive(void*in)
  {
    vw*all = (vw*)in;
    // initialize everything
    
    const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";

    fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
    fprintf(stderr, header_fmt, "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
    cerr.precision(5);

    initialize_memory();

    example* ec = NULL;
    read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { // semiblocking operation
        process_next_example(*all, ec);
      } else if (parser_done(all->p)) {
        if (!is_singleline)
          do_actual_learning(*all);
        break;
      }
    }

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << (current_policy+1);
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_trained_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_trained_nb_policies", ss1.str()); 
      ss2 << total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_total_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_total_nb_policies", ss2.str());
    }
  }


}


