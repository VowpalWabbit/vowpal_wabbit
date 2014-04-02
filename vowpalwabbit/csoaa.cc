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
    label* ld = (label*)ec.ld;
    size_t prediction = 1;
    float score = FLT_MAX;
    label_data simple_temp = { 0., 0., 0. };
    ec.ld = &simple_temp;
    for (wclass *cl = ld->costs.begin; cl != ld->costs.end; cl ++)
      {
        uint32_t i = cl->weight_index;
	if (is_learn)
	  {
	    if (cl->x == FLT_MAX || !all->training)
	      {
		simple_temp.label = FLT_MAX;
		simple_temp.weight = 0.;
	      }
	    else
	      {
		simple_temp.label = cl->x;
		simple_temp.weight = 1.;
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
    ec.ld = ld;
    ec.final_prediction = (float)prediction;
  }

  void finish_example(vw& all, csoaa&, example& ec)
  {
    output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    csoaa* c=(csoaa*)calloc_or_die(1,sizeof(csoaa));
    c->all = &all;
    //first parse for number of actions
    uint32_t nb_actions = 0;
    if( vm_file.count("csoaa") ) { //if loaded options from regressor
      nb_actions = (uint32_t)vm_file["csoaa"].as<size_t>();
      if( vm.count("csoaa") && (uint32_t)vm["csoaa"].as<size_t>() != nb_actions ) //if csoaa was also specified in commandline, warn user if its different
        std::cerr << "warning: you specified a different number of actions through --csoaa than the one loaded from predictor. Pursuing with loaded value of: " << nb_actions << endl;
    }
    else {
      nb_actions = (uint32_t)vm["csoaa"].as<size_t>();

      //append csoaa with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --csoaa " << nb_actions;
      all.options_from_file.append(ss.str());
    }

    all.p->lp = cs_label_parser;
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
  bool size_t_eq(size_t a, size_t b) { return (a==b); }

  size_t hash_lab(size_t lab) { return 328051 + 94389193 * lab; }
  
  bool ec_is_label_definition(example& ec) // label defs look like "___:-1"
  {
    v_array<COST_SENSITIVE::wclass> costs = ((COST_SENSITIVE::label*)ec.ld)->costs;
    for (size_t j=0; j<costs.size(); j++)
      if (costs[j].x >= 0.) return false;
    if (ec.indices.size() == 0) return false;
    if (ec.indices.size() >  2) return false;
    if (ec.indices[0] != 'l') return false;
    return true;    
  }

  bool ec_is_example_header(example& ec)  // example headers look like "0:-1"
  {
    v_array<COST_SENSITIVE::wclass> costs = ((COST_SENSITIVE::label*)ec.ld)->costs;
    if (costs.size() != 1) return false;
    if (costs[0].weight_index != 0) return false;
    if (costs[0].x >= 0) return false;
    return true;    
  }

  bool ec_seq_is_label_definition(ldf& l, v_array<example*>ec_seq)
  {
    if (l.ec_seq.size() == 0) return false;
    bool is_lab = ec_is_label_definition(*l.ec_seq[0]);
    for (size_t i=1; i<l.ec_seq.size(); i++) {
      if (is_lab != ec_is_label_definition(*l.ec_seq[i])) {
        if (!((i == l.ec_seq.size()-1) && (example_is_newline(*l.ec_seq[i])))) {
          cerr << "error: mixed label definition and examples in ldf data!" << endl;
          throw exception();
        }
      }
    }
    return is_lab;
  }

  void del_example_namespace(example& ec, char ns, v_array<feature> features) {
    size_t numf = features.size();
    ec.num_features -= numf;

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

  void add_example_namespace_from_memory(ldf& l, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = l.label_features.get(lab, lab_hash);
    if (features.size() == 0) return;
    add_example_namespace(ec, 'l', features);
  }

  void del_example_namespace_from_memory(ldf& l, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = l.label_features.get(lab, lab_hash);
    if (features.size() == 0) return;
    del_example_namespace(ec, 'l', features);
  }

  void set_label_features(ldf& l, size_t lab, v_array<feature>features) {
    size_t lab_hash = hash_lab(lab);
    if (l.label_features.contains(lab, lab_hash)) { return; }
    l.label_features.put_after_get(lab, lab_hash, features);
  }

  void free_label_features(ldf& l) {
    void* label_iter = l.label_features.iterator();
    while (label_iter != NULL) {
      v_array<feature> *features = l.label_features.iterator_get_value(label_iter);
      features->erase();
      features->delete_v();

      label_iter = l.label_features.iterator_next(label_iter);
    }
    l.label_features.clear();
    l.label_features.delete_v();
  }
}

  inline bool cmp_wclass_ptr(const COST_SENSITIVE::wclass* a, const COST_SENSITIVE::wclass* b) { return a->x < b->x; }

  void compute_wap_values(vector<COST_SENSITIVE::wclass*> costs) {
    std::sort(costs.begin(), costs.end(), cmp_wclass_ptr);
    costs[0]->wap_value = 0.;
    for (size_t i=1; i<costs.size(); i++)
      costs[i]->wap_value = costs[i-1]->wap_value + (costs[i]->x - costs[i-1]->x) / (float)i;
  }

  void subtract_example(vw& all, example *ec, example *ecsub)
  {
    float norm_sq = 0.;
    size_t num_f = 0;
    for (unsigned char* i = ecsub->indices.begin; i != ecsub->indices.end; i++) {
      size_t feature_index = 0;
      for (feature *f = ecsub->atomics[*i].begin; f != ecsub->atomics[*i].end; f++) {
        feature temp = { -f->x, (uint32_t) (f->weight_index) };
        ec->atomics[wap_ldf_namespace].push_back(temp);
        norm_sq += f->x * f->x;
        num_f ++;

        if (all.audit) {
          if (! (ecsub->audit_features[*i].size() >= feature_index)) {
            audit_data b_feature = ecsub->audit_features[*i][feature_index];
            audit_data a_feature = { NULL, NULL, (uint32_t) (f->weight_index), -f->x, false };
            a_feature.space = b_feature.space;
            a_feature.feature = b_feature.feature;
            ec->audit_features[wap_ldf_namespace].push_back(a_feature);
            feature_index++;
          }
        }
      }
    }
    ec->indices.push_back(wap_ldf_namespace);
    ec->sum_feat_sq[wap_ldf_namespace] = norm_sq;
    ec->total_sum_feat_sq += norm_sq;
    ec->num_features += num_f;
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
    if (all.audit) {
      if (ec->audit_features[wap_ldf_namespace].begin != ec->audit_features[wap_ldf_namespace].end) {
        for (audit_data *f = ec->audit_features[wap_ldf_namespace].begin; f != ec->audit_features[wap_ldf_namespace].end; f++) {
          if (f->alloced) {
            free(f->space);
            free(f->feature);
            f->alloced = false;
          }
        }
      }

      ec->audit_features[wap_ldf_namespace].erase();
    }
    ec->indices.decr();
  }

  void make_single_prediction(vw& all, ldf& l, learner& base, example& ec, size_t*prediction, float*min_score, float*min_cost, float*max_cost) {
    label   *ld = (label*)ec.ld;
    v_array<COST_SENSITIVE::wclass> costs = ld->costs;
    label_data simple_label;

    if (costs.size() == 0) {
      simple_label.initial = 0.;
      simple_label.label = FLT_MAX;
      simple_label.weight = 0.;
      ec.partial_prediction = 0.;
      
      ec.ld = &simple_label;
      base.predict(ec); // make a prediction
    } else {
      for (size_t j=0; j<costs.size(); j++) {
        simple_label.initial = 0.;
        simple_label.label = FLT_MAX;
        simple_label.weight = 0.;
        ec.partial_prediction = 0.;

        LabelDict::add_example_namespace_from_memory(l, ec, costs[j].weight_index);
      
        ec.ld = &simple_label;
        base.predict(ec); // make a prediction
        costs[j].partial_prediction = ec.partial_prediction;
        //cdbg << "costs[" << j << "].partial_prediction = " << ec.partial_prediction << endl;

        if (min_score && prediction && (ec.partial_prediction < *min_score)) {
          *min_score = ec.partial_prediction;
          *prediction = costs[j].weight_index;
        }

        if (min_cost && (costs[j].x < *min_cost)) *min_cost = costs[j].x;
        if (max_cost && (costs[j].x > *max_cost)) *max_cost = costs[j].x;

        LabelDict::del_example_namespace_from_memory(l, ec, costs[j].weight_index);
      }
    }
    
    ec.ld = ld;
  }


  template <bool is_learn>
  void do_actual_learning_wap(vw& all, ldf& l, learner& base, size_t start_K)
  {
    size_t K = l.ec_seq.size();
    bool   isTest = COST_SENSITIVE::example_is_test(*l.ec_seq[start_K]);
    size_t prediction = 0;
    float  min_score = FLT_MAX;

    for (size_t k=start_K; k<K; k++) {
      example *ec = l.ec_seq.begin[k];

      if (COST_SENSITIVE::example_is_test(*ec) != isTest) {
        isTest = true;
        cerr << "warning: wap_ldf got mix of train/test data; assuming test" << endl;
      }
      if (LabelDict::ec_is_example_header(*l.ec_seq[k])) {
        cerr << "warning: example headers at position " << k << ": can only have in initial position!" << endl;
        throw exception();
      }

      make_single_prediction(all, l, base, *ec, &prediction, &min_score, NULL, NULL);
    }

    // do actual learning
    vector<COST_SENSITIVE::wclass*> all_costs;
    if (is_learn && all.training && !isTest) {
      for (size_t k=start_K; k<K; k++) {
        v_array<COST_SENSITIVE::wclass> this_costs = ((label*)l.ec_seq.begin[k]->ld)->costs;
        for (size_t j=0; j<this_costs.size(); j++)
          all_costs.push_back(&this_costs[j]);
      }
      compute_wap_values(all_costs);

      l.csoaa_example_t += 1.;
    }

    label_data simple_label;
    for (size_t k1=start_K; k1<K; k1++) {
      example *ec1 = l.ec_seq.begin[k1];
      label   *ld1 = (label*)ec1->ld;
      v_array<COST_SENSITIVE::wclass> costs1 = ld1->costs;
      bool prediction_is_me = false;
      ec1->ld = &simple_label;
      float example_t1 = ec1->example_t;

      for (size_t j1=0; j1<costs1.size(); j1++) {
        if (costs1[j1].weight_index == (uint32_t)-1) continue;
        if (is_learn && all.training && !isTest) {
          LabelDict::add_example_namespace_from_memory(l, *ec1, costs1[j1].weight_index);

          for (size_t k2=k1+1; k2<K; k2++) {
            example *ec2 = l.ec_seq.begin[k2];
            label   *ld2 = (label*)ec2->ld;
            v_array<COST_SENSITIVE::wclass> costs2 = ld2->costs;

            for (size_t j2=0; j2<costs2.size(); j2++) {
              if (costs2[j2].weight_index == (uint32_t)-1) continue;
              float value_diff = fabs(costs2[j2].wap_value - costs1[j1].wap_value);
              //float value_diff = fabs(costs2[j2].x - costs1[j1].x);
              if (value_diff < 1e-6)
                continue;

              LabelDict::add_example_namespace_from_memory(l, *ec2, costs2[j2].weight_index);

              // learn
              ec1->example_t = l.csoaa_example_t;
              simple_label.initial = 0.;
              simple_label.label = (costs1[j1].x < costs2[j2].x) ? -1.0f : 1.0f;
              simple_label.weight = value_diff;
              ec1->partial_prediction = 0.;
              subtract_example(all, ec1, ec2);
	      if (is_learn)
		base.learn(*ec1);
	      else
		base.predict(*ec1);
              unsubtract_example(all, ec1);
              
              LabelDict::del_example_namespace_from_memory(l, *ec2, costs2[j2].weight_index);
            }
          }
          LabelDict::del_example_namespace_from_memory(l, *ec1, costs1[j1].weight_index);
        }

        if (prediction == costs1[j1].weight_index) prediction_is_me = true;
      }
      ec1->final_prediction = prediction_is_me ? (float)prediction : 0;
      ec1->ld = ld1;
      ec1->example_t = example_t1;
    }
  }

  template <bool is_learn>
  void do_actual_learning_oaa(vw& all, ldf& l, learner& base, size_t start_K)
  {
    size_t K = l.ec_seq.size();
    size_t prediction = 0;
    bool   isTest = COST_SENSITIVE::example_is_test(*l.ec_seq[start_K]);
    float  min_score = FLT_MAX;
    float  min_cost  = FLT_MAX;
    float  max_cost  = -FLT_MAX;

    //cdbg << "isTest=" << isTest << " start_K=" << start_K << " K=" << K << endl;
    
    for (size_t k=start_K; k<K; k++) {
      example *ec = l.ec_seq.begin[k];
      if (COST_SENSITIVE::example_is_test(*ec) != isTest) {
        isTest = true;
        cerr << "warning: ldf got mix of train/test data; assuming test" << endl;
      }
      if (LabelDict::ec_is_example_header(*l.ec_seq[k])) {
        cerr << "warning: example headers at position " << k << ": can only have in initial position!" << endl;
        throw exception();
      }
      //cdbg << "msp k=" << k << endl;
      make_single_prediction(all, l, base, *ec, &prediction, &min_score, &min_cost, &max_cost);
    }

    // do actual learning
    if (is_learn && all.training && !isTest)
      l.csoaa_example_t += 1.;
    for (size_t k=start_K; k<K; k++) {
      example *ec = l.ec_seq.begin[k];
      label   *ld = (label*)ec->ld;
      v_array<COST_SENSITIVE::wclass> costs = ld->costs;

      // learn
      label_data simple_label;
      bool prediction_is_me = false;
      for (size_t j=0; j<costs.size(); j++) {
        //cdbg << "j=" << j << " costs.size=" << costs.size() << endl;
        if (is_learn && all.training && !isTest) {
          float example_t = ec->example_t;
          ec->example_t = l.csoaa_example_t;

          simple_label.initial = 0.;
          simple_label.weight = 1.;
          if (!l.treat_as_classifier) { // treat like regression
            simple_label.label = costs[j].x;
          } else { // treat like classification
            if (costs[j].x <= min_cost) {
              simple_label.label = -1.;
              simple_label.weight = max_cost - min_cost;
            } else {
              simple_label.label = 1.;
              simple_label.weight = costs[j].x - min_cost;
            }
          }
          // TODO: check the example->done and ec->partial_prediction = costs[j].partial_prediciton here
          //cdbg << "k=" << k << " j=" << j << " label=" << simple_label.label << " cost=" << simple_label.weight << endl;
          ec->ld = &simple_label;
          //ec->partial_prediction = costs[j].partial_prediction;
          //cerr << "[" << ec->partial_prediction << "," << ec->done << "]";
          //ec->done = false;
          LabelDict::add_example_namespace_from_memory(l, *ec, costs[j].weight_index);
	  if (is_learn)
	    base.learn(*ec);
	  else
	    base.predict(*ec);
          LabelDict::del_example_namespace_from_memory(l, *ec, costs[j].weight_index);
          ec->example_t = example_t;
        }

        // fill in test predictions
        ec->partial_prediction = costs[j].partial_prediction;
        if (prediction == costs[j].weight_index) prediction_is_me = true;
      }
      ec->final_prediction = prediction_is_me ? (float)prediction : 0;

      if (isTest && (costs.size() == 1)) {
        ec->final_prediction = costs[0].partial_prediction;
      }

      // restore label
      ec->ld = ld;
    }
  }

  template <bool is_learn>
  void do_actual_learning(vw& all, ldf& l, learner& base)
  {
    //cdbg << "do_actual_learning size=" << l.ec_seq.size() << endl;
    if (l.ec_seq.size() <= 0) return;  // nothing to do

    /////////////////////// handle label definitions
    if (LabelDict::ec_seq_is_label_definition(l, l.ec_seq)) {  
      for (size_t i=0; i<l.ec_seq.size(); i++) {
        v_array<feature> features;
        for (feature*f=l.ec_seq[i]->atomics[l.ec_seq[i]->indices[0]].begin; f!=l.ec_seq[i]->atomics[l.ec_seq[i]->indices[0]].end; f++) {
          feature fnew = { f->x,  f->weight_index };
          features.push_back(fnew);
        }

        v_array<COST_SENSITIVE::wclass> costs = ((COST_SENSITIVE::label*)l.ec_seq[i]->ld)->costs;
        for (size_t j=0; j<costs.size(); j++) {
          size_t lab = costs[j].weight_index;
          LabelDict::set_label_features(l, lab, features);
        }
      }
      return;
    }
    /////////////////////// check for headers
    size_t K = l.ec_seq.size();
    size_t start_K = 0;
    if (LabelDict::ec_is_example_header(*l.ec_seq[0])) {
      start_K = 1;
      for (size_t k=1; k<K; k++)
        LabelDict::add_example_namespaces_from_example(*l.ec_seq[k], *l.ec_seq[0]);
    }

    /////////////////////// learn
    if (l.is_wap) do_actual_learning_wap<is_learn>(all, l, base, start_K);
    else          do_actual_learning_oaa<is_learn>(all, l, base, start_K);
    
    /////////////////////// remove header
    if (start_K > 0)
      for (size_t k=1; k<K; k++)
        LabelDict::del_example_namespaces_from_example(*l.ec_seq[k], *l.ec_seq[0]);

  }

  void output_example(vw& all, example& ec, bool& hit_loss)
  {
    label* ld = (label*)ec.ld;
    v_array<COST_SENSITIVE::wclass> costs = ld->costs;

    if (example_is_newline(ec)) return;
    if (LabelDict::ec_is_example_header(ec)) return;
    if (LabelDict::ec_is_label_definition(ec)) return;

    all.sd->total_features += ec.num_features;

    float loss = 0.;
    size_t final_pred = (size_t)ec.final_prediction;

    if (!COST_SENSITIVE::example_is_test(ec)) {
      for (size_t j=0; j<costs.size(); j++) {
        if (hit_loss) break;
        if (final_pred == costs[j].weight_index) {
          loss = costs[j].x;
          hit_loss = true;
        }
      }

      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      assert(loss >= 0);
    }
  
    for (int* sink = all.final_prediction_sink.begin; sink != all.final_prediction_sink.end; sink++)
      all.print(*sink, ec.final_prediction, 0, ec.tag);

    if (all.raw_prediction > 0) {
      string outputString;
      stringstream outputStringStream(outputString);
      for (size_t i = 0; i < costs.size(); i++) {
        if (i > 0) outputStringStream << ' ';
        outputStringStream << costs[i].weight_index << ':' << costs[i].partial_prediction;
      }
      //outputStringStream << endl;
      all.print_text(all.raw_prediction, outputStringStream.str(), ec.tag);
    }
    

    COST_SENSITIVE::print_update(all, COST_SENSITIVE::example_is_test(ec), ec);
  }

  void output_example_seq(vw& all, ldf& l)
  {
    if ((l.ec_seq.size() > 0) && !LabelDict::ec_seq_is_label_definition(l, l.ec_seq)) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;

      bool hit_loss = false;
      for (example** ecc=l.ec_seq.begin; ecc!=l.ec_seq.end; ecc++)
        output_example(all, **ecc, hit_loss);

      if (!l.is_singleline && (all.raw_prediction > 0))
        all.print_text(all.raw_prediction, "", l.ec_seq[0]->tag);
    }
  }

  void clear_seq_and_finish_examples(vw& all, ldf& l)
  {
    if (l.ec_seq.size() > 0) 
      for (example** ecc=l.ec_seq.begin; ecc!=l.ec_seq.end; ecc++)
        if ((*ecc)->in_use)
          VW::finish_example(all, *ecc);
    l.ec_seq.erase();
  }

  void end_pass(ldf& l)
  {
    l.first_pass = false;
  }

/*
  void learn(void* data, learner& base, example *ec) 
=======
  template <bool is_learn>
  void predict_or_learn(ldf* l, learner& base, example *ec) 
>>>>>>> d73d8f6aef1c02dd05554f91ca57e1d304336130
  {
    vw* all = l->all;
    l->base = &base;

    if ((!all->training) || COST_SENSITIVE::example_is_test(ec)) {
      size_t prediction = 0;
      float  min_score = FLT_MAX;
      make_single_prediction(*all, *l, base, ec, &prediction, &min_score, NULL, NULL);
    }
    if (l->is_singleline) {
      // must be test mode
    } else if (example_is_newline(ec) || l->ec_seq.size() >= all->p->ring_size - 2) {
      cerr << "newline, example_is_newline=" << example_is_newline(ec) << ", size=" << l->ec_seq.size() << ", indices.size=" << ec->indices.size() << endl;
      if (l->ec_seq.size() >= all->p->ring_size - 2 && l->first_pass)
        cerr << "warning: length of sequence at " << ec->example_counter << " exceeds ring size; breaking apart" << endl;
	
      do_actual_learning<is_learn>(*all, *l, base);

      if (!LabelDict::ec_seq_is_label_definition(*l, l->ec_seq) && l->ec_seq.size() > 0)
        global_print_newline(*all);

      if (ec->in_use)
        VW::finish_example(*all, ec);
      l->need_to_clear = true;
    } else if (LabelDict::ec_is_label_definition(ec)) {
      if (l->ec_seq.size() > 0)
        cerr << "warning: label definition encountered in data block -- ignoring data!" << endl;
    
      if (!((!all->training) || COST_SENSITIVE::example_is_test(ec))) {
        l->ec_seq.erase();
        l->ec_seq.push_back(ec);
        do_actual_learning<is_learn>(*all, *l, base);
        l->ec_seq.erase();
      }

      if (ec->in_use)
        VW::finish_example(*all, ec);
    } else {
      cerr << "push_back" << endl;
      l->ec_seq.push_back(ec);
    }
    
    if (l->need_to_clear) {
      output_example_seq(*all, *l);
      clear_seq(*all, *l);
      l->need_to_clear = false;
    }
  }
*/

  void finish_singleline_example(vw& all, ldf&, example& ec)
  {
    if (! LabelDict::ec_is_label_definition(ec)) {
      all.sd->weighted_examples += 1;
      all.sd->example_number++;
    }
    bool hit_loss = false;
    output_example(all, ec, hit_loss);
    VW::finish_example(all, &ec);
  }

  void finish_multiline_example(vw& all, ldf& l, example& ec)
  {
    if (l.need_to_clear) {
      if (l.ec_seq.size() > 0) {
	output_example_seq(all, l);
        global_print_newline(all);
      }        
      clear_seq_and_finish_examples(all, l);
      l.need_to_clear = false;
      if (ec.in_use) VW::finish_example(all, &ec);
    }
  }

  void end_examples(ldf& l)
  {
    if (l.need_to_clear)
      l.ec_seq.erase();
  }


  void finish(ldf& l)
  {
    //vw* all = l->all;
    l.ec_seq.delete_v();
    LabelDict::free_label_features(l);
  }

  template <bool is_learn>
  void predict_or_learn(ldf& l, learner& base, example &ec) {
    vw* all = l.all;
    l.base = &base;
    bool is_test = COST_SENSITIVE::example_is_test(ec) || !all->training;
    
    if (is_test)
      make_single_prediction(*all, l, base, ec, NULL, NULL, NULL, NULL);

    bool need_to_break = l.ec_seq.size() >= all->p->ring_size - 2;
    
    if (l.is_singleline)
      assert(is_test);
    else if ((example_is_newline(ec) && COST_SENSITIVE::example_is_test(ec)) || need_to_break) {
      if (need_to_break && l.first_pass)
        cerr << "warning: length of sequence at " << ec.example_counter << " exceeds ring size; breaking apart" << endl;
      do_actual_learning<is_learn>(*all, l, base);
      l.need_to_clear = true;
    } else if (LabelDict::ec_is_label_definition(ec)) {
      if (l.ec_seq.size() > 0) {
        cerr << "error: label definition encountered in data block" << endl;
        throw exception();
      }

      if (is_learn && ! is_test) {
        l.ec_seq.push_back(&ec);
        do_actual_learning<is_learn>(*all, l, base);
        l.need_to_clear = true;
      }
    } else {
      if (l.need_to_clear) {  // should only happen if we're NOT driving
        l.ec_seq.erase();
        l.need_to_clear = false;
      }
      l.ec_seq.push_back(&ec);
    }
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    ldf* ld = (ldf*)calloc_or_die(1, sizeof(ldf));

    ld->all = &all;
    ld->need_to_clear = true;
    ld->first_pass = true;
 
    string ldf_arg;
    if(vm_file.count("csoaa_ldf")) {
      ldf_arg = vm_file["csoaa_ldf"].as<string>();
      
      if(vm.count("csoaa_ldf") && ldf_arg.compare(vm["csoaa_ldf"].as<string>()) != 0) {
        ldf_arg = vm["csoaa_ldf"].as<string>();
        //std::cerr << "warning: you specified a different ldf argument through --csoaa_ldf than the one loaded from regressor. Proceeding with value of: " << ldf_arg << endl;
      }
    }
    else if( vm.count("csoaa_ldf") ){
      ldf_arg = vm["csoaa_ldf"].as<string>();
      all.options_from_file.append(" --csoaa_ldf ");
      all.options_from_file.append(ldf_arg);
    }
    else if( vm_file.count("wap_ldf") ) {
      ldf_arg = vm_file["wap_ldf"].as<string>();
      ld->is_wap = true;
      
      if(vm.count("wap_ldf") && ldf_arg.compare(vm["wap_ldf"].as<string>()) != 0) {
        ldf_arg = vm["csoaa_ldf"].as<string>();
        //std::cerr << "warning: you specified a different value for --wap_ldf than the one loaded from regressor. Proceeding with value of: " << ldf_arg << endl;
      }
    }
    else {
      ldf_arg = vm["wap_ldf"].as<string>();
      ld->is_wap = true;
      all.options_from_file.append(" --wap_ldf ");
      all.options_from_file.append(ldf_arg);
    }

    all.p->lp = COST_SENSITIVE::cs_label_parser;

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


