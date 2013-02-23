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
  uint32_t history_constant    = 8290741;
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

  void add_policy_offset(vw&all, example *ec, uint32_t increment, uint32_t policy)
  {
    update_example_indicies(all.audit, ec, policy * increment);
  }

  void remove_policy_offset(vw&all, example *ec, uint32_t increment, uint32_t policy)
  {
    update_example_indicies(all.audit, ec, -policy * increment);
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

  void add_history_to_example(vw&all, history_info *hinfo, example* ec, history h)
  {
    size_t v0, v, max_string_length = 0;
    size_t total_length = max(hinfo->features, hinfo->length);

    if (total_length == 0) return;
    if (h == NULL) {
      cerr << "error: got empty history in add_history_to_example" << endl;
      throw exception();
    }

    if (all.audit) {
      max_string_length = max((int)(ceil( log10((float)total_length+1) )),
                              (int)(ceil( log10((float)MAX_ACTION_ID+1) ))) + 1;
    }

    for (size_t t=1; t<=total_length; t++) {
      v0 = (h[hinfo->length-t] * quadratic_constant + t) * quadratic_constant + history_constant;

      // add the basic history features
      feature temp = {1., (uint32_t) ( (2*v0) & all.parse_mask )};
      ec->atomics[history_namespace].push_back(temp);

      if (all.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & all.parse_mask), 1., true };
        a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)calloc_or_die(5 + 2*max_string_length, sizeof(char));
        sprintf(a_feature.feature, "ug@%d=%d", (int)t, (int)h[hinfo->length-t]);

        ec->audit_features[history_namespace].push_back(a_feature);
      }

      // add the bigram features
      if ((t > 1) && hinfo->bigrams) {
        v0 = ((v0 - history_constant) * quadratic_constant + h[hinfo->length-t+1]) * quadratic_constant + history_constant;

        feature temp = {1., (uint32_t) ( (2*v0) & all.parse_mask )};
        ec->atomics[history_namespace].push_back(temp);

        if (all.audit) {
          audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & all.parse_mask), 1., true };
          a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
          strcpy(a_feature.space, audit_feature_space.c_str());

          a_feature.feature = (char*)calloc_or_die(6 + 3*max_string_length, sizeof(char));
          sprintf(a_feature.feature, "bg@%d=%d-%d", (int)t-1, (int)h[hinfo->length-t], (int)h[hinfo->length-t+1]);

          ec->audit_features[history_namespace].push_back(a_feature);
        }

      }
    }

    string fstring;

    if (hinfo->features > 0) {
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

          for (size_t t=1; t<=hinfo->features; t++) {
            v0 = (h[hinfo->length-t] * quadratic_constant + t) * quadratic_constant;
          
            // add the history/feature pair
            feature temp = {1., (uint32_t) ( (2*(v0 + v)) & all.parse_mask )};
            ec->atomics[history_namespace].push_back(temp);

            if (all.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & all.parse_mask), 1., true };
              a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)calloc_or_die(8 + 2*max_string_length + fstring.length(), sizeof(char));
              sprintf(a_feature.feature, "ug+f@%d=%d=%s", (int)t, (int)h[hinfo->length-t], fstring.c_str());

              ec->audit_features[history_namespace].push_back(a_feature);
            }


            // add the bigram
            if ((t > 0) && hinfo->bigram_features) {
              v0 = (v0 + h[hinfo->length-t+1]) * quadratic_constant;

              feature temp = {1., (uint32_t) ( (2*(v + v0)) & all.parse_mask )};
              ec->atomics[history_namespace].push_back(temp);

              if (all.audit) {
                audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & all.parse_mask), 1., true };
                a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
                strcpy(a_feature.space, audit_feature_space.c_str());

                a_feature.feature = (char*)calloc_or_die(9 + 3*max_string_length + fstring.length(), sizeof(char));
                sprintf(a_feature.feature, "bg+f@%d=%d-%d=%s", (int)t-1, (int)h[hinfo->length-t], (int)h[hinfo->length-t+1], fstring.c_str());

                ec->audit_features[history_namespace].push_back(a_feature);
              }

            }
          }
        }
      }
    }

    ec->indices.push_back(history_namespace);
    ec->sum_feat_sq[history_namespace] += ec->atomics[history_namespace].size();
    ec->total_sum_feat_sq += ec->sum_feat_sq[history_namespace];
    ec->num_features += ec->atomics[history_namespace].size();
  }

  void remove_history_from_example(vw&all, history_info *hinfo, example* ec)
  {
    size_t total_length = max(hinfo->features, hinfo->length);
    if (total_length == 0) return;

    if (ec->indices.size() == 0) {
      cerr << "internal error (bug): trying to remove history, but there are no namespaces!" << endl;
      return;
    }

    if (ec->indices.last() != history_namespace) {
      cerr << "internal error (bug): trying to remove history, but either it wasn't added, or something was added after and not removed!" << endl;
      return;
    }

    ec->num_features -= ec->atomics[history_namespace].size();
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
  // rollout
  struct rollout_item {
    state st;
    bool  is_finished;
    bool  alive;
    size_t hash;
  };

    // debug stuff
  const bool PRINT_DEBUG_INFO =0;
  const bool PRINT_UPDATE_EVERY_EXAMPLE =0;
    
  struct searn {
    // task stuff
    search_task task;
    bool is_singleline;
    bool is_ldf;
    bool has_hash;
    bool constrainted_actions;
    size_t input_label_size;
    
    // options
    size_t max_action;
    size_t max_rollout;
    size_t passes_per_policy;     //this should be set to the same value as --passes for dagger
    float  beta;
    float gamma;
    bool   do_recombination;
    bool   allow_current_policy; //this should be set to true for dagger
    bool   rollout_oracle; //if true then rollout are performed using oracle instead (optimal approximation discussed in searn's paper). this should be set to true for dagger
    bool   adaptive_beta; //used to implement dagger through searn. if true, beta = 1-(1-alpha)^n after n updates, and policy is mixed with oracle as \pi' = (1-beta)\pi^* + beta \pi
    float  alpha; //parameter used to adapt beta for dagger (see above comment), should be in (0,1)
    bool   rollout_all_actions;  //by default we rollout all actions. This is set to false when searn is used with a contextual bandit base learner, where we rollout only one sampled action
    
    // memory
    rollout_item* rollout;
    v_array<example*> ec_seq;
    example** global_example_set;
    example* empty_example;
    OAA::mc_label empty_label;
    CSOAA::label loss_vector;
    CB::label loss_vector_cb;
    v_array<void*>old_labels;
    v_array<OAA::mc_label>new_labels;
    CSOAA::label testall_labels;
    CSOAA::label allowed_labels;
    CB::label testall_labels_cb;
    CB::label allowed_labels_cb;
    // we need a hashmap that maps from STATES to ACTIONS
    v_hashmap<state,action> *past_states;
    v_array<state> unfreed_states;
    
    // tracking of example
    size_t read_example_this_loop;
    size_t read_example_last_id;
    size_t passes_since_new_policy;
    size_t read_example_last_pass;
    size_t total_examples_generated;
    size_t total_predictions_made;
    size_t searn_num_features;
    
    // variables
    uint32_t current_policy;
    uint32_t total_number_of_policies;
    uint32_t increment; //for policy offset
    
    learner base;
  };

  void drive(void*in, void*d);
  
  void simple_print_example_features(vw&all, example *ec)
  {
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
      {
        feature* end = ec->atomics[*i].end;
        for (feature* f = ec->atomics[*i].begin; f!= end; f++) {
          cerr << "\t" << f->weight_index << ":" << f->x << ":" << all.reg.weight_vector[f->weight_index & all.weight_mask];
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

  bool will_global_print_label(vw& all, searn& s)
  {
    if (!s.task.to_string) return false;
    if (all.final_prediction_sink.size() == 0) return false;
    return true;
  }

  void global_print_label(vw& all, searn& s, example*ec, state s0, std::vector<action> last_action_sequence)
  {
    if (!s.task.to_string) return;
    if (all.final_prediction_sink.size() == 0) return;

    string str = s.task.to_string(s0, false, last_action_sequence);
    for (size_t i=0; i<all.final_prediction_sink.size(); i++) {
      int f = all.final_prediction_sink[i];
      all.print_text(f, str, ec->tag);
    }
  }

  void print_update(vw& all, searn& s, state s0, std::vector<action> last_action_sequence)
  {
    if (!should_print_update(all))
      return;

    char true_label[21];
    char pred_label[21];
    if (s.task.to_string) {
      to_short_string(s.task.to_string(s0, true , empty_action_vector ), 20, true_label);
      to_short_string(s.task.to_string(s0, false, last_action_sequence), 20, pred_label);
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
            (long unsigned int)s.searn_num_features,
            (int)s.read_example_last_pass,
            (int)s.current_policy,
            (long unsigned int)s.total_predictions_made,
            (long unsigned int)s.total_examples_generated);

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    all.sd->dump_interval *= 2;
  }

  void clear_seq(vw&all, searn& s)
  {
    if (s.ec_seq.size() > 0) 
      for (example** ecc=s.ec_seq.begin; ecc!=s.ec_seq.end; ecc++) {
	VW::finish_example(all, *ecc);
      }
    s.ec_seq.erase();
  }

  void free_unfreed_states(searn& s)
  {
    while (!s.unfreed_states.empty()) {
      state st = s.unfreed_states.pop();
      s.task.finish(st);
    }
  }

  void initialize_memory(searn& s)
  {
    // initialize searn's memory
    s.rollout = (rollout_item*)SearnUtil::calloc_or_die(s.max_action, sizeof(rollout_item));
    s.global_example_set = (example**)SearnUtil::calloc_or_die(s.max_action, sizeof(example*));

    for (uint32_t k=1; k<=s.max_action; k++) {
      CSOAA::wclass cost = { FLT_MAX, k, 1., 0. };
      s.testall_labels.costs.push_back(cost);
      CB::cb_class cost_cb = { FLT_MAX, k, 0. };
      s.testall_labels_cb.costs.push_back(cost_cb);
    }

    s.empty_example = alloc_example(sizeof(OAA::mc_label));
    OAA::default_label(s.empty_example->ld);
    //    cerr << "create: empty_example->ld = " << empty_example->ld << endl;
    s.empty_example->in_use = true;
  }
  
  void free_memory(vw&all, searn& s)
  {
    dealloc_example(NULL, *s.empty_example);
    free(s.empty_example);

    SearnUtil::free_it(s.rollout);

    s.loss_vector.costs.delete_v();

    s.old_labels.delete_v();

    s.new_labels.delete_v();

    free_unfreed_states(s);
    s.unfreed_states.delete_v();

    s.ec_seq.delete_v();

    SearnUtil::free_it(s.global_example_set);

    s.testall_labels.costs.delete_v();
    s.testall_labels_cb.costs.delete_v();
    s.allowed_labels.costs.delete_v();
    s.allowed_labels_cb.costs.delete_v();

    if (s.do_recombination) {
      delete s.past_states;
      s.past_states = NULL;
    }
  }



  void learn(void*in, void*d, example *ec)
  {
    //vw*all = (vw*)in;
    // TODO
  }

  void finish(void*in, void*d)
  {
    searn* s = (searn*)d;
    s->base.finish(in,s->base.data);
    vw*all = (vw*)in;
    // free everything
    if (s->task.finalize != NULL)
      s->task.finalize();
    free_memory(*all,*s);
    free(s);
  }

  void parse_flags(vw&all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    searn* s = (searn*)calloc(1,sizeof(searn));

    s->max_action           = 1;
    s->max_rollout          = INT_MAX;
    s->passes_per_policy    = 1;     //this should be set to the same value as --passes for dagger
    s->beta                 = 0.5;
    s->gamma                = 1.;
    s->alpha                = 0.001f; //parameter used to adapt beta for dagger (see above comment), should be in (0,1)
    s->rollout_all_actions  = true;  //by default we rollout all actions. This is set to false when searn is used with a contextual bandit base learner, where we rollout only one sampled action
    s->ec_seq = v_array<example*>();
    s->loss_vector.costs = v_array<CSOAA::wclass>();
    s->loss_vector_cb.costs = v_array<CB::cb_class>();
    s->old_labels = v_array<void*>();
    s->new_labels = v_array<OAA::mc_label>();
    s->testall_labels.costs = v_array<CSOAA::wclass>();
    s->allowed_labels.costs = v_array<CSOAA::wclass>();
    s->testall_labels_cb.costs = v_array<CB::cb_class>();
    s->allowed_labels_cb.costs = v_array<CB::cb_class>();
    // we need a hashmap that maps from STATES to ACTIONS
    s->unfreed_states = v_array<state>();
    s->total_number_of_policies = 1;

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
        throw exception();
      }
      task_string = vm["searn_task"].as<std::string>();

      //append the searn task to options_from_file so it is saved in the regressor file later
      all.options_from_file.append(" --searn_task ");
      all.options_from_file.append(task_string);
    }

    if (task_string.compare("sequence") == 0) {
      s->task.final = SequenceTask::final;
      s->task.loss = SequenceTask::loss;
      s->task.step = SequenceTask::step; 
      s->task.oracle = SequenceTask::oracle;
      s->task.copy = SequenceTask::copy;
      s->task.finish = SequenceTask::finish; 
      s->task.searn_label_parser = OAA::mc_label_parser;
      s->task.is_test_example = SequenceTask::is_test_example;
      s->input_label_size = sizeof(OAA::mc_label);
      s->task.start_state = NULL;
      s->task.start_state_multiline = SequenceTask::start_state_multiline;
      if (1) {
        s->task.cs_example = SequenceTask::cs_example;
        s->task.cs_ldf_example = NULL;
      } else {
        s->task.cs_example = NULL;
        s->task.cs_ldf_example = SequenceTask::cs_ldf_example;
      }
      s->task.initialize = SequenceTask::initialize;
      s->task.finalize = NULL;
      s->task.equivalent = SequenceTask::equivalent;
      s->task.hash = SequenceTask::hash;
      s->task.allowed = SequenceTask::allowed;
      s->task.to_string = SequenceTask::to_string;
    } else {
      std::cerr << "error: unknown search task '" << task_string << "'" << std::endl;
      throw exception();
    }

    *(all.p->lp)=s->task.searn_label_parser;

    if(vm_file.count("searn")) { //we loaded searn flag from regressor file 
      s->max_action = vm_file["searn"].as<size_t>();
      if( vm.count("searn") && vm["searn"].as<size_t>() != s->max_action )
        std::cerr << "warning: you specified a different number of actions through --searn than the one loaded from predictor. Pursuing with loaded value of: " << s->max_action << endl;
    }
    else {
      s->max_action = vm["searn"].as<size_t>();

      //append searn with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --searn " << s->max_action;
      all.options_from_file.append(ss.str());
    }

    if(vm_file.count("searn_beta")) { //we loaded searn_beta flag from regressor file 
      s->beta = vm_file["searn_beta"].as<float>();
      if (vm.count("searn_beta") && vm["searn_beta"].as<float>() != s->beta )
        std::cerr << "warning: you specified a different value through --searn_beta than the one loaded from predictor. Pursuing with loaded value of: " << s->beta << endl;

    }
    else {
      if (vm.count("searn_beta")) s->beta = vm["searn_beta"].as<float>();

      //append searn_beta to options_from_file so it is saved in the regressor file later
      std::stringstream ss;
      ss << " --searn_beta " << s->beta;
      all.options_from_file.append(ss.str());
    }

    if (vm.count("searn_rollout"))                 s->max_rollout          = vm["searn_rollout"].as<size_t>();
    if (vm.count("searn_passes_per_policy"))       s->passes_per_policy    = vm["searn_passes_per_policy"].as<size_t>();
      
    if (vm.count("searn_gamma"))                   s->gamma                = vm["searn_gamma"].as<float>();
    if (vm.count("searn_norecombine"))             s->do_recombination     = false;
    if (vm.count("searn_allow_current_policy"))    s->allow_current_policy = true;
    if (vm.count("searn_rollout_oracle"))    	   s->rollout_oracle       = true;

    //check if the base learner is contextual bandit, in which case, we dont rollout all actions.
    if ( vm.count("cb") || vm_file.count("cb") ) s->rollout_all_actions = false;

    //if we loaded a regressor with -i option, --searn_trained_nb_policies contains the number of trained policies in the file
    // and --searn_total_nb_policies contains the total number of policies in the file
    if ( vm_file.count("searn_total_nb_policies") )
    {
      s->current_policy = (uint32_t)vm_file["searn_trained_nb_policies"].as<size_t>();
      s->total_number_of_policies = (uint32_t)vm_file["searn_total_nb_policies"].as<size_t>();
      if (vm.count("searn_total_nb_policies") && (uint32_t)vm["searn_total_nb_policies"].as<size_t>() != s->total_number_of_policies)
          std::cerr << "warning: --searn_total_nb_policies doesn't match the total number of policies stored in initial predictor. Using loaded value of: " << s->total_number_of_policies << endl;
    }
    else if (vm.count("searn_total_nb_policies"))
    {
      s->total_number_of_policies = (uint32_t)vm["searn_total_nb_policies"].as<size_t>();
    }

    if (vm.count("searn_as_dagger"))
    {
      //overide previously loaded options to set searn as dagger
      s->allow_current_policy = true;
      s->passes_per_policy = all.numpasses;
      //s->rollout_oracle = true;
      if( s->current_policy > 1 ) 
        s->current_policy = 1;

      //indicate to adapt beta for each update
      s->adaptive_beta = true;
      s->alpha = vm["searn_as_dagger"].as<float>();
    }

    if (s->beta <= 0 || s->beta >= 1) {
      std::cerr << "warning: searn_beta must be in (0,1); resetting to 0.5" << std::endl;
      s->beta = 0.5;
    }

    if (s->gamma <= 0 || s->gamma > 1) {
      std::cerr << "warning: searn_gamma must be in (0,1); resetting to 1.0" << std::endl;
      s->gamma = 1.0;
    }

    if (s->alpha < 0 || s->alpha > 1) {
      std::cerr << "warning: searn_adaptive_beta must be in (0,1); resetting to 0.001" << std::endl;
      s->alpha = 0.001f;
    }

    if (s->task.initialize != NULL)
      if (!s->task.initialize(all, opts, vm, vm_file)) {
        std::cerr << "error: task did not initialize properly" << std::endl;
        throw exception();
      }

    // check to make sure task is valid and set up our variables
    if (s->task.final  == NULL ||
        s->task.loss   == NULL ||
        s->task.step   == NULL ||
        s->task.oracle == NULL ||
        s->task.copy   == NULL ||
        s->task.finish == NULL ||
        ((s->task.start_state == NULL) == (s->task.start_state_multiline == NULL)) ||
        ((s->task.cs_example  == NULL) == (s->task.cs_ldf_example        == NULL))) {
      std::cerr << "error: searn task malformed" << std::endl;
      throw exception();
    }

    s->is_singleline  = (s->task.start_state != NULL);
    s->is_ldf         = (s->task.cs_example  == NULL);
    s->has_hash       = (s->task.hash        != NULL);
    s->constrainted_actions = (s->task.allowed != NULL);

    if (s->do_recombination && (s->task.hash == NULL)) {
      std::cerr << "warning: cannot do recombination when hashing is unavailable -- turning off recombination" << std::endl;
      s->do_recombination = false;
    }
    if (s->do_recombination) {
      // 0 is an invalid action
      s->past_states = new v_hashmap<state,action>(1023, 0, s->task.equivalent);
    }

    if (s->is_ldf && !s->constrainted_actions) {
      std::cerr << "error: LDF requires allowed" << std::endl;
      throw exception();
    }

    all.searn = true;

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    uint32_t tmp_number_of_policies = s->current_policy; 
    if( all.training )
	tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)s->passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls, 
    //so only set total_number_of_policies to computed value if it is larger
    if( tmp_number_of_policies > s->total_number_of_policies )
    {
	s->total_number_of_policies = tmp_number_of_policies;
        if( s->current_policy > 0 ) //we loaded a file but total number of policies didn't match what is needed for training
        {
          std::cerr << "warning: you're attempting to train more classifiers than was allocated initially. Likely to cause bad performance." << endl;
        }  
    }

    //current policy currently points to a new policy we would train
    //if we are not training and loaded a bunch of policies for testing, we need to subtract 1 from current policy
    //so that we only use those loaded when testing (as run_prediction is called with allow_current to true)
    if( !all.training && s->current_policy > 0 )
	s->current_policy--;

    //std::cerr << "Current Policy: " << s->current_policy << endl;
    //std::cerr << "Total Number of Policies: " << total_number_of_policies << endl;

    std::stringstream ss1;
    std::stringstream ss2;
    ss1 << s->current_policy;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_trained_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--searn_trained_nb_policies", ss1.str()); 
    ss2 << s->total_number_of_policies;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_total_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--searn_total_nb_policies", ss2.str());

    all.base_learner_nb_w *= s->total_number_of_policies;
    s->increment = ((uint32_t)all.length() / all.base_learner_nb_w) * all.stride;
    //cerr << "searn increment = " << s->increment << endl;
    
    learner l = {s, drive, learn, finish, all.l.save_load};
    s->base = all.l;
    all.l = l;
  }

  uint32_t searn_predict(vw&all, searn& s, state s0, size_t step, bool allow_oracle, bool allow_current, v_array< pair<uint32_t,float> >* partial_predictions)  // TODO: partial_predictions
  {
    int policy = SearnUtil::random_policy(s.read_example_last_id * 2147483 + step * 2147483647 /* s.has_hash ? s.task.hash(s0) : step */, s.beta, allow_current, (int)s.current_policy, allow_oracle, s.rollout_all_actions);
    if (PRINT_DEBUG_INFO) { cerr << "predicing with policy " << policy << " (allow_oracle=" << allow_oracle << ", allow_current=" << allow_current << "), current_policy=" << s.current_policy << endl; }
    if (policy == -1) {
      return s.task.oracle(s0);
    }

    example *ec;

    if (!s.is_ldf) {
      s.task.cs_example(all, s0, ec, true);
      SearnUtil::add_policy_offset(all, ec, s.increment, policy);

      void* old_label = ec->ld;
      if(s.rollout_all_actions) { //this means we have a cost-sensitive base learner
        ec->ld = (void*)&s.testall_labels;
        if (s.task.allowed != NULL) {  // we need to check which actions are allowed
          s.allowed_labels.costs.erase();
          bool all_allowed = true;
          for (uint32_t k=1; k<=s.max_action; k++)
            if (s.task.allowed(s0, k)) {
              CSOAA::wclass cost = { FLT_MAX, k, 1., 0. };
              s.allowed_labels.costs.push_back(cost);
            } else
              all_allowed = false;

          if (!all_allowed)
            ec->ld = (void*)&s.allowed_labels;
        }
      }
      else { //if we have a contextual bandit base learner
        ec->ld = (void*)&s.testall_labels_cb;
        if (s.task.allowed != NULL) {  // we need to check which actions are allowed
          s.allowed_labels_cb.costs.erase();
          bool all_allowed = true;
          for (uint32_t k=1; k<=s.max_action; k++)
            if (s.task.allowed(s0, k)) {
              CB::cb_class cost = { FLT_MAX, k, 0. };
              s.allowed_labels_cb.costs.push_back(cost);
            } else
              all_allowed = false;

          if (!all_allowed)
            ec->ld = (void*)&s.allowed_labels_cb;
        }
      }
      //cerr << "searn>";
      //simple_print_example_features(all,ec);
      s.base.learn(&all,s.base.data,ec); 
      s.total_predictions_made++;  
      s.searn_num_features += ec->num_features;
      uint32_t final_prediction = (uint32_t)ec->final_prediction;
      ec->ld = old_label;

      SearnUtil::remove_policy_offset(all, ec, s.increment, policy);
      s.task.cs_example(all, s0, ec, false);

      return final_prediction;
    } else {  // is_ldf
      //TODO: modify this to handle contextual bandit base learner with ldf
      float best_prediction = 0;
      uint32_t best_action = 0;
      for (uint32_t action=1; action <= s.max_action; action++) {
        if (!s.task.allowed(s0, action))
          break;   // for LDF, there are no more actions

        s.task.cs_ldf_example(all, s0, action, ec, true);
        //cerr << "created example: " << ec << ", label: " << ec->ld << endl;
        SearnUtil::add_policy_offset(all, ec, s.increment, policy);
        s.base.learn(&all,s.base.data,ec);  s.total_predictions_made++;  s.searn_num_features += ec->num_features;
        //cerr << "base_learned on example: " << ec << endl;
        s.empty_example->in_use = true;
        s.base.learn(&all,s.base.data,s.empty_example);
        //cerr << "base_learned on empty example: " << s.empty_example << endl;
        SearnUtil::remove_policy_offset(all, ec, s.increment, policy);

        if (action == 1 || 
            ec->partial_prediction < best_prediction) {
          best_prediction = ec->partial_prediction;
          best_action     = action;
        }
        //cerr << "releasing example: " << ec << ", label: " << ec->ld << endl;
        s.task.cs_ldf_example(all, s0, action, ec, false);
      }

      if (best_action < 1) {
        std::cerr << "warning: internal error on search -- could not find an available action; quitting!" << std::endl;
        throw exception();
      }
      return best_action;
    }
  }

  float single_rollout(vw&all, searn& s, state s0, uint32_t action)
  {
    //first check if action is valid for current state
    if( action < 1 || action > s.max_action || (s.task.allowed && !s.task.allowed(s0,action)) )
    {
	std::cerr << "warning: asked to rollout an unallowed action: " << action << "; not performing rollout." << std::endl;
	return 0;
    }
    
    //copy state and step it with current action
    s.rollout[action-1].alive = true;
    s.rollout[action-1].st = s.task.copy(s0);
    s.task.step(s.rollout[action-1].st, action);
    s.rollout[action-1].is_finished = s.task.final(s.rollout[action-1].st);
    if (s.do_recombination) s.rollout[action-1].hash = s.task.hash(s.rollout[action-1].st);

    //if not finished complete rollout
    if (!s.rollout[action-1].is_finished) {
      for (size_t step=1; step<s.max_rollout; step++) {
        uint32_t act_tmp = 0;
        if (s.do_recombination)
          act_tmp = s.past_states->get(s.rollout[action-1].st, s.rollout[action-1].hash);

        if (act_tmp == 0) {  // this means we didn't find it or we're not recombining
          if( !s.rollout_oracle )
            act_tmp = searn_predict(all, s, s.rollout[action-1].st, step, true, s.allow_current_policy, NULL);
	  else
            act_tmp = s.task.oracle(s.rollout[action-1].st);

          if (s.do_recombination) {
            // we need to make a copy of the state
            state copy = s.task.copy(s.rollout[action-1].st);
            s.past_states->put_after_get(copy, s.rollout[action-1].hash, act_tmp);
            s.unfreed_states.push_back(copy);
          }
        }          
          
        s.task.step(s.rollout[action-1].st, act_tmp);
        s.rollout[action-1].is_finished = s.task.final(s.rollout[action-1].st);
        if (s.do_recombination) s.rollout[action-1].hash = s.task.hash(s.rollout[action-1].st);
        if (s.rollout[action-1].is_finished) break;
      }
    }

    // finally, compute losses and free copies
    float l = s.task.loss(s.rollout[action-1].st);
    if ((l == FLT_MAX) && (!s.rollout[action-1].is_finished) && (s.max_rollout < INT_MAX)) {
      std::cerr << "error: you asked for short rollouts, but your task does not support pre-final losses" << std::endl;
      throw exception();
    }
    s.task.finish(s.rollout[action-1].st);

    return l;
  }

  void parallel_rollout(vw&all, searn& s, state s0)
  {
    // first, make K copies of s0 and step them
    bool all_finished = true;
    for (size_t k=1; k<=s.max_action; k++) 
      s.rollout[k-1].alive = false;
    
    for (uint32_t k=1; k<=s.max_action; k++) {
      // in the case of LDF, we might run out of actions early
      if (s.task.allowed && !s.task.allowed(s0, k)) {
        if (s.is_ldf) break;
        else continue;
      }
      s.rollout[k-1].alive = true;
      s.rollout[k-1].st = s.task.copy(s0);
      s.task.step(s.rollout[k-1].st, k);
      s.rollout[k-1].is_finished = s.task.final(s.rollout[k-1].st);
      if (s.do_recombination) s.rollout[k-1].hash = s.task.hash(s.rollout[k-1].st);
      all_finished = all_finished && s.rollout[k-1].is_finished;
    }

    // now, complete all rollouts
    if (!all_finished) {
      for (size_t step=1; step<s.max_rollout; step++) {
        all_finished = true;
        for (size_t k=1; k<=s.max_action; k++) {
          if (s.rollout[k-1].is_finished) continue;
          
          uint32_t action = 0;
          if (s.do_recombination)
            action = s.past_states->get(s.rollout[k-1].st, s.rollout[k-1].hash);

          if (action == 0) {  // this means we didn't find it or we're not recombining
            if( !s.rollout_oracle )
              action = searn_predict(all, s, s.rollout[k-1].st, step, true, s.allow_current_policy, NULL);
	    else
              action = s.task.oracle(s.rollout[k-1].st);

            if (s.do_recombination) {
              // we need to make a copy of the state
              state copy = s.task.copy(s.rollout[k-1].st);
              s.past_states->put_after_get(copy, s.rollout[k-1].hash, action);
              s.unfreed_states.push_back(copy);
            }
          }          
          
          s.task.step(s.rollout[k-1].st, action);
          s.rollout[k-1].is_finished = s.task.final(s.rollout[k-1].st);
          if (s.do_recombination) s.rollout[k-1].hash = s.task.hash(s.rollout[k-1].st);
          all_finished = all_finished && s.rollout[k-1].is_finished;
        }
        if (all_finished) break;
      }
    }

    // finally, compute losses and free copies
    float min_loss = 0;
    s.loss_vector.costs.erase();
    for (uint32_t k=1; k<=s.max_action; k++) {
      if (!s.rollout[k-1].alive)
        break;

      float l = s.task.loss(s.rollout[k-1].st);
      if ((l == FLT_MAX) && (!s.rollout[k-1].is_finished) && (s.max_rollout < INT_MAX)) {
        std::cerr << "error: you asked for short rollouts, but your task does not support pre-final losses" << std::endl;
        throw exception();
      }

      CSOAA::wclass temp = { l, k, 1., 0. };
      s.loss_vector.costs.push_back(temp);
      if ((k == 1) || (l < min_loss)) { min_loss = l; }

      s.task.finish(s.rollout[k-1].st);
    }

    // subtract the smallest loss
    for (size_t k=1; k<=s.max_action; k++)
      if (s.rollout[k-1].alive)
        s.loss_vector.costs[k-1].x -= min_loss;
  }

  uint32_t uniform_exploration(searn& s, state s0, float& prob_sampled_action)
  {
    //find how many valid actions
    size_t nb_allowed_actions = s.max_action;
    if( s.task.allowed ) {  
      for (uint32_t k=1; k<=s.max_action; k++) {
        if( !s.task.allowed(s0,k) ) {
          nb_allowed_actions--;
          if (s.is_ldf) {
            nb_allowed_actions = k-1;
            break;
          }
        }
      }
    }

    uint32_t action = (size_t)(frand48() * nb_allowed_actions) + 1;
    if( s.task.allowed && nb_allowed_actions < s.max_action && !s.is_ldf) {
      //need to adjust action to the corresponding valid action
      for (uint32_t k=1; k<=action; k++) {
        if( !s.task.allowed(s0,k) ) action++;
      }
    }
    prob_sampled_action = (float) (1.0/nb_allowed_actions);
    return action;
  }

  void get_contextual_bandit_loss_vector(vw&all, searn& s, state s0)
  {
    float prob_sampled = 1.;
    uint32_t act = uniform_exploration(s, s0,prob_sampled);
    float loss = single_rollout(all,s, s0,act);

    s.loss_vector_cb.costs.erase();
    for (uint32_t k=1; k<=s.max_action; k++) {
      if( s.task.allowed && !s.task.allowed(s0,k))
	break;
      
      CB::cb_class temp;
      temp.x = FLT_MAX;
      temp.weight_index = k;
      temp.prob_action = 0.;
      if( act == k ) {
        temp.x = loss;
        temp.prob_action = prob_sampled;
      }
      s.loss_vector_cb.costs.push_back(temp);
    }
  }

  void generate_state_example(vw&all, searn& s, state s0)
  {
    // start by doing rollouts so we can get costs
    s.loss_vector.costs.erase();
    s.loss_vector_cb.costs.erase();
    if( s.rollout_all_actions ) {
      parallel_rollout(all, s, s0);      
    }
    else {
      get_contextual_bandit_loss_vector(all, s, s0);
    }

    if (s.loss_vector.costs.size() <= 1 && s.loss_vector_cb.costs.size() == 0) {
      // nothing interesting to do!
      return;
    }

    // now, generate training examples
    if (!s.is_ldf) {
      s.total_examples_generated++;

      example* ec;
      s.task.cs_example(all, s0, ec, true);
      void* old_label = ec->ld;

      if(s.rollout_all_actions) 
        ec->ld = (void*)&s.loss_vector;
      else 
        ec->ld = (void*)&s.loss_vector_cb;
 
      SearnUtil::add_policy_offset(all, ec, s.increment, s.current_policy);
	  
	  s.base.learn(&all,s.base.data,ec);
      SearnUtil::remove_policy_offset(all, ec, s.increment, s.current_policy);
      ec->ld = old_label;
      s.task.cs_example(all, s0, ec, false);
    } else { // is_ldf
      //TODO: support ldf with contextual bandit base learner
      s.old_labels.erase();
      s.new_labels.erase();

      for (uint32_t k=1; k<=s.max_action; k++) {
        if (s.rollout[k-1].alive) {
          OAA::mc_label ld = { (float)k, s.loss_vector.costs[k-1].x };
          s.new_labels.push_back(ld);
        } else {
          OAA::mc_label ld = { (float)k, 0. };
          s.new_labels.push_back(ld);
        }
      }

      //      cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;

      for (uint32_t k=1; k<=s.max_action; k++) {
        if (!s.rollout[k-1].alive) break;

        s.total_examples_generated++;

        s.task.cs_ldf_example(all, s0, k, s.global_example_set[k-1], true);
        s.old_labels.push_back(s.global_example_set[k-1]->ld);
        s.global_example_set[k-1]->ld = (void*)(&s.new_labels[k-1]);
        SearnUtil::add_policy_offset(all, s.global_example_set[k-1], s.increment, s.current_policy);
        if (PRINT_DEBUG_INFO) { cerr << "add_policy_offset, s.max_action=" << s.max_action << ", total_number_of_policies=" << s.total_number_of_policies << ", current_policy=" << s.current_policy << endl;}
        s.base.learn(&all,s.base.data,s.global_example_set[k-1]);
      }

      //      cerr << "============================ (empty = " << s.empty_example << ")" << endl;
      s.empty_example->in_use = true;
      s.base.learn(&all,s.base.data,s.empty_example);

      for (uint32_t k=1; k<=s.max_action; k++) {
        if (!s.rollout[k-1].alive) break;
        SearnUtil::remove_policy_offset(all, s.global_example_set[k-1], s.increment, s.current_policy);
        s.global_example_set[k-1]->ld = s.old_labels[k-1];
        s.task.cs_ldf_example(all, s0, k, s.global_example_set[k-1], false);
      }
      //      cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    }

  }

  void run_prediction(vw&all, searn& s, state s0, bool allow_oracle, bool allow_current, bool track_actions, std::vector<action>* action_sequence)
  {
    int step = 1;
    while (!s.task.final(s0)) {
      uint32_t action = searn_predict(all, s, s0, step, allow_oracle, allow_current, NULL);
      if (track_actions)
        action_sequence->push_back(action);

      s.task.step(s0, action);
      step++;
    }
  }

  void do_actual_learning(vw&all, searn& s)
  {
    // there are two cases:
    //   * is_singleline --> look only at ec_seq[0]
    //   * otherwise     --> look at everything

    if (s.ec_seq.size() == 0)
      return;

    // generate the start state
    state s0;
    if (s.is_singleline)
      s.task.start_state(s.ec_seq[0], &s0);
    else
      s.task.start_state_multiline(s.ec_seq.begin, s.ec_seq.size(), &s0);

    state s0copy = NULL;
    bool  is_test = s.task.is_test_example(s.ec_seq.begin, s.ec_seq.size());
    if (!is_test) {
      s0copy = s.task.copy(s0);
      all.sd->example_number++;
      all.sd->total_features    += s.searn_num_features;
      all.sd->weighted_examples += 1.;
    }
    bool will_print = is_test || should_print_update(all) || will_global_print_label(all, s);

    s.searn_num_features = 0;
    std::vector<action> action_sequence;

    // if we are using adaptive beta, update it to take into account the latest updates
    if( s.adaptive_beta ) s.beta = 1.f - powf(1.f - s.alpha,(float)s.total_examples_generated);
    
    run_prediction(all, s, s0, false, true, will_print, &action_sequence);
    global_print_label(all, s, s.ec_seq[0], s0, action_sequence);

    if (!is_test) {
      float loss = s.task.loss(s0);
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
    }

    print_update(all, s, s0, action_sequence);
    
    s.task.finish(s0);

    if (is_test || !all.training)
	{
	  if (!s.is_singleline)
		clear_seq(all, s);
      return;
	}

    s0 = s0copy;

    // training examples only get here
    int step = 1;
    while (!s.task.final(s0)) {
      // if we are using adaptive beta, update it to take into account the latest updates
      if( s.adaptive_beta ) s.beta = 1.f - powf(1.f - s.alpha,(float)s.total_examples_generated);

      // first, make a prediction (we don't want to bias ourselves if
      // we're using the current policy to predict)
      uint32_t action = searn_predict(all, s, s0, step, true, s.allow_current_policy, NULL);

      // generate training example for the current state
      generate_state_example(all, s, s0);

      // take the prescribed step
      s.task.step(s0, action);

      step++;
    }
    s.task.finish(s0);
    if (s.do_recombination) {  // we need to free a bunch of memory
      //      s.past_states->iter(&hm_free_state_copies);
      free_unfreed_states(s);
      s.past_states->clear();
    }
	if (!s.is_singleline)
		clear_seq(all, s);
  }

  void process_next_example(vw&all, searn& s, example *ec)
  {
    bool is_real_example = true;

    if (s.is_singleline) {
      if (s.ec_seq.size() == 0)
        s.ec_seq.push_back(ec);
      else
        s.ec_seq[0] = ec;

      do_actual_learning(all, s);
    } else {  
      // is multiline
      if (s.ec_seq.size() >= all.p->ring_size - 2) { // give some wiggle room
        std::cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << std::endl;
        do_actual_learning(all, s);
      }

      if (OAA::example_is_newline(ec)) {
        do_actual_learning(all, s);
        //CSOAA_LDF::global_print_newline(all);
	VW::finish_example(all, ec);
        is_real_example = false;
      } else {
        s.ec_seq.push_back(ec);
      }
    }

    // for both single and multiline
    if (is_real_example) {
      s.read_example_this_loop++;
      s.read_example_last_id = ec->example_counter;
      if (ec->pass != s.read_example_last_pass) {
        s.read_example_last_pass = ec->pass;
        s.passes_since_new_policy++;
        if (s.passes_since_new_policy >= s.passes_per_policy) {
          s.passes_since_new_policy = 0;
          if(all.training)
            s.current_policy++;
          if (s.current_policy > s.total_number_of_policies) {
            std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
            s.current_policy = s.total_number_of_policies;
          }
          //reset searn_trained_nb_policies in options_from_file so it is saved to regressor file later
          std::stringstream ss;
          ss << s.current_policy;
          VW::cmd_string_replace_value(all.options_from_file,"--searn_trained_nb_policies", ss.str());
        }
      }
    }
  }

  void drive(void*in, void*d)
  {
    vw*all = (vw*)in;
    // initialize everything
    searn* s = (searn*)d;

    const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";

    fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
    fprintf(stderr, header_fmt, "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
    cerr.precision(5);

    initialize_memory(*s);

    example* ec = NULL;
    s->read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { // semiblocking operation
        process_next_example(*all, *s, ec);
      } else if (parser_done(all->p)) {
        if (!s->is_singleline)
          do_actual_learning(*all, *s);
        break;
      }
    }

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << (s->current_policy+1);
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_trained_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_trained_nb_policies", ss1.str()); 
      ss2 << s->total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searn_total_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searn_total_nb_policies", ss2.str());
    }
  }


}


