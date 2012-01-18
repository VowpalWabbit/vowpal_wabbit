#include <stdio.h>
#include <math.h>
#include "io.h"
#include "sequence.h"
#include "parser.h"
#include "constant.h"
#include "oaa.h"
#include "csoaa.h"

size_t sequence_history           = 1;
bool   sequence_bigrams           = false;
size_t sequence_features          = 0;
bool   sequence_bigram_features   = false;
size_t sequence_rollout           = 256;
size_t sequence_passes_per_policy = 1;
float  sequence_beta              = 0.5;
size_t sequence_k                 = 2;
float  sequence_rollout_prob      = 1.;

size_t history_length             = 1;
size_t current_policy             = 0;
size_t total_number_of_policies   = 1;

uint32_t      history_constant    = 8290741;
history       current_history     = NULL;

example**     ec_seq        = NULL;
size_t*       pred_seq      = NULL;
int*          policy_seq    = NULL;
history*      all_histories = NULL;
history_item* hcache        = NULL;

v_array<float> loss_vector  = v_array<float>();

size_t        max_string_length = 8;

void* malloc_or_die(size_t size)
{
  void* data = malloc(size);
  if (data == NULL) {
    std::cerr << "internal error: memory allocation failed; dying!" << std::endl;
    exit(-1);
  }
  return data;
}

example* sequence_get()
{
  example* ec = get_example();
  return ec;
}

void sequence_return(example* ec)
{
  free_example(ec);
}

void parse_sequence_args(po::variables_map& vm, example* (**gf)(), void (**rf)(example*))
{
  *(global.lp)=OAA::oaa_label;
  *gf = sequence_get;
  *rf = sequence_return;
  sequence_k = vm["sequence"].as<size_t>();

  if (vm.count("sequence_bigrams"))
    sequence_bigrams = true;
  if (vm.count("sequence_bigram_features"))
    sequence_bigrams = true;

  if (vm.count("sequence_features"))
    sequence_features = vm["sequence_features"].as<size_t>();
  if (vm.count("sequence_rollout"))
    sequence_rollout = vm["sequence_rollout"].as<size_t>();
  if (vm.count("sequence_passes_per_policy"))
    sequence_passes_per_policy = vm["sequence_passes_per_policy"].as<size_t>();
  if (vm.count("sequence_beta"))
    sequence_beta = vm["sequence_beta"].as<float>();
  if (vm.count("sequence_rollout_prob"))
    sequence_beta = vm["sequence_rollout_prob"].as<float>();

  history_length = ( sequence_history > sequence_features ) ? sequence_history : sequence_features;
  total_number_of_policies = global.numpasses / sequence_passes_per_policy;

  max_string_length = max((int)(ceil( log10((float)history_length+1) )),
                          (int)(ceil( log10((float)sequence_k+1) ))) + 1;

}

inline void append_history(history h, uint32_t p)
{
  for (size_t i=0; i<history_length-1; i++)
    h[i] = h[i+1];
  h[history_length-1] = p;
}

void append_history_item(history_item hi, uint32_t p)
{
  // TODO: make this more efficient by being smart about the hash
  append_history(hi.predictions, p);
  uint32_t hash = 1;
  for (size_t i=0; i<history_length; i++)
    hash = (hash + hi.predictions[i]) * history_constant;
  hi.predictions_hash = hash;
  hi.same = 0;
}

inline size_t last_prediction(history h)
{
  return h[history_length-1];
}

int order_history_item(const void* a, const void* b)
{
  int j = ((history_item*)a)->predictions_hash - ((history_item*)b)->predictions_hash;
  if (j != 0)
    return j;
  else
    for (size_t i=history_length-1; i>=0; i--) {
      j = ((history_item*)a)->predictions[i] - ((history_item*)b)->predictions[i];
      if (j != 0)
        return j;
    }
  return 0;
}

inline int cache_item_same_as_before(size_t i)
{
  return (i > 0) && (order_history_item(&hcache[i], &hcache[i-1]) == 0);
}

