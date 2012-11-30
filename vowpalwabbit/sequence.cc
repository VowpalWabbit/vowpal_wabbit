/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
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
#include "searn.h"
#include "vw.h"

namespace Sequence {

  typedef size_t* history;  // histories have the most recent prediction at the END

  struct history_item {
    history  predictions;
    size_t   predictions_hash;
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

  //non-reentrant
  size_t sequence_rollout           = 256;
  size_t sequence_passes_per_policy = 1;
  float  sequence_beta              = 0.5;
  size_t sequence_k                 = 2;
  float sequence_gamma             = 1.;
  bool   sequence_allow_current_policy = false;
  size_t sequence_beam              = 1;

  size_t increment                  = 0; //for policy offset

  bool   all_transitions_allowed    = true;
  bool** valid_transition           = NULL;

  size_t current_policy             = 0;
  size_t read_example_last_pass     = 0;
  size_t total_number_of_policies   = 1;

  size_t constant_pow_length = 0;

  size_t total_predictions_made     = 0;
  size_t total_examples_generated   = 0;

  history       current_history     = NULL;
  size_t      current_history_hash = 0;

  SearnUtil::history_info hinfo;

  v_array<example*> ec_seq    = v_array<example*>();
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
    FILE *f;
#ifdef _WIN32
	f = fopen(filename, "rb");
#else
	f = fopen(filename, "r");
#endif
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
            CSOAA::wclass feat = { FLT_MAX, j+1, 1., 0., 0. };
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



  void allocate_required_memory(vw& all)
  {
    loss_vector.erase();

    if (pred_seq == NULL)
      pred_seq = (size_t*)calloc_or_die(all.p->ring_size, sizeof(size_t));

    if (policy_seq == NULL)
      policy_seq = (int*)calloc_or_die(all.p->ring_size, sizeof(int));

    if (all_histories == NULL) {
      all_histories = (history*)calloc_or_die(sequence_k * sequence_beam, sizeof(history));
      for (size_t i=0; i<sequence_k * sequence_beam; i++)
        all_histories[i] = (history)calloc_or_die(hinfo.length, sizeof(size_t));
    }

    if (hcache == NULL) {
      hcache = (history_item*)calloc_or_die(sequence_k * sequence_beam, sizeof(history_item));
      for (size_t i=0; i<sequence_k * sequence_beam; i++)
        hcache[i].total_predictions = (history)calloc_or_die(all.p->ring_size, sizeof(size_t));
    }

    if (current_history == NULL)
      current_history = (history)calloc_or_die(hinfo.length, sizeof(size_t));

    for (size_t i = 1; i <= all.sd->k; i++)
      {
        CSOAA::wclass cost = {FLT_MAX, i, 1., 0., 0.};
        push(testall_costs.costs, cost);
      }
  }

