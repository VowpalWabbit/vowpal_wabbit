/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>

#include "reductions.h"
#include "cost_sensitive.h"
#include "simple_label.h"
#include "v_hashmap.h"
#include "vw.h"
#include "gd.h" // GD::foreach_feature() needed in subtract_example()

using namespace std;

using namespace LEARNER;

using namespace COST_SENSITIVE;

namespace CSOAA {
  struct csoaa{
    vw* all;
  };

  template <bool is_learn>
  void predict_or_learn(csoaa& c, learner& base, example& ec) {
    vw* all = c.all;
    COST_SENSITIVE::label ld = ec.l.cs;
    uint32_t prediction = 1;
    float score = FLT_MAX;
    ec.l.simple = { 0., 0., 0. };
    for (wclass *cl = ld.costs.begin; cl != ld.costs.end; cl ++)
      {
        uint32_t i = cl->class_index;
        if (is_learn)
          {
            if (cl->x == FLT_MAX || !all->training)
              {
                ec.l.simple.label = FLT_MAX;
                ec.l.simple.weight = 0.;
              }
            else
              {
                ec.l.simple.label = cl->x;
                ec.l.simple.weight = 1.;
              }
            base.learn(ec, i-1);
          }
        else
          base.predict(ec, i-1);

        cl->partial_prediction = ec.partial_prediction;
        if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction)) {
          score = ec.partial_prediction;
          prediction = i;
        }
        ec.partial_prediction = 0.;
      }

    ec.pred.multiclass = prediction;
    ec.l.cs = ld;
  }

  void finish_example(vw& all, csoaa&, example& ec)
  {
    output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    csoaa* c=(csoaa*)calloc_or_die(1,sizeof(csoaa));
    c->all = &all;
    //first parse for number of actions
    uint32_t nb_actions = 0;

    nb_actions = (uint32_t)vm["csoaa"].as<size_t>();

    //append csoaa with nb_actions to file_options so it is saved to regressor later
    std::stringstream ss;
    ss << " --csoaa " << nb_actions;
    all.file_options.append(ss.str());

    all.p->lp = cs_label;
    all.sd->k = nb_actions;

    learner* l = new learner(c, all.l, nb_actions);
    l->set_learn<csoaa, predict_or_learn<true> >();
    l->set_predict<csoaa, predict_or_learn<false> >();
    l->set_finish_example<csoaa,finish_example>();
    return l;
  }
}

namespace CSOAA_AND_WAP_LDF {
  struct ldf {
    v_array<example*> ec_seq;
    v_hashmap< size_t, v_array<feature> > label_features;

    size_t read_example_this_loop;
    bool need_to_clear;
    bool is_wap;
    bool first_pass;
    bool treat_as_classifier;
    bool is_singleline;
    float csoaa_example_t;
    vw* all;

    learner* base;
  };

namespace LabelDict { 
  bool size_t_eq(size_t &a, size_t &b) { return (a==b); }

  size_t hash_lab(size_t lab) { return 328051 + 94389193 * lab; }
  
  bool ec_is_label_definition(example& ec) // label defs look like "0:___" or just "label:___"
  {
    if (ec.indices.size() != 1) return false;
    if (ec.indices[0] != 'l') return false;
    v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
    for (size_t j=0; j<costs.size(); j++)
      if ((costs[j].class_index != 0) || (costs[j].x <= 0.)) return false;
    return true;    
  }

  bool ec_is_example_header(example& ec)  // example headers look like "0:-1" or just "shared"
  {
    v_array<COST_SENSITIVE::wclass> costs = ec.l.cs.costs;
    if (costs.size() != 1) return false;
    if (costs[0].class_index != 0) return false;
    if (costs[0].x >= 0) return false;
    return true;    
  }

  bool ec_seq_is_label_definition(ldf& data, v_array<example*>ec_seq)
  {
    if (data.ec_seq.size() == 0) return false;
    bool is_lab = ec_is_label_definition(*data.ec_seq[0]);
    for (size_t i=1; i<data.ec_seq.size(); i++) {
      if (is_lab != ec_is_label_definition(*data.ec_seq[i])) {
        if (!((i == data.ec_seq.size()-1) && (example_is_newline(*data.ec_seq[i])))) {
          cerr << "error: mixed label definition and examples in ldf data!" << endl;
          throw exception();
        }
      }
    }
    return is_lab;
  }

