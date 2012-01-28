/*
This is an implementation of the Searn algorithm, specialized to
sequence labeling.  It is based on:

  @article{daume09searn,
    author =       {Hal {Daum\'e III} and John Langford and Daniel Marcu},
    title =        {Search-based Structured Prediction},
    year =         {2009},
    booktitle =    {Machine Learning Journal (MLJ)},
    abstract =     {
      We present Searn, an algorithm for integrating search and
      learning to solve complex structured prediction problems such
      as those that occur in natural language, speech, computational
      biology, and vision.  Searn is a meta-algorithm that transforms
      these complex problems into simple classification problems to which
      any binary classifier may be applied.  Unlike current algorithms for
      structured learning that require decomposition of both the loss
      function and the feature functions over the predicted structure,
      Searn is able to learn prediction functions for any loss
      function and any class of features.  Moreover, Searn comes
      with a strong, natural theoretical guarantee: good performance on the
      derived classification problems implies good performance on the
      structured prediction problem.
    },
    keywords = {sp nlp ml},
    url = {http://pub.hal3.name/#daume09searn}
  }

It was initially implemented by Hal Daume III, while visiting Yahoo!
Email questions/comments to me@hal3.name.
*/

#include <iostream>
#include <float.h>
#include <stdio.h>
#include <math.h>
// #include <sys/timeb.h>
#include "gd.h"
#include "io.h"
#include "sequence.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "csoaa.h"

typedef uint32_t* history;  // histories have the most recent prediction at the END

struct history_item {
  history  predictions;
  uint32_t predictions_hash;
  float    loss;
  size_t   original_label;
  bool     same;
  bool     alive;  // false if this isn't a valid transition
};

bool PRINT_DEBUG_INFO             = 0;
bool PRINT_UPDATE_EVERY_EXAMPLE   = 0;
bool OPTIMIZE_SHARED_HISTORIES    = 1;

#define PRINT_LEN 21

// struct timeb t_start_global;

size_t sequence_history           = 1;
bool   sequence_bigrams           = false;
size_t sequence_features          = 0;
bool   sequence_bigram_features   = false;
size_t sequence_rollout           = 256;
size_t sequence_passes_per_policy = 1;
float  sequence_beta              = 0.5;
size_t sequence_k                 = 2;
size_t sequence_gamma             = 1.;
bool   sequence_allow_current_policy = false;
bool   sequence_training_transitions = false;

bool   all_transitions_allowed    = true;
bool** valid_transition           = NULL;

size_t history_length             = 1;
size_t current_policy             = 0;
size_t read_example_last_pass     = 0;
size_t total_number_of_policies   = 1;

size_t constant_pow_history_length = 0;

size_t total_predictions_made     = 0;
size_t total_examples_generated   = 0;

uint32_t      history_constant    = 8290741;
history       current_history     = NULL;

example**     ec_seq        = NULL;
size_t*       pred_seq      = NULL;
int*          policy_seq    = NULL;
history*      all_histories = NULL;
history_item* hcache        = NULL;
v_array<OAA::mc_label*> true_labels = v_array<OAA::mc_label*>();

CSOAA::label testall_costs = { v_array<feature>() };
v_array<feature> loss_vector  = v_array<feature>();
v_array<CSOAA::label> transition_prediction_costs = v_array<CSOAA::label>();


size_t        max_string_length = 8;


using namespace std;

/********************************************************************************************
 *** GENERIC HELPER FUNCTIONS
 ********************************************************************************************/

void* calloc_or_die(size_t nmemb, size_t size)
{
  void* data = calloc(nmemb, size);
  if (data == NULL) {
    cerr << "internal error: memory allocation failed; dying!" << endl;
    exit(-1);
  }
  return data;
}