void remove_history_from_example(example* ec)
{
  if (ec->indices.index() == 0) {
    std::cerr << "internal error (bug): trying to remove history, but there are no namespaces!" << std::endl;
    return;
  }

  if (ec->indices.last() != history_namespace) {
    std::cerr << "internal error (bug): trying to remove history, but either it wasn't added, or something was added after and not removed!" << std::endl;
    return;
  }

  ec->num_features -= ec->atomics[history_namespace].index();
  ec->total_sum_feat_sq -= ec->sum_feat_sq[history_namespace];
  ec->sum_feat_sq[history_namespace] = 0;
  ec->atomics[history_namespace].erase();
  ec->indices.decr();
}

std::string audit_feature_space("history");

void add_history_to_example(example* ec, history h)
{
  size_t v0, v;

  for (size_t t=1; t<=sequence_history; t++) {
    v0 = (h[history_length-t] * quadratic_constant + history_constant) * quadratic_constant + t + history_constant;

    // add the basic history features
    feature temp = {1, (uint32_t) ( v0 & global.parse_mask )};
    push(ec->atomics[history_namespace], temp);
    ec->sum_feat_sq[history_namespace] ++;

    if (global.audit) {
      audit_data a_feature = { NULL, NULL, (uint32_t)(v0 & global.parse_mask), 1., true };
      a_feature.space = (char*)malloc_or_die(sizeof(char) * (audit_feature_space.length()+1));
      strcpy(a_feature.space, audit_feature_space.c_str());

      a_feature.feature = (char*)malloc_or_die(sizeof(char) * (5 + 2*max_string_length));
      sprintf(a_feature.feature, "ug@%d=%d", t, h[history_length-t]);

      push(ec->audit_features[history_namespace], a_feature);
    }

    // add the bigram features
    if ((t > 1) && sequence_bigrams) {
      v0 *= quadratic_constant;
      v0 += h[history_length-t+1] * quadratic_constant + history_constant;

      feature temp = {1, (uint32_t) ( v0 & global.parse_mask )};
      push(ec->atomics[history_namespace], temp);
      ec->sum_feat_sq[history_namespace] ++;

      if (global.audit) {
        audit_data a_feature = { NULL, NULL, (uint32_t)(v0 & global.parse_mask), 1., true };
        a_feature.space = (char*)malloc_or_die(sizeof(char) * (audit_feature_space.length()+1));
        strcpy(a_feature.space, audit_feature_space.c_str());

        a_feature.feature = (char*)malloc_or_die(sizeof(char) * (6 + 3*max_string_length));
        sprintf(a_feature.feature, "bg@%d=%d-%d", t, h[history_length-t], h[history_length-t+1]);

        push(ec->audit_features[history_namespace], a_feature);
      }

    }
  }

  std::string fstring;

  if (sequence_features > 0) {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
      int feature_index = 0;
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++) {

        if (global.audit) {
          fstring = std::string(ec->audit_features[*i][feature_index].feature);
          feature_index++;
        }

        v = (f->weight_index + history_constant) * quadratic_constant;

        for (size_t t=1; t<=sequence_features; t++) {
          v0 = (t + constant) * quadratic_constant + h[history_length-t] + history_constant;
          
          // add the history/feature pair
          feature temp = {1, (uint32_t) ( (v + v0) & global.parse_mask )};
          push(ec->atomics[history_namespace], temp);
          ec->sum_feat_sq[history_namespace] ++;

          if (global.audit) {
            audit_data a_feature = { NULL, NULL, (uint32_t)((v+v0) & global.parse_mask), 1., true };
            a_feature.space = (char*)malloc_or_die(sizeof(char) * (audit_feature_space.length()+1));
            strcpy(a_feature.space, audit_feature_space.c_str());

            a_feature.feature = (char*)malloc_or_die(sizeof(char) * (7 + 2*max_string_length + fstring.length()));
            sprintf(a_feature.feature, "ug+f@%d=%d=%s", t, h[history_length-t], fstring.c_str());

            push(ec->audit_features[history_namespace], a_feature);
          }


          // add the bigram
          if ((t > 0) && sequence_bigram_features) {
            v0 *= quadratic_constant;
            v0 += h[history_length-t+1] + history_constant;

            feature temp = {1, (uint32_t) ( (v + v0) & global.parse_mask )};
            push(ec->atomics[history_namespace], temp);
            ec->sum_feat_sq[history_namespace] ++;

            if (global.audit) {
              audit_data a_feature = { NULL, NULL, (uint32_t)((v+v0) & global.parse_mask), 1., true };
              a_feature.space = (char*)malloc_or_die(sizeof(char) * (audit_feature_space.length()+1));
              strcpy(a_feature.space, audit_feature_space.c_str());

              a_feature.feature = (char*)malloc_or_die(sizeof(char) * (8 + 3*max_string_length + fstring.length()));
              sprintf(a_feature.feature, "bg+f@%d-%d=%d=%s", t, h[history_length-t], h[history_length-t+1], fstring.c_str());

              push(ec->audit_features[history_namespace], a_feature);
            }

          }
        }
      }
    }
  }

  push(ec->indices, history_namespace);
  ec->total_sum_feat_sq += ec->sum_feat_sq[history_namespace];
  ec->num_features += ec->atomics[history_namespace].index();
}