  void del_example_namespace(example& ec, char ns, v_array<feature> features) {
    size_t numf = features.size();
    // print_update is called after this del_example_namespace,
    // so we need to keep the ec.num_features correct,
    // so shared features are included in the reported number of "current features"
    //ec.num_features -= numf;

    assert (ec.atomics[(size_t)ns].size() >= numf);
    if (ec.atomics[(size_t)ns].size() == numf) { // did NOT have ns
      assert(ec.indices.size() > 0);
      assert(ec.indices[ec.indices.size()-1] == (size_t)ns);
      ec.indices.pop();
      ec.total_sum_feat_sq -= ec.sum_feat_sq[(size_t)ns];
      ec.atomics[(size_t)ns].erase();
      ec.sum_feat_sq[(size_t)ns] = 0.;
    } else { // DID have ns
      for (feature*f=features.begin; f!=features.end; f++) {
        ec.sum_feat_sq[(size_t)ns] -= f->x * f->x;
        ec.atomics[(size_t)ns].pop();
      }
    }
  }

  void add_example_namespace(example& ec, char ns, v_array<feature> features) {
    bool has_ns = false;
    for (size_t i=0; i<ec.indices.size(); i++) {
      if (ec.indices[i] == (size_t)ns) {
        has_ns = true;
        break;
      }
    }
    if (has_ns) {
      ec.total_sum_feat_sq -= ec.sum_feat_sq[(size_t)ns];
    } else {
      ec.indices.push_back((size_t)ns);
      ec.sum_feat_sq[(size_t)ns] = 0;
    }

    for (feature*f=features.begin; f!=features.end; f++) {
      ec.sum_feat_sq[(size_t)ns] += f->x * f->x;
      ec.atomics[(size_t)ns].push_back(*f);
    }

    ec.num_features += features.size();
    ec.total_sum_feat_sq += ec.sum_feat_sq[(size_t)ns];
  }

  void add_example_namespaces_from_example(example& target, example& source) {
    for (unsigned char* idx=source.indices.begin; idx!=source.indices.end; idx++) {
      if (*idx == constant_namespace) continue;
      add_example_namespace(target, (char)*idx, source.atomics[*idx]);
    }
  }

  void del_example_namespaces_from_example(example& target, example& source) {
    //for (size_t*idx=source.indices.begin; idx!=source.indices.end; idx++) {
    unsigned char* idx = source.indices.end;
    idx--;
    for (; idx>=source.indices.begin; idx--) {
      if (*idx == constant_namespace) continue;
      del_example_namespace(target, (char)*idx, source.atomics[*idx]);
    }
  }

  void add_example_namespace_from_memory(ldf& data, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = data.label_features.get(lab, lab_hash);
    if (features.size() == 0) return;
    add_example_namespace(ec, 'l', features);
  }

  void del_example_namespace_from_memory(ldf& data, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = data.label_features.get(lab, lab_hash);
    if (features.size() == 0) return;
    del_example_namespace(ec, 'l', features);
  }

  void set_label_features(ldf& data, size_t lab, v_array<feature>features) {
    size_t lab_hash = hash_lab(lab);
    if (data.label_features.contains(lab, lab_hash)) { return; }
    data.label_features.put_after_get(lab, lab_hash, features);
  }

  void free_label_features(ldf& data) {
    void* label_iter = data.label_features.iterator();
    while (label_iter != NULL) {
      v_array<feature> *features = data.label_features.iterator_get_value(label_iter);
      features->erase();
      features->delete_v();

      label_iter = data.label_features.iterator_next(label_iter);
    }
    data.label_features.clear();
    data.label_features.delete_v();
  }
}

  inline bool cmp_wclass_ptr(const COST_SENSITIVE::wclass* a, const COST_SENSITIVE::wclass* b) { return a->x < b->x; }

  void compute_wap_values(vector<COST_SENSITIVE::wclass*> costs) {
    std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
    costs[0]->wap_value = 0.;
    for (size_t i=1; i<costs.size(); i++)
      costs[i]->wap_value = costs[i-1]->wap_value + (costs[i]->x - costs[i-1]->x) / (float)i;
  }

  // Substract a given feature from example ec.
  // Rather than finding the corresponding namespace and feature in ec,
  // add a new feature with opposite value (but same index) to ec to a special wap_ldf_namespace.
  // This is faster and allows fast undo in unsubtract_example().
  void subtract_feature(example& ec, float feature_value_x, uint32_t weight_index)
  {
    feature temp = { -feature_value_x, weight_index };
    ec.atomics[wap_ldf_namespace].push_back(temp);
    ec.sum_feat_sq[wap_ldf_namespace] += feature_value_x * feature_value_x;
  }