void read_transition_file(const char* filename)
{
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    cerr << "warning: could not read file " << filename << "; assuming all transitions are valid" << endl;
    return;
  }
  int rd;
  int n = fscanf(f, "%d", &rd);
  if (n == 0) {
    cerr << "warning: could not read transitions final; assuming all valid" << endl;
    all_transitions_allowed = true;
    fclose(f);
    return;
  }
  if (rd < (int)sequence_k) {
    cerr << "warning: number of classes in transition file (" << rd << ") is too small (need " << sequence_k << "); assuming all valid" << endl;
    all_transitions_allowed = true;
    fclose(f);
    return;
  }
  int file_k = rd;
  if (rd > (int)sequence_k) {
    cerr << "warning: number of classes in transition file (" << rd << ") is too big; reading a subset" << endl;
  }

  all_transitions_allowed = false;
  if (valid_transition == NULL) {
    valid_transition = (bool**)calloc_or_die(sizeof(bool*), sequence_k+1);
    for (size_t i=0; i<=sequence_k; i++)   // this is FROM, identified by line number; k+1 total lines for INITIAL transition
      valid_transition[i] = (bool*)calloc_or_die(sizeof(bool), sequence_k);
  }

  for (size_t i=0; i<=sequence_k; i++)
    for (size_t j=0; j<sequence_k; j++)
      valid_transition[i][j] = true;
  
  transition_prediction_costs.erase();
  for (int i=0; i<=file_k; i++)  {   // this is FROM, identified by line number; k+1 total lines for INITIAL transition
    v_array<feature> this_costs = v_array<feature>();
    for (int j=0; j<file_k; j++) {   // this is TO, identified by col number; k total columns
      n = fscanf(f, "%d", &rd);
      if (n == 0) {
        cerr << "warning: could not read transitions; assuming all remaining are valid after " << i << "," << (j+1) << endl;
        return;
      }
      if ((i <= (int)sequence_k) && (j < (int)sequence_k)) {
        valid_transition[i][j] = (rd > 0);
        if (valid_transition[i][j]) {
          feature feat = { FLT_MAX, j+1 };
          push(this_costs, feat);
        }
      }
    }
    if (i <= (int)sequence_k) {
      CSOAA::label label = { this_costs };
      push(transition_prediction_costs, label);
    }
  }
  fclose(f);

}

int random_policy(int allow_optimal)
{
  if (sequence_beta >= 1) {
    if (sequence_allow_current_policy) return (int)current_policy;
    if (current_policy > 0) return (((int)current_policy)-1);
    if (allow_optimal) return -1;
    cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << endl;
    return (int)current_policy;
  }

  int num_valid_policies = (int)current_policy + allow_optimal + sequence_allow_current_policy;
  int pid = -1;

  if (num_valid_policies == 0) {
    cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << endl;
    return (int)current_policy;
  } else if (num_valid_policies == 1) {
    pid = 0;
  } else {
    float r = drand48();
    pid = 0;
    if (r > sequence_beta) {
      r -= sequence_beta;
      while ((r > 0) && (pid < num_valid_policies-1)) {
        pid ++;
        r -= sequence_beta * powf(1. - sequence_beta, (float)pid);
      }
    }
  }

  // figure out which policy pid refers to
  if (allow_optimal && (pid == num_valid_policies-1))
    return -1; // this is the optimal policy
  
  pid = (int)current_policy - pid;
  if (!sequence_allow_current_policy)
    pid--;

  return pid;
}


void allocate_required_memory()
{
  if (ec_seq == NULL) {
    ec_seq = (example**)calloc_or_die(global.ring_size, sizeof(example*));
    for (size_t i=0; i<global.ring_size; i++)
      ec_seq[i] = NULL;
  }

  loss_vector.erase();

  if (pred_seq == NULL)
    pred_seq = (size_t*)calloc_or_die(global.ring_size, sizeof(size_t));

  if (policy_seq == NULL)
    policy_seq = (int*)calloc_or_die(global.ring_size, sizeof(int));

  if (all_histories == NULL) {
    all_histories = (history*)calloc_or_die(sequence_k, sizeof(history));
    for (size_t i=0; i<sequence_k; i++)
      all_histories[i] = (history)calloc_or_die(history_length, sizeof(size_t));
  }

  if (hcache == NULL)
    hcache = (history_item*)calloc_or_die(sequence_k, sizeof(history_item));

  if (current_history == NULL)
    current_history = (history)calloc_or_die(history_length, sizeof(uint32_t));

  for (size_t i = 1; i <= global.k; i++)
    {
      feature f = {FLT_MAX, i};
      push(testall_costs.costs, f);
    }
}

void free_required_memory()
{
  free(ec_seq);          ec_seq          = NULL;
  free(pred_seq);        pred_seq        = NULL;
  free(policy_seq);      policy_seq      = NULL;
  free(hcache);          hcache          = NULL;
  free(current_history); current_history = NULL;

  for (size_t i=0; i<sequence_k; i++)
    free(all_histories[i]);

  if (!all_transitions_allowed) {
    for (size_t i=0; i<sequence_k+1; i++)
      free(valid_transition[i]);
    free(valid_transition);
    valid_transition = NULL;
  }

  free(all_histories);   all_histories   = NULL;

  true_labels.erase();
  free(true_labels.begin);

  if (testall_costs.costs.begin != NULL)
    free(testall_costs.costs.begin);
}


