#include "io.h"
#include "sequence.h"
#include "parser.h"
#include "constant.h"

size_t sequence_history=1;
bool sequence_bigrams = false;
size_t sequence_features = 0;
bool sequence_bigram_features = false;
size_t sequence_rollout = 256;
size_t sequence_passes_per_policy = 1;
float sequence_beta = 0.5;
size_t sequence_k=2;
size_t history_length = 1;
size_t current_policy = 0;

uint32_t history_constant = 8290741;


history   current_history;

example**     ec_seq        = NULL;
size_t*       pred_seq      = NULL;
size_t*       policy_seq    = NULL;
history*      all_histories = NULL;
history_item* hcache        = NULL;
float*        loss_vector   = NULL;

#define sort_hcache() qsort(hcache, sequence_k, sizeof(history_item), order_history_item)

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
  *(global.lp)=oaa_label;
  *gf = sequence_get;
  global.get_example = sequence_get;
  *rf = sequence_return;
  global.return_example = sequence_return;
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

  history_length = ( sequence_history > sequence_features ) ? sequence_history : sequence_features;
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

void add_history_to_example(example* ec, history h)
{
  size_t v0, v;

  for (size_t t=1; t<=sequence_history; t++) {
    v0 = h[history_length-t] * quadratic_constant + history_constant;

    // add the basic history features
    feature temp = {1, (uint32_t) ( v0 & global.parse_mask )};
    push(ec->atomics[history_namespace], temp);
    ec->sum_feat_sq[history_namespace] ++;

    // add the bigram features
    if ((t > 1) && sequence_bigrams) {
      v0 *= quadratic_constant;
      v0 += h[history_length-t+1] * quadratic_constant + history_constant;

      feature temp = {1, (uint32_t) ( v0 & global.parse_mask )};
      push(ec->atomics[history_namespace], temp);
      ec->sum_feat_sq[history_namespace] ++;
    }
  }

  if (sequence_features > 0) {
    for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; f++) {
        v = (f->weight_index + history_constant) * quadratic_constant;

        for (size_t t=1; t<=sequence_features; t++) {
          v0 = h[history_length-t] + history_constant;
          
          // add the history/feature pair
          feature temp = {1, (uint32_t) ( (v + v0) & global.parse_mask )};
          push(ec->atomics[history_namespace], temp);
          ec->sum_feat_sq[history_namespace] ++;

          // add the bigram
          if ((t > 0) && sequence_bigram_features) {
            v0 *= quadratic_constant;
            v0 += h[history_length-1-t] + history_constant;

            feature temp = {1, (uint32_t) ( (v + v0) & global.parse_mask )};
            push(ec->atomics[history_namespace], temp);
            ec->sum_feat_sq[history_namespace] ++;
          }
        }
      }
    }
  }

  push(ec->indices, history_namespace);
  ec->total_sum_feat_sq += ec->sum_feat_sq[history_namespace];
  ec->num_features += ec->atomics[history_namespace].index();
}

int hcache_all_equal()
{
  for (size_t i=1; i<sequence_k; i++)
    if (order_history_item(&hcache[i], &hcache[i-1]) != 0)
      return 0;
  return 1;
}

inline int example_is_newline(example* ec)
{
  // TODO
  return 0;
}

inline int example_is_test(example* ec)
{
  // TODO
  return 0;
}

inline void clear_history(history h)
{
  for (size_t t=0; t<history_length; t++)
    h[t] = 0;
}

void generate_training_example(example *ec, float* loss)
{
  // use policy current_policy
  // TODO: do something
}

size_t predict(example *ec, history h, size_t policy)
{
  add_history_to_example(ec, h);
  size_t yhat = 1; // TODO: predict(ec, policy);
  remove_history_from_example(ec);
  return yhat;
}

int run_test(example* ec)  // returns 0 if eof, otherwise returns 1
{
  size_t yhat = 0;
  int warned = 0;
  clear_history(current_history);

  while ((ec != NULL) && (! example_is_newline(ec))) {
    size_t policy = (size_t)(rand() * (float)(current_policy));

    if (! example_is_test(ec)) {
      if (!warned) {
        std::cerr << "warning: mix of train and test data in sequence prediction at " << ec->example_counter << "; assuming all test" << std::endl;
        warned = 1;
      }
    }

    yhat = predict(ec, current_history, policy);
    append_history(current_history, yhat);
    ec = get_example();
  }

  return (ec != NULL);
}