namespace ImperativeSearn {
  const char INIT_TEST  = 0;
  const char INIT_TRAIN = 1;
  const char LEARN      = 2;

  inline bool isLDF(searn& srn) { return (srn.A == 0); }

  uint32_t choose_policy(searn& srn, bool allow_current, bool allow_optimal)
  {
    uint32_t seed = 0; // TODO: srn.read_example_last_id * 2147483 + srn.t * 2147483647;
    return SearnUtil::random_policy(seed, srn.beta, allow_current, srn.current_policy, allow_optimal, srn.rollout_all_actions);
  }

  CSOAA::label get_all_labels(searn& srn, size_t num_ec, v_array<uint32_t> *yallowed)
  {
    if (isLDF(srn)) {
		CSOAA::label ret;  // TODO: cache these!
      for (uint32_t i=0; i<num_ec; i++) {
        CSOAA::wclass cost = { FLT_MAX, i, 1., 0. };
		ret.costs.push_back(cost);
      }
      return ret;
    }
    // is not LDF
    if (yallowed == NULL) {
      CSOAA::label ret;  // TODO: cache this!
      for (uint32_t i=1; i<=srn.A; i++) {
        CSOAA::wclass cost = { FLT_MAX, i, 1., 0. };
        ret.costs.push_back(cost);
      }
      return ret;
    }
    CSOAA::label ret;
    for (size_t i=0; i<yallowed->size(); i++) {
      CSOAA::wclass cost = { FLT_MAX, (*yallowed)[i], 1., 0. };
      ret.costs.push_back(cost);
    }
    return ret;
  }