/********************************************************************************************
 *** OUTPUTTING FUNCTIONS
 ********************************************************************************************/

void global_print_label(example *ec, size_t label)
{
  for (size_t i=0; i<global.final_prediction_sink.index(); i++) {
    int f = global.final_prediction_sink[i];
    global.print(f, label, 0., ec->tag);
  }
}


void print_history(history h)
{
  clog << "[ ";
  for (size_t t=0; t<history_length; t++)
    clog << h[t] << " ";
  clog << "]" << endl;
}

size_t read_example_this_loop  = 0;
size_t read_example_last_id    = 0;
int    read_example_should_warn_eof = 1;
size_t passes_since_new_policy = 0;

/*
void show_big_number(uintmax_t *out, char *out_c, uintmax_t in)
{
  if (in < 1000) {
    *out = in;
    *out_c = ' ';
  } else if (in < 1000000) {
    *out = in/1000;
    *out_c = 'k';
  } else if (in < 1000000000) {
    *out = in/1000000;
    *out_c = 'm';
  } else if (in < 1000000000000) {
    *out = in/1000000000;
    *out_c = 'g';
  } else if (in < 1000000000000000) {
    *out = in/1000000000000;
    *out_c = 'p';
  } else if (in < 1000000000000000000) {
    *out = in/1000000000000000;
    *out_c = 'e';
  } else {
    *out = (uintmax_t)in;
    *out_c = ' ';
  }
}
*/

void print_update(bool wasKnown, long unsigned int seq_num_features)
{
  if (!(global.sd->weighted_examples > global.sd->dump_interval && !global.quiet && !global.bfgs)) {
    if (!PRINT_UPDATE_EVERY_EXAMPLE) return;
  }

  char true_label[PRINT_LEN];
  char pred_label[PRINT_LEN];
  for (size_t i=0; i<PRINT_LEN-1; i++) {
    true_label[i] = ' ';
    pred_label[i] = ' ';
  }
  true_label[PRINT_LEN-1] = 0;
  pred_label[PRINT_LEN-1] = 0;

  int num_len = (int)ceil(log10f((float)sequence_k)+1);
  int pos = 0;
  int i = 0;
  int numspr = 0; int strlen;
  while (pos < PRINT_LEN-num_len-1) {
    if (true_labels.begin+i == true_labels.end) { break; }

    strlen = num_len - (int)ceil(log10f((float)true_labels[i]->label+1));
    numspr = sprintf(true_label+pos+strlen, "%d", true_labels[i]->label);
    true_label[pos+numspr+strlen] = ' ';

    strlen = num_len - (int)ceil(log10f((float)pred_seq[i]+1));
    numspr = sprintf(pred_label+pos+strlen, "%d", (int)pred_seq[i]);
    pred_label[pos+numspr+strlen] = ' ';

    pos += num_len + 1;
    i++;
  }

  /*long unsigned int pred_made, ex_gen;
  char pred_made_c, ex_gen_c;
  show_big_number(&pred_made, &pred_made_c, total_predictions_made);
  show_big_number(&ex_gen   , &ex_gen_c   , total_examples_generated); */

  //  timeb t_end_global;
  //  ftime(&t_end_global);
  //  int net_time = (int) (t_end_global.time - t_start_global.time);
  fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   [%s] [%s] %8lu %5d %5d %10lu %10lu\n",
          global.sd->sum_loss/global.sd->weighted_examples,
          global.sd->sum_loss_since_last_dump / (global.sd->weighted_examples - global.sd->old_weighted_examples),
          (long int)global.sd->example_number,
          global.sd->weighted_examples,
          true_label,
          pred_label,
          seq_num_features,
          (int)read_example_last_pass,
          (int)current_policy,
          (long unsigned int)total_predictions_made,
          (long unsigned int)total_examples_generated);
  //          net_time);
     
  global.sd->sum_loss_since_last_dump = 0.0;
  global.sd->old_weighted_examples = global.sd->weighted_examples;
  global.sd->dump_interval *= 2;
}

void simple_print_example_features(example *ec)
{
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
    {
      feature* end = ec->atomics[*i].end;
      for (feature* f = ec->atomics[*i].begin; f!= end; f++) {
        cerr << "\t" << f->weight_index << ":" << f->x << ":" << global.reg.weight_vectors[f->weight_index & global.weight_mask];
      }
    }
  cerr << endl;
}