  // Iterate over all features of ecsub including quadratic and cubic features and subtract them from ec.
  void subtract_example(vw& all, example *ec, example *ecsub)
  {
    ec->sum_feat_sq[wap_ldf_namespace] = 0;
    GD::foreach_feature<example&, uint32_t, subtract_feature>(all, *ecsub, *ec);
    ec->indices.push_back(wap_ldf_namespace);
    ec->num_features += ec->atomics[wap_ldf_namespace].size();
    ec->total_sum_feat_sq += ec->sum_feat_sq[wap_ldf_namespace];
  }

  void unsubtract_example(vw& all, example *ec)
  {
    if (ec->indices.size() == 0) {
      cerr << "internal error (bug): trying to unsubtract_example, but there are no namespaces!" << endl;
      return;
    }
    
    if (ec->indices.last() != wap_ldf_namespace) {
      cerr << "internal error (bug): trying to unsubtract_example, but either it wasn't added, or something was added after and not removed!" << endl;
      return;
    }

    ec->num_features -= ec->atomics[wap_ldf_namespace].size();
    ec->total_sum_feat_sq -= ec->sum_feat_sq[wap_ldf_namespace];
    ec->sum_feat_sq[wap_ldf_namespace] = 0;
    ec->atomics[wap_ldf_namespace].erase();
    ec->indices.decr();
  }

  void make_single_prediction(ldf& data, learner& base, example& ec) {
    COST_SENSITIVE::label ld = ec.l.cs;
    label_data simple_label;
    simple_label.initial = 0.;
    simple_label.label = FLT_MAX;
    simple_label.weight = 0.;
    ec.partial_prediction = 0.;
    
    LabelDict::add_example_namespace_from_memory(data, ec, ld.costs[0].class_index);
    
    ec.l.simple = simple_label;
    base.predict(ec); // make a prediction
    ld.costs[0].partial_prediction = ec.partial_prediction;

    LabelDict::del_example_namespace_from_memory(data, ec, ld.costs[0].class_index);
    ec.l.cs = ld;
  }

  bool check_ldf_sequence(ldf& data, size_t start_K)
  {
    bool isTest = COST_SENSITIVE::example_is_test(*data.ec_seq[start_K]);
    for (size_t k=start_K; k<data.ec_seq.size(); k++) {
      example *ec = data.ec_seq[k];
      
      // Each sub-example must have just one cost
      assert(ec->l.cs.costs.size()==1);
      
      if (COST_SENSITIVE::example_is_test(*ec) != isTest) {
        isTest = true;
        cerr << "warning: ldf example has mix of train/test data; assuming test" << endl;
      }
      if (LabelDict::ec_is_example_header(*ec)) {
        cerr << "warning: example headers at position " << k << ": can only have in initial position!" << endl;
        throw exception();
      }
    }
    return isTest;
  }

  void do_actual_learning_wap(vw& all, ldf& data, learner& base, size_t start_K)
  {
    size_t K = data.ec_seq.size();
    vector<COST_SENSITIVE::wclass*> all_costs;
    for (size_t k=start_K; k<K; k++)
      all_costs.push_back(&data.ec_seq[k]->l.cs.costs[0]);
    compute_wap_values(all_costs);

    data.csoaa_example_t += 1.;
    for (size_t k1=start_K; k1<K; k1++) {
      example *ec1 = data.ec_seq[k1];

      // save original variables
      COST_SENSITIVE::label   save_cs_label = ec1->l.cs;
      label_data& simple_label = ec1->l.simple;
      float save_example_t1 = ec1->example_t;

      v_array<COST_SENSITIVE::wclass> costs1 = save_cs_label.costs;
      if (costs1[0].class_index == (uint32_t)-1) continue;
      
      LabelDict::add_example_namespace_from_memory(data, *ec1, costs1[0].class_index);
      
      for (size_t k2=k1+1; k2<K; k2++) {
        example *ec2 = data.ec_seq[k2];
        v_array<COST_SENSITIVE::wclass> costs2 = ec2->l.cs.costs;
        
        if (costs2[0].class_index == (uint32_t)-1) continue;
        float value_diff = fabs(costs2[0].wap_value - costs1[0].wap_value);
        //float value_diff = fabs(costs2[0].x - costs1[0].x);
        if (value_diff < 1e-6)
          continue;
        
        LabelDict::add_example_namespace_from_memory(data, *ec2, costs2[0].class_index);
        
        // learn
        ec1->example_t = data.csoaa_example_t;
        simple_label.initial = 0.;
        simple_label.label = (costs1[0].x < costs2[0].x) ? -1.0f : 1.0f;
        simple_label.weight = value_diff;
        ec1->partial_prediction = 0.;
        subtract_example(all, ec1, ec2);
        base.learn(*ec1);
        unsubtract_example(all, ec1);
        
        LabelDict::del_example_namespace_from_memory(data, *ec2, costs2[0].class_index);
      }
      LabelDict::del_example_namespace_from_memory(data, *ec1, costs1[0].class_index);
      
      // restore original cost-sensitive label, sum of importance weights
      ec1->l.cs = save_cs_label;
      ec1->example_t = save_example_t1;
      // TODO: What about partial_prediction? See do_actual_learning_oaa.
    }
  }