  uint32_t single_prediction_LDF(vw& all, example** ecs, size_t num_ec, size_t pol)
  {
    assert(pol > 0);
    // TODO
    return 0;
  }

  uint32_t single_prediction_notLDF(vw& all, searn& srn, example* ec, CSOAA::label valid_labels, uint32_t pol)
  {
    assert(pol > 0);

    void* old_label = ec->ld;
    ec->ld = (void*)&valid_labels;
    SearnUtil::add_policy_offset(all, ec, srn.increment, pol);

    srn.base.learn(&all,srn.base.data, ec);
    srn.total_predictions_made++;
    srn.num_features += ec->num_features;
    uint32_t final_prediction = (uint32_t)ec->final_prediction;

    SearnUtil::remove_policy_offset(all, ec, srn.increment, pol);
    ec->ld = old_label;

    return final_prediction;
  }

  template<class T> T choose_random(v_array<T> opts) {
    float r = frand48();
    assert(opts.size() > 0);
    return opts[(size_t)(((float)opts.size()) * r)];
  }

  uint32_t single_action(vw& all, searn& srn, example** ecs, size_t num_ec, CSOAA::label valid_labels, uint32_t pol, v_array<uint32_t> *ystar) {
    //cerr << "pol=" << pol << " ystar.size()=" << ystar->size() << " ystar[0]=" << ((ystar->size() > 0) ? (*ystar)[0] : 0) << endl;
    if (pol == 0) { // optimal policy
      if ((ystar == NULL) || (ystar->size() == 0))
        return choose_random<CSOAA::wclass>(valid_labels.costs).weight_index;
      else
        return choose_random<uint32_t>(*ystar);
    } else {        // learned policy
      if (isLDF(srn))
        return single_prediction_LDF(all, ecs, num_ec, pol);
      else
        return single_prediction_notLDF(all, srn, *ecs, valid_labels, pol);
    }
  }