void simple_print_costs(CSOAA::label *c)
{
  for (feature *f = c->costs.begin; f != c->costs.end; f++) {
    clog << "\t" << f->weight_index << ":" << f->x;
  }
  clog << endl;
}

/********************************************************************************************
 *** HISTORY MANIPULATION
 ********************************************************************************************/

inline void append_history(history h, uint32_t p)
{
  for (size_t i=1; i<history_length; i++)
    h[i-1] = h[i];
  if (history_length > 0)
    h[history_length-1] = (size_t)p;
}

void append_history_item(history_item hi, uint32_t p)
{
  if (history_length > 0) {
    int old_val = hi.predictions[0];
    hi.predictions_hash -= old_val * constant_pow_history_length;
    hi.predictions_hash += p;
    hi.predictions_hash *= quadratic_constant;
    append_history(hi.predictions, p);
  }

  hi.same = 0;
}

inline size_t last_prediction(history h)
{
  return h[history_length-1];
}

int order_history_item(const void* a, const void* b)  // put dead items at end
{
  history_item *ha = (history_item*)a;
  history_item *hb = (history_item*)b;
  if (ha->alive && !hb->alive)
    return -1;
  else if (!ha->alive && hb->alive)
    return 1;
  else if (!ha->alive && !hb->alive)
    return 0;
  else if (ha->predictions_hash < hb->predictions_hash)
    return -1;
  else if (ha->predictions_hash < hb->predictions_hash)
    return  1;
  else if (history_length > 0)
    for (size_t i=history_length-1; i>=0; i--) {
      if (ha->predictions[i] < hb->predictions[i])
        return -1;
      else if (ha->predictions[i] > hb->predictions[i])
        return  1;
      else
        return 0;
    }
  return 0;
}

inline int cache_item_same_as_before(size_t i)
{
  return (i > 0) && (order_history_item(&hcache[i], &hcache[i-1]) == 0);
}


void sort_hcache_and_mark_equality()
{
  qsort(hcache, sequence_k, sizeof(history_item), order_history_item);
  hcache[0].same = 0;
  for (size_t i=1; i<sequence_k; i++) {
    int order = order_history_item(&hcache[i], &hcache[i-1]);
    hcache[i].same = (order == 0);
  }
}

int hcache_all_equal_or_dead()
{
  if (!hcache[0].alive) return 1;
  for (size_t i=1; i<sequence_k; i++) {
    if (!hcache[i].alive) return 1;
    if (!hcache[i].same)  return 0;
  }
  return 1;
}


inline void clear_history(history h)
{
  for (size_t t=0; t<history_length; t++)
    h[t] = 0;
}

/********************************************************************************************
 *** EXAMPLE MANIPULATION
 ********************************************************************************************/


string audit_feature_space("history");

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


void add_history_to_example(example* ec, history h)
{
  size_t v0, v;

  for (size_t t=1; t<=sequence_history; t++) {
    v0 = (h[history_length-t] * quadratic_constant + t) * quadratic_constant + history_constant;

    // add the basic history features
    feature temp = {1., (uint32_t) ( (2*v0) & global.parse_mask )};
    push(ec->atomics[history_namespace], temp);

    if (global.audit) {
      audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & global.parse_mask), 1., true };
      a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
      strcpy(a_feature.space, audit_feature_space.c_str());

      a_feature.feature = (char*)calloc_or_die(5 + 2*max_string_length, sizeof(char));
      sprintf(a_feature.feature, "ug@%d=%d", (int)t, (int)h[history_length-t]);

      push(ec->audit_features[history_namespace], a_feature);
    }

    // add the bigram features
    if ((t > 1) && sequence_bigrams) {
      v0 = ((v0 - history_constant) * quadratic_constant + h[history_length-t+1]) * quadratic_constant + history_constant;

      feature temp = {1., (uint32_t) ( (2*v0) & global.parse_mask )};
      push(ec->atomics[history_namespace], temp);

      if (global.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)((2*v0) & global.parse_mask), 1., true };
        a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)calloc_or_die(6 + 3*max_string_length, sizeof(char));
        sprintf(a_feature.feature, "bg@%d=%d-%d", (int)t-1, (int)h[history_length-t], (int)h[history_length-t+1]);

        push(ec->audit_features[history_namespace], a_feature);
      }

    }
  }

  string fstring;

  if (sequence_features > 0) {
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

        for (size_t t=1; t<=sequence_features; t++) {
          v0 = (h[history_length-t] * quadratic_constant + t) * quadratic_constant;
          
          // add the history/feature pair
          feature temp = {1., (uint32_t) ( (2*(v0 + v)) & global.parse_mask )};
          push(ec->atomics[history_namespace], temp);

          if (global.audit) {
            audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & global.parse_mask), 1., true };
            a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
            strcpy(a_feature.space, audit_feature_space.c_str());

            a_feature.feature = (char*)calloc_or_die(8 + 2*max_string_length + fstring.length(), sizeof(char));
            sprintf(a_feature.feature, "ug+f@%d=%d=%s", (int)t, (int)h[history_length-t], fstring.c_str());

            push(ec->audit_features[history_namespace], a_feature);
          }


          // add the bigram
          if ((t > 0) && sequence_bigram_features) {
            v0 = (v0 + h[history_length-t+1]) * quadratic_constant;

            feature temp = {1., (uint32_t) ( (2*(v + v0)) & global.parse_mask )};
            push(ec->atomics[history_namespace], temp);

            if (global.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)((2*(v+v0)) & global.parse_mask), 1., true };
              a_feature.space = (char*)calloc_or_die(audit_feature_space.length()+1, sizeof(char));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)calloc_or_die(9 + 3*max_string_length + fstring.length(), sizeof(char));
              sprintf(a_feature.feature, "bg+f@%d=%d-%d=%s", (int)t-1, (int)h[history_length-t], (int)h[history_length-t+1], fstring.c_str());

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