  void clear_seq(vw&all)
  {
    if (ec_seq.index() > 0) 
      for (example** ecc=ec_seq.begin; ecc!=ec_seq.end; ecc++) 
	VW::finish_example(all, *ecc);
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

      for (size_t t=0; t<hinfo.length; t++) {
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
    size_t total_length = max(hinfo.features, hinfo.length);
    for (size_t k=0; k<sz; k++) {
      clog << "k=" <<k << "\tol=" << hcache[k].original_label << "\t(l=" << hcache[k].loss << ")\t" << (hcache[k].alive ? "alive" : "dead") << "\t" << (hcache[k].same ? "same" : "diff") << "\t<";

      for (size_t t=0; t<total_length; t++) {
        clog << " " << hcache[k].predictions[t];
      }

      clog << " >" << endl;
    }
  }
    

  void initialize_beam(vw&all)
  {
    if (beam != NULL) return;
    size_t sz = sequence_beam + sequence_k;
    beam = (history_item*)calloc_or_die(sz, sizeof(history_item));
    for (size_t k=0; k<sz; k++) {
      beam[k].predictions       = (history)calloc_or_die(hinfo.length, sizeof(size_t));
      beam[k].total_predictions = (history)calloc_or_die(all.p->ring_size, sizeof(size_t));
    }
    beam_backup = (history_item*)calloc_or_die(sequence_beam, sizeof(history_item));
    for (size_t k=0; k<sequence_beam; k++) {
      beam_backup[k].predictions       = (history)calloc_or_die(hinfo.length, sizeof(size_t));
      beam_backup[k].total_predictions = (history)calloc_or_die(all.p->ring_size, sizeof(size_t));
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

  void free_required_memory(vw&all)
  {
    clear_seq(all);
    if (ec_seq.begin != NULL)
      free(ec_seq.begin);

    loss_vector.erase();
    free(loss_vector.begin);

    transition_prediction_costs.erase();
    free(transition_prediction_costs.begin);

    for (size_t i=0; i<sequence_k * sequence_beam; i++)
      free(hcache[i].total_predictions);

    free(pred_seq);        pred_seq        = NULL;
    free(policy_seq);      policy_seq      = NULL;
    free(hcache);          hcache          = NULL;
    free(current_history); current_history = NULL;

    if (all_histories) {
      for (size_t i=0; i<sequence_k * sequence_beam; i++)
        free(all_histories[i]);
      free(all_histories);   all_histories   = NULL;
    }

    if (!all_transitions_allowed) {
      for (size_t i=0; i<sequence_k+1; i++)
        free(valid_transition[i]);
      free(valid_transition);
      valid_transition = NULL;
    }

    true_labels.erase();
    free(true_labels.begin);

    if (testall_costs.costs.begin != NULL)
      free(testall_costs.costs.begin);

    free_beam();
  }


  /********************************************************************************************
   *** OUTPUTTING FUNCTIONS
   ********************************************************************************************/

  void global_print_label(vw&all, example *ec, size_t label)
  {
    for (size_t i=0; i<all.final_prediction_sink.index(); i++) {
      int f = (int)all.final_prediction_sink[i];
      all.print(f, (float)label, 0., ec->tag);
    }
  }


  void print_history(history h)
  {
    clog << "[ ";
    for (size_t t=0; t<hinfo.length; t++)
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

  void print_update(vw& all, long unsigned int seq_num_features)
  {
    if (!(all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)) {
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
      numspr = sprintf(true_label+pos+strlen, "%lu", (long unsigned int)true_labels[i]->label);
      true_label[pos+numspr+strlen] = ' ';

      strlen = num_len - (int)ceil(log10f((float)pred_seq[i]+1));
      numspr = sprintf(pred_label+pos+strlen, "%lu", (long unsigned int)pred_seq[i]);
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
    //  int net_time = (int) (t_end_all.time - t_start_all.time);
    fprintf(stderr, "%-10.6f %-10.6f %8ld %15f   [%s] [%s] %8lu %5d %5d %15lu %15lu\n",
            all.sd->sum_loss/all.sd->weighted_examples,
            all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
            (long int)all.sd->example_number,
            all.sd->weighted_examples,
            true_label,
            pred_label,
            seq_num_features,
            (int)read_example_last_pass,
            (int)current_policy,
            (long unsigned int)total_predictions_made,
            (long unsigned int)total_examples_generated);
    //          net_time);
     
    all.sd->sum_loss_since_last_dump = 0.0;
    all.sd->old_weighted_examples = all.sd->weighted_examples;
    all.sd->dump_interval *= 2;
  }

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

  /********************************************************************************************
   *** HISTORY MANIPULATION
   ********************************************************************************************/

  inline void append_history(history h, size_t p)
  {
    for (size_t i=1; i<hinfo.length; i++)
      h[i-1] = h[i];
    if (hinfo.length > 0)
      h[hinfo.length-1] = (size_t)p;
  }

  void append_history_item(history_item hi, size_t p)
  {
    if (hinfo.length > 0) {
      int old_val = (int)hi.predictions[0];
      hi.predictions_hash -= old_val * constant_pow_length;
      hi.predictions_hash += p;
      hi.predictions_hash *= quadratic_constant;
      append_history(hi.predictions, p);
    }

    hi.same = 0;
  }

  void assign_append_history_item(history_item to, history_item from, size_t p)
  {
    memcpy(to.predictions, from.predictions, hinfo.length * sizeof(size_t));
    to.predictions_hash = from.predictions_hash;
    append_history_item(to, p);
  }    

  inline size_t last_prediction(history h)
  {
    return h[hinfo.length-1];
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
    if (hinfo.length > 0)
      for (size_t j=0; j<hinfo.length; j++) {
        size_t i = hinfo.length - 1 - j;
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
    if (hinfo.length > 0)
      for (size_t j=0; j<hinfo.length; j++) {
        size_t i = hinfo.length - 1 - j;
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
    for (size_t t=0; t<hinfo.length; t++)
      h[t] = 0;
  }

  inline void copy_history(history to, history from) // TODO: memcpy this
  {
    for (size_t t=0; t<hinfo.length; t++)
      to[t] = from[t];
  }

  void copy_history_item(vw&all,history_item *to, history_item from, bool copy_pred, bool copy_total)
  {
    if (copy_pred) memcpy((*to).predictions, from.predictions, hinfo.length * sizeof(size_t));
    (*to).predictions_hash = from.predictions_hash;
    (*to).loss = from.loss;
    (*to).original_label = from.original_label;
    (*to).same = from.same;
    (*to).alive = from.alive;
    (*to).pred_score = from.pred_score;
    if (copy_total) memcpy((*to).total_predictions, from.total_predictions, all.p->ring_size * sizeof(size_t));
  }

  /********************************************************************************************
   *** EXAMPLE MANIPULATION
   ********************************************************************************************/


  void (*base_learner)(void*,example*) = NULL;
  void (*base_finish)(void*) = NULL;


  void generate_training_example(vw&all, example *ec, history h, v_array<CSOAA::wclass>costs)
  {
    CSOAA::label ld = { costs };

    SearnUtil::add_history_to_example(all, &hinfo, ec, h);
    SearnUtil::add_policy_offset(all, ec, increment, current_policy);

    //cerr<< "add_policy_offset: " << increment << " * " << current_policy << endl;

    if (PRINT_DEBUG_INFO) {clog << "before train: costs = ["; for (CSOAA::wclass*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x; clog << " ]\t"; simple_print_example_features(all,ec);}
    ec->ld = (void*)&ld;
    total_examples_generated++;
    base_learner(&all, ec);
    if (PRINT_DEBUG_INFO) {clog << " after train: costs = ["; for (CSOAA::wclass*c=costs.begin; c!=costs.end; c++) clog << " " << c->weight_index << ":" << c->x << "::" << c->partial_prediction; clog << " ]\t"; simple_print_example_features(all,ec);}

    SearnUtil::remove_history_from_example(all, &hinfo, ec);
    SearnUtil::remove_policy_offset(all, ec, increment, current_policy);
  }

  size_t predict(vw&all, example *ec, history h, int policy, size_t truth)
  {
    size_t yhat;
    if (policy == -1) { // this is the optimal policy!
      yhat = truth;
      if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1) {
        //CSOAA::label *label = all_transitions_allowed ? &testall_costs : &transition_prediction_costs[last_prediction(h)];
        CSOAA::label *label = true ? &testall_costs : &transition_prediction_costs[last_prediction(h)];
        ec->ld = (void*)label;
        for (CSOAA::wclass *f = label->costs.begin; f != label->costs.end; f++)
          f->partial_prediction = (f->weight_index == truth) ? 0.f : 1.f;
      }
    } else {
      SearnUtil::add_history_to_example(all, &hinfo, ec, h);
      SearnUtil::add_policy_offset(all, ec, increment, policy);

      if (PRINT_DEBUG_INFO) {
        clog << "all_costs="; simple_print_costs(&testall_costs);;
        if (! all_transitions_allowed) {
          clog << "tpr_costs="; simple_print_costs(&transition_prediction_costs[last_prediction(h)]);
        }
        clog << "  (last prediction=" << last_prediction(h) << ")" << endl;
      }
      //ec->ld = all_transitions_allowed ? (void*)&testall_costs : (void*)&transition_prediction_costs[last_prediction(h)];
      ec->ld = true ? (void*)&testall_costs : (void*)&transition_prediction_costs[last_prediction(h)];

      if (PRINT_DEBUG_INFO) {clog << "before test: "; simple_print_example_features(all, ec);clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }
      total_predictions_made++;
      base_learner(&all, ec);
      yhat = (size_t)(*(OAA::prediction_t*)&(ec->final_prediction));
      if (PRINT_DEBUG_INFO) {clog << " after test: " << yhat << ", pp=" << ec->partial_prediction << endl;clog << "costs = "; simple_print_costs((CSOAA::label*)ec->ld); }

      SearnUtil::remove_history_from_example(all, &hinfo, ec);
      SearnUtil::remove_policy_offset(all, ec, increment, policy);
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

  void clear_beam(vw&all,history_item* b, size_t sz)
  {
    for (size_t k=0; k<sz; k++) {
      clear_history(b[k].predictions);
      b[k].predictions_hash = 0;
      b[k].loss = 0.;
      b[k].original_label = 0;
      b[k].same = false;
      b[k].alive = false;
      b[k].pred_score = 0.;
      memset(b[k].total_predictions, 0, all.p->ring_size * sizeof(size_t));
    }
  }

  void run_test_beam(vw&all)
  {
    size_t n = ec_seq.index();
    OAA::mc_label* old_label;
    size_t sz = sequence_beam + sequence_k;

    clear_beam(all, beam, sz);

    // create initial "old" beam
    clear_beam(all, beam_backup, sequence_beam);
    beam_backup[0].alive = true;

    for (size_t t=0; t<n; t++) {
      old_label = (OAA::mc_label*)ec_seq[t]->ld;

      size_t idx = 0;
      for (size_t k=0; k<sequence_beam; k++) {
        if (! beam_backup[k].alive) break;  // the rest are guaranteed to be dead
        predict(all, ec_seq[t], beam_backup[k].predictions, policy_seq[t], -1);
        take_beam_step(((CSOAA::label*)ec_seq[t]->ld)->costs, t, k, &idx);
        ec_seq[t]->ld = old_label;
        finalize_beam(&idx);
      }

      // copy top k to beam_backup
      if (t < n-1) {
        for (size_t k=0; k<idx; k++)
          copy_history_item(all, &beam_backup[k], beam[k], true, true);
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

  void run_test(vw&all)
  {
    OAA::mc_label* old_label;

    if (PRINT_DEBUG_INFO) {clog << "-------------------------------------------------------------------" << endl;}

    clear_history(current_history); current_history_hash = 0;
    for (size_t t=0; t<ec_seq.index(); t++) {
      old_label = (OAA::mc_label*)ec_seq[t]->ld;
      pred_seq[t] = predict(all, ec_seq[t], current_history, policy_seq[t], -1);
      append_history(current_history, pred_seq[t]);
      ec_seq[t]->ld = old_label;
    }
  }


  void run_test_common_init()
  {
    for (size_t t=0; t<ec_seq.index(); t++)
      policy_seq[t] = (current_policy == 0) ? 0 : SearnUtil::random_policy(t, sequence_beta, sequence_allow_current_policy, current_policy, false, true);
    if (PRINT_DEBUG_INFO) {
      clog << "test policies:";
      for (size_t t=0; t<ec_seq.index(); t++) clog << " " << policy_seq[t];
      clog << endl;
    }
  }

  void run_test_common_final(vw&all, bool do_printing)
  {
    size_t seq_num_features = 0;
    for (size_t t=0; t<ec_seq.index(); t++) {
      if (do_printing) global_print_label(all, ec_seq[t], pred_seq[t]);
      seq_num_features += ec_seq[t]->num_features;
    }
    if (do_printing) print_update(all, seq_num_features);
  }

  // some edit
  void run_train_common_init(vw&all)
  {
    size_t n = ec_seq.index();
    size_t seq_num_features = 0;
    true_labels.erase();
    for (size_t t=0; t<n; t++) {
      push(true_labels, (OAA::mc_label*)ec_seq[t]->ld);

      seq_num_features             += ec_seq[t]->num_features;
      all.sd->total_features    += ec_seq[t]->num_features;
      all.sd->weighted_examples += true_labels[t]->weight;

      if (pred_seq[t] != true_labels[t]->label) { // incorrect prediction
        all.sd->sum_loss += true_labels[t]->weight;
        all.sd->sum_loss_since_last_dump += true_labels[t]->weight;
      }

      // global_print_label(ec_seq[t], pred_seq[t]);

      // allow us to use the optimal policy for the future
      if (SearnUtil::random_policy(t, sequence_beta, sequence_allow_current_policy, current_policy, true, true) == -1)
        policy_seq[t] = -1;
    }
    if (PRINT_DEBUG_INFO) {
      clog << "train policies (curp=" << current_policy<<", allow_current="<<sequence_allow_current_policy<<":";
      for (size_t t=0; t<ec_seq.index(); t++) clog << " " << policy_seq[t];
      clog << endl;
    }
    all.sd->example_number++;
    print_update(all, seq_num_features);
  }

  void run_train_common_final()
  {
    for (size_t i=0; i<ec_seq.index(); i++)
      ec_seq[i]->ld = (void*)true_labels[i];
  }


  void run_train(vw&all)
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
        assert(last_prediction(hcache[i].predictions) <= sequence_k);
        hcache[i].predictions_hash = current_history_hash;
        hcache[i].loss = true_labels[t]->weight * (float)((i+1) != true_labels[t]->label);
        hcache[i].same = 0;
        //hcache[i].alive = all_transitions_allowed || valid_transition[last_prediction(current_history)][i] || (i+1==pred_seq[t]);
        hcache[i].alive = true || valid_transition[last_prediction(current_history)][i] || (i+1==pred_seq[t]);
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
            if (last_new == (size_t)-1) {
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

            //cerr<<"t2="<<t2<<" t="<<t<<" last_prediction="<<last_prediction(hcache[i].predictions)<<" pred_seq="<<pred_seq[t]<<" pol="<<policy_seq[t2]<<endl;
            prediction_matches_history = (t2 == t+1) && (last_prediction(hcache[i].predictions) == pred_seq[t]);

            /*
              clog << "predict @ " << t2 << " via " << t << " with hcache[" << i << "] := ";
              clog << "\tol=" << hcache[i].original_label << "\t(l=" << hcache[i].loss << ")\t" << (hcache[i].alive ? "alive" : "dead") << "\t" << (hcache[i].same ? "same" : "diff") << "\t<";
              for (size_t ttt=0; ttt<total_length; ttt++) { clog << " " << hcache[i].predictions[ttt]; }
              clog << " >" << endl;
            */

            size_t yhat = predict(all, ec_seq[t2], hcache[i].predictions, policy_seq[t2], true_labels[t2]->label);
            append_history_item(hcache[i], yhat);
            //cerr<<"i="<<i<<" yhat=" << yhat<<endl;
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
        //cerr<<"last_prediction=" << pred_seq[t+1]<<endl;
        //cerr << "internal error (bug): did not find actual predicted path at " << t << "; defaulting to 1" << endl;
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
          float cost = hcache[i].loss - min_loss;
          CSOAA::wclass temp  = { cost, lab+1, 1., 0., 0. };
          push(loss_vector, temp);
        }
      }
      generate_training_example(all, ec_seq[t], current_history, loss_vector);

      // update state
      append_history(current_history, pred_seq[t]);

      if ((!entered_rollout) && (t < n-1)) {
        pred_seq[t+1] = predict(all, ec_seq[t+1], current_history, policy_seq[t+1], true_labels[t+1]->label);
      }
    }
  }

  void run_train_beam(vw&all)
  {
    run_train(all);
    return;
    size_t n = ec_seq.index();
    size_t sz = sequence_k + sequence_beam;
    OAA::mc_label* old_label;

    clear_beam(all, beam, sz);

    // create initial "old" beam
    clear_beam(all, beam_backup, sequence_beam);
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
            copy_history_item(all, &hcache[id], beam_backup[k], true, false);
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
          clear_beam(all, beam, sz);
          size_t idx = 0;
          for (size_t k=0; k<sequence_beam; k++) {
            size_t id = i * sequence_beam + k;
            //if (! beam[k].alive) break;
            predict(all, ec_seq[t2], hcache[id].predictions, policy_seq[t2], true_labels[t2]->label);
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
            copy_history_item(all, &hcache[id], beam[k], true, false);
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
        copy_history_item(all, &beam[k], beam_backup[k], true, true);

        // create the example
        loss_vector.erase();
        for (size_t i=0; i<sequence_k; i++) {
          size_t id = k * sequence_k + i;
          if (hcache[id].alive) {
            size_t lab  = hcache[id].original_label % sequence_k;
            float cost = hcache[id].loss - min_loss;
            CSOAA::wclass temp  = { cost, lab+1, 1., 0., 0. };
            push(loss_vector, temp);
          }
        }
        //clog << "generate_training_example based on beam[" << k << "].predictions = <";
        //for (size_t tt=0; tt<hinfo.length; tt++) { clog << " " << beam[k].predictions[tt]; }
        //clog << " >" << endl;
        generate_training_example(all, ec_seq[t], beam[k].predictions, loss_vector);
      }

      // take a step
      //clog << "before final step for iteration:" << endl;
      //print_beam(t+1);
      size_t idx = sequence_beam;
      for (size_t k=0; k<sequence_beam; k++) {
        if (! beam[k].alive) break;
        predict(all, ec_seq[t], beam[k].predictions, policy_seq[t], true_labels[t]->label);
        take_beam_step(((CSOAA::label*)ec_seq[t]->ld)->costs, t, k, &idx);
      }
      //clog << "before finalize step for iteration idx=" << idx << ":" << endl;
      //print_beam(t+1);
      finalize_beam(&idx);
      //clog << "final step for iteration:" << endl;
      //print_beam(t+1);
    }
  }

  void do_actual_learning(vw&all)
  {
    if (ec_seq.index() <= 0) return; // nothing to do

    bool any_train = false;
    bool any_test  = false;

    for (example **ec = ec_seq.begin; ec != ec_seq.end; ec++)
      if (OAA::example_is_test(*ec)) { any_test = true; }
      else { any_train = true; }

    if (any_train && any_test)
      cerr << "warning: mix of train and test data in sequence prediction at " << ec_seq[0]->example_counter << "; treating as test" << endl;

    run_test_common_init();
    if (sequence_beam > 1 || DEBUG_FORCE_BEAM_ONE) run_test_beam(all);
    else                                           run_test(all);
    run_test_common_final(all, any_test);

    if (! any_test ) {
      run_train_common_init(all);
      if (sequence_beam > 1 || DEBUG_FORCE_BEAM_ONE) run_train_beam(all);
      else                                           run_train(all);
      run_train_common_final();
    }
  }
 


  /********************************************************************************************
   *** MAIN ALGORITHM
   ********************************************************************************************/


  void finish(void*in)
  {
    vw*all = (vw*)in;
    free_required_memory(*all);
    base_finish(in);
  }

  void learn(void*in, example *ec) {
    vw*all = (vw*)in;
    if (ec_seq.index() >= all->p->ring_size - 2) { // give some wiggle room
      cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << endl;
      do_actual_learning(*all);
      clear_seq(*all);
    }

    if (OAA::example_is_newline(ec)) {
      do_actual_learning(*all);
      clear_seq(*all);
      CSOAA_AND_WAP_LDF::global_print_newline(*all);
      VW::finish_example(*all, ec);
    } else {
      read_example_this_loop++;
      read_example_last_id = ec->example_counter;
      if (ec->pass != read_example_last_pass) {
        read_example_last_pass = ec->pass;
        passes_since_new_policy++;
        if (passes_since_new_policy >= sequence_passes_per_policy) {
          passes_since_new_policy = 0;
          if(all->training)
            current_policy++;
          if (current_policy > total_number_of_policies) {
            cerr << "internal error (bug): too many policies; not advancing" << endl;
            current_policy = total_number_of_policies;
          }
          //reset sequence_trained_nb_policies in options_from_file so it is saved to regressor file later
          std::stringstream ss;
          ss << current_policy;
          VW::cmd_string_replace_value(all->options_from_file,"--sequence_trained_nb_policies", ss.str());
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

  void drive(void* in)
  {
    vw* all = (vw*)in;
    const char * header_fmt = "%-10s %-10s %8s %15s %24s %22s %8s %5s %5s %15s %15s\n";

    fprintf(stderr, header_fmt, "average", "since", "sequence", "example",   "current label", "current predicted",  "current",  "cur", "cur", "predic.", "examples");
    fprintf(stderr, header_fmt,    "loss",  "last",  "counter",  "weight", "sequence prefix",   "sequence prefix", "features", "pass", "pol",    "made",   "gener.");
    cerr.precision(5);

    allocate_required_memory(*all);

    example* ec = NULL;
    read_example_this_loop = 0;
    while (true) {
      if ((ec = get_example(all->p)) != NULL) { // semiblocking operation
        learn(all, ec);
      } else if (parser_done(all->p)) {
        do_actual_learning(*all);
        //finish(all);
        break;
      }
    }

    if( all->training ) {
      std::stringstream ss1;
      std::stringstream ss2;
      ss1 << (current_policy+1);
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --sequence_trained_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--sequence_trained_nb_policies", ss1.str()); 
      ss2 << total_number_of_policies;
      //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --sequence_total_nb_policies
      VW::cmd_string_replace_value(all->options_from_file,"--sequence_total_nb_policies", ss2.str());
    }
  }

  void parse_flags(vw&all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    po::options_description desc("Sequence options");
    desc.add_options()
      ("sequence_history", po::value<size_t>(), "Prediction history length for sequences")
      ("sequence_bigrams", "enable bigrams on prediction history")
      ("sequence_features", po::value<size_t>(), "create history predictions x features")
      ("sequence_bigram_features", "enable history bigrams for sequence_features")
      ("sequence_rollout", po::value<size_t>(), "maximum rollout length")
      ("sequence_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
      ("sequence_beta", po::value<float>(), "interpolation rate for policies")
      ("sequence_gamma", po::value<float>(), "discount rate for policies")
      ("sequence_max_length", po::value<size_t>(), "maximum length of sequences (default 256)")
      ("sequence_transition_file", po::value<string>(), "read valid transitions from file (default all valid)")
      ("sequence_allow_current_policy", "allow sequence labeling to use the current policy")
      ("sequence_total_nb_policies", po::value<size_t>(), "Number of policies that will eventually be trained")
      ("sequence_beam", po::value<size_t>(), "set the beam size for sequence prediction (default: 1 == greedy)");

    po::options_description add_desc_file("Sequence options in regressor file");
    add_desc_file.add_options()
      ("sequence_trained_nb_policies", po::value<size_t>(), "Number of policies trained in the file");

    po::options_description desc_file("Sequence options in regressor file");
    desc_file.add(desc).add(add_desc_file);

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(all.options_from_file_argc,all.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc_file).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);

    all.driver = drive;
    base_learner = all.learn;
    all.learn = learn;
    base_finish = all.finish;
    all.finish = finish;
    *(all.p->lp)=OAA::mc_label_parser;

    all.sequence = true;

    if( vm_file.count("sequence") ) { //we loaded a regressor file containing all the sequence options, use the ones in the file
      sequence_k = vm_file["sequence"].as<size_t>();

      if( vm.count("sequence") && vm["sequence"].as<size_t>() != sequence_k )
        std::cerr << "warning: you specified a different number of actions through --sequence than the one loaded from regressor. Pursuing with loaded value of: " << sequence_k << endl;

      if (vm_file.count("sequence_bigrams"))
        hinfo.bigrams = true;
      if (vm_file.count("sequence_bigram_features"))
        hinfo.bigram_features = true;
      if (vm_file.count("sequence_history"))
        hinfo.length = vm_file["sequence_history"].as<size_t>();
      if (vm_file.count("sequence_features"))
        hinfo.features = vm_file["sequence_features"].as<size_t>();

      if (vm_file.count("sequence_beta"))
        sequence_beta = vm_file["sequence_beta"].as<float>();

      if (vm_file.count("sequence_beam")) {
        sequence_beam = vm_file["sequence_beam"].as<size_t>();
        if (sequence_beam < 1) {
          cerr << "cannot have --sequence_beam < 1; resetting to 1" << endl;
          sequence_beam = 1;
        }
        if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1)
          initialize_beam(all);
      }

      if( vm_file.count("sequence_total_nb_policies") ) {
        total_number_of_policies = vm_file["sequence_total_nb_policies"].as<size_t>();
        if (vm.count("sequence_total_nb_policies") && vm["sequence_total_nb_policies"].as<size_t>() != total_number_of_policies)
          std::cerr << "warning: --sequence_total_nb_policies doesn't match the total number of policies stored in initial predictor. Using loaded value of: " << total_number_of_policies << endl;
      }

      if( vm_file.count("sequence_trained_nb_policies") ) {
        current_policy = vm_file["sequence_trained_nb_policies"].as<size_t>();
      }

      //check if there are a discrepancies with what user has specified in command line
      if( vm.count("sequence_bigrams") && !hinfo.bigrams )
        cerr << "warning: you specified --sequence_bigrams but loaded predictor does not use bigrams. Pursuing without bigrams." << endl;

      if( vm.count("sequence_bigram_features") && !hinfo.bigram_features )
        cerr << "warning: you specified --sequence_bigram_features but loaded predictor does not use bigram features. Pursuing without bigram features." << endl;

      if( vm.count("sequence_history") && hinfo.length != vm["sequence_history"].as<size_t>() )
        cerr << "warning: you specified a different value for --sequence_history than the one stored in loaded predictor. Pursuing with loaded value of: " << hinfo.length << endl;

      if( vm.count("sequence_features") && hinfo.features != vm["sequence_features"].as<size_t>() )
        cerr << "warning: you specified a different value for --sequence_features than the one stored in loaded predictor. Pursuing with loaded value of: " << hinfo.features << endl;

      if( vm.count("sequence_beta") && sequence_beta != vm["sequence_beta"].as<float>() )
        cerr << "warning: you specified a different value for --sequence_beta than the one stored in loaded predictor. Pursuing with loaded value of: " << sequence_beta << endl;

      if( vm.count("sequence_beam") && sequence_beam != vm["sequence_beam"].as<size_t>() )
        cerr << "warning: you specified a different value for --sequence_beam than the one stored in loaded predictor. Pursuing with loaded value of: " << sequence_beam << endl;

    }

    else {
      sequence_k = vm["sequence"].as<size_t>();

      if (vm.count("sequence_bigrams")) {
        hinfo.bigrams = true;
	all.options_from_file.append(" --sequence_bigrams");
      }

      if (vm.count("sequence_bigram_features")){
        hinfo.bigram_features = true;
	all.options_from_file.append(" --sequence_bigram_features");
      }
      
      if (vm.count("sequence_history"))
        hinfo.length = vm["sequence_history"].as<size_t>();
      else 
        hinfo.length = 1;

      if (hinfo.length == 0) {
        cerr << "warning: cannot have --sequence_history < 1: resetting to 1";
        hinfo.length = 1;
      }

      {
        stringstream ss;
        ss << " --sequence_history " << hinfo.length;
        all.options_from_file.append(ss.str());
      }

      if (vm.count("sequence_features")){
        hinfo.features = vm["sequence_features"].as<size_t>();
        stringstream ss;
        ss << " --sequence_features " << hinfo.features;
        all.options_from_file.append(ss.str());
      }
      
      if (vm.count("sequence_beam")) {
        sequence_beam = vm["sequence_beam"].as<size_t>();
        if (sequence_beam < 1) {
          cerr << "cannot have --sequence_beam < 1; resetting to 1" << endl;
          sequence_beam = 1;
        }

        stringstream ss;
        ss << " --sequence_beam " << sequence_beam;
        all.options_from_file.append(ss.str());

        if (DEBUG_FORCE_BEAM_ONE || sequence_beam > 1)
          initialize_beam(all);
      }

      if( vm.count("sequence_total_nb_policies") ) {
        total_number_of_policies = vm["sequence_total_nb_policies"].as<size_t>();
      }

      if (vm.count("sequence_beta"))
        sequence_beta = vm["sequence_beta"].as<float>();
      
      if (sequence_beta <= 0) {
        sequence_beta = 0.5;
        cerr << "warning: sequence_beta set to a value <= 0; resetting to 0.5" << endl;
      }

      //append sequence with nb_actionsand sequence_beta to options_from_file so it is saved to regressor later
      stringstream ss;
      ss << " --sequence " << sequence_k;
      ss << " --sequence_beta " << sequence_beta;
      all.options_from_file.append(ss.str());
    }

    //these remaining options are not stored in the file so always load them from command line

    if (vm.count("sequence_gamma"))
      sequence_beta = vm["sequence_gamma"].as<float>();

    if (vm.count("sequence_rollout"))
      sequence_rollout = vm["sequence_rollout"].as<size_t>();
    
    if (vm.count("sequence_passes_per_policy"))
      sequence_passes_per_policy = vm["sequence_passes_per_policy"].as<size_t>();

    if (vm.count("sequence_allow_current_policy"))
      sequence_allow_current_policy = true;

    if (vm.count("sequence_transition_file")) {
        all_transitions_allowed = false;
        read_transition_file(vm["sequence_transition_file"].as<string>().c_str());
      } else
        all_transitions_allowed = true;

    if (!all_transitions_allowed && (hinfo.length == 0)) {
      cerr << "cannot have --sequence_transition_file and zero history length, setting history length to 1" << endl;
      hinfo.length = 1;
    }

    constant_pow_length = 1;
    for (size_t i=0; i < hinfo.length; i++)
      constant_pow_length *= quadratic_constant;

    //compute total number of policies we will have at end of training
    // we add current_policy for cases where we start from an initial set of policies loaded through -i option
    size_t tmp_number_of_policies = current_policy;
    if(all.training)
      tmp_number_of_policies += (int)ceil(((float)all.numpasses) / ((float)sequence_passes_per_policy));

    //the user might have specified the number of policies that will eventually be trained through multiple vw calls, 
    //so only set total_number_of_policies to computed value if it is larger
    if(tmp_number_of_policies > total_number_of_policies) {
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

    std::stringstream ss1;
    std::stringstream ss2;
    ss1 << current_policy;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --sequence_trained_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--sequence_trained_nb_policies", ss1.str()); 
    ss2 << total_number_of_policies;
    //use cmd_string_replace_value in case we already loaded a predictor which had a value stored for --sequence_total_nb_policies
    VW::cmd_string_replace_value(all.options_from_file,"--sequence_total_nb_policies", ss2.str());

    all.base_learner_nb_w *= total_number_of_policies;
    increment = (all.length() / all.base_learner_nb_w / 2) * all.stride;
    //cerr<<"increment=" << increment<<endl;
  }
}

/*
TODO: position-based history features?
*/