size_t get_label(example* ec)
{
  // TODO: john?
  return 0;
}

float get_weight(example* ec)
{
  // TODO: john?
  return 1.;
}

void allocate_required_memory()
{
  if (ec_seq == NULL)
    ec_seq = (example**)malloc_or_die(sizeof(example*) * global.ring_size);

  if (pred_seq == NULL)
    pred_seq = (size_t*)malloc_or_die(sizeof(size_t) * global.ring_size);

  if (policy_seq == NULL)
    policy_seq = (size_t*)malloc_or_die(sizeof(size_t) * global.ring_size);

  if (all_histories == NULL)
    all_histories = (history*)malloc_or_die(sizeof(history) * sequence_k);

  if (hcache == NULL)
    hcache = (history_item*)malloc_or_die(sizeof(history_item) * sequence_k);

  if (loss_vector == NULL)
    loss_vector = (float*)malloc_or_die(sizeof(float) * sequence_k);
}

void free_required_memory()
{
  free(ec_seq);         ec_seq        = NULL;
  free(pred_seq);       pred_seq      = NULL;
  free(policy_seq);     policy_seq    = NULL;
  free(all_histories);  all_histories = NULL;
  free(hcache);         hcache        = NULL;
  free(loss_vector);    loss_vector   = NULL;
}

int process_next_example_sequence()  // returns 0 if EOF, otherwise returns 1
{
  example *cur_ec = get_example();
  if (cur_ec == NULL) {
    // TODO: john this is EOF, what should we do?
    free_required_memory();
    return 0;
  }

  // skip initial newlines
  while (example_is_newline(cur_ec)) {
    cur_ec = get_example();
    if (cur_ec == NULL) {
      // TODO: john this is EOF, what should we do?
      free_required_memory();
      return 0;
    }
  }

  if (example_is_test(cur_ec))
    return run_test(cur_ec);

  // we know we're training
  allocate_required_memory();

  size_t n = 0;
  int skip_this_one = 0;
  while ((cur_ec != NULL) && (! example_is_newline(cur_ec))) {
    // TODO: check to see if n is too big and fail if so
    // john: what happens???

    if (example_is_test(cur_ec) && !skip_this_one) {
      std::cerr << "warning: mix of train and test data in sequence prediction at " << cur_ec->example_counter << "; skipping" << std::endl;
      skip_this_one = 1;
    }

    ec_seq[n] = cur_ec;
    n++;
    cur_ec = get_example();
  }

  if (skip_this_one)
    return 1;

  // we've now read in all the examples up to n, time to pick some
  // policies; policy -1 is optimal policy
  for (size_t i=0; i<n; i++) {
    policy_seq[i] = (size_t)(rand() * (float)(current_policy + 1)) - 1;
    pred_seq[i] = -1;
  }

  // start learning
  clear_history(current_history);
  for (size_t i=0; i<sequence_k; i++)
    clear_history(hcache[i].predictions);

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
      append_history_item(hcache[i], i);
    }

    size_t end_pos = (n < t+1+sequence_rollout) ? n : (t+1+sequence_rollout);
    for (size_t t2=t+1; t2<end_pos; t2++) {
      sort_hcache();
      for (size_t i=0; i < sequence_k; i++) {
        prediction_matches_history = 0;
        if (cache_item_same_as_before(i)) {
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

        if (prediction_matches_history) // this is what we would have predicted
          pred_seq[t+1] = last_prediction(hcache[i].predictions);
      }

      if (hcache_all_equal())
        break;
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

    for (size_t i=0; i < sequence_k; i++)
      loss_vector[i] = hcache[i].loss - min_loss;

    add_history_to_example(ec_seq[t], current_history);
    generate_training_example(ec_seq[t], loss_vector);
    remove_history_from_example(ec_seq[t]);

    // udpate state
    append_history(current_history, pred_seq[t]);
    if ((sequence_rollout == 0) && (t < n-1))
      pred_seq[t+1] = predict(ec_seq[t+1], current_history, policy_seq[t+1]);

  }

  return 1;
}
 