void add_policy_offset(example *ec, size_t policy)
{
  size_t amount = (policy * global.length() / sequence_k / total_number_of_policies) * global.stride;
  OAA::update_indicies(ec, amount);
}

void remove_policy_offset(example *ec, size_t policy)
{
  size_t amount = (policy * global.length() / sequence_k / total_number_of_policies) * global.stride;
  OAA::update_indicies(ec, -amount);
}





/********************************************************************************************
 *** INTERFACE TO VW
 ********************************************************************************************/

  void (*base_learner)(example*) = NULL;
  void (*base_finish)() = NULL;

void parse_sequence_args(po::variables_map& vm, void (*base_l)(example*), void (*base_f)())
{
  base_learner = base_l;
  base_finish = base_f;
  *(global.lp)=OAA::mc_label_parser;
  sequence_k = vm["sequence"].as<size_t>();

  if (vm.count("sequence_bigrams"))
    sequence_bigrams = true;
  if (vm.count("sequence_bigram_features"))
    sequence_bigrams = true;
  if (vm.count("sequence_allow_current_policy"))
    sequence_allow_current_policy = true;

  if (vm.count("sequence_history"))
    sequence_history = vm["sequence_history"].as<size_t>();
  if (vm.count("sequence_features"))
    sequence_features = vm["sequence_features"].as<size_t>();
  if (vm.count("sequence_rollout"))
    sequence_rollout = vm["sequence_rollout"].as<size_t>();
  if (vm.count("sequence_passes_per_policy"))
    sequence_passes_per_policy = vm["sequence_passes_per_policy"].as<size_t>();
  if (vm.count("sequence_beta"))
    sequence_beta = vm["sequence_beta"].as<float>();
  if (vm.count("sequence_gamma"))
    sequence_beta = vm["sequence_gamma"].as<float>();

  if (sequence_beta <= 0) {
    sequence_beta = 0.5;
    cerr << "warning: sequence_beta set to a value <= 0; resetting to 0.5" << endl;
  }

  if (vm.count("sequence_training_transitions")) {
    sequence_training_transitions = true;
    all_transitions_allowed = false;
  }

  if (vm.count("sequence_transition_file")) {
    if (sequence_training_transitions)
      cerr << "cannot use both --sequence_training_transitions and --sequence_transition_file; ignoring file" << endl;
    else {
      all_transitions_allowed = false;
      read_transition_file(vm["sequence_transition_file"].as<string>().c_str());
    }
  }
  else
    all_transitions_allowed = true;

  history_length = ( sequence_history > sequence_features ) ? sequence_history : sequence_features;
  if (!all_transitions_allowed && (history_length == 0))
    history_length = 1;

  constant_pow_history_length = 1;
  for (size_t i=0; i < history_length; i++)
    constant_pow_history_length *= quadratic_constant;

  total_number_of_policies = (int)ceil(((float)global.numpasses) / ((float)sequence_passes_per_policy));

  max_string_length = max((int)(ceil( log10((float)history_length+1) )),
                          (int)(ceil( log10((float)sequence_k+1) ))) + 1;

}

