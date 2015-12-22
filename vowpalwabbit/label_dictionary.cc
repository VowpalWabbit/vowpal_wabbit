#include "reductions.h"
#include "cost_sensitive.h"
#include "label_dictionary.h"

namespace LabelDict
{
size_t hash_lab(size_t lab) { return 328051 + 94389193 * lab; }

void del_example_namespace(example& ec, char ns, v_array<feature>& features, bool audit)
{ size_t numf = features.size();
  // print_update is called after this del_example_namespace,
  // so we need to keep the ec.num_features correct,
  // so shared features are included in the reported number of "current features"
  //ec.num_features -= numf;

  assert (ec.atomics[(size_t)ns].size() >= numf);
  if (ec.atomics[(size_t)ns].size() == numf)   // did NOT have ns
  { assert(ec.indices.size() > 0);
    assert(ec.indices[ec.indices.size()-1] == (size_t)ns);
    ec.indices.pop();
    ec.total_sum_feat_sq -= ec.sum_feat_sq[(size_t)ns];
    ec.atomics[(size_t)ns].erase();
    ec.sum_feat_sq[(size_t)ns] = 0.;
    if (audit)
      ec.audit_features[(size_t)ns].erase();
  }
  else     // DID have ns
  { for (feature*f=features.begin; f!=features.end; f++)
    { ec.sum_feat_sq[(size_t)ns] -= f->x * f->x;
      ec.atomics[(size_t)ns].pop();
      if (audit)
        ec.audit_features[(size_t)ns].pop();
    }
  }
}

void add_example_namespace(example& ec, char ns, v_array<feature>& features, v_array<audit_data>* audit)
{ bool has_ns = false;
  for (size_t i=0; i<ec.indices.size(); i++)
  { if (ec.indices[i] == (size_t)ns)
    { has_ns = true;
      break;
    }
  }
  if (has_ns)
  { ec.total_sum_feat_sq -= ec.sum_feat_sq[(size_t)ns];
  }
  else
  { ec.indices.push_back((size_t)ns);
    ec.sum_feat_sq[(size_t)ns] = 0;
  }

  for (feature*f=features.begin; f!=features.end; f++)
  { ec.sum_feat_sq[(size_t)ns] += f->x * f->x;
    ec.atomics[(size_t)ns].push_back(*f);
  }

  ec.num_features += features.size();
  ec.total_sum_feat_sq += ec.sum_feat_sq[(size_t)ns];

  if (audit != nullptr)
    for (audit_data*f = audit->begin; f != audit->end; ++f)
    { audit_data f2 = { f->space, f->feature, f->weight_index, f->x};
      ec.audit_features[(size_t)ns].push_back(f2);
    }
}

void add_example_namespaces_from_example(example& target, example& source, bool audit)
{ for (unsigned char* idx=source.indices.begin; idx!=source.indices.end; idx++)
  { if (*idx == constant_namespace) continue;
    add_example_namespace(target, (char)*idx, source.atomics[*idx],
                          audit ? &source.audit_features[*idx] : nullptr);
  }
}

void del_example_namespaces_from_example(example& target, example& source, bool audit)
{ //for (size_t*idx=source.indices.begin; idx!=source.indices.end; idx++) {
  unsigned char* idx = source.indices.end;
  idx--;
  for (; idx>=source.indices.begin; idx--)
  { if (*idx == constant_namespace) continue;
    del_example_namespace(target, (char)*idx, source.atomics[*idx], audit);
  }
}

void add_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab, bool audit)
{ size_t lab_hash = hash_lab(lab);
  feature_audit& res = lfm.get(lab, lab_hash);
  if (res.features.size() == 0) return;
  add_example_namespace(ec, 'l', res.features, audit ? &res.audit : nullptr);
}

void del_example_namespace_from_memory(label_feature_map& lfm, example& ec, size_t lab, bool audit)
{ size_t lab_hash = hash_lab(lab);
  feature_audit& res = lfm.get(lab, lab_hash);
  if (res.features.size() == 0) return;
  del_example_namespace(ec, 'l', res.features, audit);
}

void set_label_features(label_feature_map& lfm, size_t lab, v_array<feature>&features, v_array<audit_data>* audit)
{ size_t lab_hash = hash_lab(lab);
  if (lfm.contains(lab, lab_hash)) { return; }
  const v_array<audit_data> empty = { nullptr, nullptr, nullptr, 0 };
  feature_audit fa = { features, audit ? (*audit) : empty };
  lfm.put_after_get(lab, lab_hash, fa);
}

void free_label_features(label_feature_map& lfm)
{ void* label_iter = lfm.iterator();
  while (label_iter != nullptr)
  { feature_audit *res = lfm.iterator_get_value(label_iter);
    res->features.erase();
    res->features.delete_v();
    res->audit.erase();
    res->audit.delete_v();

    label_iter = lfm.iterator_next(label_iter);
  }
  lfm.clear();
  lfm.delete_v();
}
}