void sort_hcache_and_mark_equality()
{
  qsort(hcache, sequence_k, sizeof(history_item), order_history_item);
  for (size_t i=1; i<sequence_k; i++)
    hcache[i].same = (order_history_item(&hcache[i], &hcache[i-1]) != 0);
}

int hcache_all_equal()
{
  for (size_t i=1; i<sequence_k; i++)
    if (!hcache[i].same)
      return 0;
  return 1;
}

inline size_t get_label(example* ec)
{
  return ((OAA::oaa_data*)ec->ld)->label;
}

inline float get_weight(example* ec)
{
  return ((OAA::oaa_data*)ec->ld)->weight;
}

inline int example_is_newline(example* ec)
{
  // if only index is constant namespace or no index
  return ((ec->indices.index() == 0) || 
          ((ec->indices.index() == 1) &&
           (ec->indices.last() == constant_namespace)));
}

inline int example_is_test(example* ec)
{
  return (get_label(ec) == (uint32_t)-1);
}

inline void clear_history(history h)
{
  for (size_t t=0; t<history_length; t++)
    h[t] = 0;
}

void add_policy_offset(example *ec, size_t policy)
{
  // TODO john: there is a weird thing that happens here if we just learn one policy cf feature overlapping...
  size_t amount = (policy * global.length() / sequence_k / total_number_of_policies) * global.stride;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
    feature* end = ec->atomics[*i].end;
    for (feature* f = ec->atomics[*i].begin; f!= end; f++)
      f->weight_index += amount;
  }
  if (global.audit) {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
        for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
          f->weight_index += amount;
  }
}

void remove_policy_offset(example *ec, size_t policy)
{
  size_t amount = (policy * global.length() / sequence_k / total_number_of_policies) * global.stride;
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
    feature* end = ec->atomics[*i].end;
    for (feature* f = ec->atomics[*i].begin; f!= end; f++)
      f->weight_index -= amount;
  }
  if (global.audit) {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) 
      if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
        for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; f++)
          f->weight_index -= amount;
  }
}


csoaa_data empty_costs = { v_array<float>() };

void generate_training_example(example *ec, history h, size_t label, v_array<float>costs)
{
  csoaa_data ld = { costs };

  add_history_to_example(ec, h);
  add_policy_offset(ec, current_policy);

  ec->ld = (void*)&ld;
  global.cs_learn(ec);

  remove_policy_offset(ec, current_policy);
  remove_history_from_example(ec);
}

size_t predict(example *ec, history h, int policy)
{
  if (policy == -1) { // this is the optimal policy!
    return get_label(ec);
  } else {
    add_history_to_example(ec, h);
    add_policy_offset(ec, policy);

    ec->ld = (void*)&empty_costs;
    global.cs_learn(ec);
    size_t yhat = (size_t)ec->final_prediction;

    remove_policy_offset(ec, policy);
    remove_history_from_example(ec);

    return yhat;
  }
}