void generate_training_example(example *ec, history h, v_array<feature>costs)
{
  CSOAA::label ld = { costs };

  add_history_to_example(ec, h);
  add_policy_offset(ec, current_policy);

  if (PRINT_DEBUG_INFO) {clog << "before train: costs = ["; for (feature*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x; clog << " ]\t"; simple_print_example_features(ec);}
  ec->ld = (void*)&ld;
  total_examples_generated++;
  base_learner(ec);
  if (PRINT_DEBUG_INFO) {clog << " after train: costs = ["; for (feature*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x; clog << " ]\t"; simple_print_example_features(ec);}

  remove_history_from_example(ec);
  remove_policy_offset(ec, current_policy);
}

size_t predict(example *ec, history h, int policy, size_t truth)
{
  size_t yhat;
  if (policy == -1) // this is the optimal policy!
    yhat = truth;
  else {
    add_history_to_example(ec, h);
    add_policy_offset(ec, policy);

    if (all_transitions_allowed)
      ec->ld = (void*)&testall_costs;
    else
      ec->ld = (void*)&transition_prediction_costs[last_prediction(h)];

    if (PRINT_DEBUG_INFO) {clog << "before test: "; simple_print_example_features(ec); clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }
    total_predictions_made++;
    base_learner(ec);
    yhat = (size_t)(*(OAA::prediction_t*)&(ec->final_prediction));
    if (PRINT_DEBUG_INFO) {clog << " after test: " << yhat << endl;clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }
    

    remove_history_from_example(ec);
    remove_policy_offset(ec, policy);
  }
  if ((yhat <= 0) || (yhat > sequence_k)) {
    clog << "internal error (bug): predict is returning an invalid class -- replacing with 1" << endl;
    return 1;
  }
  return yhat;
}

bool warned_about_class_overage = false;
bool got_null = false;

// safe_get_example(allow_past_eof)
// reads the next example and returns it.
//
// returns NULL if we don't get a real example (either got null or end of ring)
// returns example otherwise
example* safe_get_example(int allow_past_eof) {
  got_null = false;
  if (read_example_this_loop == global.ring_size) {
    cerr << "warning: length of sequence at " << read_example_last_id << " exceeds ring size; breaking apart" << endl;
    return NULL;
  }
  example* ec = get_example();
  if (ec == NULL) {
    got_null = true;
    return NULL;
  }

  read_example_this_loop++;
  read_example_last_id = ec->example_counter;

  if (ec->pass != read_example_last_pass) {
    read_example_last_pass = ec->pass;

    if ((!allow_past_eof) && read_example_should_warn_eof) {
      cerr << "warning: sequence data does not end in empty example; please fix your data" << endl;
      read_example_should_warn_eof = 0;
    }

    // we've hit the end, we may need to switch policies
    passes_since_new_policy++;
    if (passes_since_new_policy >= sequence_passes_per_policy) {
      passes_since_new_policy = 0;
      current_policy++;
      if (current_policy > total_number_of_policies) {
        cerr << "internal error (bug): too many policies; not advancing" << endl;
        current_policy = total_number_of_policies;
      }
    }
  }

  size_t y = ((OAA::mc_label*)ec->ld)->label;
  if (y > sequence_k) {
    if (!warned_about_class_overage) {
      cerr << "warning: specified " << sequence_k << " classes, but found class " << y << "; replacing with " << sequence_k << endl;
      warned_about_class_overage = true;
    }
    ((OAA::mc_label*)ec->ld)->label = sequence_k;
  }

  return ec;
}

void run_test(example* ec)
{
  size_t yhat = 0;
  int warned = 0;
  int seq_num_features = 0;
  OAA::mc_label* old_label;

  clear_history(current_history);

  while ((ec != NULL) && (! CSOAA_LDF::example_is_newline(ec))) {
    int policy = random_policy(0);
    old_label = (OAA::mc_label*)ec->ld;

    seq_num_features += ec->num_features;
    global.sd->weighted_examples += old_label->weight;
    global.sd->total_features += ec->num_features;

    if (! CSOAA_LDF::example_is_test(ec)) {
      if (!warned) {
        cerr << "warning: mix of train and test data in sequence prediction at " << ec->example_counter << "; assuming all test" << endl;
        warned = 1;
      }
    }

    yhat = predict(ec, current_history, policy, -1);
    global_print_label(ec, yhat);

    ec->ld = old_label;

    append_history(current_history, yhat);

    free_example(ec);
    ec = safe_get_example(0);
  }
  if (ec != NULL) {
    free_example(ec);
    CSOAA_LDF::global_print_newline();
  }

  global.sd->example_number++;
  print_update(0, seq_num_features);
}

void process_next_example_sequence()
{
  int seq_num_features = 0;
  read_example_this_loop = 0;

  example *cur_ec = safe_get_example(1);
  if (cur_ec == NULL)
    return;

  // skip initial newlines
  while (CSOAA_LDF::example_is_newline(cur_ec)) {
    CSOAA_LDF::global_print_newline();
    free_example(cur_ec);
    cur_ec = safe_get_example(1);
    if (cur_ec == NULL)
      return;
  }

  if (CSOAA_LDF::example_is_test(cur_ec)) {
    run_test(cur_ec);
    return;
  }

  // we know we're training
  size_t n = 0;
  int skip_this_one = 0;
  while ((cur_ec != NULL) && (! CSOAA_LDF::example_is_newline(cur_ec))) {
    if (CSOAA_LDF::example_is_test(cur_ec) && !skip_this_one) {
      cerr << "warning: mix of train and test data in sequence prediction at " << cur_ec->example_counter << "; skipping" << endl;
      skip_this_one = 1;
    }

    ec_seq[n] = cur_ec;
    n++;
    cur_ec = safe_get_example(0);
  }

  if (skip_this_one) {
    for (size_t i=0; i<n; i++)
      free_example(ec_seq[n]);
    if (cur_ec != NULL)
      free_example(cur_ec);
    return;
  }

  // we've now read in all the examples up to n, time to pick some
  // policies; policy -1 is optimal policy
  clear_history(current_history);
  true_labels.erase();
  for (size_t t=0; t<n; t++) {
    policy_seq[t] = (current_policy == 0) ? 0 : random_policy(0);
    push(true_labels, (OAA::mc_label*)ec_seq[t]->ld);

    seq_num_features += ec_seq[t]->num_features;
    global.sd->weighted_examples += true_labels[t]->weight;
    global.sd->total_features += ec_seq[t]->num_features;

    // predict everything and accumulate loss
    pred_seq[t] = predict(ec_seq[t], current_history, policy_seq[t], -1);
    global_print_label(ec_seq[t], pred_seq[t]);
    append_history(current_history, pred_seq[t]);
    if (pred_seq[t] != true_labels[t]->label) { // incorrect prediction
      global.sd->sum_loss += true_labels[t]->weight;
      global.sd->sum_loss_since_last_dump += true_labels[t]->weight;
    }

    // allow us to use the optimal policy
    if (random_policy(1) == -1)
      policy_seq[t] = -1;
  }
  CSOAA_LDF::global_print_newline();

  global.sd->example_number++;
  print_update(1, seq_num_features);

  bool all_policies_optimal = true;
  for (size_t t=0; t<n; t++) {
    if (policy_seq[t] >= 0) all_policies_optimal = false;
    if (t == 0) {
      if (policy_seq[0] == -1)
        pred_seq[0] = true_labels[0]->label;
    } else {
      pred_seq[t] = -1;
    }
  }

  // start learning
  if (PRINT_DEBUG_INFO) {clog << "===================================================================" << endl;}
  clear_history(current_history);

  size_t last_new = -1;
  int prediction_matches_history = 0;
  for (size_t t=0; t<n; t++) {
    // we're making examples at position t
    for (size_t i=0; i<sequence_k; i++) {
      clear_history(all_histories[i]);
      // NOTE: have to keep all_histories and hcache[i].predictions
      // seperate to avoid copy by value versus copy by pointer issues
      hcache[i].predictions = all_histories[i];
      hcache[i].predictions_hash = 0;
      hcache[i].loss = true_labels[t]->weight * (float)((i+1) != true_labels[t]->label);
      hcache[i].same = 0;
      hcache[i].alive = all_transitions_allowed || valid_transition[last_prediction(current_history)][i] || (i+1==pred_seq[t]);
      hcache[i].original_label = i;
      append_history_item(hcache[i], i+1);
    }

    size_t end_pos = (n < t+1+sequence_rollout) ? n : (t+1+sequence_rollout);

    // so we can break early, figure out when the position AFTER the LAST non-optimal policy
    // this is the position AFTER which we can stop
    if (all_policies_optimal)
      end_pos = t+1;
    else
      while (end_pos >= t+1) {
        if (policy_seq[end_pos-1] >= 0)
          break;
        end_pos--;
      }

    bool entered_rollout = false;
    float gamma = 1;
    for (size_t t2=t+1; t2<end_pos; t2++) {
      gamma *= sequence_gamma;
      if (OPTIMIZE_SHARED_HISTORIES) {
        sort_hcache_and_mark_equality();
        if (hcache_all_equal_or_dead())
          break;
      }
      entered_rollout = true;
      for (size_t i=0; i < sequence_k; i++) {
        if (!hcache[i].alive) { break; }  // we hit the dead rollouts!

        prediction_matches_history = 0;
        if (OPTIMIZE_SHARED_HISTORIES && hcache[i].same) {
          // copy from the previous cache
          if (last_new < 0) {
            cerr << "internal error (bug): sequence histories match, but no new items; skipping" << endl;
            goto NOT_REALLY_NEW;
          }

          prediction_matches_history  = (t2 == t+1) && (last_prediction(hcache[i].predictions) == pred_seq[t]);

          hcache[i].predictions       = hcache[last_new].predictions;
          hcache[i].predictions_hash  = hcache[last_new].predictions_hash;
          hcache[i].loss             += gamma * true_labels[t2]->weight * (float)(last_prediction(hcache[last_new].predictions) != true_labels[t2]->label);
        } else {
NOT_REALLY_NEW:
          // compute new
          last_new = i;

          prediction_matches_history = (t2 == t+1) && (last_prediction(hcache[i].predictions) == pred_seq[t]);

          size_t yhat = predict(ec_seq[t2], hcache[i].predictions, policy_seq[t2], true_labels[t2]->label);
          append_history_item(hcache[i], yhat);
          hcache[i].loss += gamma * true_labels[t2]->weight * (float)(yhat != true_labels[t2]->label);
        }
        hcache[i].same = 0;

        if (prediction_matches_history) { // this is what we would have predicted
          pred_seq[t+1] = last_prediction(hcache[i].predictions);
          if ((pred_seq[t+1] <= 0) || (pred_seq[t+1] > sequence_k)) {
            cerr << "internal error (bug): last_prediction is returning an invalid prediction; replacing with 1" << endl;
            pred_seq[t+1] = 1;
          }
        }
      }
    }

    if (entered_rollout && ((pred_seq[t+1] <= 0) || (pred_seq[t+1] > sequence_k))) {
      cerr << "internal error (bug): did not find actual predicted path at " << t << "; defaulting to 1" << endl;
      pred_seq[t] = 1;
    }

    // generate the training example
    float min_loss = hcache[0].loss;
    for (size_t i=1; i < sequence_k; i++)
      if (hcache[i].loss < min_loss)
        min_loss = hcache[i].loss;

    loss_vector.erase();
    for (size_t i=0; i<sequence_k; i++) {
      if (hcache[i].alive) {
        size_t lab  = hcache[i].original_label;
        size_t cost = hcache[i].loss - min_loss;
        feature temp  = { cost, lab+1 };
        push(loss_vector, temp);
      }
    }
    generate_training_example(ec_seq[t], current_history, loss_vector);

    // update state
    append_history(current_history, pred_seq[t]);

    if ((!entered_rollout) && (t < n-1)) {
      pred_seq[t+1] = predict(ec_seq[t+1], current_history, policy_seq[t+1], true_labels[t+1]->label);
    }
  }

  for (size_t i=0; i<n; i++)
    ec_seq[i]->ld = (void*)true_labels[i];


  for (size_t i=0; i<n; i++)
    free_example(ec_seq[i]);

  if (cur_ec != NULL)
    free_example(cur_ec);
}
 

void drive_sequence()
{
  const char * header_fmt = "%-10s %-10s %8s %8s %24s %22s %8s %5s %5s %10s %10s\n";
  fprintf(stderr, header_fmt,
          "average", "since", "sequence", "example",
          "current label", "current predicted", "current", "cur", "cur", "predic.", "examples");
  fprintf(stderr, header_fmt,
          "loss", "last", "counter", "weight", "sequence prefix", "sequence prefix", "features", "pass", "pol", "made", "gener.");
  cerr.precision(5);

  allocate_required_memory();

  read_example_this_loop = 0;
  while (true) {
    process_next_example_sequence();
    if (got_null && parser_done()) // we're done learning
      break;
  }
  
  free_required_memory();
  base_finish();
}

/*
TODO: position-based history features?
*/