  void do_actual_learning_oaa(vw& all, ldf& data, learner& base, size_t start_K)
  {
    size_t K = data.ec_seq.size();
    float  min_cost  = FLT_MAX;
    float  max_cost  = -FLT_MAX;

    for (size_t k=start_K; k<K; k++) {
      float ec_cost = data.ec_seq[k]->l.cs.costs[0].x;
      if (ec_cost < min_cost) min_cost = ec_cost;
      if (ec_cost > max_cost) max_cost = ec_cost;
    }

    data.csoaa_example_t += 1.;
    for (size_t k=start_K; k<K; k++) {
      example *ec = data.ec_seq[k];
      
      // save original variables
      label save_cs_label = ec->l.cs;
      float save_example_t = ec->example_t;
      v_array<COST_SENSITIVE::wclass> costs = save_cs_label.costs;

      // build example for the base learner
      label_data simple_label;
      ec->example_t = data.csoaa_example_t;
      
      simple_label.initial = 0.;
      simple_label.weight = 1.;
      if (!data.treat_as_classifier) { // treat like regression
            simple_label.label = costs[0].x;
      } else { // treat like classification
            if (costs[0].x <= min_cost) {
              simple_label.label = -1.;
              simple_label.weight = max_cost - min_cost;
            } else {
              simple_label.label = 1.;
              simple_label.weight = costs[0].x - min_cost;
            }
      }
      ec->l.simple = simple_label;

      // learn
      LabelDict::add_example_namespace_from_memory(data, *ec, costs[0].class_index);
      base.learn(*ec);
      LabelDict::del_example_namespace_from_memory(data, *ec, costs[0].class_index);
      
      // restore original cost-sensitive label, sum of importance weights and partial_prediction
      ec->l.cs = save_cs_label;
      ec->example_t = save_example_t;
      ec->partial_prediction = costs[0].partial_prediction;
    }
  }

  template <bool is_learn>
  void do_actual_learning(vw& all, ldf& data, learner& base)
  {
    //cdbg << "do_actual_learning size=" << data.ec_seq.size() << endl;
    if (data.ec_seq.size() <= 0) return;  // nothing to do

    /////////////////////// handle label definitions
    if (LabelDict::ec_seq_is_label_definition(data, data.ec_seq)) {
      for (size_t i=0; i<data.ec_seq.size(); i++) {
        v_array<feature> features = v_init<feature>();
        for (feature*f=data.ec_seq[i]->atomics[data.ec_seq[i]->indices[0]].begin; f!=data.ec_seq[i]->atomics[data.ec_seq[i]->indices[0]].end; f++) {
          feature fnew = { f->x,  f->weight_index };
          features.push_back(fnew);
        }

        v_array<COST_SENSITIVE::wclass> costs = data.ec_seq[i]->l.cs.costs;
        for (size_t j=0; j<costs.size(); j++) {
          size_t lab = (size_t)costs[j].x;
          LabelDict::set_label_features(data, lab, features);
        }
      }
      return;
    }

    /////////////////////// add headers
    size_t K = data.ec_seq.size();
    size_t start_K = 0;
    if (LabelDict::ec_is_example_header(*data.ec_seq[0])) {
      start_K = 1;
      for (size_t k=1; k<K; k++)
        LabelDict::add_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);
    }
    bool isTest = check_ldf_sequence(data, start_K);