  void clear_snapshot(vw& all, searn& srn)
  {
    for (size_t i=0; i<srn.snapshot_data.size(); i++)
      free(srn.snapshot_data[i].data_ptr);
    srn.snapshot_data.erase();
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
  uint32_t searn_predict(vw& all, example** ecs, size_t num_ec, v_array<uint32_t> *yallowed, v_array<uint32_t> *ystar)  // num_ec == 0 means normal example, >0 means ldf, yallowed==NULL means all allowed, ystar==NULL means don't know
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
      uint32_t pol = choose_policy(*srn, true, false);
      CSOAA::label valid_labels = get_all_labels(*srn, num_ec, yallowed);
      uint32_t a = single_action(all, *srn, ecs, num_ec, valid_labels, pol, ystar);
      srn->t++;
      valid_labels.costs.erase(); valid_labels.costs.delete_v();
      return a;
    }
    if (srn->state == INIT_TRAIN) {
      uint32_t pol = choose_policy(*srn, srn->allow_current_policy, true);
      CSOAA::label valid_labels = get_all_labels(*srn, num_ec, yallowed);
      uint32_t a = single_action(all, *srn, ecs, num_ec, valid_labels, pol, ystar);
      srn->train_action.push_back(a);
      srn->train_labels.push_back(valid_labels);
      srn->t++;
      return a;
    }
    if (srn->state == LEARN) {
      if (srn->t < srn->learn_t) {
        assert(srn->t < srn->train_action.size());
        srn->t++;
        return srn->train_action[srn->t-1];
      } else if (srn->t == srn->learn_t) {
        if (srn->learn_example_copy == NULL) {
          size_t num_to_copy = (num_ec == 0) ? 1 : num_ec;
          srn->learn_example_len = num_to_copy;
          srn->learn_example_copy = (example**)SearnUtil::calloc_or_die(num_to_copy, sizeof(example*));
          for (size_t n=0; n<num_to_copy; n++) {
            srn->learn_example_copy[n] = alloc_example(sizeof(CSOAA::label));
            VW::copy_example_data(srn->learn_example_copy[n], ecs[n], sizeof(CSOAA::label), all.p->lp->copy_label);
          }
          //cerr << "copying example to " << srn->learn_example_copy << endl;
        }
        srn->t++;
        return srn->learn_a;
      } else {
        uint32_t pol = choose_policy(*srn, srn->allow_current_policy, true);
        CSOAA::label valid_labels = get_all_labels(*srn, num_ec, yallowed);
        uint32_t a = single_action(all, *srn, ecs, num_ec, valid_labels, pol, ystar);
        srn->t++;
        valid_labels.costs.erase(); valid_labels.costs.delete_v();
        return a;
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
    if (srn->state == INIT_TEST)
      srn->test_loss += incr_loss;
    else if (srn->state == INIT_TRAIN)
      srn->train_loss += incr_loss;
    else
      srn->learn_loss += incr_loss;
  }

  bool snapshot_linear_search(v_array<snapshot_item> a, size_t desired_t, size_t tag, size_t &pos) {
    if (a.size() == 0) return false;
    for (pos=a.size()-1; ; pos--) {
      if ((a[pos].pred_step <= desired_t) && (tag == a[pos].tag))
        return true;
      if (pos == 0) return false;
    }
    return false;
  }

  void searn_snapshot(vw& all, size_t index, size_t tag, void* data_ptr, size_t sizeof_data)
  {
    searn* srn=(searn*)all.searnstr;
    if (! srn->do_snapshot) return;

    //cerr << "snapshot called with:   { index=" << index << ", tag=" << tag << ", data_ptr=" << *(size_t*)data_ptr << ", t=" << srn->t << " }" << endl;
    

    if (srn->state == INIT_TEST) return;
    if (srn->state == INIT_TRAIN) {  // training means "record snapshots"
      if ((srn->snapshot_data.size() > 0) &&
          ((srn->snapshot_data.last().index > index) ||
           ((srn->snapshot_data.last().index == index) && (srn->snapshot_data.last().tag > tag)))) 
        cerr << "warning: trying to snapshot in a non-monotonic order! ignoring this snapshot" << endl;
      else {
        void* new_data = malloc(sizeof_data);
        memcpy(new_data, data_ptr, sizeof_data);
        snapshot_item item = { index, tag, new_data, sizeof_data, srn->t };
        srn->snapshot_data.push_back(item);
      }
      return;
    }
    if (srn->t > srn->learn_t) return;

    //cerr << "index=" << index << " tag=" << tag << endl;

    // otherwise, we're restoring snapshots -- we want to find the index of largest value that has .t<=learn_t
    size_t i;
    bool found;
    found = snapshot_linear_search(srn->snapshot_data, srn->learn_t, tag, i);
    if (!found) return;  // can't do anything

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
  }

  v_array<size_t> get_training_timesteps(vw& all, searn& srn)
  {
    v_array<size_t> timesteps;
    for (size_t t=0; t<srn.T; t++)
      timesteps.push_back(t);
    return timesteps;
  }

  void generate_training_example(vw& all, searn& srn, example** ec, size_t len, CSOAA::label labels, v_array<float> losses)
  {
    assert(labels.costs.size() == losses.size());
	for (size_t i=0; i<labels.costs.size(); i++)
      labels.costs[i].x = losses[i];

    if (!isLDF(srn)) {
      void* old_label = ec[0]->ld;
      ec[0]->ld = (void*)&labels;
      SearnUtil::add_policy_offset(all, ec[0], srn.increment, srn.current_policy);
      srn.base.learn(&all,srn.base.data, ec[0]);
      SearnUtil::remove_policy_offset(all, ec[0], srn.increment, srn.current_policy);
      ec[0]->ld = old_label;
      srn.total_examples_generated++;
    } else { // isLDF
      //TODO
    }
  }

  void train_single_example(vw& all, searn& srn, example**ec, size_t len)
  {
    // do an initial test pass to compute output (and loss)
    //cerr << "======================================== INIT TEST ========================================" << endl;
    srn.state = INIT_TEST;
    srn.t = 0;
    srn.T = 0;
    srn.loss_last_step = 0;
    srn.test_loss = 0.f;
    srn.train_loss = 0.f;
    srn.learn_loss = 0.f;
    srn.learn_example_copy = NULL;
    srn.learn_example_len  = 0;
    srn.train_action.erase();
    srn.num_features = 0;
 
    srn.task->structured_predict(all, srn, ec, len, srn.pred_string, srn.truth_string);

    if (srn.t == 0)
      return;  // there was no data!

    // do a pass over the data allowing oracle and snapshotting
    //cerr << "======================================== INIT TRAIN ========================================" << endl;
    srn.state = INIT_TRAIN;
    srn.t = 0;
    srn.loss_last_step = 0;
    clear_snapshot(all, srn);

    srn.task->structured_predict(all, srn, ec, len, NULL, NULL);

    if (srn.t == 0) {
      clear_snapshot(all, srn);
      return;  // there was no data
    }

    srn.T = srn.t;

    // generate training examples on which to learn
    //cerr << "======================================== LEARN ========================================" << endl;
    srn.state = LEARN;
    v_array<size_t> tset = get_training_timesteps(all, srn);
    for (size_t t=0; t<tset.size(); t++) {
      CSOAA::label aset = srn.train_labels[t];
      srn.learn_t = t;
      srn.learn_losses.erase();

      for (size_t i=0; i<aset.costs.size(); i++) {
        if (aset.costs[i].weight_index == srn.train_action[srn.learn_t])
          srn.learn_losses.push_back( srn.train_loss );
        else {
          srn.t = 0;
          srn.learn_a = aset.costs[i].weight_index;
          srn.loss_last_step = 0;
          srn.learn_loss = 0.f;

          //cerr << "learn_t = " << srn.learn_t << " || learn_a = " << srn.learn_a << endl;
          srn.task->structured_predict(all, srn, ec, len, NULL, NULL);

          srn.learn_losses.push_back( srn.learn_loss );
          //cerr << "total loss: " << srn.learn_loss << endl;
        }
      }

      if (srn.learn_example_copy != NULL) {
        generate_training_example(all, srn, srn.learn_example_copy, srn.learn_example_len, aset, srn.learn_losses);

        for (size_t n=0; n<srn.learn_example_len; n++) {
          dealloc_example(CSOAA::delete_label, *srn.learn_example_copy[n]);
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

    clear_snapshot(all, srn);
    srn.train_action.erase();
    srn.train_action.delete_v();
    for (size_t i=0; i<srn.train_labels.size(); i++) {
      srn.train_labels[i].costs.erase();
      srn.train_labels[i].costs.delete_v();
    }
    srn.train_labels.erase();
    srn.train_labels.delete_v();
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
 
  void print_update(vw& all, searn& srn)
  {
    if (!Searn::should_print_update(all))
      return;

    char true_label[21];
    char pred_label[21];
    Searn::to_short_string(srn.truth_string ? srn.truth_string->str() : "", 20, true_label);
    Searn::to_short_string(srn.pred_string  ? srn.pred_string->str()  : "", 20, pred_label);

    fprintf(stderr, "%-10.6f %-10.6f %8ld %15f   [%s] [%s] %8lu %5d %5d %15lu %15lu\n",
            safediv((float)all.sd->sum_loss, (float)all.sd->weighted_examples),
            safediv((float)all.sd->sum_loss_since_last_dump, (float) (all.sd->weighted_examples - all.sd->old_weighted_examples)),
            (long int)all.sd->example_number,
            all.sd->weighted_examples,
            true_label,
            pred_label,
            (long unsigned int)srn.num_features,
            (int)srn.read_example_last_pass,
            (int)srn.current_policy,
            (long unsigned int)srn.total_predictions_made,
            (long unsigned int)srn.total_examples_generated);

    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    all.sd->dump_interval *= 2;
  }


  void do_actual_learning(vw&all, searn& srn)
  {
    if (srn.ec_seq.size() == 0)
      return;  // nothing to do :)

    if (Searn::should_print_update(all)) {
      srn.truth_string = new stringstream();
      srn.pred_string  = new stringstream();
    }

    train_single_example(all, srn, srn.ec_seq.begin, srn.ec_seq.size());
    if (srn.test_loss >= 0.f) {
      all.sd->sum_loss += srn.test_loss;
      all.sd->sum_loss_since_last_dump += srn.test_loss;
      all.sd->example_number++;
      all.sd->total_features += srn.num_features;
      all.sd->weighted_examples += 1.f;
    }

    print_update(all, srn);

    if (srn.truth_string != NULL) {
      delete srn.truth_string;
      srn.truth_string = NULL;
    }
    if (srn.pred_string != NULL) {
      delete srn.pred_string;
      srn.pred_string = NULL;
    }
  }

  void searn_learn(void*in, void*d, example*ec) {
    vw* all = (vw*)in;
    searn *srn = (searn*)d;

    if (srn->ec_seq.size() >= all->p->ring_size - 2) { // give some wiggle room
      std::cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << std::endl;
      do_actual_learning(*all, *srn);
      clear_seq(*all, *srn);
    }
    
    bool is_real_example = true;

    if (OAA::example_is_newline(ec)) {
      do_actual_learning(*all, *srn);
      clear_seq(*all, *srn);
      VW::finish_example(*all, ec);
      is_real_example = false;
    } else {
      srn->ec_seq.push_back(ec);
    }
        
    if (is_real_example) {
      srn->read_example_this_loop++;
      srn->read_example_last_id = ec->example_counter;
      if (ec->pass != srn->read_example_last_pass) {
        srn->read_example_last_pass = ec->pass;
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
    }
  }

  void searn_drive(void*in, void *d) {
    vw* all = (vw*)in;
    searn *srn = (searn*)d;

    const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";
    
    fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
    fprintf(stderr, header_fmt, "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
    cerr.precision(5);

    example* ec = NULL;
    srn->read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { // semiblocking operation
        searn_learn(in,d, ec);
      } else if (parser_done(all->p)) {
        do_actual_learning(*all, *srn);
        break;
      }
    }

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << (srn->current_policy+1);
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searnimp_trained_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searnimp_trained_nb_policies", ss1.str()); 
      ss2 << srn->total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --searnimp_total_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--searnimp_total_nb_policies", ss2.str());
    }
  }

  void searn_initialize(vw& all, searn& srn)
  {
    srn.predict = searn_predict;
    srn.declare_loss = searn_declare_loss;
    srn.snapshot = searn_snapshot;

    srn.beta = 0.5;
    srn.allow_current_policy = false;
    srn.rollout_all_actions = true;
    srn.num_features = 0;
    srn.current_policy = 1;
    srn.state = 0;
    srn.do_snapshot = true;

    srn.passes_per_policy = 1;     //this should be set to the same value as --passes for dagger

    srn.read_example_this_loop = 0;
    srn.read_example_last_id = 0;
    srn.passes_since_new_policy = 0;
    srn.read_example_last_pass = 0;
    srn.total_examples_generated = 0;
    srn.total_predictions_made = 0;
  }

  void searn_finish(void*in, void* d)
  {
    vw*all = (vw*)in;
    searn *srn = (searn*)d;

    //cerr << "searn_finish" << endl;

    srn->ec_seq.delete_v();

    clear_snapshot(*all, *srn);
    srn->snapshot_data.delete_v();

    for (size_t i=0; i<srn->train_labels.size(); i++) {
      srn->train_labels[i].costs.erase();
	  srn->train_labels[i].costs.delete_v();
    }
    srn->train_labels.erase(); srn->train_labels.delete_v();
    srn->train_action.erase(); srn->train_action.delete_v();
    srn->learn_losses.erase(); srn->learn_losses.delete_v();

    if (srn->task->finish != NULL)
      srn->task->finish(*all);
    if (srn->task->finish != NULL)
      srn->base.finish(all, srn->base.data);
  }

  void ensure_param(float &v, float lo, float hi, float def, const char* string) {
    if ((v < lo) || (v > hi)) {
      cerr << string << endl;
      v = def;
    }
  }

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
      ss << " " << opt_name << " " << ret;
      all.options_from_file.append(ss.str());
    } else if (strlen(required_error_string)>0) {
      std::cerr << required_error_string << endl;
      throw exception();
    }
  }  

  bool string_equal(string a, string b) { return a.compare(b) == 0; }
  bool float_equal(float a, float b) { return fabs(a-b) < 1e-6; }
  bool uint32_equal(uint32_t a, uint32_t b) { return a==b; }

  void parse_flags(vw&all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    searn* srn = (searn*)calloc(1,sizeof(searn));

    searn_initialize(all, *srn);

    po::options_description desc("Imperative Searn options");
    desc.add_options()
      ("searn_task", po::value<string>(), "the searn task")
      ("searn_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
      ("searn_beta", po::value<float>(), "interpolation rate for policies")
      ("searn_allow_current_policy", "allow searn labeling to use the current policy")
      ("searn_total_nb_policies", po::value<size_t>(), "if we are going to train the policies through multiple separate calls to vw, we need to specify this parameter and tell vw how many policies are eventually going to be trained")
      ("searn_no_snapshot", "turn off snapshotting capabilities");

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
    check_option<uint32_t>(srn->A, all, vm, vm_file, "searnimp", false, uint32_equal,
                         "warning: you specified a different number of actions through --searnimp than the one loaded from predictor. using loaded value of: ", "");

    if (vm.count("searn_passes_per_policy"))       srn->passes_per_policy    = vm["searn_passes_per_policy"].as<size_t>();
    if (vm.count("searn_allow_current_policy"))    srn->allow_current_policy = true;
    if (vm.count("searn_no_snapshot"))             srn->do_snapshot          = false;

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

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    uint32_t tmp_number_of_policies = srn->current_policy; 
    if( all.training )
	tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)srn->passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls, 
    //so only set total_number_of_policies to computed value if it is larger
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

    all.base_learner_nb_w *= srn->total_number_of_policies;
    srn->increment = ((uint32_t)all.length() / all.base_learner_nb_w) * all.stride;

    if (task_string.compare("sequence") == 0) {
      searn_task* mytask = (searn_task*)calloc(1, sizeof(searn_task));
      mytask->initialize = SequenceTask_Easy::initialize;
      mytask->finish = SequenceTask_Easy::finish;
      mytask->structured_predict = SequenceTask_Easy::structured_predict_v1;

      srn->task = mytask;
    } else {
      cerr << "fail: unknown task for --searn_task: " << task_string << endl;
      throw exception();
    }

    srn->task->initialize(all, srn->A);

    
    learner l = {srn, searn_drive, searn_learn, searn_finish, all.l.save_load};
    srn->base = all.l;
    all.l = l;
  }
}
/*
time ./vw --searn 45 --searn_task sequence -k -c -d ../test/train-sets/wsj_small.dat2.gz --passes 5 --searn_passes_per_policy 4

old searn: 11.524 11.450 11.448 || 3.333 4.035
new searn: 28.377 28.443 28.160 || 2.756 4.623
snapshots: 27.681 28.495 27.838 || 2.756 4.623
*/
