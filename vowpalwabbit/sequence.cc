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
  // the following are for beam search
  float    pred_score;
  history  total_predictions;
};

bool PRINT_DEBUG_INFO             = 0;
bool PRINT_UPDATE_EVERY_EXAMPLE   = 0 | PRINT_DEBUG_INFO;
bool OPTIMIZE_SHARED_HISTORIES    = 1;
bool DEBUG_FORCE_BEAM_ONE         = 0;

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
size_t sequence_beam              = 1;

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
uint32_t      current_history_hash = 0;

v_array<example*> ec_seq        = v_array<example*>();
size_t*       pred_seq      = NULL;
int*          policy_seq    = NULL;
history*      all_histories = NULL;
history_item* hcache        = NULL;
v_array<OAA::mc_label*> true_labels = v_array<OAA::mc_label*>();
history_item* beam = NULL;
history_item* beam_backup = NULL;

CSOAA::label testall_costs = { v_array<CSOAA::wclass>() };
v_array<CSOAA::wclass> loss_vector  = v_array<CSOAA::wclass>();
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
    v_array<CSOAA::wclass> this_costs = v_array<CSOAA::wclass>();
    for (int j=0; j<file_k; j++) {   // this is TO, identified by col number; k total columns
      n = fscanf(f, "%d", &rd);
      if (n == 0) {
        cerr << "warning: could not read transitions; assuming all remaining are valid after " << i << "," << (j+1) << endl;
        return;
      }
      if ((i <= (int)sequence_k) && (j < (int)sequence_k)) {
        valid_transition[i][j] = (rd > 0);
        if (valid_transition[i][j]) {
          CSOAA::wclass feat = { FLT_MAX, j+1, 0. };
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
        r -= sequence_beta * pow(1. - sequence_beta, (float)pid);
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
  loss_vector.erase();

  if (pred_seq == NULL)
    pred_seq = (size_t*)calloc_or_die(global.ring_size, sizeof(size_t));

  if (policy_seq == NULL)
    policy_seq = (int*)calloc_or_die(global.ring_size, sizeof(int));

  if (all_histories == NULL) {
    all_histories = (history*)calloc_or_die(sequence_k * sequence_beam, sizeof(history));
    for (size_t i=0; i<sequence_k * sequence_beam; i++)
      all_histories[i] = (history)calloc_or_die(history_length, sizeof(size_t));
  }

  if (hcache == NULL) {
    hcache = (history_item*)calloc_or_die(sequence_k * sequence_beam, sizeof(history_item));
    for (size_t i=0; i<sequence_k * sequence_beam; i++)
      hcache[i].total_predictions = (history)calloc_or_die(global.ring_size, sizeof(size_t));
  }

  if (current_history == NULL)
    current_history = (history)calloc_or_die(history_length, sizeof(uint32_t));

  for (size_t i = 1; i <= global.k; i++)
    {
      CSOAA::wclass f = {FLT_MAX, i, 0.};
      push(testall_costs.costs, f);
    }
}

void clear_seq()
{
  if (ec_seq.index() > 0) 
    for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++) {
      free_example(*ecc);
    }
  ec_seq.erase();
}


void print_beam(size_t last_time)
{
  if (beam == NULL) return;
  size_t sz = sequence_beam + sequence_k;
  clog << "........................................................................................................" << endl;
  for (size_t k=0; k<sz; k++) {
    if (k < sequence_beam) clog << "*"; else clog << " ";
    clog << " " << k << "\tsc=" << beam[k].pred_score << "\t(l=" << beam[k].loss << ")\t" << (beam[k].alive ? "alive" : "dead") << "\t" << (beam[k].same ? "same" : "diff") << "\t<";

    for (size_t t=0; t<history_length; t++) {
      clog << " " << beam[k].predictions[t];
    }

    clog << " >\t|";

    for (size_t t=0; t<last_time; t++) {
      clog << " " << beam[k].total_predictions[t];
    }

    clog << " |" << endl;
  }
}
    
void print_hcache()
{
  clog << "********************************************************************************************************" << endl;
  size_t sz = sequence_k * sequence_beam;
  for (size_t k=0; k<sz; k++) {
    clog << "k=" <<k << "\tol=" << hcache[k].original_label << "\t(l=" << hcache[k].loss << ")\t" << (hcache[k].alive ? "alive" : "dead") << "\t" << (hcache[k].same ? "same" : "diff") << "\t<";

    for (size_t t=0; t<sequence_history; t++) {
      clog << " " << hcache[k].predictions[t];
    }

    clog << " >" << endl;
  }
}
    

void initialize_beam()
{
  if (beam != NULL) return;
  size_t sz = sequence_beam + sequence_k;
  beam = (history_item*)calloc_or_die(sz, sizeof(history_item));
  for (size_t k=0; k<sz; k++) {
    beam[k].predictions       = (history)calloc_or_die(history_length, sizeof(uint32_t));
    beam[k].total_predictions = (history)calloc_or_die(global.ring_size, sizeof(uint32_t));
  }
  beam_backup = (history_item*)calloc_or_die(sequence_beam, sizeof(history_item));
  for (size_t k=0; k<sequence_beam; k++) {
    beam_backup[k].predictions       = (history)calloc_or_die(history_length, sizeof(uint32_t));
    beam_backup[k].total_predictions = (history)calloc_or_die(global.ring_size, sizeof(uint32_t));
  }
}

void free_beam()
{
  size_t sz = sequence_beam + sequence_k;
  if (beam != NULL) {
    for (size_t k=0; k<sz; k++) {
      free(beam[k].predictions);
      free(beam[k].total_predictions);
    }
    free(beam);
    beam = NULL;

    for (size_t k=0; k<sequence_beam; k++) {
      free(beam_backup[k].predictions);
      free(beam_backup[k].total_predictions);
    }
    free(beam_backup);
    beam = NULL;
  }
}

void free_required_memory()
{
  clear_seq();
  if (ec_seq.begin != NULL)
    free(ec_seq.begin);

  if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1)
    for (size_t i=0; i<sequence_k * sequence_beam; i++)
      free(hcache[i].total_predictions);

  free(pred_seq);        pred_seq        = NULL;
  free(policy_seq);      policy_seq      = NULL;
  free(hcache);          hcache          = NULL;
  free(current_history); current_history = NULL;

  for (size_t i=0; i<sequence_k * sequence_beam; i++)
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

  free_beam();
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

void print_update(long unsigned int seq_num_features)
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
  fprintf(stderr, "%-10.6f %-10.6f %8ld %15f   [%s] [%s] %8lu %5d %5d %15lu %15lu\n",
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
  for (CSOAA::wclass *f = c->costs.begin; f != c->costs.end; f++) {
    clog << "\t" << f->weight_index << ":" << f->x << "::" << f->partial_prediction;
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

void assign_append_history_item(history_item to, history_item from, uint32_t p)
{
  memcpy(to.predictions, from.predictions, history_length * sizeof(size_t));
  to.predictions_hash = from.predictions_hash;
  append_history_item(to, p);
}    

inline size_t last_prediction(history h)
{
  return h[history_length-1];
}

#define order_on(a,b) { if (a < b) return -1; else if (a > b) return 1; }

int order_history_item_by_score(const void* a, const void* b)  // put dead items at end, same items at end, otherwise top scoring items first
{
  history_item *ha = (history_item*)a;
  history_item *hb = (history_item*)b;
  order_on(hb->alive, ha->alive);
  order_on(ha->same , hb->same);
  order_on(ha->pred_score, hb->pred_score);
  return 0;
}

int order_history_item(const void* a, const void* b)  // put dead items at end, put higher-scoring items earlier
{
  history_item *ha = (history_item*)a;
  history_item *hb = (history_item*)b;
  order_on(hb->alive, ha->alive);
  order_on(ha->predictions_hash, hb->predictions_hash);
  if (history_length > 0)
    for (size_t j=0; j<history_length; j++) {
      size_t i = history_length - 1 - j;
      order_on(ha->predictions[i], hb->predictions[i]);
    }
  return 0;
}

int order_history_item_total(const void* a, const void* b)  // put dead items at end, put higher-scoring items earlier
{
  history_item *ha = (history_item*)a;
  history_item *hb = (history_item*)b;
  order_on(hb->alive, ha->alive);
  order_on(ha->predictions_hash, hb->predictions_hash);
  if (history_length > 0)
    for (size_t j=0; j<history_length; j++) {
      size_t i = history_length - 1 - j;
      order_on(ha->predictions[i], hb->predictions[i]);
    }
  order_on(ha->total_predictions, hb->total_predictions);
  return 0;
}

inline int cache_item_same_as_before(size_t i)
{
  return (i > 0) && (order_history_item(&hcache[i], &hcache[i-1]) == 0);
}


void sort_and_mark_equality(history_item* list, size_t len)
{
  qsort(list, len, sizeof(history_item), order_history_item);
  hcache[0].same = 0;
  for (size_t i=1; i<len; i++) {
    int order = order_history_item(&list[i], &list[i-1]);
    list[i].same = (order == 0);
  }
}

int hcache_all_equal_or_dead()
{
  if (!hcache[0].alive) return 1;
  for (size_t i=1; i<sequence_k * sequence_beam; i++) {
    if (!hcache[i].alive) return 1;
    if (!hcache[i].same)  return 0;
  }
  return 1;
}


inline void clear_history(history h)  // TODO: memset this
{
  for (size_t t=0; t<history_length; t++)
    h[t] = 0;
}

inline void copy_history(history to, history from) // TODO: memcpy this
{
  for (size_t t=0; t<history_length; t++)
    to[t] = from[t];
}

void copy_history_item(history_item *to, history_item from, bool copy_pred, bool copy_total)
{
  if (copy_pred) memcpy((*to).predictions, from.predictions, history_length * sizeof(uint32_t));
  (*to).predictions_hash = from.predictions_hash;
  (*to).loss = from.loss;
  (*to).original_label = from.original_label;
  (*to).same = from.same;
  (*to).alive = from.alive;
  (*to).pred_score = from.pred_score;
  if (copy_total) memcpy((*to).total_predictions, from.total_predictions, global.ring_size * sizeof(size_t));
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




void (*base_learner)(example*) = NULL;
void (*base_finish)() = NULL;


void generate_training_example(example *ec, history h, v_array<CSOAA::wclass>costs)
{
  CSOAA::label ld = { costs };

  add_history_to_example(ec, h);
  add_policy_offset(ec, current_policy);

  if (PRINT_DEBUG_INFO) {clog << "before train: costs = ["; for (CSOAA::wclass*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x; clog << " ]\t"; simple_print_example_features(ec);}
  ec->ld = (void*)&ld;
  total_examples_generated++;
  base_learner(ec);
  if (PRINT_DEBUG_INFO) {clog << " after train: costs = ["; for (CSOAA::wclass*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x << "::" << c->partial_prediction; clog << " ]\t"; simple_print_example_features(ec);}

  remove_history_from_example(ec);
  remove_policy_offset(ec, current_policy);
}

size_t predict(example *ec, history h, int policy, size_t truth)
{
  size_t yhat;
  if (policy == -1) { // this is the optimal policy!
    yhat = truth;
    if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1) {
      CSOAA::label *label = all_transitions_allowed ? &testall_costs : &transition_prediction_costs[last_prediction(h)];
      ec->ld = (void*)label;
      for (CSOAA::wclass *f = label->costs.begin; f != label->costs.end; f++)
        f->partial_prediction = (f->weight_index == truth) ? 0. : 1.;
    }
  } else {
    add_history_to_example(ec, h);
    add_policy_offset(ec, policy);

    ec->ld = all_transitions_allowed ? (void*)&testall_costs : (void*)&transition_prediction_costs[last_prediction(h)];

    if (PRINT_DEBUG_INFO) {clog << "before test: "; simple_print_example_features(ec);clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }
    total_predictions_made++;
    base_learner(ec);
    yhat = (size_t)(*(OAA::prediction_t*)&(ec->final_prediction));
    if (PRINT_DEBUG_INFO) {clog << " after test: " << yhat << ", pp=" << ec->partial_prediction << endl;clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }

    remove_history_from_example(ec);
    remove_policy_offset(ec, policy);
  }
  if ((yhat <= 0) || (yhat > sequence_k)) {
    clog << "internal error (bug): predict is returning an invalid class [" << yhat << "] -- replacing with 1" << endl;
    return 1;
  }
  return yhat;
}

bool warned_about_class_overage = false;

void take_beam_step(v_array<CSOAA::wclass> costs, size_t t, size_t k, size_t *idx)
{
  for (CSOAA::wclass *wc = costs.begin; wc != costs.end; wc++) {
    size_t yhat = wc->weight_index;
    
    memcpy(beam[*idx].total_predictions, beam_backup[k].total_predictions, t * sizeof(size_t));
    assign_append_history_item(beam[*idx], beam_backup[k], yhat);
    beam[*idx].loss = beam_backup[k].loss;
    beam[*idx].original_label = beam_backup[k].original_label;
    beam[*idx].same = false;
    beam[*idx].alive = true;
    beam[*idx].pred_score = beam_backup[k].pred_score + wc->partial_prediction;
    beam[*idx].total_predictions[t] = yhat;
    (*idx)++;
  }
}

void finalize_beam(size_t *idx)
{
  if (*idx <= 1) return;
  // now, sort so that we can find cases for merging
  sort_and_mark_equality(beam, *idx);

  // we only need to keep the top sequence_beam *alive* AND not(same) items
  // we can do this by re-sorting on score
  qsort(beam, *idx, sizeof(history_item), order_history_item_by_score);

  // just go through the remaining items and kill them
  (*idx) = sequence_beam;
  while (true) { // initially, *idx > 1
    if (beam[*idx-1].alive) break;
    (*idx) --;
    if (*idx == 0) break;
  }
}

void clear_beam(history_item* b, size_t sz)
{
  for (size_t k=0; k<sz; k++) {
    clear_history(b[k].predictions);
    b[k].predictions_hash = 0;
    b[k].loss = 0.;
    b[k].original_label = 0;
    b[k].same = false;
    b[k].alive = false;
    b[k].pred_score = 0.;
    memset(b[k].total_predictions, 0, global.ring_size * sizeof(size_t));
  }
}

void run_test_beam()
{
  size_t n = ec_seq.index();
  OAA::mc_label* old_label;
  size_t sz = sequence_beam + sequence_k;

  clear_beam(beam, sz);

  // create initial "old" beam
  clear_beam(beam_backup, sequence_beam);
  beam_backup[0].alive = true;

  for (size_t t=0; t<n; t++) {
    old_label = (OAA::mc_label*)ec_seq[t]->ld;

    size_t idx = 0;
    for (size_t k=0; k<sequence_beam; k++) {
      if (! beam_backup[k].alive) break;  // the rest are guaranteed to be dead
      predict(ec_seq[t], beam_backup[k].predictions, policy_seq[t], -1);
      take_beam_step(((CSOAA::label*)ec_seq[t]->ld)->costs, t, k, &idx);
      ec_seq[t]->ld = old_label;
      finalize_beam(&idx);
    }

    // copy top k to beam_backup
    if (t < n-1) {
      for (size_t k=0; k<idx; k++)
        copy_history_item(&beam_backup[k], beam[k], true, true);
      for (size_t k=idx; k<sz; k++)
        beam[k].alive = false;
    }
    //clog << "test @ t=" << t << endl;
    //print_beam(t+1);
  }

  // the top scoring output should be in beam[0]
  for (size_t t=0; t<n; t++)
    pred_seq[t] = beam[0].total_predictions[t];
}

void run_test()
{
  OAA::mc_label* old_label;

  clear_history(current_history); current_history_hash = 0;
  for (size_t t=0; t<ec_seq.index(); t++) {
    old_label = (OAA::mc_label*)ec_seq[t]->ld;
    pred_seq[t] = predict(ec_seq[t], current_history, policy_seq[t], -1);
    append_history(current_history, pred_seq[t]);
    ec_seq[t]->ld = old_label;
  }
}

void run_test_common_init()
{
  for (size_t t=0; t<ec_seq.index(); t++)
    policy_seq[t] = (current_policy == 0) ? 0 : random_policy(0);
}

void run_test_common_final(bool do_printing)
{
  size_t seq_num_features = 0;
  for (size_t t=0; t<ec_seq.index(); t++) {
    if (do_printing) global_print_label(ec_seq[t], pred_seq[t]);
    seq_num_features += ec_seq[t]->num_features;
  }
  if (do_printing) print_update(seq_num_features);
}

void run_train_common_init()
{
  size_t n = ec_seq.index();
  size_t seq_num_features = 0;
  true_labels.erase();
  for (size_t t=0; t<n; t++) {
    push(true_labels, (OAA::mc_label*)ec_seq[t]->ld);

    seq_num_features             += ec_seq[t]->num_features;
    global.sd->total_features    += ec_seq[t]->num_features;
    global.sd->weighted_examples += true_labels[t]->weight;

    if (pred_seq[t] != true_labels[t]->label) { // incorrect prediction
      global.sd->sum_loss += true_labels[t]->weight;
      global.sd->sum_loss_since_last_dump += true_labels[t]->weight;
    }

    // global_print_label(ec_seq[t], pred_seq[t]);

    // allow us to use the optimal policy for the future
    if (random_policy(1) == -1)
      policy_seq[t] = -1;
  }
  global.sd->example_number++;
  print_update(seq_num_features);
}

void run_train_common_final()
{
  for (size_t i=0; i<ec_seq.index(); i++)
    ec_seq[i]->ld = (void*)true_labels[i];
}


void run_train()
{
  size_t n = ec_seq.index();
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
  clear_history(current_history); current_history_hash = 0;

  size_t last_new = -1;
  int prediction_matches_history = 0;

  for (size_t t=0; t<n; t++) {
    // we're making examples at position t
    for (size_t i=0; i<sequence_k; i++) {
      copy_history(all_histories[i], current_history);
      // NOTE: have to keep all_histories and hcache[i].predictions
      // seperate to avoid copy by value versus copy by pointer issues
      hcache[i].predictions = all_histories[i];
      hcache[i].predictions_hash = current_history_hash;
      hcache[i].loss = true_labels[t]->weight * (float)((i+1) != true_labels[t]->label);
      hcache[i].same = 0;
      hcache[i].alive = all_transitions_allowed || valid_transition[last_prediction(current_history)][i] || (i+1==pred_seq[t]);
      hcache[i].original_label = i;
      hcache[i].pred_score = 0.;
      //hcache[i].total_predictions = NULL;
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
        sort_and_mark_equality(hcache, sequence_k);
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

          /*
          clog << "predict @ " << t2 << " via " << t << " with hcache[" << i << "] := ";
            clog << "\tol=" << hcache[i].original_label << "\t(l=" << hcache[i].loss << ")\t" << (hcache[i].alive ? "alive" : "dead") << "\t" << (hcache[i].same ? "same" : "diff") << "\t<";
            for (size_t ttt=0; ttt<sequence_history; ttt++) { clog << " " << hcache[i].predictions[ttt]; }
            clog << " >" << endl;
          */

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
      if (hcache[i].alive && (hcache[i].loss < min_loss))
        min_loss = hcache[i].loss;

    loss_vector.erase();
    for (size_t i=0; i<sequence_k; i++) {
      if (hcache[i].alive) {
        size_t lab  = hcache[i].original_label;
        size_t cost = hcache[i].loss - min_loss;
        CSOAA::wclass temp  = { cost, lab+1, 0. };
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
}

void run_train_beam()
{
  run_train();
  return;
  size_t n = ec_seq.index();
  size_t sz = sequence_k + sequence_beam;
  OAA::mc_label* old_label;

  clear_beam(beam, sz);

  // create initial "old" beam
  clear_beam(beam_backup, sequence_beam);
  beam_backup[0].alive = true;

  for (size_t t=0; t<n; t++) {
    // we're making examples at position t

    // we're going to have sequence_k many roll-outs for the TOP
    // sequence_beam items in the beam.  initialize the cache.
    for (size_t k=0; k<sequence_beam; k++) {
      if (!beam_backup[k].alive)
        for (size_t i=0; i<sequence_k; i++)
          hcache[i * sequence_beam + k].alive = false;
      else 
        for (size_t i=0; i<sequence_k; i++) {
          size_t id = i * sequence_beam + k;
          hcache[id].predictions = all_histories[id];
          copy_history_item(&hcache[id], beam_backup[k], true, false);
          hcache[id].loss = true_labels[t]->weight * (float)((i+1) != true_labels[t]->label);
          hcache[id].original_label = i;
          hcache[id].alive = true;
          hcache[id].same = false;
          append_history_item(hcache[id], i+1);
        }
    }

    // it's possible that we can already kill some of these; do so
    sort_and_mark_equality(hcache, sequence_beam*sequence_k);

    //clog << "initialize hcache:" << endl;
    //print_hcache();

    size_t end_pos = (n < t+1+sequence_rollout) ? n : (t+1+sequence_rollout);
    float gamma = 1.;
    for (size_t t2=t+1; t2<end_pos; t2++) {
      old_label = (OAA::mc_label*)ec_seq[t2]->ld;
      gamma *= sequence_gamma;

      // TODO: optimize shared history

      // for each HYPOTHESIS, we want to do a single-step BEAM rollout ... we can do all of them at once
      for (size_t i=0; i<sequence_k; i++) {
        // set it up like this is our only example
        //clog << "t=" << t << " t2=" << t2 << " i=" << i << endl;
        //        for (size_t k=0; k<sequence_beam; k++) {
          //          size_t id = i * sequence_beam + k;
          //          copy_history_item(&beam[k], hcache[id], true, false);
          //clog << "copy beam[" << k << "] <- hcache[" << id << "]" << endl;
          //        }
        //clog << "before step:" << endl; print_beam(t2+1);
        // make step
        clear_beam(beam, sz);
        size_t idx = 0;
        for (size_t k=0; k<sequence_beam; k++) {
          size_t id = i * sequence_beam + k;
          //if (! beam[k].alive) break;
          predict(ec_seq[t2], hcache[id].predictions, policy_seq[t2], true_labels[t2]->label);
          //simple_print_costs((CSOAA::label*)ec_seq[t2]->ld);
          take_beam_step(((CSOAA::label*)ec_seq[t2]->ld)->costs, t2, k, &idx);
          ec_seq[t2]->ld = old_label;
          finalize_beam(&idx);
          //clog << "after step " << k << ":" << endl; print_beam(t2+1);
        }
        //clog << "after finalize:" << endl; print_beam(t2+1);
        // now, the top K elements of beam contain our next positions, which we just move back into hcache
        for (size_t k=0; k<sequence_beam; k++) {
          size_t id = k * sequence_k + i;
          copy_history_item(&hcache[id], beam[k], true, false);
          //clog << "copy hcache[" << id << "] <- beam[" << k << "]" << endl;
          //hcache[id].loss += gamma * true_labels[t2]->weight * (float)(last_prediction(hcache[id].predictions) != true_labels[t2]->label);
        }
        //clog << "new hcache:" << endl; print_hcache();
      }
    }

    // generate the training examples
    float min_loss = hcache[0].loss;
    for (size_t id=1; id < sequence_k * sequence_beam; id++)  // TODO: should this be only over beam?
      if (hcache[id].alive && (hcache[id].loss < min_loss))
        min_loss = hcache[id].loss;
    min_loss = 0;

    for (size_t k=0; k<sequence_beam; k++) {
      // restore the beam
      copy_history_item(&beam[k], beam_backup[k], true, true);

      // create the example
      loss_vector.erase();
      for (size_t i=0; i<sequence_k; i++) {
        size_t id = k * sequence_k + i;
        if (hcache[id].alive) {
          size_t lab  = hcache[id].original_label % sequence_k;
          size_t cost = hcache[id].loss - min_loss;
          CSOAA::wclass temp  = { cost, lab+1, 0. };
          push(loss_vector, temp);
        }
      }
      //clog << "generate_training_example based on beam[" << k << "].predictions = <";
      //for (size_t tt=0; tt<history_length; tt++) { clog << " " << beam[k].predictions[tt]; }
      //clog << " >" << endl;
      generate_training_example(ec_seq[t], beam[k].predictions, loss_vector);
    }

    // take a step
    //clog << "before final step for iteration:" << endl;
    //print_beam(t+1);
    size_t idx = sequence_beam;
    for (size_t k=0; k<sequence_beam; k++) {
      if (! beam[k].alive) break;
      predict(ec_seq[t], beam[k].predictions, policy_seq[t], true_labels[t]->label);
      take_beam_step(((CSOAA::label*)ec_seq[t]->ld)->costs, t, k, &idx);
    }
    //clog << "before finalize step for iteration idx=" << idx << ":" << endl;
    //print_beam(t+1);
    finalize_beam(&idx);
    //clog << "final step for iteration:" << endl;
    //print_beam(t+1);
  }
}

void do_actual_learning()
{
  if (ec_seq.index() <= 0) return; // nothing to do

  bool any_train = false;
  bool any_test  = false;

  for (example **ec = ec_seq.begin; ec != ec_seq.end; ec++)
    if (CSOAA_LDF::example_is_test(*ec)) { any_test = true; }
    else { any_train = true; }

  if (any_train && any_test)
    cerr << "warning: mix of train and test data in sequence prediction at " << ec_seq[0]->example_counter << "; treating as test" << endl;

  run_test_common_init();
  if (sequence_beam > 1 || DEBUG_FORCE_BEAM_ONE) run_test_beam();
  else                                           run_test();
  run_test_common_final(true);

  if (! any_test && global.training) {
    run_train_common_init();
    if (sequence_beam > 1 || DEBUG_FORCE_BEAM_ONE) run_train_beam();
    else                                           run_train();
    run_train_common_final();
  }
}
 


/********************************************************************************************
 *** MAIN ALGORITHM
 ********************************************************************************************/


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

  if (vm.count("sequence_transition_file")) {
    all_transitions_allowed = false;
    read_transition_file(vm["sequence_transition_file"].as<string>().c_str());
  } else
    all_transitions_allowed = true;

  if (vm.count("sequence_beam")) {
    sequence_beam = vm["sequence_beam"].as<size_t>();
    if (sequence_beam < 1) {
      cerr << "cannot have --sequence_beam < 1; resetting to 1" << endl;
      sequence_beam = 1;
    }
    if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1)
      initialize_beam();
  }

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


void finish()
{
  free_required_memory();
  base_finish();
}

void learn(example *ec) {
  if (ec_seq.index() >= global.ring_size - 2) { // give some wiggle room
    cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << endl;
    do_actual_learning();
    clear_seq();
  }

  if (CSOAA_LDF::example_is_newline(ec)) {
    do_actual_learning();
    clear_seq();
    CSOAA_LDF::global_print_newline();
    free_example(ec);
  } else {
    read_example_this_loop++;
    read_example_last_id = ec->example_counter;
    if (ec->pass != read_example_last_pass) {
      read_example_last_pass = ec->pass;
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
    if (((OAA::mc_label*)ec->ld)->label > sequence_k) {
      if (!warned_about_class_overage) {
        cerr << "warning: specified " << sequence_k << " classes, but found class " << ((OAA::mc_label*)ec->ld)->label << "; replacing with " << sequence_k << endl;
        warned_about_class_overage = true;
      }
      ((OAA::mc_label*)ec->ld)->label = sequence_k;
    }

    push(ec_seq, ec);
  }
}

void drive_sequence()
{
  const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";

  if (global.training) {
    fprintf(stderr, header_fmt,
            "average", "since", "sequence", "example",
            "current label", "current predicted", "current", "cur", "cur", "predic.", "examples");
    fprintf(stderr, header_fmt,
            "loss", "last", "counter", "weight", "sequence prefix", "sequence prefix", "features", "pass", "pol", "made", "gener.");
  }
  cerr.precision(5);

  allocate_required_memory();

  example* ec = NULL;
  read_example_this_loop = 0;
  while (true) {
    if ((ec = get_example()) != NULL) { // semiblocking operation
      learn(ec);
    } else if (parser_done()) {
      do_actual_learning();
      finish();
      return;
    }
  }
}

/*
TODO: position-based history features?

  make && rm -f foo && ./vw --cache_file foo -d z --passes 1 --sequence 45 --sequence_beam 10
  gprof ./vw
  // > vw.gprof

*/

// namespace BEAM {
//   beam* create(size_t mx_el, 
//                uint32_t(*hs_el)(const void *),
//                float(*cs_el)(const void *),
//                int(*cmp_el)(const void *, const void *))
//   {
//     beam* b = (beam*)malloc(sizeof(beam));
//     if (b == NULL)
//       return NULL;

//     b->mx_el = mx_el;
//     b->nm_el = 0;
//     b->hs_el = hs_el;
//     b->cs_el = cs_el;
//     b->cmp_el = cmp_el;

//     b->cost_array = (cost_item*)malloc(mx_el * sizeof(cost_item));
//     if (b->cost_array == NULL) {
//       free(b);
//       return NULL;
//     }

//     b->hash_array = (hash_item*)malloc(mx_el * sizeof(hash_item));
//     if (b->hash_array == NULL) {
//       free(b->cost_array);
//       free(b);
//       return NULL;
//     }

//     return b;
//   }

//   void erase(beam* b)
//   {
//     if (b == NULL) return;
//     b->nm_el = 0;
//   }

//   void free_all(beam* b)
//   {
//     free(b->cost_array);
//     free(b->hash_array);
//   }

//   size_t to_array(beam* b, void** array)
//   {
//     if (b == NULL) return 0;
//     for (size_t i=0; i<b->nm_el; i++)
//       array[i] = b->cost_array[i].elem;
//     return b->nm_el;
//   }

//   size_t num_elems(beam* b)
//   {
//     if (b == NULL) return 0;
//     return b->nm_el;
//   }

//   bool full(beam* b) {
//     if (b == NULL) return true;
//     return (b->nm_el == b->mx_el);
//   }

//   float best_cost(beam *b)
//   {
//     if (b == NULL) return FLT_MAX;
//     if (b->nm_el == 0) return FLT_MAX;
//     return b->cost_array[0].cost;
//   }

//   float worst_cost(beam *b)
//   {
//     if (b == NULL) return FLT_MIN;
//     if (b->nm_el == 0) return FLT_MIN;
//     return b->cost_array[b->nm_el-1].cost;
//   }

//   // returns where this element SHOULD be.  if it's already there, found
//   // is set to true.
//   size_t find_elem(beam* b, void* elem, bool *found)
//   {
//     hash_item*arr = b->hash_array;
//     uint32_t hash = b->hs_el(elem);

//     *found = false;

//     // do binary search on HASH
//     size_t   lo   = 0;
//     size_t   hi   = b->nm_el;  // this is NOT inclusive
//     size_t   mi   = 0;

//     while (true) {
//       if (hi - lo < 4) {  // just do linear search
//         if (arr[lo].hash == hash) { mi = lo  ; break; }
//         if ((lo+1<hi) && (arr[lo+1].hash == hash)) { mi = lo+1; break; }
//         if ((lo+2<hi) && (arr[lo+2].hash == hash)) { mi = lo+2; break; }
//         if ((lo+3<hi) && (arr[lo+3].hash == hash)) { mi = lo+3; break; }
//         return b->mx_el;
//       }
//       // these's a gap between lo and hi, so mi is well defined and we
//       // can do meaningful comparisons
//       mi = (lo+hi)/2;

//       if (arr[mi].hash == hash) { break; }
//       else if (arr[mi].hash <  hash) { lo = mi+1; }
//       else { hi = mi; }
//     }

//     // now, mi points to something that has the same _hash_.  we'll do
//     // linear search forward and backward from that position to find
//     // the same element.

//     // maybe we got lucky :)
//     void* mi_elem = b->cost_array[ arr[mi].cost_index ].elem;
//     size_t ord = b->cmp_el( elem, mi_elem );
//     if (ord == 0) { *found = true; return mi; }  // we found it!
//     else if (ord < 0) {
//       // we are less; search backward
//       for (mi = mi-1; mi>=0; mi++) {
//         if (arr[mi].hash != hash) return b->mx_el;
//         mi_elem = b->cost_array[ arr[mi].cost_index ].elem;
//         size_t ord = b->cmp_el( elem, mi_elem );
//         if (ord == 0) { *found = true; return mi; }  // we found it!
//         else if (ord > 0) { return b->mx_el; }
//       }
//       return b->mx_el;
//     } else {
//       // we are greater; search forward
//       for (mi = mi+1; mi<b->nm_el; mi++) {
//         if (arr[mi].hash != hash) return b->mx_el;
//         mi_elem = b->cost_array[ arr[mi].cost_index ].elem;
//         size_t ord = b->cmp_el( elem, mi_elem );
//         if (ord == 0) { *found = true; return mi; }  // we found it!
//         else if (ord < 0) { return b->mx_el; }
//       }
//       return b->mx_el;
//     }
//   }

//   void swap_elements_cost(beam* b, size_t i, size_t j)
//   {
//     // we need to exchange items i and j=i+1 in the cost array
//     size_t hash_i = b->cost_array[i].hash_index;
//     size_t hash_j = b->cost_array[j].hash_index;

//     float  cost_i = b->cost_array[i].cost;
//     void*  elem_i = b->cost_array[i].elem;
    
//     b->cost_array[i].cost = b->cost_array[j].cost;
//     b->cost_array[i].elem = b->cost_array[j].elem;
//     b->cost_array[i].hash_index = hash_j;

//     b->cost_array[j].cost = cost_i;
//     b->cost_array[j].elem = elem_i;
//     b->cost_array[j].hash_index = hash_i;

//     b->hash_array[hash_i].cost_index = j;
//     b->hash_array[hash_j].cost_index = i;
//   }

//   void update(beam* b, void* elem)
//   {
//     float new_cost = b->cs_el(elem);
//     if (full(b) && new_cost >= worst_cost(b)) // not even worth trying
//       return;

//     bool   found;
//     size_t hs_idx = find_elem(b, elem, &found);
//     if (found) {
//       // this is an old element
//       size_t cs_idx = b->hash_array[hs_idx].cost_index;
//       cost_item old_item = b->cost_array[cs_idx];
//       if (new_cost >= old_item.cost) // not worth updating
//         return;

//       // if we're already at the top, just adjust our cost
//       if (cs_idx == 0) {
//         b->cost_array[0].cost = new_cost;
//         return;
//       }

//       // at this point, we need to percolate old_elem higher
//       // and adjust all corresponding pointers
//       size_t new_idx = cs_idx;
//       while ((new_idx >= 0) && (b->cost_array[new_idx].cost > new_cost))
//         new_idx--;

//       // now, we rearrange so that cs_idx -> new_idx and then for
//       // all i st new_idx < i < cs_idx, position i -> i+1
//       for (size_t i=cs_idx-1; i>=new_idx && i < b->mx_el; i--) 
//         swap_elements_cost(b, i, i+1); // exchange item i with item i+1
//       // TODO: the above could be more efficient: from 2n to n+1 operations

//       // success
//       return;
//     }

//     // this is a new element that NEEDS to get inserted because we
//     // know it doesn't have too bad of a cost.  figure out where (in
//     // the cost array) it needs to go
//     size_t cs_idx = find_cost(b, new_cost);

//     // now, i <- i-1, for everything at cs_idx+1 to end
//     size_t end = full(b) ? b->mx_el : (b->nm_el + 1);
//     for (size_t i=end-1; i>=cs_idx+1 && i<end; i--)
//       swap_elements_cost(b, i, i-1);

//     // finally, insert the new item at position cs_idx
//     hash_item hi = { b->hs_el(elem), cs_idx };
//     cost_item ci = { new_cost, hs_idx, elem };
    
//   }
// }
