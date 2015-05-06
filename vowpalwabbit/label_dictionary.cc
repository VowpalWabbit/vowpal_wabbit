#include "reductions.h"
#include "cost_sensitive.h"
#include "label_dictionary.h"

namespace LabelDict { 
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

  bool ec_seq_is_label_definition(v_array<example*>ec_seq)
  {
    if (ec_seq.size() == 0) return false;
    bool is_lab = ec_is_label_definition(*ec_seq[0]);
    for (size_t i=1; i<ec_seq.size(); i++) {
      if (is_lab != ec_is_label_definition(*ec_seq[i])) {
        if (!((i == ec_seq.size()-1) && (example_is_newline(*ec_seq[i])))) {
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

  void add_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = lfm.get(lab, lab_hash);
    if (features.size() == 0) return;
    add_example_namespace(ec, 'l', features);
  }

  void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab) {
    size_t lab_hash = hash_lab(lab);
    v_array<feature> features = lfm.get(lab, lab_hash);
    if (features.size() == 0) return;
    del_example_namespace(ec, 'l', features);
  }

  void set_label_features(label_feature_map& lfm, size_t lab, v_array<feature>features) {
    size_t lab_hash = hash_lab(lab);
    if (lfm.contains(lab, lab_hash)) { return; }
    lfm.put_after_get(lab, lab_hash, features);
  }

  void free_label_features(label_feature_map& lfm) {
    void* label_iter = lfm.iterator();
    while (label_iter != nullptr) {
      v_array<feature> *features = lfm.iterator_get_value(label_iter);
      features->erase();
      features->delete_v();

      label_iter = lfm.iterator_next(label_iter);
    }
    lfm.clear();
    lfm.delete_v();
  }
}