int random_policy(int allow_optimal, int allow_current)
{
  if ((sequence_beta <= 0) || (sequence_beta >= 1)) {
    if (allow_current) return (int)current_policy;
    if (current_policy > 0) return (((int)current_policy)-1);
    if (allow_optimal) return -1;
    std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
    return (int)current_policy;
  }

  int num_valid_policies = (int)current_policy + allow_optimal + allow_current;
  int pid = -1;

  if (num_valid_policies == 0) {
    std::cerr << "internal error (bug): no valid policies to choose from!  defaulting to current" << std::endl;
    return (int)current_policy;
  } else if (num_valid_policies == 1) {
    pid = 0;
  } else {
    float r = rand();
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
  if (!allow_current)
    pid--;

  return pid;
}

size_t read_example_this_loop  = 0;
size_t read_example_last_id    = 0;
int    read_example_ring_error = 0;

example* safe_get_example() {
  if (read_example_this_loop == global.ring_size) {
    std::cerr << "warning: length of sequence at " << read_example_last_id << " exceeds ring size; breaking apart" << std::endl;
    read_example_ring_error = 1;
    return NULL;
  }
  read_example_ring_error = 0;
  example* ec = get_example();
  if (ec == NULL)
    return NULL;
  read_example_this_loop++;
  read_example_last_id = ec->example_counter;
  return ec;
}

int run_test(example* ec)  // returns 0 if eof, otherwise returns 1
{
  size_t yhat = 0;
  int warned = 0;

  if (current_history == NULL)
    current_history = (history)malloc_or_die(sizeof(uint32_t) * history_length);

  clear_history(current_history);

  while ((ec != NULL) && (! example_is_newline(ec))) {
    int policy = random_policy(0, 0);

    if (! example_is_test(ec)) {
      if (!warned) {
        std::cerr << "warning: mix of train and test data in sequence prediction at " << ec->example_counter << "; assuming all test" << std::endl;
        warned = 1;
      }
    }

    yhat = predict(ec, current_history, policy);
    append_history(current_history, yhat);
    ec = safe_get_example();
  }

  return ((ec != NULL) || read_example_ring_error);
}

void allocate_required_memory()
{
  if (ec_seq == NULL) {
    ec_seq = (example**)malloc_or_die(sizeof(example*) * global.ring_size);
    for (size_t i=0; i<global.ring_size; i++)
      ec_seq[i] = NULL;
  }

  if (pred_seq == NULL)
    pred_seq = (size_t*)malloc_or_die(sizeof(size_t) * global.ring_size);

  if (policy_seq == NULL)
    policy_seq = (int*)malloc_or_die(sizeof(int) * global.ring_size);

  if (all_histories == NULL) {
    all_histories = (history*)malloc_or_die(sizeof(history) * sequence_k);
    for (size_t i=0; i<sequence_k; i++)
      all_histories[i] = (history)malloc_or_die(sizeof(size_t) * history_length);
  }

  if (hcache == NULL)
    hcache = (history_item*)malloc_or_die(sizeof(history_item) * sequence_k);

  if (current_history == NULL)
    current_history = (history)malloc_or_die(sizeof(uint32_t) * history_length);
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

  free(all_histories);   all_histories   = NULL;
}

int process_next_example_sequence()  // returns 0 if EOF, otherwise returns 1
{
  read_example_this_loop = 0;

  example *cur_ec = safe_get_example();
  if ((cur_ec == NULL) && (!read_example_ring_error))
    return 0;

  // skip initial newlines
  while (example_is_newline(cur_ec)) {
    cur_ec = safe_get_example();
    if ((cur_ec == NULL) && (!read_example_ring_error))
      return 0;
  }

  if (example_is_test(cur_ec))
    return run_test(cur_ec);

  // we know we're training

  size_t n = 0;
  int skip_this_one = 0;
  while ((cur_ec != NULL) && (! example_is_newline(cur_ec))) {
    if (example_is_test(cur_ec) && !skip_this_one) {
      std::cerr << "warning: mix of train and test data in sequence prediction at " << cur_ec->example_counter << "; skipping" << std::endl;
      skip_this_one = 1;
    }

    ec_seq[n] = cur_ec;
    n++;
    cur_ec = safe_get_example();
  }

  if (skip_this_one)
    return 1;

  // we've now read in all the examples up to n, time to pick some
  // policies; policy -1 is optimal policy
  for (size_t i=0; i<n; i++) {
    policy_seq[i] = random_policy(1, 0);
    pred_seq[i] = -1;
  }

  // start learning
  clear_history(current_history);
  for (size_t i=0; i<sequence_k; i++) {
    clear_history(all_histories[i]);
    hcache[i].predictions = NULL;
    hcache[i].same = 0;
  }

  // predict the first one
  pred_seq[0] = predict(ec_seq[0], current_history, policy_seq[0]);

  size_t last_new = -1;
  int prediction_matches_history = 0;
  for (size_t t=0; t<n; t++) {
    // we're making examples at position t
    for (size_t i=0; i<sequence_k; i++) {
      clear_history(all_histories[i]);
      hcache[i].predictions = all_histories[i];
      hcache[i].predictions_hash = 0;
      hcache[i].loss = get_weight(ec_seq[t]) * (float)(i != get_label(ec_seq[t]));
      hcache[i].same = 0;
      append_history_item(hcache[i], i);
    }

    size_t end_pos = (n < t+1+sequence_rollout) ? n : (t+1+sequence_rollout);
    for (size_t t2=t+1; t2<end_pos; t2++) {
      sort_hcache_and_mark_equality();
      if (hcache_all_equal())
        break;
      for (size_t i=0; i < sequence_k; i++) {
        prediction_matches_history = 0;
        if (hcache[i].same) {
          // copy from the previous cache
          if (last_new < 0) {
            std::cerr << "internal error (bug): sequence histories match, but no new items; skipping" << std::endl;
            goto NOT_REALLY_NEW;
          }

          prediction_matches_history  = (t2 == t+1) && (last_prediction(hcache[i].predictions) == pred_seq[t]);

          hcache[i].predictions       = hcache[last_new].predictions;
          hcache[i].predictions_hash  = hcache[last_new].predictions_hash;
          hcache[i].loss             += get_weight(ec_seq[t2]) * (float)(last_prediction(hcache[last_new].predictions) != get_label(ec_seq[t2]));
        } else {
NOT_REALLY_NEW:
          // compute new
          last_new = i;

          prediction_matches_history = (t2 == t+1) && (last_prediction(hcache[i].predictions) == pred_seq[t]);
          size_t yhat = predict(ec_seq[t2], hcache[i].predictions, policy_seq[t2]);
          append_history_item(hcache[i], yhat);
          hcache[i].loss += get_weight(ec_seq[t2]) * (float)(yhat != get_label(ec_seq[t2]));
        }
        hcache[i].same = 0;

        if (prediction_matches_history) // this is what we would have predicted
          pred_seq[t+1] = last_prediction(hcache[i].predictions);
      }
    }

    if (pred_seq[t] < 0) {
      std::cerr << "internal error (bug): did not find actual predicted path; defaulting to 1" << std::endl;
      pred_seq[t] = 1;
    }


    // generate the training example
    float min_loss = hcache[0].loss;
    size_t best_label = 0;
    for (size_t i=1; i < sequence_k; i++)
      if (hcache[i].loss < min_loss) {
        min_loss = hcache[i].loss;
        best_label = i;
      }

    loss_vector.erase();
    for (size_t i=0; i < sequence_k; i++)
      push(loss_vector, hcache[i].loss - min_loss);

    generate_training_example(ec_seq[t], current_history, min_loss, loss_vector);

    // udpate state
    append_history(current_history, pred_seq[t]);
    if ((sequence_rollout == 0) && (t < n-1))
      pred_seq[t+1] = predict(ec_seq[t+1], current_history, policy_seq[t+1]);

  }

  return 1;
}
 

void drive_sequence()
{
  size_t passes_since_new_policy = 0;
  
  global.cs_initialize();
  allocate_required_memory();

  while (true) {
    int more_to_go = process_next_example_sequence();
    if (!more_to_go) { // we've reached the end of the file
      if (parser_done()) // we're done learning
        break;

      // we've hit the end, we may need to switch policies
      passes_since_new_policy++;
      if (passes_since_new_policy >= sequence_passes_per_policy) {
        passes_since_new_policy = 0;
        current_policy++;
        if (current_policy > total_number_of_policies) {
          std::cerr << "internal error (bug): too many policies; not advancing" << std::endl;
          current_policy = total_number_of_policies;
        }
      }
    }
  }

  free_required_memory();
  global.cs_finish();
}