    /////////////////////// do prediction
    float  min_score = FLT_MAX;
    size_t predicted_K = start_K;   
    for (size_t k=start_K; k<K; k++) {
      example *ec = data.ec_seq[k];
      make_single_prediction(data, base, *ec);
      if (ec->partial_prediction < min_score) {
        min_score = ec->partial_prediction;
        predicted_K = k;
      }
    }   

    /////////////////////// learn
    if (is_learn && !isTest){
      if (data.is_wap) do_actual_learning_wap(all, data, base, start_K);
      else             do_actual_learning_oaa(all, data, base, start_K);
    }

    // Mark the predicted subexample with its class_index, all other with 0
    for (size_t k=start_K; k<K; k++)
      data.ec_seq[k]->pred.multiclass = (k == predicted_K) ? data.ec_seq[k]->l.cs.costs[0].class_index : 0;

    /////////////////////// remove header
    if (start_K > 0)
      for (size_t k=1; k<K; k++)
        LabelDict::del_example_namespaces_from_example(*data.ec_seq[k], *data.ec_seq[0]);
  }

  void output_example(vw& all, example& ec, bool& hit_loss, v_array<example*>* ec_seq)
  {
    label& ld = ec.l.cs;
    v_array<COST_SENSITIVE::wclass> costs = ld.costs;
    
    if (example_is_newline(ec)) return;
    if (LabelDict::ec_is_example_header(ec)) return;
    if (LabelDict::ec_is_label_definition(ec)) return;

    all.sd->total_features += ec.num_features;

    float loss = 0.;

    if (!COST_SENSITIVE::example_is_test(ec)) {
      for (size_t j=0; j<costs.size(); j++) {
        if (hit_loss) break;
        if (ec.pred.multiclass == costs[j].class_index) {
          loss = costs[j].x;
          hit_loss = true;
        }
      }

      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      assert(loss >= 0);
    }
  
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, (float)ec.pred.multiclass, 0, ec.tag);

    if (all.raw_prediction > 0) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (size_t i = 0; i < costs.size(); i++) {
        if (i > 0) outputStringStream << ' ';
        outputStringStream << costs[i].class_index << ':' << costs[i].partial_prediction;
      }
      //outputStringStream << endl;
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }
    
    COST_SENSITIVE::print_update(all, COST_SENSITIVE::example_is_test(ec), ec, ec_seq);
  }

  void output_example_seq(vw& all, ldf& data)
  {
    if ((data.ec_seq.size() > 0) && !LabelDict::ec_seq_is_label_definition(data, data.ec_seq)) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;

      bool hit_loss = false;
      for (example** ecc=data.ec_seq.begin; ecc!=data.ec_seq.end; ecc++)
        output_example(all, **ecc, hit_loss, &(data.ec_seq));

      if (!data.is_singleline && (all.raw_prediction > 0))
        all.print_text(all.raw_prediction, "", data.ec_seq[0]->tag);
    }
  }

  void clear_seq_and_finish_examples(vw& all, ldf& data)
  {
    if (data.ec_seq.size() > 0) 
      for (example** ecc=data.ec_seq.begin; ecc!=data.ec_seq.end; ecc++)
        if ((*ecc)->in_use)
          VW::finish_example(all, *ecc);
    data.ec_seq.erase();
  }

  void end_pass(ldf& data)
  {
    data.first_pass = false;
  }

  void finish_singleline_example(vw& all, ldf&, example& ec)
  {
    if (! LabelDict::ec_is_label_definition(ec)) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;
    }
    bool hit_loss = false;
    output_example(all, ec, hit_loss, NULL);
    VW::finish_example(all, &ec);
  }

  void finish_multiline_example(vw& all, ldf& data, example& ec)
  {
    if (data.need_to_clear) {
      if (data.ec_seq.size() > 0) {
        output_example_seq(all, data);
        global_print_newline(all);
      }        
      clear_seq_and_finish_examples(all, data);
      data.need_to_clear = false;
      if (ec.in_use) VW::finish_example(all, &ec);
    }
  }

  void end_examples(ldf& data)
  {
    if (data.need_to_clear)
      data.ec_seq.erase();
  }


  void finish(ldf& data)
  {
    //vw* all = l->all;
    data.ec_seq.delete_v();
    LabelDict::free_label_features(data);
  }

  template <bool is_learn>
  void predict_or_learn(ldf& data, learner& base, example &ec) {
    vw* all = data.all;
    data.base = &base;
    bool is_test_ec = COST_SENSITIVE::example_is_test(ec);
    bool need_to_break = data.ec_seq.size() >= all->p->ring_size - 2;

    // singleline is used by library/ezexample_predict
    if (data.is_singleline) {
      assert(is_test_ec); // Only test examples are supported with singleline
      assert(ec.l.cs.costs.size() > 0); // headers not allowed with singleline
      make_single_prediction(data, base, ec);
    } else if (LabelDict::ec_is_label_definition(ec)) {
      if (data.ec_seq.size() > 0) {
        cerr << "error: label definition encountered in data block" << endl;
        throw exception();
      }
      data.ec_seq.push_back(&ec);
      do_actual_learning<is_learn>(*all, data, base);
      data.need_to_clear = true;
    } else if ((example_is_newline(ec) && is_test_ec) || need_to_break) {
      if (need_to_break && data.first_pass)
        cerr << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << endl;
      do_actual_learning<is_learn>(*all, data, base);
      data.need_to_clear = true;
    } else {
      if (data.need_to_clear) {  // should only happen if we're NOT driving
        data.ec_seq.erase();
        data.need_to_clear = false;
      }
      data.ec_seq.push_back(&ec);
    }
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    po::options_description ldf_opts("LDF Options");
    ldf_opts.add_options()
        ("ldf_override", po::value<string>(), "Override singleline or multiline from csoaa_ldf or wap_ldf, eg if stored in file")
        ;

    vm = add_options(all, ldf_opts);
    
    ldf* ld = (ldf*)calloc_or_die(1, sizeof(ldf));

    ld->all = &all;
    ld->need_to_clear = true;
    ld->first_pass = true;
 
    string ldf_arg;

    if( vm.count("csoaa_ldf") ){
      ldf_arg = vm["csoaa_ldf"].as<string>();
      all.file_options.append(" --csoaa_ldf ");
      all.file_options.append(ldf_arg);
    }
    else {
      ldf_arg = vm["wap_ldf"].as<string>();
      ld->is_wap = true;
      all.file_options.append(" --wap_ldf ");
      all.file_options.append(ldf_arg);
    }
    if ( vm.count("ldf_override") )
      ldf_arg = vm["ldf_override"].as<string>();

    all.p->lp = COST_SENSITIVE::cs_label;

    all.sd->k = (uint32_t)-1;

    ld->treat_as_classifier = false;
    ld->is_singleline = false;
    if (ldf_arg.compare("multiline") == 0 || ldf_arg.compare("m") == 0) {
      ld->treat_as_classifier = false;
    } else if (ldf_arg.compare("multiline-classifier") == 0 || ldf_arg.compare("mc") == 0) {
      ld->treat_as_classifier = true;
    } else {
      if (all.training) {
        cerr << "ldf requires either m/multiline or mc/multiline-classifier, except in test-mode which can be s/sc/singleline/singleline-classifier" << endl;
        throw exception();
      }
      if (ldf_arg.compare("singleline") == 0 || ldf_arg.compare("s") == 0) {
        ld->treat_as_classifier = false;
        ld->is_singleline = true;
      } else if (ldf_arg.compare("singleline-classifier") == 0 || ldf_arg.compare("sc") == 0) {
        ld->treat_as_classifier = true;
        ld->is_singleline = true;
      }
    }

    all.p->emptylines_separate_examples = true; // TODO: check this to be sure!!!  !ld->is_singleline;

    if (all.add_constant) {
      all.add_constant = false;
    }
    ld->label_features.init(256, v_array<feature>(), LabelDict::size_t_eq);
    ld->label_features.get(1, 94717244); // TODO: figure this out

    ld->read_example_this_loop = 0;
    ld->need_to_clear = false;
    learner* l = new learner(ld, all.l);
    l->set_learn<ldf, predict_or_learn<true> >();
    l->set_predict<ldf, predict_or_learn<false> >();
    if (ld->is_singleline)
      l->set_finish_example<ldf,finish_singleline_example>();
    else
      l->set_finish_example<ldf,finish_multiline_example>();
    l->set_finish<ldf,finish>();
    l->set_end_examples<ldf,end_examples>(); 
    l->set_end_pass<ldf,end_pass>();
    return l;
  }

  void global_print_newline(vw& all)
  {
    char temp[1];
    temp[0] = '\n';
    for (size_t i=0; i<all.final_prediction_sink.size(); i++) {
      int f = all.final_prediction_sink[i];
      ssize_t t;
      t = io_buf::write_file_or_socket(f, temp, 1);
      if (t != 1)
        std::cerr << "write error" << std::endl;
    }
  }
}


